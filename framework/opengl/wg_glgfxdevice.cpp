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

#include <wg_glgfxdevice.h>
#include <wg_glsurface.h>
#include <wg_glsurfacefactory.h>
#include <wg_base.h>

#ifdef __APPLE___
#	include <OpenGL/glu.h>
#endif

#ifdef WIN32
#	include <windows.h>
#	include <gl/glu.h>
#endif

#include <wg3_glgfxdevice.h>
#include <wg_versionbridge.h>

using namespace std;


//____ Constructor _____________________________________________________________

WgGlGfxDevice::WgGlGfxDevice( WgSize canvas )
{
	m_pRealDevice = wg::GlGfxDevice::create(_convert(canvas));

}

//____ Destructor ______________________________________________________________

WgGlGfxDevice::~WgGlGfxDevice()
{
}

//____ SetViewport() ________________________________________________________________

void WgGlGfxDevice::SetViewportOffset(WgCoord ofs)
{
	bool ret = ((wg::GlGfxDevice*)m_pRealDevice.rawPtr())->setCanvas(wg::Rect(_convert(ofs), m_pRealDevice->canvasSize()));
    if (ret)
        m_pCanvas = nullptr;
}

//____ SetCanvas() __________________________________________________________________

bool WgGlGfxDevice::SetCanvas( WgSize dimensions )
{
	bool ret = ((wg::GlGfxDevice*)m_pRealDevice.rawPtr())->setCanvas(_convert(dimensions));
    if(ret)
        m_pCanvas = nullptr;
}

//____ StretchBlitSubPixelWithInvert() _______________________________________________

void WgGlGfxDevice::StretchBlitSubPixelWithInvert(const WgSurface * pSrc, float sx, float sy, float sw, float sh,
                                      float dx, float dy, float dw, float dh)
{
    ((wg::GlGfxDevice*)m_pRealDevice.rawPtr())->stretchBlitSubPixelWithInvert( pSrc->m_pRealSurface, sx, sy, sw, sh, dx, dy, dw, dh );
}
