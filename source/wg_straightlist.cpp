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

#include <wg_straightlist.h>
#include <wg_patches.h>
#include <wg_eventhandler.h>

const char WgStraightList::CLASSNAME[] = {"StraightList"};
const char WgStraightListHook::CLASSNAME[] = {"StraightListHook"};


//____ WgStraightListHook::IsInstanceOf() __________________________________________

bool WgStraightListHook::IsInstanceOf( const char * pClassName ) const
{ 
	if( pClassName==CLASSNAME )
		return true;

	return WgListHook::IsInstanceOf(pClassName);
}

//____ WgStraightListHook::ClassName() _____________________________________________

const char * WgStraightListHook::ClassName( void ) const
{ 
	return CLASSNAME; 
}

//____ WgStraightListHook::Cast() ________________________________________________

WgStraightListHookPtr WgStraightListHook::Cast( const WgHookPtr& pHook )
{
	if( pHook && pHook->IsInstanceOf(CLASSNAME) )
		return WgStraightListHookPtr( static_cast<WgStraightListHook*>(pHook.GetRealPtr()) );

	return 0;
}

//____ WqStraightListHook::Pos() _________________________________________________

WgCoord WgStraightListHook::Pos() const
{
	WgRect	geo;
	m_pParent->_getChildGeo(geo,this);
	return geo.Pos();
}

WgSize WgStraightListHook::Size() const
{
	WgRect	geo;
	m_pParent->_getChildGeo(geo,this);
	return geo.Size();
}

WgRect WgStraightListHook::Geo() const
{
	WgRect	geo;
	m_pParent->_getChildGeo(geo,this);
	return geo;
}

WgCoord WgStraightListHook::ScreenPos() const
{
	WgRect	geo;
	m_pParent->_getChildGeo(geo,this);
	return m_pParent->ScreenPos() + geo.Pos();
}

WgRect WgStraightListHook::ScreenGeo() const
{
	WgRect	geo;
	m_pParent->_getChildGeo(geo,this);
	return geo + ScreenPos();
}


void WgStraightListHook::_requestRender()
{
	m_pParent->_onRequestRender(this);
}

void WgStraightListHook::_requestRender( const WgRect& rect )
{
	m_pParent->_onRequestRender(this, rect);
}

void WgStraightListHook::_requestResize()
{
	m_pParent->_onRequestResize(this);
}

WgHook *  WgStraightListHook::_prevHook() const
{
	return m_pParent->m_hooks.Prev(this);
}

WgHook *  WgStraightListHook::_nextHook() const
{
	return m_pParent->m_hooks.Next(this);
}

WgContainer *  WgStraightListHook::_parent() const
{
	return m_pParent;
}

//____ Constructor ____________________________________________________________

WgStraightList::WgStraightList()
{
	m_bSiblingsOverlap = false;
	m_bHorizontal = false;
	m_sortOrder = WG_SORT_ASCENDING;
	m_pSortFunc = 0;

	m_maxEntrySize = WgSize(INT_MAX,INT_MAX);		//TODO: Test so m_maxEntrySize matters!

	m_contentBreadth = 0;
	m_contentLength = 0;
	m_contentPreferredLength = 0;
	m_contentPreferredBreadth = 0;
	m_nbPreferredBreadthEntries = 0;

	m_pHoveredChild = 0;

	header.m_pHolder = this;
	header.m_height = 0;
	header.m_preferredWidth = 0;
	header.icon._setHolder(&header);
	header.arrow._setHolder(&header);
	header.label._setHolder(&header);
	header.label.SetWrap(false);			// Labels by default don't wrap.
}

//____ Destructor _____________________________________________________________

WgStraightList::~WgStraightList()
{
}

//____ IsInstanceOf() _________________________________________________________

bool WgStraightList::IsInstanceOf( const char * pClassName ) const
{ 
	if( pClassName==CLASSNAME )
		return true;

	return WgList::IsInstanceOf(pClassName);
}

//____ ClassName() ____________________________________________________________

const char * WgStraightList::ClassName( void ) const
{ 
	return CLASSNAME; 
}

//____ Cast() _________________________________________________________________

WgStraightListPtr WgStraightList::Cast( const WgObjectPtr& pObj )
{
	if( pObj && pObj->IsInstanceOf(CLASSNAME) )
		return WgStraightListPtr( static_cast<WgStraightList*>(pObj.GetRealPtr()) );

	return 0;
}

//____ AddWidget() ____________________________________________________________

WgStraightListHookPtr WgStraightList::AddWidget( const WgWidgetPtr& pWidget )
{
	WgStraightListHook * pHook = m_hooks.Add();
	pHook->m_pParent = this;
	pHook->_setWidget(pWidget.GetRealPtr());

	_onWidgetAppeared( pHook );
	return pHook;
}


//____ InsertWidget() ____________________________________________________________

