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

#include <wg_layer.h>
#include <wg_patches.h>

namespace wg
{

	const char Layer::CLASSNAME[] = {"Layer"};


	//____ Constructor ____________________________________________________________

	Layer::Layer() : base( &m_baseSlot, this )
	{
	}

	//____ isInstanceOf() _________________________________________________________

	bool Layer::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;

		return Container::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * Layer::className( void ) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	Layer_p Layer::cast( Object * pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return Layer_p( static_cast<Layer*>(pObject) );

		return 0;
	}



	//____ matchingHeight() _______________________________________________________

	int Layer::matchingHeight( int width ) const
	{
		if( m_baseSlot.pWidget )
			return m_baseSlot.pWidget->matchingHeight( width );
		else
			return Widget::matchingHeight(width);
	}

	//____ matchingWidth() _______________________________________________________

	int Layer::matchingWidth( int height ) const
	{
		if( m_baseSlot.pWidget )
			return m_baseSlot.pWidget->matchingWidth( height );
		else
			return Widget::matchingWidth(height);
	}

	//____ preferredSize() _____________________________________________________________

	Size Layer::preferredSize() const
	{
		if( m_baseSlot.pWidget )
			return m_baseSlot.pWidget->preferredSize();
		else
			return Size(1,1);
	}


	//____ _onRequestRender() _____________________________________________________

