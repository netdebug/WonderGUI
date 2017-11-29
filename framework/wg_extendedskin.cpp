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

#include <wg_extendedskin.h>


//____ SetContentPadding() ____________________________________________________

void WgExtendedSkin::SetContentPadding( WgBorders padding )
{
	m_contentPadding = padding;
}


//____ SetContentShift() ______________________________________________________

void WgExtendedSkin::SetContentShift( WgStateEnum state, WgCoord shift )
{
	int index = _stateToIndex(state);
	m_contentShift[index] = shift;
}

//____ SetHoveredContentShift() _______________________________________________

void WgExtendedSkin::SetHoveredContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_HOVERED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED_SELECTED)] = shift;

	m_contentShift[_stateToIndex(WG_STATE_PRESSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ SetPressedContentShift() _______________________________________________

void WgExtendedSkin::SetPressedContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_PRESSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ SetSelectedContentShift() ______________________________________________

void WgExtendedSkin::SetSelectedContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ SetFocusedContentShift() _______________________________________________

void WgExtendedSkin::SetFocusedContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ MinSize() ______________________________________________________________

WgSize WgExtendedSkin::MinSize(int scale) const
{
	return _scaledContentPadding(scale);
}

//____ MinSize() ______________________________________________________________

WgSize WgExtendedSkin::PreferredPixelSize(int scale) const
{
	return _scaledContentPadding(scale);
}

//____ SizeForContent() _______________________________________________________

WgSize WgExtendedSkin::SizeForContent( const WgSize contentSize, int scale ) const
{
	return contentSize + _scaledContentPadding(scale);
}

//____ ContentPadding() _______________________________________________________

WgSize WgExtendedSkin::ContentPadding(int scale) const
{
	// Note: The borders need to be scaled individually before they are added together.

	return WgSize((m_contentPadding.left * scale >> WG_SCALE_BINALS) +
			(m_contentPadding.right * scale >> WG_SCALE_BINALS),
			(m_contentPadding.top * scale >> WG_SCALE_BINALS) +
			(m_contentPadding.bottom * scale >> WG_SCALE_BINALS) );
}

//____ ContentOfs() ___________________________________________________________

WgCoord	WgExtendedSkin::ContentOfs(WgState state, int scale) const
{
	WgCoord shift = m_contentShift[_stateToIndex(state)];
	shift.x = shift.x * scale >> WG_SCALE_BINALS;
	shift.y = shift.y * scale >> WG_SCALE_BINALS;

	return WgCoord((m_contentPadding.left * scale >> WG_SCALE_BINALS), (m_contentPadding.top * scale >> WG_SCALE_BINALS)) + shift;
}


//____ ContentRect() __________________________________________________________

WgRect WgExtendedSkin::ContentRect( const WgRect& canvas, WgState state, int scale ) const
{
	WgCoord shift = m_contentShift[_stateToIndex(state)];
	shift.x = shift.x * scale >> WG_SCALE_BINALS;
	shift.y = shift.y * scale >> WG_SCALE_BINALS;

	WgBorders padding(m_contentPadding.left *scale >> WG_SCALE_BINALS, m_contentPadding.top *scale >> WG_SCALE_BINALS, m_contentPadding.right *scale >> WG_SCALE_BINALS, m_contentPadding.bottom *scale >> WG_SCALE_BINALS);
	return (canvas - padding) + shift;
}
