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

#include <wg_surfacefactory_soft.h>
#include <wg_surface_soft.h>




//____ WgSurfaceFactorySoft::MaxSize() ___________________________________

WgSize WgSurfaceFactorySoft::MaxSize() const
{
	return WgSurfaceSoft::MaxSize();
}


//____ WgSurfaceFactorySoft::CreateSurface() ___________________________________

WgSurface * WgSurfaceFactorySoft::CreateSurface( const WgSize& size, WgPixelType type ) const
{
	return new WgSurfaceSoft( size, type );
}

WgSurface * WgSurfaceFactorySoft::CreateSurface( const WgSize& size, WgPixelType type, uint8_t * pPixels, int pitch, const WgPixelFormat& pixelFormat ) const
{
    return 	new WgSurfaceSoft(size,type,pPixels, pitch, pixelFormat);	
}