WgStraightListHookPtr WgStraightList::InsertWidget( const WgWidgetPtr& pWidget, const WgWidgetPtr& pSibling )
{
	int index = 0;
	if( !pSibling )
		index = m_hooks.Size()-1;
	else 
	{
		WgStraightListHook * pHook = static_cast<WgStraightListHook*>(pSibling->_hook());
		if( pHook && pHook->_parent() == this )
			index = m_hooks.Index(pHook);
		else
			return 0;
	}

	WgStraightListHook * pHook = m_hooks.Insert(index);
	pHook->m_pParent = this;
	pHook->_setWidget(pWidget.GetRealPtr());

	_onWidgetAppeared( pHook );
	return pHook;
}

//____ InsertWidgetSorted() ___________________________________________________

WgStraightListHookPtr WgStraightList::InsertWidgetSorted( const WgWidgetPtr& pWidget )
{
	//TODO: Implement!

	return 0;
}

//____ RemoveWidget() _________________________________________________________

bool WgStraightList::RemoveWidget( const WgWidgetPtr& pWidget )
{
	if( !pWidget || !pWidget->_hook() )
		return false;

	WgStraightListHook * pHook = static_cast<WgStraightListHook*>(pWidget->_hook());
	if( pHook->_parent() != this )
		return false;

	int index = m_hooks.Index(pHook);
	_onWidgetDisappeared( pHook );
	m_hooks.Remove(index);
	return true;
}

//____ Clear() ________________________________________________________________

bool WgStraightList::Clear()
{
	m_hooks.Clear();
	_onRefresh();
	return true;
}

//____ SetOrientation() _______________________________________________________

void WgStraightList::SetOrientation( WgOrientation orientation )
{
	bool bHorizontal = (orientation == WG_HORIZONTAL);

	if( bHorizontal != m_bHorizontal )
	{
		m_bHorizontal = bHorizontal;
		_onRefresh();
	}
}

//____ SortWidgets() __________________________________________________________

void WgStraightList::SortWidgets()
{
	//TODO: Implement!!!
}

//____ SetSortOrder() _________________________________________________________

void WgStraightList::SetSortOrder( WgSortOrder order )
{
	//TODO: Implement!!!
}

//____ SetSortFunction() ______________________________________________________

void WgStraightList::SetSortFunction( WgWidgetSortFunc pSortFunc )
{
	//TODO: Implement!!!
}

//____ PreferredSize() ________________________________________________________

WgSize WgStraightList::PreferredSize() const
{
	WgSize sz = m_bHorizontal ? WgSize(m_contentPreferredLength, m_contentPreferredBreadth) : WgSize(m_contentPreferredBreadth,m_contentPreferredLength);

	if( header.m_preferredWidth > sz.w )
		sz.w = header.m_preferredWidth;

	sz.h += header.m_height;

	if( m_pSkin )
		sz += m_pSkin->ContentPadding();

	return sz;
}

//____ SetMinEntrySize() ______________________________________________________

bool WgStraightList::SetMinEntrySize( WgSize min )
{
	if( min == m_minEntrySize )
		return true;

	if( min.w > m_maxEntrySize.w || min.h > m_maxEntrySize.h )
		return false;

	m_minEntrySize = min;
	_onRefresh();
	return true;
}

//____ SetMaxEntrySize() ______________________________________________________

bool WgStraightList::SetMaxEntrySize( WgSize max )
{
	if( max == m_maxEntrySize )
		return true;

	if( max.w < m_minEntrySize.w || max.h < m_minEntrySize.h )
		return false;

	m_maxEntrySize = max;
	_onRefresh();
	return true;
}


//____ _onCollectPatches() ____________________________________________________

void WgStraightList::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	if( m_pSkin )
		container.Add( WgRect( geo, clip ) );
	else
	{
		if( m_bHorizontal )
			container.Add( WgRect( WgRect( geo.x, geo.y, WgMin(geo.w,m_contentLength), geo.h ), clip ) );
		else
			container.Add( WgRect( WgRect( geo.x, geo.y, geo.w, WgMin(geo.h,m_contentLength) ), clip ) );
	}
}

//____ _onMaskPatches() _______________________________________________________

void WgStraightList::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	if( (m_bOpaque && blendMode == WG_BLENDMODE_BLEND) || blendMode == WG_BLENDMODE_OPAQUE)
		patches.Sub( WgRect(geo,clip) );
	else if( m_bOpaqueEntries && blendMode == WG_BLENDMODE_BLEND )
	{
		if( m_bHorizontal )
			patches.Sub( WgRect( WgRect( geo.x, geo.y, WgMin(geo.w,m_contentLength), geo.h ), clip ) );
		else
			patches.Sub( WgRect( WgRect( geo.x, geo.y, geo.w, WgMin(geo.h,m_contentLength) ), clip ) );
	}
	else
	{
		WgRect childGeo;
		WgStraightListHook * p = static_cast<WgStraightListHook*>(_firstHookWithGeo( childGeo ));

		while(p)
		{
			if( p->_isVisible() )
				p->_widget()->_onMaskPatches( patches, childGeo + geo.Pos(), clip, blendMode );
			p = static_cast<WgStraightListHook*>(_nextHookWithGeo( childGeo, p ));
		}
	}

}

