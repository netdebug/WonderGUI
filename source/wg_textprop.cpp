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

#include <wg_textprop.h>
#include <wg_util.h>
#include <wg_color.h>
#include <wg_char.h>
#include <wg_font.h>

#include <memory.h>

using namespace WgUtil;

//____ Constructor ____________________________________________________________

WgTextprop::WgTextprop()
{
	ClearAll();
}

WgTextprop::WgTextprop( const WgTextpropPtr& pProp )
{
	m_pLink				= pProp->m_pLink;
	m_pFont				= pProp->m_pFont;
	m_visibilityFlags	= pProp->m_visibilityFlags;
	m_breakLevel		= pProp->m_breakLevel;

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_stateProp[i].m_bColored	= pProp->m_stateProp[i].m_bColored;
		m_stateProp[i].m_bBgColor	= pProp->m_stateProp[i].m_bBgColor;
		m_stateProp[i].m_bUnderlined = pProp->m_stateProp[i].m_bUnderlined;
		m_stateProp[i].m_color		= pProp->m_stateProp[i].m_color;
		m_stateProp[i].m_bgColor		= pProp->m_stateProp[i].m_bgColor;
		m_stateProp[i].m_style		= pProp->m_stateProp[i].m_style;
		m_stateProp[i].m_size		= pProp->m_stateProp[i].m_size;
	}
}


//____ Register() _____________________________________________________________

WgTextpropPtr WgTextprop::Register() const
{
	return	WgTextpropPtr(WgTextpropManager::RegisterProp(*this));
}

//____ ClearAll() _____________________________________________________________

void WgTextprop::ClearAll()
{
	m_visibilityFlags = 0;
	m_pFont = 0;
	m_pLink = 0;
	m_breakLevel = -1;

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_stateProp[i].m_bColored	= false;
		m_stateProp[i].m_color.argb = 0xFFFFFFFF;

		m_stateProp[i].m_bBgColor	= false;
		m_stateProp[i].m_bgColor.argb = 0xFFFFFFFF;

		m_stateProp[i].m_style = WG_STYLE_NORMAL;
		m_stateProp[i].m_size = 0;
		m_stateProp[i].m_bUnderlined = false;
	}
}



//____ SetColor() _____________________________________________________________

void WgTextprop::SetColor( WgColor col )
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_stateProp[i].m_bColored	= true;
		m_stateProp[i].m_color		= col;
	}
}

void WgTextprop::SetColor( WgColor col, WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_bColored	= true;
	m_stateProp[i].m_color		= col;
}


//____ SetBgColor() ___________________________________________________________

void WgTextprop::SetBgColor( WgColor col )
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_stateProp[i].m_bBgColor	= true;
		m_stateProp[i].m_bgColor		= col;
	}
}

void WgTextprop::SetBgColor( WgColor col, WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_bBgColor	= true;
	m_stateProp[i].m_bgColor	= col;
}



//____ SetStyle() _____________________________________________________________

void WgTextprop::SetStyle( WgFontStyle style )
{
	assert( style<WG_NB_FONTSTYLES );

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
		m_stateProp[i].m_style = style;
}

void WgTextprop::SetStyle( WgFontStyle style, WgState state )
{
	assert( style<WG_NB_FONTSTYLES );

	int i = _stateToIndex(state);
	m_stateProp[i].m_style = style;
}

//____ SetSize() ______________________________________________________________

void WgTextprop::SetSize( int size )
{
	assert( size >= 0 && size < 2048 );

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
		m_stateProp[i].m_size = size;
}

void WgTextprop::SetSize( int size, WgState state )
{
	assert( size >= 0 && size < 2048 );

	int i = _stateToIndex(state);
	m_stateProp[i].m_size = size;
}


//____ SetUnderlined() ________________________________________________________

void WgTextprop::SetUnderlined()
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
		m_stateProp[i].m_bUnderlined	= true;
}

void WgTextprop::SetUnderlined( WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_bUnderlined = true;
}

//____ ClearColor() _____________________________________________________________

void WgTextprop::ClearColor()
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_stateProp[i].m_bColored	= false;
		m_stateProp[i].m_color.argb = 0xFFFFFFFF;
	}
}

void WgTextprop::ClearColor( WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_bColored	= false;
	m_stateProp[i].m_color.argb = 0xFFFFFFFF;
}

//____ ClearBgColor() _________________________________________________________

void WgTextprop::ClearBgColor()
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_stateProp[i].m_bBgColor	= false;
		m_stateProp[i].m_bgColor.argb = 0xFFFFFFFF;
	}
}

void WgTextprop::ClearBgColor( WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_bBgColor	= false;
	m_stateProp[i].m_bgColor.argb = 0xFFFFFFFF;
}


//____ ClearStyle() _____________________________________________________________

