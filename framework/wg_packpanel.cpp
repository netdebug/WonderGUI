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

static const char	c_widgetType[] = {"PackPanel"};
static const char	c_hookType[] = {"PackHook"};


//____ WgPackHook::Constructor ________________________________________________

WgPackHook::WgPackHook( WgPackPanel * pParent )
{
	m_pParent = pParent;
	m_weight = 1.f;
}

//____ WgPackHook::Type() _____________________________________________________

const char * WgPackHook::Type( void ) const
{
	return ClassType();
}

//____ WgPackHook::ClassType() ________________________________________________

const char * WgPackHook::ClassType()
{
	return c_hookType;
}

//____ WgPackHook::Parent() ___________________________________________________

WgPackPanel * WgPackHook::Parent() const 
{ 
	return m_pParent; 
}

//____ WgPackHook::SetWeight() ________________________________________________

bool WgPackHook::SetWeight( float weight )
{
	if( weight < 0 )
		return false;

	if( weight != m_weight )
	{
		m_weight = weight;
		Parent()->_refreshChildGeo();
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
	m_bHorizontal = true;
    m_pSizeBroker = 0;
}

//____ Destructor _____________________________________________________________

WgPackPanel::~WgPackPanel()
{
}

//____ Type() _________________________________________________________________

const char * WgPackPanel::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgPackPanel::GetClass()
{
	return c_widgetType;
}

//____ SetOrientation() ______________________________________________________

void WgPackPanel::SetOrientation( WgOrientation orientation )
{
	bool bHorizontal = orientation==WG_HORIZONTAL?true:false;
	if( m_bHorizontal != bHorizontal )
	{
		m_bHorizontal = bHorizontal;
		_updatePreferredPixelSize();
		_refreshChildGeo();
	}
}


//____ SetSizeBroker() _______________________________________________________

void WgPackPanel::SetSizeBroker( WgSizeBroker * pBroker )
{
	if( m_pSizeBroker != pBroker )
	{
		m_pSizeBroker = pBroker;
		_updatePreferredPixelSize();
		_refreshChildGeo();
	}
}

//____ PreferredPixelSize() _______________________________________________________

WgSize WgPackPanel::PreferredPixelSize() const
{
	return m_preferredSize;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgPackPanel::MatchingPixelHeight( int width ) const
{
	int height = 0;

	if( m_bHorizontal )
	{
		if( m_pSizeBroker )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.Size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::MemStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea);		
		
			// Retrieve item lengths and find height of highest item.

			m_pSizeBroker->SetItemLengths( pItemArea, nItems, width );

			WgPackHook * pH = FirstHook();
			WgSizeBrokerItem * pI = pItemArea;

			while( pH )
			{
				if( pH->IsVisible() )
				{
					int itemHeight = pH->_paddedMatchingPixelHeight( pI->output, m_scale );
					if( itemHeight > height )
						height = itemHeight;
					pI++;
				}

				pH = pH->Next();
			}

			// Release temporary memory area
		
			WgBase::MemStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * pH = FirstHook();

			while( pH )
			{
				if( pH->IsVisible() && pH->m_preferredSize.h > height )
						height = pH->m_preferredSize.h;

				pH = pH->Next();
			}
		}
	}
	else
	{
		if( m_pSizeBroker && m_pSizeBroker->MayAlterPreferredLengths() )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.Size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::MemStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea, width);		
		
			// Retrieve preferred length

			height = m_pSizeBroker->SetPreferredLengths( pItemArea, nItems );
				
			// Release temporary memory area
		
			WgBase::MemStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * p = FirstHook();

			while( p )
			{
				if( p->IsVisible() )
					height += p->_paddedMatchingPixelHeight( width, m_scale );

				p = p->Next();
			}
		}
	}
	return height;
}

//____ MatchingPixelWidth() _______________________________________________________