//____ _onCloneContent() ______________________________________________________

void WgStraightList::_onCloneContent( const WgWidget * _pOrg )
{
	//TODO: Implement!!!
}

//____ _renderPatches() _______________________________________________________

void WgStraightList::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	// We start by eliminating dirt outside our geometry

	WgPatches 	patches( _pPatches->Size() );								// TODO: Optimize by pre-allocating?

	for( const WgRect * pRect = _pPatches->Begin() ; pRect != _pPatches->End() ; pRect++ )
	{
		if( _canvas.IntersectsWith( *pRect ) )
			patches.Push( WgRect(*pRect,_canvas) );
	}

	// Render container itself
	
	for( const WgRect * pRect = patches.Begin() ; pRect != patches.End() ; pRect++ )
		_onRender(pDevice, _canvas, _window, *pRect );
		
	
	// Render children

	WgRect	dirtBounds = patches.Union();
	
	{
		WgRect childGeo;
		WgStraightListHook * p = (WgStraightListHook*)_firstHookWithGeo( childGeo );

		while(p)
		{
			WgRect canvas = childGeo + _canvas.Pos();
			if( p->_isVisible() && canvas.IntersectsWith( dirtBounds ) )
				p->_widget()->_renderPatches( pDevice, canvas, canvas, &patches );
			p = (WgStraightListHook*) _nextHookWithGeo( childGeo, p );
		}
	}

	// Render header

	if( header.m_height != 0 )
	{
		bool bInvertedSort = true;
		WgRect canvas( _window.x, _window.y, _window.w, header.m_height );

		for( const WgRect * pRect = patches.Begin() ; pRect != patches.End() ; pRect++ )
			_renderHeader( pDevice, canvas, *pRect, header.m_pSkin, &header.label, &header.icon, &header.arrow, header.m_state, true, bInvertedSort );
	}

	// Render Lasso

	if( m_pLassoSkin && m_lassoBegin != m_lassoEnd )
	{
		WgRect lasso( m_lassoBegin, m_lassoEnd );
		lasso += _canvas.Pos();


		for( const WgRect * pRect = patches.Begin() ; pRect != patches.End() ; pRect++ )
			m_pLassoSkin->Render( pDevice, lasso, m_state, WgRect( lasso, *pRect ) );
	}
}


//____ _onRender() ____________________________________________________________

void WgStraightList::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	WgWidget::_onRender( pDevice, _canvas, _window, _clip );

	WgRect contentRect = _canvas;
	if( m_pSkin )
		contentRect = m_pSkin->ContentRect( _canvas, m_state );

	int startOfs = m_bHorizontal ? _clip.x-_canvas.x : _clip.y-_canvas.y;
	if( startOfs < 0 )
		startOfs = 0;

	for( int i = _getEntryAt( startOfs ) ; i < m_hooks.Size() ; i++ )
	{
		WgStraightListHook * pHook = m_hooks.Hook(i);
		WgWidget * pChild = pHook->_widget();

		// Get entry geometry, skin and state

		WgRect entryGeo( contentRect );
		if( m_bHorizontal )
		{
			if( pHook->m_ofs >= contentRect.w )
				break;

			entryGeo.x += pHook->m_ofs;
			entryGeo.w = pHook->m_length;
		}
		else
		{
			if( pHook->m_ofs >= contentRect.h )
				break;

			entryGeo.y += pHook->m_ofs;
			entryGeo.h = pHook->m_length;
		}
		
		WgSkin * pEntrySkin	= m_pEntrySkin[i&0x1].GetRealPtr();
		WgState	state		= pChild->State();
//		WgRect	childGeo( entryGeo );

		// Render entry skin, shrink child geo

		if( pEntrySkin )
		{
			pEntrySkin->Render( pDevice, entryGeo, state, _clip );
//			childGeo = pEntrySkin->ContentRect( entryGeo, state );
		}

		// Render child

//		pChild->_onRender( pDevice, childGeo, childGeo, _clip );

	}
}

//____ _onNewSize() ___________________________________________________________

