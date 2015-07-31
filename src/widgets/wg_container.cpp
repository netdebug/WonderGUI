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

#include <vector>
#include <wg_container.h>
#include <wg_panel.h>
#include <wg_rootpanel.h>

#include <wg_patches.h>

#ifndef WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif

namespace wg 
{
	
	const char Container::CLASSNAME[] = {"Container"};
	
	//____ Constructor _____________________________________________________________
	
	Container::Container() : m_bSiblingsOverlap(true)
	{
	}
	
	//____ isInstanceOf() _________________________________________________________
	
	bool Container::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Widget::isInstanceOf(pClassName);	
	}
	
	//____ className() ____________________________________________________________
	
	const char * Container::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	//____ cast() _________________________________________________________________
	
	Container_p Container::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return Container_p( static_cast<Container*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ isContainer() ______________________________________________________________
	
	bool Container::isContainer() const
	{
		return true;
	}
	
	//____ _isPanel() ______________________________________________________________
	
	bool Container::_isPanel() const
	{
		return false;
	}
	
	//____ _firstWidget() __________________________________________________________
	
	Widget * Container::_firstWidget() const 
	{ 
		Hook * p = _firstHook(); 
		if( p ) 
			return p->_widget(); 
		else 
			return 0;
	}
	
	//____ _lastWidget() ___________________________________________________________
	
	Widget * Container::_lastWidget() const 
	{ 
		Hook * p = _lastHook(); 
		if( p ) 
			return p->_widget(); 
		else 
			return 0; 
	}
	
	//____ _findWidget() ____________________________________________________________
	
	Widget * Container::_findWidget( const Coord& ofs, WgSearchMode mode )
	{
		Rect childGeo;
		Hook * pHook = _lastHookWithGeo( childGeo );
		Widget * pResult = 0;
	
		while( pHook && !pResult )
		{
			if( pHook->_isVisible() && childGeo.contains( ofs ) )
			{
				if( pHook->_widget()->isContainer() )
				{
					pResult = static_cast<Container*>(pHook->_widget())->_findWidget( ofs - childGeo.pos(), mode );
				}
				else if( mode == WG_SEARCH_GEOMETRY || pHook->_widget()->markTest( ofs - childGeo.pos() ) )
				{
						pResult = pHook->_widget();
				}
			}
			pHook = _prevHookWithGeo( childGeo, pHook );
		}
	
		// Check against ourselves
	
		if( !pResult && ( mode == WG_SEARCH_GEOMETRY || markTest(ofs)) )
			pResult = this;
			
		return pResult;
	}
	
	//____ _focusRequested() _______________________________________________________
	
	bool Container::_focusRequested( Hook * pBranch, Widget * pWidgetRequesting )
	{
		Hook * p = _hook();
		if( p )
		{
			RootPanel * r = p->_root();
			if( r )
				return r->_focusRequested( p, pWidgetRequesting );
		}
		return false;
	}
	
	//____ _focusReleased() ________________________________________________________
	
	bool Container::_focusReleased( Hook * pBranch, Widget * pWidgetReleasing )
	{
		Hook * p = _hook();
		if( p )
		{
			RootPanel * r = p->_root();
			if( r )
				return r->_focusReleased( p, pWidgetReleasing );
		}
		return false;
	}
	
	
	ModalLayer *  Container::_getModalLayer() const
	{
		const Container * p = _parent();
	
		if( p )
			return p->_getModalLayer();
		else
			return 0;
	}
	
	PopupLayer * Container::_getPopupLayer() const
	{
		const Container * p = _parent();
	
		if( p )
			return p->_getPopupLayer();
		else
			return 0;
	}
	
	//____ _onStateChanged() ______________________________________________________
	
	void Container::_onStateChanged( State oldState )
	{
		Widget::_onStateChanged(oldState);
	
		if( oldState.isEnabled() != m_state.isEnabled() )
		{
			bool bEnabled = m_state.isEnabled();
			Widget * p = _firstWidget();
			while( p )
			{
				p->setEnabled(bEnabled);
				p = p->_nextSibling();
			}
		}
	
		if( oldState.isSelected() != m_state.isSelected() )
		{
			bool bSelected = m_state.isSelected();
			Widget * p = _firstWidget();
			while( p )
			{
				State old = p->m_state;
				p->m_state.setSelected(bSelected);
				p->_onStateChanged( old );
				p = p->_nextSibling();
			}
		}
	
	}
	
	//____ _renderPatches() _____________________________________________________
	// Default implementation for panel rendering patches.
	class WidgetRenderContext
	{
	public:
		WidgetRenderContext() : pWidget(0) {}
		WidgetRenderContext( Widget * pWidget, const Rect& geo ) : pWidget(pWidget), geo(geo) {}
	
		Widget *	pWidget;
		Rect		geo;
		Patches	patches;
	};
	
	void Container::_renderPatches( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window, Patches * _pPatches )
	{
	
		// We start by eliminating dirt outside our geometry
	
		Patches 	patches( _pPatches->size() );								// TODO: Optimize by pre-allocating?
	
		for( const Rect * pRect = _pPatches->begin() ; pRect != _pPatches->end() ; pRect++ )
		{
			if( _canvas.intersectsWith( *pRect ) )
				patches.push( Rect(*pRect,_canvas) );
		}
	
	
		// Render container itself
		
		for( const Rect * pRect = patches.begin() ; pRect != patches.end() ; pRect++ )
			_onRender(pDevice, _canvas, _window, *pRect );
			
		
		// Render children
	
		Rect	dirtBounds = patches.getUnion();
		
		if( m_bSiblingsOverlap )
		{
	
			// Create WidgetRenderContext's for siblings that might get dirty patches
	
			std::vector<WidgetRenderContext> renderList;
	
			Rect childGeo;
			Hook * p = _firstHookWithGeo( childGeo );
			while(p)
			{
				Rect geo = childGeo + _canvas.pos();
	
				if( p->_isVisible() && geo.intersectsWith( dirtBounds ) )
					renderList.push_back( WidgetRenderContext(p->_widget(), geo ) );
	
				p = _nextHookWithGeo( childGeo, p );
			}
	
			// Go through WidgetRenderContexts in reverse order (topmost first), push and mask dirt
	
			for( int i = renderList.size()-1 ; i >= 0 ; i-- )
			{
				WidgetRenderContext * p = &renderList[i];
	
				p->patches.push( &patches );
	
				p->pWidget->_onMaskPatches( patches, p->geo, p->geo, pDevice->getBlendMode() );		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!
	
				if( patches.isEmpty() )
					break;
			}
	
			// Go through WidgetRenderContexts and render the patches
	
			for( int i = 0 ; i < (int) renderList.size() ; i++ )
			{
				WidgetRenderContext * p = &renderList[i];
				p->pWidget->_renderPatches( pDevice, p->geo, p->geo, &p->patches );
			}
	
		}
		else
		{
			Rect childGeo;
			Hook * p = _firstHookWithGeo( childGeo );
	
			while(p)
			{
				Rect canvas = childGeo + _canvas.pos();
				if( p->_isVisible() && canvas.intersectsWith( dirtBounds ) )
					p->_widget()->_renderPatches( pDevice, canvas, canvas, &patches );
				p = _nextHookWithGeo( childGeo, p );
			}
	
		}
	}
	
	//____ _onCloneContent() _______________________________________________________
	
	void Container::_onCloneContent( const Container * _pOrg )
	{
		m_bSiblingsOverlap 	= _pOrg->m_bSiblingsOverlap;
	}
	
	//____ _onCollectPatches() _______________________________________________________
	
	void Container::_onCollectPatches( Patches& container, const Rect& geo, const Rect& clip )
	{
		if( m_pSkin )
			container.add( Rect( geo, clip ) );
		else
		{
			Rect childGeo;
			Hook * p = _firstHookWithGeo( childGeo );
	
			while(p)
			{
				if( p->_isVisible() )
					p->_widget()->_onCollectPatches( container, childGeo + geo.pos(), clip );
				p = _nextHookWithGeo( childGeo, p );
			}
		}
	}
	
	//____ _onMaskPatches() __________________________________________________________
	
	void Container::_onMaskPatches( Patches& patches, const Rect& geo, const Rect& clip, WgBlendMode blendMode )
	{
		//TODO: Don't just check isOpaque() globally, check rect by rect.
		if( (m_bOpaque && blendMode == WG_BLENDMODE_BLEND) || blendMode == WG_BLENDMODE_OPAQUE)
			patches.sub( Rect(geo,clip) );
		else
		{
			Rect childGeo;
			Hook * p = _firstHookWithGeo( childGeo );
	
			while(p)
			{
				if( p->_isVisible() )
					p->_widget()->_onMaskPatches( patches, childGeo + geo.pos(), clip, blendMode );
				p = _nextHookWithGeo( childGeo, p );
			}
		}
	}

} // namespace wg