void WgTextprop::ClearStyle()
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
		m_stateProp[i].m_style = WG_STYLE_NORMAL;
}

void WgTextprop::ClearStyle( WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_style = WG_STYLE_NORMAL;
}


//____ ClearSize() ______________________________________________________________

void WgTextprop::ClearSize()
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
		m_stateProp[i].m_size = 0;
}

void WgTextprop::ClearSize( WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_size = 0;
}


//____ ClearUnderlined() ________________________________________________________

void WgTextprop::ClearUnderlined()
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
		m_stateProp[i].m_bUnderlined = false;
}

void WgTextprop::ClearUnderlined( WgState state )
{
	int i = _stateToIndex(state);
	m_stateProp[i].m_bUnderlined = false;
}


//____ IsColorStatic() _________________________________________________________

bool WgTextprop::IsColorStatic() const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[0].m_bColored != m_stateProp[i].m_bColored || m_stateProp[0].m_color != m_stateProp[i].m_color )
			return false;

	return true;
}

//____ IsBgColorStatic() _______________________________________________________

bool WgTextprop::IsBgColorStatic() const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[0].m_bBgColor != m_stateProp[i].m_bBgColor || m_stateProp[0].m_bgColor != m_stateProp[i].m_bgColor )
			return false;

	return true;
}


//____ IsStyleStatic() _________________________________________________________

bool WgTextprop::IsStyleStatic() const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[0].m_style != m_stateProp[i].m_style )
			return false;

	return true;
}

//____ IsSizeStatic() __________________________________________________________

bool WgTextprop::IsSizeStatic() const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[0].m_size != m_stateProp[i].m_size )
			return false;

	return true;
}


//____ IsUnderlineStatic() _____________________________________________________

bool WgTextprop::IsUnderlineStatic() const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[0].m_bUnderlined != m_stateProp[i].m_bUnderlined )
			return false;

	return true;
}

//____ CompareColorTo() ________________________________________________________

bool WgTextprop::CompareColorTo( const WgTextpropPtr& pProp ) const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[i].m_bColored != pProp->m_stateProp[i].m_bColored || m_stateProp[i].m_color != pProp->m_stateProp[i].m_color )
			return false;

	return true;
}

//____ CompareBgColorTo() ______________________________________________________

bool WgTextprop::CompareBgColorTo( const WgTextpropPtr& pProp ) const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[i].m_bBgColor != pProp->m_stateProp[i].m_bBgColor || m_stateProp[i].m_bgColor != pProp->m_stateProp[i].m_bgColor )
			return false;

	return true;
}


//____ CompareStyleTo() ________________________________________________________

bool WgTextprop::CompareStyleTo( const WgTextpropPtr& pProp ) const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[i].m_style != pProp->m_stateProp[i].m_style )
			return false;

	return true;
}

//____ CompareSizeTo() _________________________________________________________

bool WgTextprop::CompareSizeTo( const WgTextpropPtr& pProp ) const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[i].m_size != pProp->m_stateProp[i].m_size )
			return false;

	return true;
}


//____ CompareUnderlineTo() ____________________________________________________
bool WgTextprop::CompareUnderlineTo( const WgTextpropPtr& pProp ) const
{
	for( int i = 1 ; i < WG_NB_STATES ; i++ )
		if( m_stateProp[i].m_bUnderlined != pProp->m_stateProp[i].m_bUnderlined )
			return false;

	return true;
}


//____ _calculateChecksum() ____________________________________________________

Uint8 WgTextprop::_calculateChecksum() const
{
	WgUtil::Checksum8	chk;

	chk.Add8( m_visibilityFlags );
	chk.Add8( m_breakLevel );
	chk.Add( &m_pLink, sizeof(WgTextLinkPtr) );
	chk.Add( &m_pFont, sizeof(WgFontPtr) );

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		chk.Add8( (Uint8) m_stateProp[i].m_bColored );
		chk.Add8( (Uint8) m_stateProp[i].m_bBgColor );
		chk.Add8( (Uint8) m_stateProp[i].m_bUnderlined );
		chk.Add32( m_stateProp[i].m_color.argb );
		chk.Add32( m_stateProp[i].m_bgColor.argb );
		chk.Add16( m_stateProp[i].m_style );
		chk.Add16( m_stateProp[i].m_size );
	}

	return chk.GetChecksum();
}

//____ _compareTo() ____________________________________________________________