void WgStraightList::_onNewSize( const WgSize& size )
{
	int newContentBreadth;

	if( m_bHorizontal )
		newContentBreadth = size.h;
	else
		newContentBreadth = size.w;

	if( newContentBreadth != m_contentBreadth )
	{
		m_contentBreadth = newContentBreadth;
		int ofs = 0;

		for( int i = 0 ; i < m_hooks.Size() ; i++ )
		{
			WgStraightListHook * pHook = m_hooks.Hook(i);
			WgWidget * pWidget = pHook->_widget();

			if( m_bHorizontal )
			{
				int newEntryLength = _paddedLimitedWidthForHeight(pWidget, newContentBreadth );
				pHook->m_ofs = ofs;
				pHook->m_length = newEntryLength;
				ofs += newEntryLength;

				pWidget->_onNewSize( WgSize(newEntryLength, newContentBreadth) );				//TODO: Should be able to do a _onNewSize() that prevents child from doing a _requestRender().
			}
			else
			{
				int newEntryLength = _paddedLimitedHeightForWidth(pWidget, newContentBreadth );
				pHook->m_ofs = ofs;
				pHook->m_length = newEntryLength;
				ofs += newEntryLength;

				pWidget->_onNewSize( WgSize(newContentBreadth, newEntryLength) );				//TODO: Should be able to do a _onNewSize() that prevents child from doing a _requestRender().
			}
		}
		m_contentLength = ofs;
	}

	m_size = size;
	_requestRender();
}

//____ _onRefresh() ___________________________________________________________

void WgStraightList::_onRefresh()
{
	//TODO: Implement!!!
}

//____ _onEvent() _____________________________________________________________

void WgStraightList::_onEvent( const WgEventPtr& _pEvent, WgEventHandler * pHandler )
{
	WgState oldState = m_state;

	switch( _pEvent->Type() )
	{
		case WG_EVENT_KEY_PRESS:
		{
			if( m_selectMode == WG_SELECT_NONE )
				break;

			int				keyCode = WgKeyPressEvent::Cast(_pEvent)->TranslatedKeyCode();
			WgModifierKeys	modKeys = WgKeyPressEvent::Cast(_pEvent)->ModKeys();
			if( (m_bHorizontal && (keyCode == WG_KEY_LEFT || keyCode == WG_KEY_RIGHT)) || 
				(!m_bHorizontal && (keyCode == WG_KEY_UP || keyCode == WG_KEY_DOWN || keyCode == WG_KEY_PAGE_UP || keyCode == WG_KEY_PAGE_DOWN)) ||
				keyCode == WG_KEY_HOME || keyCode == WG_KEY_END ||
				(m_selectMode == WG_SELECT_FLIP && keyCode == WG_KEY_SPACE ) )
					pHandler->SwallowEvent(_pEvent);
			break;
		}

		case WG_EVENT_KEY_REPEAT:
		case WG_EVENT_KEY_RELEASE:
		{
			if( m_selectMode == WG_SELECT_NONE )
				break;

			int				keyCode = WgKeyEvent::Cast(_pEvent)->TranslatedKeyCode();
			WgModifierKeys	modKeys = WgKeyEvent::Cast(_pEvent)->ModKeys();
			if( (m_bHorizontal && (keyCode == WG_KEY_LEFT || keyCode == WG_KEY_RIGHT)) || 
				(!m_bHorizontal && (keyCode == WG_KEY_UP || keyCode == WG_KEY_DOWN || keyCode == WG_KEY_PAGE_UP || keyCode == WG_KEY_PAGE_DOWN)) ||
				keyCode == WG_KEY_HOME || keyCode == WG_KEY_END ||
				(m_selectMode == WG_SELECT_FLIP && keyCode == WG_KEY_SPACE ) )
					pHandler->SwallowEvent(_pEvent);
			break;
		}
	
		default:
			WgList::_onEvent(_pEvent, pHandler);
			return;
	}

	if( m_state != oldState )
		_onStateChanged(oldState);
}

//____ _onLassoUpdated() ______________________________________________________