int WgPackPanel::MatchingPixelWidth( int height ) const
{
	int width = 0;

	if( !m_bHorizontal )
	{
		if( m_pSizeBroker )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.Size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::MemStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea);		
		
			// Retrieve item lengths and find height of highest item.

			m_pSizeBroker->SetItemLengths( pItemArea, nItems, height );

			WgPackHook * pH = FirstHook();
			WgSizeBrokerItem * pI = pItemArea;

			while( pH )
			{
				if( pH->IsVisible() )
				{
					int itemWidth = pH->_paddedMatchingPixelWidth( pI->output, m_scale );
					if( itemWidth > width )
						width = itemWidth;
					pI++;
				}

				pH = pH->Next();
			}

			// Release temporary memory area
		
			WgBase::MemStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * pH = FirstHook();

			while( pH )
			{
				if( pH->IsVisible() && pH->m_preferredSize.w > width )
						width = pH->m_preferredSize.w;

				pH = pH->Next();
			}
		}
	}
	else
	{
		if( m_pSizeBroker && m_pSizeBroker->MayAlterPreferredLengths() )
		{
			// Allocate and populate SizeBroker array
		
			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.Size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::MemStackAlloc(arrayBytes));
		
			int nItems = _populateSizeBrokerArray(pItemArea, height);		
		
			// Retrieve preferred length

			width = m_pSizeBroker->SetPreferredLengths( pItemArea, nItems );
				
			// Release temporary memory area
		
			WgBase::MemStackRelease(arrayBytes);
		}
		else 
		{
			WgPackHook * p = FirstHook();

			while( p )
			{
				if( p->IsVisible() )
					width += p->_paddedMatchingPixelWidth( height, m_scale );

				p = p->Next();
			}
		}
	}
	return width;
}



//____ _firstHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_firstHookWithGeo( WgRect& geo ) const
{	
	WgPackHook * p = FirstHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _nextHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook)->Next();
	if( p )
		geo = p->m_geo;
	return p;	
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_lastHookWithGeo( WgRect& geo ) const
{
	WgPackHook * p = LastHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _prevHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook)->Prev();
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
	p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);

	//
	
	_refreshAllWidgets();
}

//____ _onWidgetAppeared() ______________________________________________________

void WgPackPanel::_onWidgetAppeared( WgVectorHook * pInserted )
{
	// Update cached preferred size of child
	
	WgPackHook * p = static_cast<WgPackHook*>(pInserted);
	p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);
	
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
	_updatePreferredPixelSize();
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


//____ _updatePreferredPixelSize() ______________________________________________________

void WgPackPanel::_updatePreferredPixelSize()
{
	int length = 0;
	int breadth = 0;

	if( m_pSizeBroker && m_pSizeBroker->MayAlterPreferredLengths() )
	{
		// Allocate and populate SizeBroker array
		
		int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.Size();
		WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::MemStackAlloc(arrayBytes));
		
		int nItems = _populateSizeBrokerArray(pItemArea);		
		
		// Retrieve preferred length and breadth
		
		length = m_pSizeBroker->SetPreferredLengths( pItemArea, nItems );
		
		WgPackHook * pH = FirstHook();
		WgSizeBrokerItem * pI = pItemArea;
		while( pH )
		{
			if( pH->IsVisible() )
			{
				int b = m_bHorizontal?pH->_paddedMatchingPixelHeight(pI->output, m_scale):pH->_paddedMatchingPixelWidth(pI->output, m_scale);
				if( b > breadth )
					breadth = b;			
				pI++;
			}
			pH = pH->Next();
		}
		
		// Release temporary memory area
		
		WgBase::MemStackRelease(arrayBytes);
		
	}
	else
	{
		WgPackHook * p = FirstHook();

		if( m_bHorizontal )
		{
            while( p )
            {
				if( p->IsVisible() )
				{
					length += p->m_preferredSize.w;
	                if( p->m_preferredSize.h > breadth )
	                    breadth = p->m_preferredSize.h;
				}
				p = p->Next();
            }
		}
		else
		{
            while( p )
            {
				if( p->IsVisible() )
				{
					length += p->m_preferredSize.h;
					if( p->m_preferredSize.w > breadth )
					    breadth = p->m_preferredSize.w;
				}
                p = p->Next();
            }
		}
	}

	//
	
	WgSize size = m_bHorizontal?WgSize(length,breadth):WgSize(breadth,length);
// This optimization caused trouble for wrap text. MatchingHeight might get different even if PreferredSize remains the same.
//	if( size != m_preferredSize )
//	{
		m_preferredSize = size;
		_requestResize();
//	}
}

//____ _refreshChildGeo() _________________________________________________________

