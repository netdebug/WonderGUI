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
#ifndef WG_EXTENDEDSKIN_DOT_H
#define WG_EXTENDEDSKIN_DOT_H

#ifndef WG_SKIN_DOT_H
#	include <wg_skin.h>
#endif

class WgExtendedSkin;
typedef	WgSmartChildPtr<WgExtendedSkin,WgSkinPtr>	WgExtendedSkinPtr;

class WgExtendedSkin : public WgSkin
{
public:
	virtual void	SetContentPadding( WgBorders padding );
	virtual void	SetContentShift( WgStateEnum state, WgCoord shift );

	virtual void	SetHoveredContentShift( WgCoord shift );
	virtual void	SetPressedContentShift( WgCoord shift );
	virtual void	SetSelectedContentShift( WgCoord shift );
	virtual void	SetFocusedContentShift( WgCoord shift );

	virtual WgSize	MinSize( int scale ) const;
	virtual WgSize	PreferredSize( int scale ) const;
	virtual WgSize	SizeForContent( const WgSize contentSize, int scale ) const;

	virtual WgSize	ContentPadding(int scale) const;
	virtual WgCoord	ContentOfs(WgState state, int scale) const;
	virtual WgRect	ContentRect( const WgRect& canvas, WgState state, int scale ) const;

	bool	IsStateIdentical(WgState state, WgState comparedTo) const;


protected:

	inline WgSize _scaledContentPadding(int scale) const { return WgSize((m_contentPadding.Width()*scale) >> WG_SCALE_BINALS, (m_contentPadding.Height()*scale) >> WG_SCALE_BINALS); }

	WgBorders	m_contentPadding;
	WgCoord		m_contentShift[WG_NB_STATES];	
};


#endif //WG_EXTENDEDSKIN_DOT_H