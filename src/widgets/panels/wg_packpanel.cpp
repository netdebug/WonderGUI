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

#include <wg_packpanel.h>
#include <wg_base.h>

const char WgPackPanel::CLASSNAME[] = {"PackPanel"};
const char WgPackHook::CLASSNAME[] = {"PackHook"};


//____ WgPackHook::Constructor ________________________________________________

WgPackHook::WgPackHook( WgPackPanel * pParent )
{
	m_pParent = pParent;
	m_weight = 1.f;
}

//____ WgPackHook::isInstanceOf() __________________________________________

bool WgPackHook::isInstanceOf( const char * pClassName ) const
{ 
	if( pClassName==CLASSNAME )
		return true;

	return WgVectorHook::isInstanceOf(pClassName);
}

//____ WgPackHook::className() _____________________________________________

const char * WgPackHook::className( void ) const
{ 
	return CLASSNAME; 
}

//____ WgPackHook::cast() __________________________________________________

WgPackHook_p WgPackHook::cast( const WgHook_p& pHook )
{
	if( pHook && pHook->isInstanceOf(CLASSNAME) )
		return WgPackHook_p( static_cast<WgPackHook*>(pHook.rawPtr()) );

	return 0;
}

//____ WgPackHook::parent() ___________________________________________________

WgPackPanel_p WgPackHook::parent() const 
{ 
	return m_pParent; 
}

//____ WgPackHook::setWeight() ________________________________________________

bool WgPackHook::setWeight( float weight )
{
	if( weight < 0 )
		return false;

	if( weight != m_weight )
	{
		m_weight = weight;
		parent()->_refreshChildGeo();
	}
	return true;
}

//____ WgPackHook::_parent() __________________________________________________

WgContainer * WgPackHook::_parent() const
{
	return m_pParent;
}

//____ Constructor ____________________________________________________________

WgPackPanel::WgPackPanel()
{
	m_bSiblingsOverlap = false;
	m_bHorizontal = true;
    m_pSizeBroker = 0;
}

//____ Destructor _____________________________________________________________

WgPackPanel::~WgPackPanel()
{
}

//____ isInstanceOf() _________________________________________________________

bool WgPackPanel::isInstanceOf( const char * pClassName ) const
{ 
	if( pClassName==CLASSNAME )
		return true;

	return WgVectorPanel::isInstanceOf(pClassName);
}

//____ className() ____________________________________________________________

const char * WgPackPanel::className( void ) const
{ 
	return CLASSNAME; 
}

//____ cast() _________________________________________________________________

WgPackPanel_p WgPackPanel::cast( const WgObject_p& pObject )
{
	if( pObject && pObject->isInstanceOf(CLASSNAME) )
		return WgPackPanel_p( static_cast<WgPackPanel*>(pObject.rawPtr()) );

	return 0;
}

//____ setOrientation() ______________________________________________________

void WgPackPanel::setOrientation( WgOrientation orientation )
{
	bool bHorizontal = orientation==WG_HORIZONTAL?true:false;
	if( m_bHorizontal != bHorizontal )
	{
		m_bHorizontal = bHorizontal;
		_updatePreferredSize();
		_refreshChildGeo();
	}
}


//____ setSizeBroker() _______________________________________________________

void WgPackPanel::setSizeBroker( const WgSizeBroker_p& pBroker )
{
	if( m_pSizeBroker != pBroker )
	{
		m_pSizeBroker = pBroker;
		_updatePreferredSize();
		_refreshChildGeo();
	}
}

//____ preferredSize() _______________________________________________________

WgSize WgPackPanel::preferredSize() const
{
	return m_preferredSize;
}

//____ matchingHeight() _______________________________________________________

