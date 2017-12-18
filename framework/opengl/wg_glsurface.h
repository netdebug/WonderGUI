/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI Graphics Toolkit is free Glware; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Glware Foundation; either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_GLSURFACE_DOT_H
#define	WG_GLSURFACE_DOT_H

#define GL_GLEXT_PROTOTYPES 1

#ifdef WIN32
#	include <gl/glew.h>
#	include <gl/gl.h>
#else
#	ifdef __APPLE__
#		include <OpenGL/gl3.h>
#	else
#		include <GL/gl.h>
#	endif
#endif


#ifndef WG_SURFACE_DOT_H
#	include <wg_surface.h>
#endif

#ifndef WG_BLOB_DOT_H
#   include <wg_blob.h>
#endif


//____ WgGlSurface _____________________________________________________________

class WgGlSurface : public WgSurface
{
    friend class WgGlSurfaceFactory;

public:
    WgGlSurface( WgSize size, WgPixelType type = WG_PIXEL_BGRA_8 );
    WgGlSurface( WgSize size, WgPixelType type, uint8_t * pPixels, int pitch, const WgPixelFormat& pixelFormat );
    WgGlSurface( WgSurface * pOther );
    ~WgGlSurface();

    const char *Type() const;
    static const char * GetClass();
    
    
    inline	GLuint	GetTexture() const { return m_texture; }

	void		setScaleMode( WgScaleMode mode );


    // Methods needed by WgSurface

    bool		IsOpaque() const;

    uint32_t	GetPixel( WgCoord coord ) const;
    uint8_t		GetOpacity( WgCoord coord ) const;

    void *		Lock( WgAccessMode mode );
    void *		LockRegion( WgAccessMode mode, const WgRect& region );
    void		Unlock();

	static WgSize	MaxSize();

	bool		unload();
	bool		isLoaded();
	void		reload();


private:

    void		_setPixelDetails( WgPixelType type );



    GLuint 		m_texture;			// GL texture handle.
    GLint       m_internalFormat;   // GL_RGB8 or GL_RGBA8.
    GLenum		m_accessFormat;		// GL_BGR or GL_BGRA.
    WgBlobPtr   m_pBlob;
    
    uint32_t	m_pixelSize;		// Size in bytes of a pixel.
};

#endif //WG_GLSURFACE_DOT_H