void WgPackPanel::_refreshChildGeo()
{
    if( m_hooks.IsEmpty() )
        return;
    
	WgSize size = PixelSize();
	
	int wantedLength = m_bHorizontal?m_preferredSize.w:m_preferredSize.h;
	int givenLength = m_bHorizontal?size.w:size.h;
	int givenBreadth = m_bHorizontal?size.h:size.w;

	// Optimized special case, just copy preferred to length.
	//TODO: We probably need to use MatchingPixelWidth()/MatchingPixelHeight() here anyway... prefered length might change with given breadth

	if( !m_pSizeBroker || (wantedLength == givenLength && !m_pSizeBroker->MayAlterPreferredLengths()) )
	{
		WgCoord pos;
		WgPackHook * p = FirstHook();
        WgRect geo;
		while( p )
		{
			if( p->IsVisible() )
			{
				geo.x = pos.x;
				geo.y = pos.y;
				if( m_bHorizontal )
				{
					geo.w = p->m_preferredSize.w;
					geo.h = size.h;
					pos.x += p->m_preferredSize.w;
				}
				else
				{
					geo.w = size.w;
					geo.h = p->m_preferredSize.h;
					pos.y += p->m_preferredSize.h;
				}
				geo -= p->m_padding.scale(m_scale);
            
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
					geo.h = size.h;
				}
				else
				{
					geo.w = size.w;
					geo.h = 0;
				}
			}

			p = p->Next();
		}
	}
	else
	{
		// Allocate and populate SizeBroker array

		int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.Size();
		WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(WgBase::MemStackAlloc(arrayBytes));

		int nItems = _populateSizeBrokerArray(pItemArea, givenBreadth);		
		
		// Retrieve length and set geo for all children, call _requestRender() and _onNewSize() where needed.
		
		m_pSizeBroker->SetItemLengths( pItemArea, nItems, givenLength );
		
		WgPackHook * pH = FirstHook();
		WgSizeBrokerItem * pI = pItemArea;

		WgCoord pos;
		WgRect geo;
		while( pH )
		{
			if( pH->IsVisible() )
			{
				geo.x = pos.x;
				geo.y = pos.y;
				if( m_bHorizontal )
				{	
					geo.w = pI->output;
					geo.h = size.h;
					pos.x += pI->output;
				}
				else
				{	
					geo.w = size.w;
					geo.h = pI->output;
					pos.y += pI->output;
				}
				geo -= pH->m_padding.scale(m_scale);
			
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
					geo.h = size.h;
				}
				else
				{
					geo.w = size.w;
					geo.h = 0;
				}
			}
			pH = pH->Next();
		}
		
		// Release SizeBroker array
		
		WgBase::MemStackRelease(arrayBytes);
	}
}

//____ _populateSizeBrokerArray() ___________________________________________

int WgPackPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray ) const
{
	WgPackHook * pH = FirstHook();
	WgSizeBrokerItem * pI = pArray;
	
	if( m_bHorizontal )
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->m_preferredSize.w;
				pI->min = pH->_paddedMinPixelSize(m_scale).w;
				pI->max = pH->_paddedMaxPixelSize(m_scale).w;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->Next();
		}
	}
	else 
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->m_preferredSize.h;
				pI->min = pH->_paddedMinPixelSize(m_scale).h;
				pI->max = pH->_paddedMaxPixelSize(m_scale).h;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->Next();
		}			
	}
	
	return pI - pArray;
}

int WgPackPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray, int forcedBreadth ) const
{
	WgPackHook * pH = FirstHook();
	WgSizeBrokerItem * pI = pArray;
	
	if( m_bHorizontal )
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->_paddedMatchingPixelWidth(forcedBreadth,m_scale);
				pI->min = pH->_paddedMinPixelSize(m_scale).w;
				pI->max = pH->_paddedMaxPixelSize(m_scale).w;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->Next();
		}
	}
	else 
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->_paddedMatchingPixelHeight(forcedBreadth, m_scale);
				pI->min = pH->_paddedMinPixelSize(m_scale).h;
				pI->max = pH->_paddedMaxPixelSize(m_scale).h;
				pI->weight = pH->m_weight;			
				pI++;
			}
			pH = pH->Next();
		}			
	}
	
	return pI - pArray;
}