bool WgTextprop::_compareTo( const WgTextprop * pProp ) const
{
	if( m_pFont != pProp->m_pFont || m_pLink != pProp->m_pLink ||
		m_visibilityFlags != pProp->m_visibilityFlags ||
		m_breakLevel != pProp->m_breakLevel )
		return false;

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		if( m_stateProp[i].m_style != pProp->m_stateProp[i].m_style ||
			m_stateProp[i].m_size != pProp->m_stateProp[i].m_size ||
			m_stateProp[i].m_bColored != pProp->m_stateProp[i].m_bColored ||
			m_stateProp[i].m_bBgColor != pProp->m_stateProp[i].m_bBgColor ||
			m_stateProp[i].m_bUnderlined != pProp->m_stateProp[i].m_bUnderlined ||
			m_stateProp[i].m_color.argb != pProp->m_stateProp[i].m_color.argb ||
			m_stateProp[i].m_bgColor.argb != pProp->m_stateProp[i].m_bgColor.argb )
			return false;
	}

	return true;
}

//____ SetCharVisibility() ____________________________________________________

bool WgTextprop::SetCharVisibility( Uint16 specialCharacter, bool bVisible )
{
	switch( specialCharacter )
	{
		case '\n':
			m_bShowLF = bVisible;
			break;
		case '\t':
			m_bShowTAB = bVisible;
			break;
		case WG_BREAK_PERMITTED:
			m_bShowBreakPermitted = bVisible;
			break;
		case WG_HYPHEN_BREAK_PERMITTED:
			m_bShowHyphenBreakPermitted = bVisible;
			break;
		case WG_NO_BREAK_SPACE:
			m_bShowNoBreakSpace = bVisible;
			break;

		default:
			return false;
	};

	return true;
}

//____ CharVisibility() ________________________________________________________

bool WgTextprop::CharVisibility( Uint16 specialCharacter ) const
{
	switch( specialCharacter )
	{
		case ' ':
			return m_bShowSpace;
		case '\n':
			return m_bShowLF;
		case '\t':
			return m_bShowTAB;
		case WG_BREAK_PERMITTED:
			return m_bShowBreakPermitted;
		case WG_HYPHEN_BREAK_PERMITTED:
			return m_bShowHyphenBreakPermitted;
		case WG_NO_BREAK_SPACE:
			return m_bShowNoBreakSpace;
		default:
			return true;		// All other characters are always visible
	};
}

//=============================================================================
//							>>> WgTextpropPtr <<<
//=============================================================================

//____ Constructor ____________________________________________________________

WgTextpropPtr::WgTextpropPtr( WgTextpropHolder * pProp )
{
	if( pProp )
		m_hProp = pProp->m_id;
	else
		m_hProp = 0;

	WgTextpropManager::IncRef(m_hProp, 1);
}

WgTextpropPtr::WgTextpropPtr( Uint16 hProp )
{
	m_hProp = hProp;
	WgTextpropManager::IncRef(m_hProp);
}


WgTextpropPtr::WgTextpropPtr(const WgTextpropPtr& r)
{
	m_hProp = r.m_hProp;
	WgTextpropManager::IncRef(m_hProp);
}


//____ Destructor _____________________________________________________________

WgTextpropPtr::~WgTextpropPtr()
{
	WgTextpropManager::DecRef(m_hProp, 1);
}


//____ operator= ______________________________________________________________

WgTextpropPtr& WgTextpropPtr::operator=(const WgTextpropPtr& ref)
{
	if(m_hProp != ref.m_hProp)
	{
		WgTextpropManager::DecRef(m_hProp);
		m_hProp = ref.m_hProp;
		WgTextpropManager::IncRef(m_hProp);
	}
	return *this;
}

WgTextpropPtr& WgTextpropPtr::operator=(int handle)
{
	if(m_hProp != handle)
	{
		WgTextpropManager::DecRef(m_hProp);
		m_hProp = handle;
		WgTextpropManager::IncRef(m_hProp);
	}
	return *this;
}

//____ operator* ______________________________________________________________

const WgTextprop & WgTextpropPtr::operator*() const
{
	return WgTextpropManager::GetProp(m_hProp);
}

//____ operator-> _____________________________________________________________

const WgTextprop * WgTextpropPtr::operator->() const
{
	return WgTextpropManager::GetPropPtr(m_hProp);
}

//_____________________________________________________________________________
void WgTextprop::AssertIntegrity() const
{
	for( int i = 0; i<WG_NB_STATES; i++ )
		assert( m_stateProp[i].m_style<WG_NB_FONTSTYLES );
}

//_____________________________________________________________________________

bool WgTextprop::IsEqual(WgState state0, WgState state1) const
{
	return	m_stateProp[state0].m_bColored == m_stateProp[state1].m_bColored &&
			m_stateProp[state0].m_bBgColor == m_stateProp[state1].m_bBgColor &&
			m_stateProp[state0].m_bUnderlined == m_stateProp[state1].m_bUnderlined &&
			m_stateProp[state0].m_color == m_stateProp[state1].m_color &&
			m_stateProp[state0].m_bgColor == m_stateProp[state1].m_bgColor &&
			m_stateProp[state0].m_style == m_stateProp[state1].m_style &&
			m_stateProp[state0].m_size == m_stateProp[state1].m_size;
}
