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

#include <wg_canvascapsule.h>
#include <wg_gfxdevice.h>

static const char	c_widgetType[] = {"CanvasCapsule"};

//____ Constructor ____________________________________________________________

WgCanvasCapsule::WgCanvasCapsule() : m_tintColor(0xFFFFFFFF), m_tintMode(WG_TINTMODE_OPAQUE), m_blendMode(WG_BLENDMODE_BLEND), m_pFactory(nullptr), m_pCanvas(nullptr)
{
}

//____ Destructor _____________________________________________________________

WgCanvasCapsule::~WgCanvasCapsule()
{
	if (m_pCanvas)
		delete m_pCanvas;
}

//____ Type() _________________________________________________________________

const char * WgCanvasCapsule::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgCanvasCapsule::GetClass()
{
	return c_widgetType;
}

//____ SetSurfaceFactory() ____________________________________________________

void WgCanvasCapsule::SetSurfaceFactory(WgSurfaceFactory * pFactory)
{
	if (pFactory == m_pFactory)
		return;

	if (m_pCanvas)
	{
		delete m_pCanvas;
		m_pCanvas = nullptr;
		_requestRender();
	}

	m_pFactory = pFactory;
}


//____ SetColor() ______________________________________________________________

void WgCanvasCapsule::SetColor( const WgColor& color)
{
	if( color != m_tintColor )
	{
		m_tintColor = color;
		_requestRender();
	}
}

//____ SetTintMode() ___________________________________________________________

void WgCanvasCapsule::SetTintMode( WgTintMode mode )
{
	if( mode != m_tintMode )
	{
		m_tintMode = mode;
		_requestRender();
	}
}

//____ SetBlendMode() __________________________________________________________

void WgCanvasCapsule::SetBlendMode( WgBlendMode mode )
{
	if( mode != m_blendMode )
	{
		m_blendMode = mode;
		_requestRender();
	}
}


//____ _getBlendMode() _________________________________________________________

WgBlendMode WgCanvasCapsule::_getBlendMode() const
{
	return m_blendMode;
}

//____ _renderPatches() ________________________________________________________

void WgCanvasCapsule::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	// Make sure we have children

	if (!m_hook.Widget())
		return;

	// Make sure we have a canvas

	if (!m_pCanvas)
	{
		if (!m_pFactory)
			return;							// No SurfaceFactory set!

		WgSize maxSize = m_pFactory->MaxSize();
		if (_canvas.w > maxSize.w || _canvas.h > maxSize.h)
			return;							// Can't create a canvas of the required size!

		m_pCanvas = m_pFactory->CreateSurface(_canvas.Size(), WG_PIXEL_BGRA_8);
		m_dirtyPatches.Clear();
		m_dirtyPatches.Add(_canvas.Size());
	}

	// Go through dirty patches from screen canvas and update our back canvas where they overlap with our own

	WgPatches renderStack;

	for (const WgRect * pScreenRect = _pPatches->Begin(); pScreenRect != _pPatches->End(); pScreenRect++)
	{
		WgRect r(0, 0, pScreenRect->w, pScreenRect->h);

		bool	bIntersected = false;

		for (const WgRect * pLocalDirt = m_dirtyPatches.Begin(); pLocalDirt != m_dirtyPatches.End(); pLocalDirt++)
		{
			if (pLocalDirt->IntersectsWith(r))
			{
				renderStack.Push(WgRect(*pLocalDirt,r));
				bIntersected = true;
			}
		}
 
		if (bIntersected)
			m_dirtyPatches.Sub(r);
	}

	// Save old tint color and blend mode.

	WgBlendMode		oldBM;
	WgColor			oldTC;

	oldBM = pDevice->GetBlendMode();
	oldTC = pDevice->GetTintColor();


	if (!renderStack.IsEmpty())
	{
		pDevice->SetBlendMode(WG_BLENDMODE_BLEND);
		pDevice->SetTintColor(WgColor::white);

		WgSurface * pOldCanvas = pDevice->Canvas();
		pDevice->SetCanvas(m_pCanvas);
		m_hook.Widget()->_renderPatches(pDevice, _canvas.Size(), _canvas.Size(), &renderStack);
		pDevice->SetCanvas(pOldCanvas);

	}

	// Set our tint color and blend mode for blitting from back canvas to screen.

	pDevice->SetBlendMode(m_blendMode);

	if( m_tintMode == WG_TINTMODE_OPAQUE )
		pDevice->SetTintColor(m_tintColor);
	else	// MULTIPLY
		pDevice->SetTintColor(m_tintColor*oldTC);

	// Render patches

	WgWidget::_renderPatches(pDevice, _canvas, _canvas, _pPatches);

	// Reset old blend mode and tint color

	pDevice->SetBlendMode(oldBM);
	pDevice->SetTintColor(oldTC);
}

//____ _onRender() ____________________________________________________________

void WgCanvasCapsule::_onRender(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip)
{
	// Make sure to render any skin we might have

	WgWidget::_onRender(pDevice, _canvas, _window, _clip);

	// Copy from our back canvas to the screen canvas

	pDevice->ClipBlitFromCanvas(_clip, m_pCanvas, { 0,0,_canvas.w,_canvas.h }, _canvas.x, _canvas.y);
}


//____ _onCloneContent() _______________________________________________________

void WgCanvasCapsule::_onCloneContent( const WgWidget * _pOrg )
{
	WgCanvasCapsule * pOrg = (WgCanvasCapsule*) _pOrg;

	m_tintColor		= pOrg->m_tintColor;
	m_blendMode		= pOrg->m_blendMode;
	m_tintMode		= pOrg->m_tintMode;
}

//____ _onNewSize() ___________________________________________________________

void WgCanvasCapsule::_onNewSize(const WgSize& size)
{
	if (m_pCanvas && size != m_pCanvas->PixelSize() )
	{
		delete m_pCanvas;
		m_pCanvas = nullptr;
		_requestRender();
	}

	WgCapsule::_onNewSize(size);
}

//____ _onRenderRequested() ___________________________________________________

void WgCanvasCapsule::_onRenderRequested()
{
	m_dirtyPatches.Clear();
	m_dirtyPatches.Add(PixelSize());
	_requestRender();
}

void WgCanvasCapsule::_onRenderRequested(const WgRect& rect)
{
	m_dirtyPatches.Add(rect);
	_requestRender();
}