int WgPackPanel::matchingHeight( int width ) const
{
	int height = 0;

	if( m_bHorizontal )
	{
		if( m_pSizeBroker )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::memStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea);		
		
			// Retrieve item lengths and find height of highest item.

			m_pSizeBroker->setItemLengths( pItemArea, nItems, width );

			WgPackHook * pH = _firstHook();
			WgSizeBrokerItem * pI = pItemArea;

			while( pH )
			{
				if( pH->isVisible() )
				{
					int itemHeight = pH->_paddedMatchingHeight( pI->output );
					if( itemHeight > height )
						height = itemHeight;
					pI++;
				}

				pH = pH->_next();
			}

			// Release temporary memory area
		
			WgBase::memStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * pH = _firstHook();

			while( pH )
			{
				if( pH->isVisible() && pH->m_preferredSize.h > height )
						height = pH->m_preferredSize.h;

				pH = pH->_next();
			}
		}
	}
	else
	{
		if( m_pSizeBroker && m_pSizeBroker->mayAlterPreferredLengths() )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::memStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea, width);		
		
			// Retrieve preferred length

			height = m_pSizeBroker->setPreferredLengths( pItemArea, nItems );
				
			// Release temporary memory area
		
			WgBase::memStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * p = _firstHook();

			while( p )
			{
				if( p->isVisible() )
					height += p->_paddedMatchingHeight( width );

				p = p->_next();
			}
		}
	}
	return height;
}

//____ matchingWidth() _______________________________________________________

int WgPackPanel::matchingWidth( int height ) const
{
	int width = 0;

	if( !m_bHorizontal )
	{
		if( m_pSizeBroker )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::memStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea);		
		
			// Retrieve item lengths and find height of highest item.

			m_pSizeBroker->setItemLengths( pItemArea, nItems, height );

			WgPackHook * pH = _firstHook();
			WgSizeBrokerItem * pI = pItemArea;

			while( pH )
			{
				if( pH->isVisible() )
				{
					int itemWidth = pH->_paddedMatchingWidth( pI->output );
					if( itemWidth > width )
						width = itemWidth;
					pI++;
				}

				pH = pH->_next();
			}

			// Release temporary memory area
		
			WgBase::memStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * pH = _firstHook();

			while( pH )
			{
				if( pH->isVisible() && pH->m_preferredSize.w > width )
						width = pH->m_preferredSize.w;

				pH = pH->_next();
			}
		}
	}
	else
	{
		if( m_pSizeBroker && m_pSizeBroker->mayAlterPreferredLengths() )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::memStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea, height);		
		
			// Retrieve preferred length

			width = m_pSizeBroker->setPreferredLengths( pItemArea, nItems );
				
			// Release temporary memory area
		
			WgBase::memStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * p = _firstHook();

			while( p )
			{
				if( p->isVisible() )
					width += p->_paddedMatchingWidth( height );

				p = p->_next();
			}
		}
	}
	return width;
}



//____ _firstHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_firstHookWithGeo( WgRect& geo ) const
{	
	WgPackHook * p = _firstHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _nextHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook)->_next();
	if( p )
		geo = p->m_geo;
	return p;	
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_lastHookWithGeo( WgRect& geo ) const
{
	WgPackHook * p = _lastHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _prevHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook)->_prev();
	if( p )
		geo = p->m_geo;
	return p;	
}

//____ _hookGeo() _____________________________________________________________

WgRect WgPackPanel::_hookGeo( const WgVectorHook * pHook )
{
	return static_cast<const WgPackHook*>(pHook)->m_geo;
}

//____ _onRenderRequested() ____________________________________________________

void WgPackPanel::_onRenderRequested( WgVectorHook * pHook )
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook);
	_requestRender( p->m_geo );
}

void WgPackPanel::_onRenderRequested( WgVectorHook * pHook, const WgRect& rect )
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook);
	_requestRender( rect + p->m_geo.pos() );
}

//____ _onResizeRequested() _____________________________________________________

void WgPackPanel::_onResizeRequested( WgVectorHook * pHook )
{
	// Update cached preferred size of child
	
	WgPackHook * p = static_cast<WgPackHook*>(pHook);
	p->m_preferredSize = p->_paddedPreferredSize();

	//
	
	_refreshAllWidgets();
}

//____ _onWidgetAppeared() ______________________________________________________

void WgPackPanel::_onWidgetAppeared( WgVectorHook * pInserted )
{
	// Update cached preferred size of child
	
	WgPackHook * p = static_cast<WgPackHook*>(pInserted);
	p->m_preferredSize = p->_paddedPreferredSize();
	
	//
	
	_refreshAllWidgets();
}

