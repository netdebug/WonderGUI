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

#include <wg_orderedselectable.h>


bool WgOrdSelHook::SetSelected( bool bSelected )
{
	switch( Parent()->m_selectMode )
	{
		case WgSelectMode::Unselectable:
		{
			if( bSelected )
				return false;

			break;
		}
		case WgSelectMode::SingleEntry:
		{
			if( bSelected && !m_bSelectable )
				return false;

			if( bSelected != m_bSelected )
			{
				if( bSelected )
				{
					if( Parent()->m_nbSelected > 0 )
					{
						WgOrdSelHook * p = Parent()->_firstSelectedHook();
						if( p )
						{
							p->m_bSelected = false;
							Parent()->_onWidgetUnselected(this);
						}
					}

					m_bSelected = true;
					Parent()->_onWidgetSelected(this);
				}
				else
				{
					m_bSelected = false;
					Parent()->_onWidgetUnselected(this);
				}
			}
			break;
		}
		case WgSelectMode::MultiEntries:
		{
			if( bSelected && !m_bSelectable )
				return false;

			if( bSelected != m_bSelected )
			{
				m_bSelected = bSelected;
				if( bSelected )
					Parent()->_onWidgetSelected(this);
				else
					Parent()->_onWidgetUnselected(this);
			}
			break;
		}
	}

	return true;
}


void WgOrdSelHook::SetSelectable( bool bSelectable )
{
	if( !bSelectable && m_bSelected )
	{
		m_bSelected = false;
		Parent()->_onWidgetUnselected(this);
	}

	m_bSelectable = bSelectable;
}


WgOrdSelHook::WgOrdSelHook() : m_bSelectable(true), m_bSelected(false)
{
}


WgOrdSelHook::~WgOrdSelHook()
{
}


WgOrdSelHook *	WgOrdSelHook::_prevSelectedHook() const
{
	WgOrdSelHook * p = _prev();
	while( p && !p->m_bSelected )
	{
		p = p->_prev();
	}
	return p;
}


WgOrdSelHook *	WgOrdSelHook::_nextSelectedHook() const
{
	WgOrdSelHook * p = _next();
	while( p && !p->m_bSelected )
	{
		p = p->_next();
	}
	return p;
}

//____ Constructor ____________________________________________________________

WgOrdSelLayout::WgOrdSelLayout()
{
	m_bScrollOnSelect = false;
	m_selectMode = WgSelectMode::SingleEntry;
	m_nbSelected = 0;

}

//____ Destructor _____________________________________________________________

WgOrdSelLayout::~WgOrdSelLayout()
{
}

//____ SelectAll() ____________________________________________________________

int WgOrdSelLayout::SelectAll()
{
	switch( m_selectMode )
	{
		case WgSelectMode::Unselectable:
			break;

		case WgSelectMode::SingleEntry:
		{
			WgOrdSelHook * p = FirstHook();
			while( p && m_nbSelected < 1 )
			{
				if( p->m_bSelectable )
				{
					p->m_bSelected = true;
					_onWidgetSelected( p );
				}
				p = p->_next();
			}
			break;
		}

		case WgSelectMode::MultiEntries:
		{
			WgOrdSelHook * p = FirstHook();
			while( p )
			{
				if( p->m_bSelectable )
					p->m_bSelected = true;

				p = p->_next();
			}
			_refreshAllWidgets();
			break;
		}
	}
	return m_nbSelected;
}

//___ UnselectAll() ___________________________________________________________

void WgOrdSelLayout::UnselectAll()
{
	if( m_nbSelected > 0 )
	{
		WgOrdSelHook * p = FirstHook();
		while( p )
		{
			p->m_bSelected = false;
			p = p->_next();
		}

		_refreshAllWidgets();
	}
}

//____ SetSelectMode() ________________________________________________________

void WgOrdSelLayout::SetSelectMode( WgSelectMode mode )
{
	if( mode == m_selectMode )
		return;

	UnselectAll();
	m_selectMode = mode;
}

//____ SetBgBlocks() _____________________________________________________

void WgOrdSelLayout::SetBgBlocks( const WgBlocksetPtr& pOddBg, const WgBlocksetPtr& pEvenBg )
{
	m_pOddBgBlocks = pOddBg;
	m_pEvenBgBlocks = pEvenBg;
	_refreshAllWidgets();
}

//____ SetFgBlocks() _____________________________________________________

void WgOrdSelLayout::SetFgBlocks( const WgBlocksetPtr& pFg )
{
	m_pFgBlocks = pFg;
	_refreshAllWidgets();
}

//____ SetBgColors() _____________________________________________________

void WgOrdSelLayout::SetBgColors( const WgColorsetPtr& pOddBg, const WgColorsetPtr& pEvenBg )
{
	m_pOddBgColors = pOddBg;
	m_pEvenBgColors = pEvenBg;
}

//____ _onCloneContent() ______________________________________________________

void WgOrdSelLayout::_onCloneContent( const WgWidget * _pOrg )
{
}


//____ _firstSelectedHook() ___________________________________________________

WgOrdSelHook * WgOrdSelLayout::_firstSelectedHook() const
{
	WgOrdSelHook * p = FirstHook();
	while( p && !p->m_bSelected )
	{
		p = p->_next();
	}
	return p;
}

//____ _lastSelectedHook() ____________________________________________________

WgOrdSelHook * WgOrdSelLayout::_lastSelectedHook() const
{
	WgOrdSelHook * p = LastHook();
	while( p && !p->m_bSelected )
	{
		p = p->_prev();
	}
	return p;
}
