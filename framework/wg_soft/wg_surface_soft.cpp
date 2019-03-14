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

#include <assert.h>
#include <memory.h>
#include <wg_surface_soft.h>
#include <wg_util.h>

#include <wg3_softsurface.h>
#include <wg_versionbridge.h>


static const char	c_surfaceType[] = {"Software"};

//____ MaxSize() _______________________________________________________________

WgSize WgSurfaceSoft::MaxSize()
{
	return _convert(wg::SoftSurface::maxSize());
}


//____ Constructor ________________________________________________________________

WgSurfaceSoft::WgSurfaceSoft( WgSize size, WgPixelType type )
{
	m_pRealSurface = wg::SoftSurface::create(_convert(size), type);
}

WgSurfaceSoft::WgSurfaceSoft( WgSize size, WgPixelType type, Uint8 * pPixels, int pitch )
{
	auto pBlob = wg::Blob::create(pPixels, nullptr);
	m_pRealSurface = wg::SoftSurface::create(_convert(size), type, pBlob, pitch);
}

WgSurfaceSoft::WgSurfaceSoft( WgSize size, WgPixelType type, Uint8 * pPixels, int pitch, const WgPixelFormat& pixelFormat )
{
	m_pRealSurface = wg::SoftSurface::create(_convert(size), type, pPixels, pitch, &pixelFormat);
}


WgSurfaceSoft::WgSurfaceSoft( const WgSurfaceSoft * pOther )
{
	m_pRealSurface = wg::SoftSurface::create(pOther->m_pRealSurface);
}

//____ Destructor ______________________________________________________________

WgSurfaceSoft::~WgSurfaceSoft()
{
}

//____ Type() __________________________________________________________________

const char * WgSurfaceSoft::Type() const
{
	return GetClass();
}

//____ GetClass() _____________________________________________________________

const char * WgSurfaceSoft::GetClass()
{
	return c_surfaceType;
}