//____ _onWidgetDisappeared() ___________________________________________________

void WgPackPanel::_onWidgetDisappeared( WgVectorHook * pToBeRemoved )
{
	_refreshAllWidgets();
}

//____ _onWidgetsReordered() ____________________________________________________

void WgPackPanel::_onWidgetsReordered()
{
	_refreshChildGeo();
}

//____ _refreshAllWidgets() _____________________________________________________

void WgPackPanel::_refreshAllWidgets()
{
	_updatePreferredSize();
	_refreshChildGeo();
}


//____ _newHook() ____________________________________________________________

WgVectorHook * WgPackPanel::_newHook()
{
	return new WgPackHook(this);
}

//____ _onNewSize() ____________________________________________________________

void WgPackPanel::_onNewSize( const WgSize& size )
{
    _refreshChildGeo();
}


//____ _updatePreferredSize() ______________________________________________________

void WgPackPanel::_updatePreferredSize()
{
	int length = 0;
	int breadth = 0;

	if( m_pSizeBroker && m_pSizeBroker->mayAlterPreferredLengths() )
	{
		// Allocate and populate SizeBroker array
		
		int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
		WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::memStackAlloc(arrayBytes));
		
		int nItems = _populateSizeBrokerArray(pItemArea);		
		
		// Retrieve preferred length and breadth
		
		length = m_pSizeBroker->setPreferredLengths( pItemArea, nItems );
		
		WgPackHook * pH = _firstHook();
		WgSizeBrokerItem * pI = pItemArea;
		while( pH )
		{
			if( pH->isVisible() )
			{
				int b = m_bHorizontal?pH->_paddedMatchingHeight(pI->output):pH->_paddedMatchingWidth(pI->output);
				if( b > breadth )
					breadth = b;			
				pI++;
			}
			pH = pH->_next();
		}
		
		// Release temporary memory area
		
		WgBase::memStackRelease(arrayBytes);
		
	}
	else
	{
		WgPackHook * p = _firstHook();

		if( m_bHorizontal )
		{
            while( p )
            {
				if( p->isVisible() )
				{
					length += p->m_preferredSize.w;
	                if( p->m_preferredSize.h > breadth )
	                    breadth = p->m_preferredSize.h;
				}
				p = p->_next();
            }
		}
		else
		{
            while( p )
            {
				if( p->isVisible() )
				{
					length += p->m_preferredSize.h;
					if( p->m_preferredSize.w > breadth )
					    breadth = p->m_preferredSize.w;
				}
                p = p->_next();
            }
		}
	}

	//
	
	WgSize size = m_bHorizontal?WgSize(length,breadth):WgSize(breadth,length);
	if( size != m_preferredSize )
	{
		m_preferredSize = size;
		_requestResize();
	}
}

//____ _refreshChildGeo() _________________________________________________________

