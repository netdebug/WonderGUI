/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_widget.h>
#include <wg_container.h>
#include <wg_rootpanel.h>
#include <wg_msgrouter.h>
#include <wg_base.h>

namespace wg
{

	const char Widget::CLASSNAME[] = {"Widget"};


	//____ Constructor ____________________________________________________________

	Widget::Widget():m_id(0), m_pHolder(0), m_pSlot(0), m_pointerStyle(PointerStyle::Default),
						m_markOpacity( 1 ), m_bOpaque(false), m_bTabLock(false),
						 m_bPressed(false), m_bSelectable(true), m_size(256,256),
						m_bPickable(false), m_bDropTarget(false)
	{
	}

	//____ Destructor _____________________________________________________________

	Widget::~Widget()
	{
	}

	//____ isInstanceOf() _________________________________________________________

	bool Widget::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;

		return Receiver::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * Widget::className( void ) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	Widget_p Widget::cast( Object * pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return Widget_p( static_cast<Widget*>(pObject) );

		return 0;
	}


	//____ isContainer() ______________________________________________________
	/**
	 * @brief Check if this widget is a container.
	 *
	 *  Check if widget is a container.
	 *
	 *  This method is a quicker way to check if the widget
	 *  is a container than calling isInstanceOf(Container::CLASS).
	 *
	 *  @return True if the widget is a subclass of Container.
	 */

	bool Widget::isContainer() const
	{
		return false;
	}


	//____ parent() _______________________________________________________________
	/**
	 * @brief Returns the parent of the widget.
	 *
	 * Returns the parent of the widget.
	 *
	 * @return Parent of the widget or null if widget has no parent.
	*/

	Container_p Widget::parent() const
	{
		if( m_pHolder )
			return m_pHolder->_childParent();
		return 0;
	}

	//____ pointerStyle() ________________________________________

	PointerStyle Widget::pointerStyle() const
	{
		return m_pointerStyle;
	}

	//____ setEnabled() _______________________________________________________________
	/**
	 * @brief Enables or disables the widget.
	 *
	 * Enables or disables the widget and all child widgets if the widget is a container.
	 *
	 * @param bEnabled	True to enable Widget, false to disable.Coordinate to check in widgets own coordinate system.
	 *
	 * A Widget that is disabled does not react on mouse or keyboard events or updates its content.
	 * A disabled Widget is rendered in disabled state, how that looks depends on the skin.
	 *
	 * This method never fails.
	 */

	void Widget::setEnabled( bool bEnabled )
	{
		if( m_state.isEnabled() != bEnabled || isContainer() )
		{
			State s = m_state;
			s.setEnabled(bEnabled);
			_setState(s);
		}
	}

	//____ setSelectable() ____________________________________________________
	/**
	 * @brief Ḿake widget (un)selectable.
	 *
	 * Sets the selectable flag of the widget. Default is true, except for some
	 * special widgets, like PopupOpener, which should perform an action instead
	 * of being selected when clicked.
	 *
	 * Parent widgets such as List that allows selection/unselection of its
	 * children should take the selectable flag into account and ignore selection
	 * of widgets that are not selectable.
	 *
	 * A typical use of this flag is to make widgets that should just be separators
	 * in lists and menus unselectable.
	 *
	 */

	void Widget::setSelectable(bool bSelectable)
	{
		m_bSelectable = bSelectable;
	}

	//____ markTest() _____________________________________________________________
	/**
	 * @brief Check if specified coordinate is inside or outside of widget.
	 *
	 * Check if specified coordinate is inside or outside of widget.
	 *
	 * @param ofs	Coordinate to check in widgets own coordinate system.
	 *
	 * This method first checks if the specified coordinate is inside the widgets
	 * box geometry. If it is, a second check is performed against the widgets
	 * alpha value (transparency) at the specified coordinate.
	 * If the alpha value is equal to or higher than the widgets MarkOpacity value, the
	 * test succeeds and MarkTest returns true.
	 *
	 * MarkOpacity is by default set to 1, which means that all but totally transparent pixels
	 * will be marked. See setMarkOpacity() for more info.
	 *
	 * This method is mainly used to determine if the pointer hovers over the widget or not.
	 *
	 * @return True if alpha value of coordinate is equal to or higher than widgets MarkOpaciy.
	 */

	bool Widget::markTest( const Coord& ofs )
	{
		if( m_markOpacity <= 0 || ofs.x < 0 || ofs.y < 0 )
			return false;

		Size sz = size();

		if( ofs.x >= sz.w || ofs.y >= sz.h )
			return false;

		if( m_markOpacity >= 256 )
			return true;

		return _alphaTest(ofs);
	}

