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



#include <wg_image.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>


static const char	c_widgetType[] = {"Image"};

//____ Constructor ____________________________________________________________

WgImage::WgImage()
{
}

//____ Destructor _____________________________________________________________

WgImage::~WgImage()
{
}

//____ Type() _________________________________________________________________

const char * WgImage::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgImage::GetClass()
{
	return c_widgetType;
}

//____ SetSkin() ______________________________________________________________

void WgImage::SetSkin(const WgSkinPtr& pSkin)
{
	if (pSkin != m_pSkin)
	{
		m_pSkin = pSkin;
		_requestResize();
		_requestRender();
	}
}


//____ SetSource() _____________________________________________________________

void WgImage::SetSource( const WgBlocksetPtr& pBlockset )
{
	if( m_pGfx != pBlockset )
	{
		m_pGfx = pBlockset;

		if( m_pGfx && m_pGfx->IsOpaque() )
			m_bOpaque = true;
		else
			m_bOpaque = false;

        _requestResize();
        _requestRender();
	}
}

//____ PreferredPixelSize() _____________________________________________________________

WgSize WgImage::PreferredPixelSize() const
{
	if( m_pGfx )
		return m_pGfx->Size(m_scale);

	return WgSize(1,1);
}



//____ _onCloneContent() _______________________________________________________

void WgImage::_onCloneContent( const WgWidget * _pOrg )
{
	WgImage * pOrg = (WgImage*) _pOrg;

	m_pGfx = pOrg->m_pGfx;
}

//____ _onRender() _____________________________________________________________

void WgImage::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	WgWidget::_onRender(pDevice, _canvas, _window, _clip);

	WgRect canvas = m_pSkin ? m_pSkin->ContentRect(_canvas, WG_STATE_NORMAL, m_scale) : _canvas;

	if( !m_pGfx )
		return;

	WgBlock	block;
	if( m_bEnabled )
		block = m_pGfx->GetBlock(WG_MODE_NORMAL, m_scale);
	else
		block = m_pGfx->GetBlock(WG_MODE_DISABLED, m_scale);

	pDevice->ClipBlitBlock( _clip, block, canvas);
}

//____ _onAlphaTest() ___________________________________________________________

bool WgImage::_onAlphaTest( const WgCoord& ofs )
{
	if( !m_pGfx )
		return	false;												// No visible pixel, so don't accept the mark...

	WgMode mode = WG_MODE_NORMAL;
	if( !m_bEnabled )
		mode = WG_MODE_DISABLED;

	WgSize sz = PixelSize();

	return WgUtil::MarkTestBlock( ofs, m_pGfx->GetBlock(mode,m_scale), WgRect(0,0,sz.w,sz.h), m_markOpacity );
}

//____ _onEnable() _____________________________________________________________

void WgImage::_onEnable()
{
	if( m_pGfx )
	{
		if( !m_pGfx->SameBlock(WG_MODE_NORMAL, WG_MODE_DISABLED) )
			_requestRender();
	}
}

//____ _onDisable() ____________________________________________________________

void WgImage::_onDisable()
{
	if( m_pGfx )
	{
		if( !m_pGfx->SameBlock(WG_MODE_NORMAL, WG_MODE_DISABLED) )
			_requestRender();
	}
}

//____ _setScale() ____________________________________________________________

void WgImage::_setScale( int scale )
{
    WgWidget::_setScale(scale);

    if( m_pGfx )
        _requestResize();
}