void WgPackPanel::_refreshChildGeo()
{
    if( m_hooks.isEmpty() )
        return;
    
	WgSize sz = size();
	
	int wantedLength = m_bHorizontal?m_preferredSize.w:m_preferredSize.h;
	int givenLength = m_bHorizontal?sz.w:sz.h;
	int givenBreadth = m_bHorizontal?sz.h:sz.w;

	// Optimized special case, just copy preferred to length.
	//TODO: We probably need to use matchingWidth()/matchingHeight() here anyway... prefered length might change with given breadth

	if( !m_pSizeBroker || (wantedLength == givenLength && !m_pSizeBroker->mayAlterPreferredLengths()) )
	{
		WgCoord pos;
		WgPackHook * p = _firstHook();
        WgRect geo;
		while( p )
		{
			if( p->isVisible() )
			{
				geo.x = pos.x;
				geo.y = pos.y;
				if( m_bHorizontal )
				{
					geo.w = p->m_preferredSize.w;
					geo.h = sz.h;
					pos.x += p->m_preferredSize.w;
				}
				else
				{
					geo.w = sz.w;
					geo.h = p->m_preferredSize.h;
					pos.y += p->m_preferredSize.h;
				}
				geo -= p->m_padding;
            
				if( geo != p->m_geo )
				{
					_requestRender(geo);
        
					int oldW = p->m_geo.w;
					int oldH = p->m_geo.h;
					p->m_geo = geo;
					if( geo.w != oldW || geo.h != oldH )
						p->m_pWidget->_onNewSize( geo.size() );
                
				}
			}
			else
			{
				if( p->m_geo.w != 0 && p->m_geo.h != 0 )
					_requestRender(p->m_geo);

				p->m_geo.x = pos.x;
				p->m_geo.y = pos.y;
				if( m_bHorizontal )
				{
					geo.w = 0;
					geo.h = sz.h;
				}
				else
				{
					geo.w = sz.w;
					geo.h = 0;
				}
			}

			p = p->_next();
		}
	}
	else
	{
		// Allocate and populate SizeBroker array

		int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
		WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::memStackAlloc(arrayBytes));

		int nItems = _populateSizeBrokerArray(pItemArea, givenBreadth);		
		
		// Retrieve length and set geo for all children, call _requestRender() and _onNewSize() where needed.
		
		m_pSizeBroker->setItemLengths( pItemArea, nItems, givenLength );
		
		WgPackHook * pH = _firstHook();
		WgSizeBrokerItem * pI = pItemArea;

		WgCoord pos;
		WgRect geo;
		while( pH )
		{
			if( pH->isVisible() )
			{
				geo.x = pos.x;
				geo.y = pos.y;
				if( m_bHorizontal )
				{	
					geo.w = pI->output;
					geo.h = sz.h;
					pos.x += pI->output;
				}
				else
				{	
					geo.w = sz.w;
					geo.h = pI->output;
					pos.y += pI->output;
				}
				geo -= pH->m_padding;
			
				if( geo != pH->m_geo )
				{					
					_requestRender(geo);
					_requestRender(pH->m_geo);

					int oldW = pH->m_geo.w;
					int oldH = pH->m_geo.h;
					pH->m_geo = geo;
					if( geo.w != oldW || geo.h != oldH )
						pH->m_pWidget->_onNewSize( geo.size() );
				}
				pI++;
			}
			else
			{
				if( pH->m_geo.w != 0 && pH->m_geo.h != 0 )
					_requestRender(pH->m_geo);

				pH->m_geo.x = pos.x;
				pH->m_geo.y = pos.y;
				if( m_bHorizontal )
				{
					geo.w = 0;
					geo.h = sz.h;
				}
				else
				{
					geo.w = sz.w;
					geo.h = 0;
				}
			}
			pH = pH->_next();
		}
		
		// Release SizeBroker array
		
		WgBase::memStackRelease(arrayBytes);
	}
}

//____ _populateSizeBrokerArray() ___________________________________________

int WgPackPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray ) const
{
	WgPackHook * pH = _firstHook();
	WgSizeBrokerItem * pI = pArray;
	
	if( m_bHorizontal )
	{
		while( pH )
		{
			if( pH->isVisible() )
			{
				pI->preferred = pH->m_preferredSize.w;
				pI->min = pH->_paddedMinSize().w;
				pI->max = pH->_paddedMaxSize().w;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->_next();
		}
	}
	else 
	{
		while( pH )
		{
			if( pH->isVisible() )
			{
				pI->preferred = pH->m_preferredSize.h;
				pI->min = pH->_paddedMinSize().h;
				pI->max = pH->_paddedMaxSize().h;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->_next();
		}			
	}
	
	return pI - pArray;
}

int WgPackPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray, int forcedBreadth ) const
{
	WgPackHook * pH = _firstHook();
	WgSizeBrokerItem * pI = pArray;
	
	if( m_bHorizontal )
	{
		while( pH )
		{
			if( pH->isVisible() )
			{
				pI->preferred = pH->_paddedMatchingWidth(forcedBreadth);
				pI->min = pH->_paddedMinSize().w;
				pI->max = pH->_paddedMaxSize().w;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->_next();
		}
	}
	else 
	{
		while( pH )
		{
			if( pH->isVisible() )
			{
				pI->preferred = pH->_paddedMatchingHeight(forcedBreadth);
				pI->min = pH->_paddedMinSize().h;
				pI->max = pH->_paddedMaxSize().h;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->_next();
		}			
	}
	
	return pI - pArray;
}
