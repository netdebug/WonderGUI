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

#include <wg_shadercapsule.h>
#include <wg_gfxdevice.h>

static const char	c_widgetType[] = {"ShaderCapsule"};

//____ Constructor ____________________________________________________________

WgShaderCapsule::WgShaderCapsule() : m_tintColor(0xFFFFFFFF), m_tintMode(WG_TINTMODE_OPAQUE), m_blendMode(WgBlendMode::Blend)
{
}

//____ Destructor _____________________________________________________________

WgShaderCapsule::~WgShaderCapsule()
{
}

//____ Type() _________________________________________________________________

const char * WgShaderCapsule::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgShaderCapsule::GetClass()
{
	return c_widgetType;
}

//____ SetColor() ______________________________________________________________

void WgShaderCapsule::SetColor( const WgColor& color)
{
	if( color != m_tintColor )
	{
		m_tintColor = color;
		_requestRender();
	}
}

//____ SetTintMode() ___________________________________________________________

void WgShaderCapsule::SetTintMode( WgTintMode mode )
{
	if( mode != m_tintMode )
	{
		m_tintMode = mode;
		_requestRender();
	}
}

//____ SetBlendMode() __________________________________________________________

void WgShaderCapsule::SetBlendMode( WgBlendMode mode )
{
	if( mode != m_blendMode )
	{
		m_blendMode = mode;
		_requestRender();
	}
}

//____ FindWidget() ___________________________________________________________

WgWidget * WgShaderCapsule::FindWidget(const WgCoord& ofs, WgSearchMode mode)
{
	if (mode == WgSearchMode::Geometry || m_tintColor.a > 0 || m_blendMode == WgBlendMode::Replace)
		return WgCapsule::FindWidget(ofs, mode);

	return nullptr;
}

//____ _getBlendMode() _________________________________________________________

WgBlendMode WgShaderCapsule::_getBlendMode() const
{
	return m_blendMode;
}

//____ _renderPatches() ________________________________________________________

void WgShaderCapsule::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	// Set our tint color and blend mode.

	WgBlendMode		oldBM;
	WgColor			oldTC;


//	if( (_layer & m_layer) != 0 )
	{
		oldBM = pDevice->GetBlendMode();
		oldTC = pDevice->GetTintColor();

		pDevice->SetBlendMode(m_blendMode);

		if( m_tintMode == WG_TINTMODE_OPAQUE )
			pDevice->SetTintColor(m_tintColor);
		else	// MULTIPLY
			pDevice->SetTintColor(m_tintColor*oldTC);
	}

	// Render children recursively

	if( m_hook.Widget() )
		m_hook.Widget()->_renderPatches( pDevice, _canvas, _canvas, _pPatches );

	// Reset old blend mode and tint color

//	if( (_layer & m_layer) != 0 )
	{
		pDevice->SetBlendMode(oldBM);
		pDevice->SetTintColor(oldTC);
	}
}

//____ _onCollectPatches() _____________________________________________________

void WgShaderCapsule::_onCollectPatches(WgPatches& container, const WgRect& geo, const WgRect& clip)
{
	//TODO: This causes trouble if another shader capsule further down the hierarchy undo the shading.

	if (m_tintColor.a > 0 || m_blendMode == WgBlendMode::Replace)
		WgCapsule::_onCollectPatches(container, geo, clip);
}

//____ _onMaskPatches() ________________________________________________________

void WgShaderCapsule::_onMaskPatches(WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode)
{
	//TODO: This causes trouble if another shader capsule further down the hierarchy undo the shading.

	if (m_tintColor.a == 255 || m_blendMode == WgBlendMode::Replace)
		WgCapsule::_onMaskPatches(patches, geo, clip, blendMode);		
}

//____ _onCloneContent() _______________________________________________________

void WgShaderCapsule::_onCloneContent( const WgWidget * _pOrg )
{
	WgShaderCapsule * pOrg = (WgShaderCapsule*) _pOrg;

	m_tintColor		= pOrg->m_tintColor;
	m_blendMode		= pOrg->m_blendMode;
	m_tintMode		= pOrg->m_tintMode;
}