	void Layer::_onRequestRender( const Rect& rect, const LayerSlot * pSlot )
	{
		// Clip our geometry and put it in a dirtyrect-list

		Patches patches;
		patches.add( Rect( rect, Rect(0,0,m_size)) );

		// Remove portions of dirty rect that are covered by opaque upper siblings,
		// possibly filling list with many small dirty rects instead.

		const LayerSlot * pCover = _beginLayerSlots();
		const LayerSlot * pEnd = pSlot ? pSlot : _endLayerSlots();

		int incNext = _sizeOfLayerSlot();

		while( pCover <  pEnd )
		{
			if( pCover->geo.intersectsWith( rect ) )
				pCover->pWidget->_maskPatches( patches, pCover->geo, Rect(0,0,INT_MAX,INT_MAX ), _getBlendMode() );

			pCover = _incLayerSlot(pSlot,incNext);
		}

		// Make request render calls

		for( const Rect * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
			_requestRender( * pRect );
	}


	//____ _firstChild() ___________________________________________________________

	Widget* Layer::_firstChild() const
	{
		const LayerSlot * p = _beginLayerSlots();
		if (p != _endLayerSlots())
			return p->pWidget;

		return m_baseSlot.pWidget;
	}

	//____ _lastChild() ____________________________________________________________

	Widget* Layer::_lastChild() const
	{
		if (m_baseSlot.pWidget)
			return m_baseSlot.pWidget;
		else
		{
			const LayerSlot * pSlot = _endLayerSlots();
			if (pSlot > _beginLayerSlots())
			{
				pSlot = _decLayerSlot(pSlot, _sizeOfLayerSlot());
				return pSlot->pWidget;
			}

			return nullptr;
		}
	}

	//____ _firstSlotWithGeo() _____________________________________________________

	void Layer::_firstSlotWithGeo( SlotWithGeo& package ) const
	{
		const LayerSlot * p = _beginLayerSlots();
		if( p < _endLayerSlots() )
		{
			package.geo = p->geo;
			package.pSlot = p;
		}
		else if (m_baseSlot.pWidget)
		{
			package.geo = Rect(0, 0, m_size);
			package.pSlot = &m_baseSlot;
		}
		else
			package.pSlot = nullptr;
	}

	//____ _nextSlotWithGeo() _______________________________________________________

	void Layer::_nextSlotWithGeo( SlotWithGeo& package ) const
	{
		const LayerSlot * p = (LayerSlot*) package.pSlot;

		if (p == &m_baseSlot)
		{
			package.pSlot = nullptr;
			return;
		}

		p = _incLayerSlot(p,_sizeOfLayerSlot());
		if( p < _endLayerSlots() )
		{
			package.geo = ((LayerSlot*)p)->geo;
			package.pSlot = p;
		}
		else if (m_baseSlot.pWidget)
		{
			package.geo = Rect(0, 0, m_size);
			package.pSlot = &m_baseSlot;
		}
		else
			package.pSlot = nullptr;
	}

	//____ _cloneContent() _______________________________________________________

	void Layer::_cloneContent( const Widget * _pOrg )
	{
		Container::_cloneContent( _pOrg );
	}

	//____ _releaseChild() ____________________________________________________

	void Layer::_releaseChild(Slot * pSlot)
	{
		pSlot->replaceWidget(this, nullptr);
		_onRequestRender( Rect(0,0,m_size), 0 );
		_requestResize();
	}

	//____ _setWidget() _______________________________________________________

	void Layer::_setWidget( Slot * pSlot, Widget * pNewWidget )
	{
		pSlot->replaceWidget(this, pNewWidget);
		pNewWidget->_setSize(size());			//TODO: Should be content size here (and in all other _setWidget() methods?)

		_onRequestRender( Rect(0,0,m_size), 0 );
		_requestResize();
	}

	//____ _setSize() _______________________________________________________

	void Layer::_setSize(const Size& size)
	{
		Container::_setSize(size);

		if (m_baseSlot.pWidget)
			m_baseSlot.pWidget->_setSize(size);
	}


	//____ _childPos() _________________________________________________________

	Coord Layer::_childPos( Slot * pSlot ) const
	{
		if( pSlot == &m_baseSlot )
			return {0,0};

		return ((LayerSlot*)pSlot)->geo;
	}

	//____ _childSize() ________________________________________________________

	Size Layer::_childSize( Slot * pSlot ) const
	{
		if( pSlot == &m_baseSlot )
			return m_size;

		return ((LayerSlot*)pSlot)->geo;
	}

	//____ _childRequestRender() _______________________________________________

	void Layer::_childRequestRender( Slot * _pSlot )
	{
		if( _pSlot == &m_baseSlot )
			_onRequestRender( Rect( 0,0, m_size ), 0 );		//TODO: Take padding into account
		else
		{
			LayerSlot * pSlot = reinterpret_cast<LayerSlot*>(_pSlot);
			_onRequestRender( pSlot->geo, pSlot );
		}
	}

	void Layer::_childRequestRender( Slot * _pSlot, const Rect& rect )
	{
		if( _pSlot == &m_baseSlot )
			_onRequestRender( rect, 0 );		//TODO: Take padding into account
		else
		{
			LayerSlot * pSlot = reinterpret_cast<LayerSlot*>(_pSlot);
			_onRequestRender( rect + pSlot->geo.pos(), pSlot );
		}
	}
/*
	//____ _childRequestResize() _______________________________________________

	void Layer::_childRequestResize( Slot * pSlot )
	{
		_requestResize();			//TODO: Smarter handling, not request resize unless we need to.
	}
*/
	//____ _prevChild() ________________________________________________________

	Widget * Layer::_prevChild( const Slot * pSlot ) const
	{
		if( pSlot == &m_baseSlot )
			pSlot = _endLayerSlots();

		if (pSlot == _beginLayerSlots())
			return nullptr;

		LayerSlot * p = _decLayerSlot((LayerSlot*)pSlot,_sizeOfLayerSlot());
		return p->pWidget;
	}

	//____ _nextChild() ________________________________________________________

	Widget * Layer::_nextChild( const Slot * pSlot ) const
	{
		if (pSlot == &m_baseSlot)
			return nullptr;

		LayerSlot * p = _incLayerSlot((LayerSlot*)pSlot, _sizeOfLayerSlot());
		if (p < _endLayerSlots())
			return p->pWidget;

		return m_baseSlot.pWidget;
	}


} // namespace wg