void WgStraightList::_onLassoUpdated( const WgRect& oldLasso, const WgRect& newLasso )
{
	// Get out content area

	WgRect listArea = _listArea();

	// Check if our lassos are inside content area or not.

	bool	bOldLassoInside = false;
	bool	bNewLassoInside = false;

	if( oldLasso.IntersectsWith(listArea ) )
		bOldLassoInside = true;
	if( newLasso.IntersectsWith(listArea ) )
		bNewLassoInside = true;

	if( !bOldLassoInside && !bNewLassoInside )
		return;										// None of the lassos inside content.

	// Get first/last entries marked by old/new lasso

	int oldOfs1, oldOfs2;
	int newOfs1, newOfs2;


	if( m_bHorizontal )
	{
		oldOfs1 = oldLasso.x - listArea.x;
		oldOfs2 = oldLasso.x + oldLasso.w - listArea.x;
		newOfs1 = newLasso.x - listArea.x;
		newOfs2 = newLasso.x + newLasso.w - listArea.x;
	}
	else
	{
		oldOfs1 = oldLasso.y - listArea.y;
		oldOfs2 = oldLasso.y + oldLasso.h - listArea.y;
		newOfs1 = newLasso.y - listArea.y;
		newOfs2 = newLasso.y + newLasso.h - listArea.y;
	}

	int oldFirst = _getEntryAt( oldOfs1 );
	int oldLast = _getEntryAt( oldOfs2 );
	int newFirst = _getEntryAt( newOfs1 );
	int newLast = _getEntryAt( newOfs2 );

	//

	if( bOldLassoInside != bNewLassoInside )
	{
		int beg, end;
		if( bNewLassoInside )
		{
			beg = newFirst;
			end = newLast;
		}
		else
		{
			beg = oldFirst;
			end = oldLast;
		}

		_flipRange( m_hooks.Hook(beg), m_hooks.Hook(end), true );
	}
	else
	{
		if( oldFirst != newFirst )
		{
			int beg = WgMin(oldFirst,newFirst);
			int end = WgMax(oldFirst,newFirst)-1;

			_flipRange( m_hooks.Hook(beg), m_hooks.Hook(end), true );
		}

		if( oldLast != newLast )
		{
			int beg = WgMin(oldLast,newLast)+1;
			int end = WgMax(oldLast,newLast);

			_flipRange( m_hooks.Hook(beg), m_hooks.Hook(end), true );
		}
	}
}


//____ _onStateChanged() ______________________________________________________

void WgStraightList::_onStateChanged( WgState oldState )
{
	WgList::_onStateChanged(oldState);
}


//____ _onRequestRender() _____________________________________________________

void WgStraightList::_onRequestRender( WgStraightListHook * pHook )
{
	WgRect geo;
	_getEntryGeo(geo, pHook);		// Render whole entry, entry skin might need to be redrawn due to state change.
	_requestRender(geo);
}

void WgStraightList::_onRequestRender( WgStraightListHook * pHook, const WgRect& rect )
{
	WgRect geo;
	_getChildGeo(geo, pHook);
	geo.x += rect.x;
	geo.y += rect.y;
	geo.w = rect.w;
	geo.h = rect.h;
	_requestRender(geo);
}

//____ _onRequestResize() _____________________________________________________

void WgStraightList::_onRequestResize( WgStraightListHook * pHook )
{
	if( !pHook->m_bVisible  || m_minEntrySize == m_maxEntrySize )
		return;

	WgWidget * pChild = pHook->_widget();
	WgSize prefEntrySize = _paddedLimitedPreferredSize(pChild);

	int prefLength = m_bHorizontal ? prefEntrySize.w : prefEntrySize.h;
	int prefBreadth = m_bHorizontal ? prefEntrySize.h : prefEntrySize.w;

	bool	bReqResize = false;

	// Update preferred sizes

	if( prefBreadth != pHook->m_prefBreadth || prefLength != pHook->m_length )
	{
		_subFromContentPreferredSize( pHook->m_length, pHook->m_prefBreadth );
		_addToContentPreferredSize( prefLength, prefBreadth );

		pHook->m_prefBreadth = prefBreadth;
		bReqResize = true;
	}

	// Calculate new length

	int length;
	if( prefBreadth == m_contentBreadth )	
		length = prefLength;
	else
		length = m_bHorizontal ? _paddedLimitedWidthForHeight(pChild, prefBreadth ) : _paddedLimitedHeightForWidth(pChild, prefBreadth );

	// Update if length has changed

	if( length != pHook->m_length )
	{
		m_contentLength += length - pHook->m_length;
		pHook->m_length = length;
		bReqResize = true;

		_updateChildOfsFrom( pHook );
		_requestRenderChildrenFrom( pHook );
	}

	if( bReqResize )
		_requestResize();
}

//____ _onWidgetAppeared() ____________________________________________________

void WgStraightList::_onWidgetAppeared( WgListHook * pInserted )
{
	WgStraightListHook * pHook = static_cast<WgStraightListHook*>(pInserted);
	WgWidget * pChild = pHook->_widget();

	WgSize pref = _paddedLimitedPreferredSize( pChild );

	if( m_bHorizontal )
	{
		_addToContentPreferredSize( pref.w, pref.h );

		// Get entry length and breadth

		if( pref.h == m_contentBreadth )
			pHook->m_length = pref.w;
		else
			pHook->m_length	= _paddedLimitedWidthForHeight(pChild, m_contentBreadth);
		pHook->m_prefBreadth = pref.h;
	}
	else
	{
		_addToContentPreferredSize( pref.h, pref.w );

		// Get entry length and breadth

		if( pref.w == m_contentBreadth )
			pHook->m_length = pref.h;
		else
			pHook->m_length = _paddedLimitedHeightForWidth(pChild, m_contentBreadth);
		pHook->m_prefBreadth = pref.w;
	}

	m_contentLength += pHook->m_length;

	// Finish up

	_updateChildOfsFrom( pHook );
	_requestRenderChildrenFrom( pHook );	// Request render on dirty area
	_requestResize();						// This should preferably be done first once we have changed the method.

}