	//____ setSkin() ______________________________________________________________
	/**
	 * @brief Sets the skin of this widget.
	 *
	 * @param pSkin 	Pointer to the skin to be used.
	 *
	 * Specifies the skin to be used by this widget. The skin will cover the
	 * whole widget and provide the background for any components placed
	 * on the widget (if any).
	 *
	 * A skin typically has different looks depending on the widgets state (normal,
	 * disabled, mouse inside, pressed, selected etc) and can also include padding.
	 *
	 * Some widgets have more than one skin, but this is always the background skin for
	 * the whole widget.
	 *
	 */

	void Widget::setSkin( Skin * pSkin )
	{
		_setSkin( pSkin );
	}


	//____ clone() _________________________________________________________

	Widget * Widget::_clone() const
	{
		Widget * p = _newOfMyType();
		p->_cloneContent( this );
		return p;
	}

	//____ _cloneContent() _____________________________________________________

	void Widget::_cloneContent( const Widget * pOrg )
	{
		m_id			= pOrg->m_id;

		m_pointerStyle 	= pOrg->m_pointerStyle;
		m_pSkin			= pOrg->m_pSkin;

		m_tooltip		= pOrg->m_tooltip;
		m_markOpacity	= pOrg->m_markOpacity;

		m_bOpaque		= pOrg->m_bOpaque;
		m_bTabLock		= pOrg->m_bTabLock;

		m_size			= pOrg->m_size;

		//TODO: We should clone stateflags selectively, like disabled.

	}


	//____ toGlobal() __________________________
	/**
	 * @brief Convert coordinate from local to global coordinate system
	 *
	 * Convert coordinate from local to global coordinate system
	 *
	 * @param coord		Coordinate in widgets local coordinate system.
	 *
	 * Please note that the widgets local coordinate system originates from the top-left
	 * corner of its box geometry and is NOT the same as the (parents) local coordinate
	 * system in which it lives.
	 * The coordinate (0,0) is always the top-left corner of the widget.
	 *
	 * @return Coordinate in gobal coordinate system
	 */

	 Coord Widget::toGlobal( const Coord& coord ) const
	{
		Coord c = globalPos();
		c.x += coord.x;
		c.y += coord.y;
		return c;
	}

	//____ toLocal() ____________________________________________________________
	/**
	 * @brief Convert coordinate from local to global coordinate system
	 *
	 * Convert coordinate from local to global coordinate system
	 *
	 * @param coord		Coordinate in widgets local coordinate system.
	 *
	 * Please note that the widgets local coordinate system originates from the top-left
	 * corner of its box geometry and is NOT the same as the (parents) local coordinate
	 * system in which it lives.
	 * The coordinate (0,0) is always the top-left corner of the widget.
	 *
	 * @return Coordinate in gobal coordinate system
	 */

	Coord Widget::toLocal( const Coord& coord ) const
	{
		Coord c = globalPos();
		return Coord( coord.x - c.x, coord.y - c.y );
	}

	//____ matchingHeight() _______________________________________________________
	/**
	 * @brief Get the widgets preferred height for the specified width.
	 *
	 * Get the widgets preferred height for the specified width.
	 *
	 * @param width		Width in pixels.
	 *
	 * This method is used by containers to get the preferred height of a widget for which
	 * it has already decided the width.
	 *
	 * @return The preferred height for the given width in pixels.
	 */

	int Widget::matchingHeight( int width ) const
	{
		return preferredSize().h;		// Default is to stick with best height no matter what width.
	}

	//____ matchingWidth() _______________________________________________________
	/**
	 * @brief Get the widgets preferred width for the specified height.
	 *
	 * Get the widgets preferred width for the specified height.
	 *
	 * @param height	Height in pixels.
	 *
	 * This method is used by containers to get the preferred width of a widget for which
	 * it has already decided the height.
	 *
	 * @return The preferred width for the given height in pixels.
	 */

	int Widget::matchingWidth( int height ) const
	{
		return preferredSize().w;		// Default is to stick with best width no matter what height.
	}

	//____ preferredSize() ________________________________________________________
	/**
	 * @brief Get the widgets preferred size.
	 *
	 * Get the widgets preferred size.
	 *
	 * Each widget has its own preferred size, which is depending on things such as
	 * skinning, content and (in the case of containers) size and layout of children.
	 *
	 * A container holding a widget will strive to give the widget its preferred size, given
	 * the constraints and limitations the container needs to work with. If a container can't
	 * give a widget its preferred size, it is likely to decide the closest width or height
	 * that it can provide and then make a second call to either matchingWidth() or matchingHeight()
	 * after which it will decide the size of the child and notify it.
	 *
	 * @return The preferred size of the widget in pixels.
	 */

