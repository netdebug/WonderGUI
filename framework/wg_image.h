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
#ifndef WG_IMAGE_DOT_H
#define WG_IMAGE_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef WG_BLOCKSET_DOT_H
#	include <wg_blockset.h>
#endif


//____ WgImage __________________________________________________________

class WgImage : public WgWidget
{
public:
	WgImage();
	virtual ~WgImage();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgImage(); };
	
	void			SetSkin(const WgSkinPtr& pSkin);

	void			SetSource( const WgBlocksetPtr& pBlockset );
	WgBlocksetPtr	GetSource() const { return m_pGfx; }

	WgSize			PreferredPixelSize() const;

	int				MatchingPixelHeight(int pixelWidth) const;
	int				MatchingPixelWidth(int pixelHeight) const;


protected:

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onEnable();
	void	_onDisable();

    void    _setScale( int scale );

private:

	WgBlocksetPtr	m_pGfx;
};


#endif //WG_FILLER_DOT_H