//____ _onWidgetDisappeared() _________________________________________________

void WgStraightList::_onWidgetDisappeared( WgListHook * pToBeRemoved )
{
	WgStraightListHook * pHook = static_cast<WgStraightListHook*>(pToBeRemoved);
	WgWidget * pChild = pHook->_widget();

	WgSize pref = _paddedLimitedPreferredSize( pChild );

	_requestRenderChildrenFrom( pHook );	// Request render on dirty area

	if( m_bHorizontal )
		_subFromContentPreferredSize( pref.w, pref.h );
	else
		_subFromContentPreferredSize( pref.h, pref.w );

	m_contentLength -= pHook->m_length;

	pHook->m_length = 0;

	_updateChildOfsFrom( m_hooks.Next(pHook) );
	_requestResize();
}

//____ _getEntryAt() __________________________________________________________

// Pixelofs is counted from beginning of container content, not widget.

int WgStraightList::_getEntryAt( int pixelofs ) const
{
	int first = 0;
	int last = m_hooks.Size() - 1;
	int middle = (first+last)/2;
 
	while( first <= last )
	{
		WgStraightListHook * pHook = m_hooks.Hook(middle);

		if( pHook->m_ofs + pHook->m_length < pixelofs )
			first = middle + 1;
		else if( pHook->m_ofs <= pixelofs ) 
			return middle;
		else
			last = middle - 1;
 
		middle = (first + last)/2;
	}

	return m_hooks.Size();
}

//____ _findEntry() ___________________________________________________________

WgListHook * WgStraightList::_findEntry( const WgCoord& ofs )
{
	WgWidget * pResult = 0;
	WgRect list = _listArea();

	if( list.Contains(ofs) )
	{
		int entry;
		if( m_bHorizontal )
			entry = _getEntryAt(ofs.x-list.x);
		else
			entry = _getEntryAt(ofs.y-list.y);

		if( entry != m_hooks.Size() )
			return m_hooks.Hook(entry);
	}

	return 0;
}

//____ _findWidget() __________________________________________________________

WgWidget * WgStraightList::_findWidget( const WgCoord& ofs, WgSearchMode mode )
{
	WgWidget * pResult = 0;
	WgRect list = _listArea();

	if( list.Contains(ofs) )
	{
		int entry;
		if( m_bHorizontal )
			entry = _getEntryAt(ofs.x-list.x);
		else
			entry = _getEntryAt(ofs.y-list.y);

		if( entry != m_hooks.Size() )
		{
			WgStraightListHook * pHook = m_hooks.Hook(entry);
			if( mode == WG_SEARCH_ACTION_TARGET )
				pResult = pHook->_widget();
			else
			{
				WgRect childGeo = list;
				if( m_bHorizontal )
				{
					childGeo.x += pHook->m_ofs;
					childGeo.w = pHook->m_length;
				}
				else
				{
					childGeo.y += pHook->m_ofs;
					childGeo.h = pHook->m_length;
				}

				if( m_pEntrySkin[entry&0x1] )
					childGeo = m_pEntrySkin[entry&0x1]->ContentRect( childGeo, pHook->_widget()->State() );
			
				if( childGeo.Contains(ofs) )
				{
					if( pHook->_widget()->IsContainer() )
					{
						pResult = static_cast<WgContainer*>(pHook->_widget())->_findWidget( ofs - childGeo.Pos(), mode );
					}
					else if( mode == WG_SEARCH_GEOMETRY || pHook->_widget()->MarkTest( ofs - childGeo.Pos() ) )
					{
							pResult = pHook->_widget();
					}
				}
			}
		}
	}

	// Check against ourselves

	if( !pResult && ( mode == WG_SEARCH_GEOMETRY || MarkTest(ofs)) )
		pResult = this;
		
	return pResult;
}

//____ _addToContentPreferredSize() ___________________________________________

void  WgStraightList::_addToContentPreferredSize( int length, int breadth )
{
	m_contentPreferredLength += length;
	
	if( breadth == m_contentPreferredBreadth )
		m_nbPreferredBreadthEntries++;
	else if( breadth > m_contentPreferredBreadth )
	{
		m_contentPreferredBreadth = breadth;
		m_nbPreferredBreadthEntries = 1;
	}
}

//____ _subFromContentPreferredSize() _________________________________________

void  WgStraightList::_subFromContentPreferredSize( int length, int breadth )
{
	m_contentPreferredLength -= length;
	
	if( breadth == m_contentPreferredBreadth )
	{
		m_nbPreferredBreadthEntries--;
		if( m_nbPreferredBreadthEntries == 0 )
		{
			int highest = 0;
			for( WgStraightListHook * p = m_hooks.Begin() ; p < m_hooks.End() ; p++ )
			{
				if( p->m_prefBreadth == highest )
					m_nbPreferredBreadthEntries++;
				else if( p->m_prefBreadth > highest )
				{
					highest = p->m_prefBreadth;
					m_nbPreferredBreadthEntries = 0;
				}
			}
			m_contentPreferredBreadth = highest;
		}
	}
}