	Size Widget::preferredSize() const
	{
		if( m_pSkin )
			return m_pSkin->preferredSize();
		else
			return Size(0,0);
	}

	//____ minSize() ______________________________________________________________
	/**
	 * @brief Get the widgets recommended minimum size.
	 *
	 * Get the widgets recommended minimum size.
	 *
	 * Each widget has its own minimum size, which is depending on things such as
	 * skinning, content and (in the case of containers) size and layout of children.
	 *
	 * The minimum size is only a hint for the container, which should strive to not
	 * make a child smaller than its minimum size, but is allowed to set the child to
	 * any size it decides, including 0.0.
	 *
	 * @return The minimum size of the widget in pixels.
	 */

	Size Widget::minSize() const
	{
		if( m_pSkin )
			return m_pSkin->minSize();
		else
			return Size(0,0);
	}

	//____ receive() _______________________________________________________________

	void Widget::receive( Msg * pMsg )
	{
		// SetRepost before _receive() so that subclasses can swallow the respost.

		switch( pMsg->type() )
		{
			case MsgType::MouseMove:
			case MsgType::MousePress:
			case MsgType::MouseRepeat:
			case MsgType::MouseDrag:
			case MsgType::MouseRelease:
			case MsgType::MouseClick:
			case MsgType::MouseDoubleClick:
			case MsgType::KeyPress:
			case MsgType::KeyRepeat:
			case MsgType::KeyRelease:
			case MsgType::WheelRoll:
			{
				Widget_p pParent = parent();
				if( pParent )
					pMsg->setRepost(pParent,pParent);
				break;
			}
			default:
				break;
		}
		_receive( pMsg );
	}

	//____ maxSize() ______________________________________________________________
	/**
	 * @brief Get the widgets recommended maximum size.
	 *
	 * Get the widgets recommended maximum size.
	 *
	 * Each widget has its own maximum size, which is depending on things such as
	 * skinning, content and (in the case of containers) size and layout of children.
	 *
	 * The maximum size is only a hint for the container, which should strive to not
	 * make a child larger than its maximum size, but is allowed to set the child to
	 * any reasonable size it decides.
	 *
	 * @return The maximum size of the widget in pixels.
	 */

	Size Widget::maxSize() const
	{
		return Size(2<<24,2<<24);
	}

	//____ setPickable() ____________________________________________________________

	void Widget::setPickable( bool bPickable, int category )
	{
		m_bPickable = bPickable;
		m_pickCategory = category;
	}

	//____ _getBlendMode() _________________________________________________________

	BlendMode Widget::_getBlendMode() const
	{
		Container * pParent = _parent();
		if( pParent )
			return pParent->_getBlendMode();
		else
			return BlendMode::Blend;		// We always start out with BlendMode::Blend.
	}

	//____ _renderPatches() ________________________________________________________

	void Widget::_renderPatches( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window, const Patches& patches )
	{
		pDevice->setClipList(patches.size(), patches.begin());
		_render( pDevice, _canvas, _window );
	}

	//____ onCollectPatches()  ____________________________________________________

	void Widget::_collectPatches( Patches& container, const Rect& geo, const Rect& clip )
	{
			container.add( Rect( geo, clip ) );
	}

	//____ _maskPatches() _______________________________________________________

	void Widget::_maskPatches( Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode )
	{
		if( (m_bOpaque && blendMode == BlendMode::Blend) || blendMode == BlendMode::Replace )
		{
			patches.sub( Rect( geo, clip ) );
		}
	}

	//____ _render() ____________________________________________________________

	void Widget::_render( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window )
	{
		if( m_pSkin )
			m_pSkin->render( pDevice, _canvas, m_state );
	}

	//____ _setSize() ___________________________________________________________

	void Widget::_setSize( const Size& size )
	{
		m_size = size;
//		_requestRender();		Do NOT request render here, it is the responsibility of ancestor initiating the series of events.
	}

	//____ _refresh() ___________________________________________________________

	void Widget::_refresh()
	{
		if( m_pSkin && m_pSkin->isOpaque(m_state) )
			m_bOpaque = true;
		else
			m_bOpaque = false;

		_requestResize();
		_requestRender();
	}

	//____ _setSkin() _______________________________________________________

	void Widget::_setSkin( Skin * pSkin )
	{
		if( !m_pSkin || !pSkin || m_pSkin->contentPadding() != pSkin->contentPadding() ||
			m_pSkin->preferredSize() != pSkin->preferredSize() ||
			m_pSkin->minSize() != pSkin->minSize() )
		{
			_requestResize();
		}


		if( pSkin && pSkin->isOpaque(m_state) )
			m_bOpaque = true;
		else
			m_bOpaque = false;

		m_pSkin = pSkin;
		_requestRender();
	}

