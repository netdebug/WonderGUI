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
#ifndef WG_GLGFXDEVICE_DOT_H
#define WG_GLGFXDEVICE_DOT_H

#ifndef WG_GEO_DOT_H
#	include <wg_geo.h>
#endif

#ifdef WIN32
#	include <gl/glew.h>
#	include <gl/gl.h>
#else
#	ifdef __APPLE__
#		include <OpenGL/gl3.h>
#	else
#		include <GLES3/gl3.h>
#	endif
#endif

#ifndef	WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif


class WgGlGfxDevice : public WgGfxDevice
{
public:
    WgGlGfxDevice( WgSize canvas );
    ~WgGlGfxDevice();

	void    SetViewportOffset( WgCoord ofs );
    bool    SetCanvas( WgSize dimensions );
    using   WgGfxDevice::SetCanvas;
    
    void    StretchBlitSubPixelWithInvert(const WgSurface * pSrc, float sx, float sy, float sw, float sh,
                                          float dx, float dy, float dw, float dh);
};

#endif //WG_GLGFXDEVICE_DOT_H