//____ _requestRenderChildrenFrom() ___________________________________________

void WgStraightList::_requestRenderChildrenFrom( WgStraightListHook * pHook )
{
	WgRect box = _listArea();

	if( m_bHorizontal )
	{
		box.x += pHook->m_ofs;
		box.w = m_contentLength - pHook->m_ofs;
	}
	else
	{
		box.y += pHook->m_ofs;
		box.h = m_contentLength - pHook->m_ofs;
	}

	_requestRender( box );
}

//____ _updateChildOfsFrom() __________________________________________________

void WgStraightList::_updateChildOfsFrom( WgStraightListHook * pHook )
{
	int ofs = 0;
	WgStraightListHook * pPrev = m_hooks.Prev(pHook);
	if( pPrev )
		ofs = pPrev->m_ofs + pPrev->m_length;

	while( pHook < m_hooks.End() )
	{
		pHook->m_ofs += ofs;
		ofs += pHook->m_length;
		pHook++;
	}	
}

//____ _onEntrySkinChanged() __________________________________________________

void WgStraightList::_onEntrySkinChanged( WgSize oldPadding, WgSize newPadding )
{
	_requestRender();

	if( oldPadding != newPadding )
	{
		m_entryPadding = newPadding;
		int nEntries = m_hooks.Size();

		int	lengthDiff, breadthDiff;
		if( m_bHorizontal )
		{
			lengthDiff = (newPadding.w - oldPadding.w)*nEntries;
			breadthDiff = (newPadding.h - oldPadding.h)*nEntries;
		}
		else
		{
			lengthDiff = (newPadding.h - oldPadding.h)*nEntries;
			breadthDiff = (newPadding.w - oldPadding.w)*nEntries;
		}

		if( lengthDiff != 0 || breadthDiff != 0 )
		{
			m_contentLength += lengthDiff;
			m_contentBreadth += breadthDiff;

			m_contentPreferredLength += lengthDiff;
			m_contentPreferredBreadth += breadthDiff;

			_requestResize();
		}
	}
}

//____ _getEntryGeo() _________________________________________________________

void WgStraightList::_getEntryGeo( WgRect& geo, const WgStraightListHook * pHook ) const
{
	geo = _listArea();

	if( m_bHorizontal )
	{
		geo.x += pHook->m_ofs;
		geo.w = pHook->m_length;
	}
	else
	{
		geo.y += pHook->m_ofs;
		geo.h = pHook->m_length;
	}
}


//____ _getChildGeo() _________________________________________________________

void WgStraightList::_getChildGeo( WgRect& geo, const WgStraightListHook * pHook ) const
{
	geo = _listArea();

	if( m_bHorizontal )
	{
		geo.x += pHook->m_ofs;
		geo.w = pHook->m_length;
	}
	else
	{
		geo.y += pHook->m_ofs;
		geo.h = pHook->m_length;
	}

	// Apply any padding from the entry skin, if entry visible

	if( pHook->m_bVisible )
	{
		int index = m_hooks.Index( pHook );
		if( m_pEntrySkin[index&0x1] )
			geo = m_pEntrySkin[index&0x1]->ContentRect( geo, pHook->_widget()->State() );
	}
}

//____ _paddedLimitedHeightForWidth() _________________________________________

int WgStraightList::_paddedLimitedHeightForWidth( WgWidget * pChild, int paddedWidth )
{
	int height = pChild->HeightForWidth( paddedWidth - m_entryPadding.w ) + m_entryPadding.h;
	WG_LIMIT( height, m_minEntrySize.h, m_maxEntrySize.h );
	return height;
}

//____ _paddedLimitedWidthForHeight() _________________________________________

int WgStraightList::_paddedLimitedWidthForHeight( WgWidget * pChild, int paddedHeight )
{
	int width = pChild->WidthForHeight( paddedHeight - m_entryPadding.h ) + m_entryPadding.w;
	WG_LIMIT( width, m_minEntrySize.w, m_maxEntrySize.w );
	return width;
}

//____ _paddedLimitedPreferredSize() __________________________________________