	//____ _setState() _________________________________________________________

	void Widget::_setState( State state )
	{
		if( m_pSkin && !m_pSkin->isStateIdentical(state, m_state) )
		{
			m_bOpaque = m_pSkin->isOpaque(state);
			_requestRender();
		}

		m_state = state;
	}

	//____ _receive() _____________________________________________________________

	void Widget::_receive( Msg * _pMsg )
	{
		State state = m_state;

		switch( _pMsg->type() )
		{
			case MsgType::MouseEnter:
				if( m_bPressed )
					state.setPressed(true);
				else
					state.setHovered(true);
				break;
			case MsgType::MouseLeave:
				state.setHovered(false);			// Also clears any pressed flag.
				break;
			case MsgType::MousePress:
			{
				MousePressMsg_p pMsg = MousePressMsg::cast(_pMsg);
				if( pMsg->button() == MouseButton::Left )
				{
					if( state.isHovered() )
						state.setPressed(true);

					m_bPressed = true;
				}
				break;
			}
			case MsgType::MouseRelease:
			{
				MouseReleaseMsg_p pMsg = MouseReleaseMsg::cast(_pMsg);
				if( pMsg->button() == MouseButton::Left )
				{
					if( state.isHovered() )
						state.setPressed(false);

					m_bPressed = false;
				}
				break;
			}
			case MsgType::FocusGained:
				state.setFocused(true);
				break;
			case MsgType::FocusLost:
				state.setFocused(false);
				break;
			case MsgType::DropEnter:
				state.setTargeted(true);
				break;
			case MsgType::DropLeave:
				state.setTargeted(false);
				break;
			default:
				break;
		}

		if( state != m_state )
			_setState( state );
	}

	//____ _alphaTest() _________________________________________________________

	bool Widget::_alphaTest( const Coord& ofs )
	{
		if( m_pSkin )
			return m_pSkin->markTest( ofs, Rect(0,0,m_size), m_state, m_markOpacity );

		return false;
	}

	//____ _windowPadding() _______________________________________________________

	Size Widget::_windowPadding() const
	{
		return Size(0,0);
	}

	//____ _componentRequestRender() _________________________________________________________

	void Widget::_componentRequestRender( const Component * pComponent )
	{
		_requestRender( _componentGeo( pComponent ) );
	}

	void Widget::_componentRequestRender( const Component * pComponent, const Rect& rect )
	{
		_requestRender( rect + _componentPos( pComponent ) );
	}

	//____ _componentRequestResize() ________________________________________________________

	void Widget::_componentRequestResize( const Component * pComponent )
	{
		_requestResize();
		_requestRender();
	}

	//____ _componentRequestFocus() ___________________________________________________

	void Widget::_componentRequestFocus( const Component * pComponent )
	{
		grabFocus();
	}

	//____ _componentRequestInView() ______________________________________________

	void Widget::_componentRequestInView( const Component * pComponent )
	{
		Rect r = _componentPos( pComponent );
		_requestInView( r, r );
	}

	void Widget::_componentRequestInView( const Component * pComponent, const Rect& preferred, const Rect& prio )
	{
		Coord ofs = _componentPos( pComponent );
		_requestInView( preferred + ofs, prio + ofs );
	}


	//____ _componentPos() ______________________________________________________________

	Coord Widget::_componentPos( const Component * pComponent ) const
	{
		if( m_pSkin )
			return m_pSkin->contentOfs( m_state );
		else
			return Coord();
	}

	//____ _componentSize() ______________________________________________________________

	Size Widget::_componentSize( const Component * pComponent ) const
	{
		if( m_pSkin )
			return m_size - m_pSkin->contentPadding();
		else
			return m_size;
	}

	//____ _componentGeo() ______________________________________________________________

	Rect Widget::_componentGeo( const Component * pComponent ) const
	{
		if( m_pSkin )
			return m_pSkin->contentRect( m_size, m_state );
		else
			return Rect( 0,0,m_size );
	}

	//____ _globalComponentPos() ________________________________________________________

	Coord Widget::_globalComponentPos( const Component * pComponent ) const
	{
		return _componentPos( pComponent ) + globalPos();
	}

	//____ _globalComponentGeo() ______________________________________________________________

	Rect Widget::_globalComponentGeo( const Component * pComponent ) const
	{
		return _componentGeo( pComponent ) + globalPos();
	}

	//____ _object() ______________________________________________________

	Object * Widget::_object()
	{
		return this;
	}

	const Object * Widget::_object() const
	{
		return this;
	}


	//____ _receiveComponentNotif() __________________________________________________

	void Widget::_receiveComponentNotif( Component * pComponent, ComponentNotif notification, void * pData )
	{
		// By default we do nothing
	}



} // namespace wg