WgSize WgStraightList::_paddedLimitedPreferredSize( WgWidget * pChild )
{
	WgSize sz = pChild->PreferredSize();
	sz += m_entryPadding;

	// Apply limits

	if( sz.w < m_minEntrySize.w )
		sz.w = m_minEntrySize.w;
	if( sz.h < m_minEntrySize.h )
		sz.h = m_minEntrySize.h;

	if( sz.w > m_maxEntrySize.w )
	{
		int h = pChild->HeightForWidth(m_maxEntrySize.w-m_entryPadding.w) + m_entryPadding.h;
		WG_LIMIT(h, m_minEntrySize.h, m_maxEntrySize.h );
	}
	else if( sz.h > m_maxEntrySize.h )
	{
		int w = pChild->WidthForHeight(m_maxEntrySize.h-m_entryPadding.h) + m_entryPadding.w;
		WG_LIMIT(w, m_minEntrySize.w, m_maxEntrySize.w );
	}

	return sz;
}

//____ _firstHook() ___________________________________________________________

WgHook* WgStraightList::_firstHook() const
{
	if( m_hooks.Size() > 0 )
		return m_hooks.Hook(0);

	return 0;
}

//____ _lastHook() ____________________________________________________________

WgHook* WgStraightList::_lastHook() const
{
	if( m_hooks.Size() > 0 )
		return m_hooks.Hook(m_hooks.Size()-1);

	return 0;
}

//____ _firstHookWithGeo() ____________________________________________________

WgHook* WgStraightList::_firstHookWithGeo( WgRect& geo ) const
{
	if( m_hooks.Size() == 0 )
		return 0;

	WgStraightListHook * p = m_hooks.Hook(0);
	_getChildGeo(geo,p);
	return p;
}

//____ _nextHookWithGeo() _____________________________________________________

WgHook* WgStraightList::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgStraightListHook * p = m_hooks.Next(static_cast<WgStraightListHook*>(pHook));
	if( p )
		_getChildGeo(geo,p);
	return p;
}

//_____ _lastHookWithGeo() ____________________________________________________

WgHook* WgStraightList::_lastHookWithGeo( WgRect& geo ) const
{
	if( m_hooks.Size() == 0 )
		return 0;

	WgStraightListHook * p = m_hooks.Hook(m_hooks.Size()-1);
	_getChildGeo(geo,p);
	return p;
}

//____ _prevHookWithGeo() _____________________________________________________

WgHook* WgStraightList::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgStraightListHook * p = m_hooks.Prev(static_cast<WgStraightListHook*>(pHook));
	if( p )
		_getChildGeo(geo,p);
	return p;
}

//____ _listArea() ____________________________________________________________

WgRect WgStraightList::_listArea() const
{
	WgRect r(0,0,m_size);

	if( m_pSkin )
		r = m_pSkin->ContentRect( r, m_state );

	r.y += header.m_height;
	r.h -= header.m_height;

	return r;
}


//____ _refreshHeader() _______________________________________________________

void WgStraightList::_refreshHeader()
{
	WgSize wantedIconSize = header.icon.PreferredSize();
	WgSize wantedArrowSize = header.arrow.PreferredSize();
	WgSize wantedTextSize = header.label.unwrappedSize();

	WgSize wantedSize;

	//TODO: Assumes icon/arrow origos to not be NORTH, SOUTH or CENTER.
	//TODO: Assumes text not wrapping.

	wantedSize.h = WgMax(wantedIconSize.h, wantedArrowSize.h, wantedTextSize.h );
	wantedSize.w = wantedTextSize.w;
	if( header.icon.Overlap() )
		wantedSize.w = WgMax(wantedSize.w,wantedIconSize.w);
	else
		wantedSize.w += wantedIconSize.w;

	if( header.arrow.Overlap() )
		wantedSize.w = WgMax(wantedSize.w,wantedArrowSize.w);
	else
		wantedSize.w += wantedArrowSize.w;

	//

	if( header.m_pSkin )
		wantedSize = header.m_pSkin->SizeForContent(wantedSize);
	//

	bool	bRequestResize = false;
	if( wantedSize.h != header.m_height )
	{
		header.m_height = wantedSize.h;
		bRequestResize = true;
	}

	// Update headers preferred width, possibly request resize.

	if( wantedSize.w != header.m_preferredWidth ) 
	{
		int contentPrefWidth = m_bHorizontal ? m_contentPreferredLength : m_contentPreferredBreadth;
		if( wantedSize.w > contentPrefWidth || header.m_preferredWidth > contentPrefWidth )
			bRequestResize = true;

		header.m_preferredWidth = wantedSize.w;
	}

	if( bRequestResize )
		_requestResize();
}

//____ Header::SetSkin() ______________________________________________________

void WgStraightList::Header::SetSkin( const WgSkinPtr& pSkin )
{
	if( pSkin != m_pSkin )
	{
		m_pSkin = pSkin;
		m_pHolder->_refreshHeader();
	}
}

//____ Header::_fieldModified() _______________________________________________________

void WgStraightList::Header::_fieldModified( WgTextField * pText )
{
	m_pHolder->_refreshHeader();
}

void WgStraightList::Header::_fieldModified( WgIconField * pField )
{
	m_pHolder->_refreshHeader();
}