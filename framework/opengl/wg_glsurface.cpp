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

#include <memory.h>

#include <wg_glsurface.h>
#include <wg_util.h>
#include <assert.h>

#ifdef __APPLE___
#	include <OpenGL/glu.h>
#endif

#ifdef WIN32
#	include <gl/glu.h>
#endif


static const char	c_surfaceType[] = {"OpenGL"};


//____ MaxSize() _______________________________________________________________

WgSize WgGlSurface::MaxSize()
{
    GLenum err;
    assert( 0 == ( err = glGetError()));
    
    GLint maxSize = 1000;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    
    assert( maxSize > 1000 );
    assert( 0 == ( err = glGetError()));
    
    return WgSize(maxSize,maxSize);
}


//____ Constructor _____________________________________________________________


WgGlSurface::WgGlSurface( WgSize size, WgPixelType type )
{
	assert(glGetError() == 0);

	_setPixelDetails(type);


    m_size	= size;
    m_pitch = ((size.w*m_pixelFormat.bits/8)+3)&0xFFFFFFFC;
    m_pBlob = WgBlob::Create(m_pitch*m_size.h);
	
    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, NULL );



	GLenum err = glGetError();
	
//	const GLubyte * pErr = gluErrorString(err);

	assert( err == 0);
}


WgGlSurface::WgGlSurface( WgSize size, WgPixelType type, uint8_t * pPixels, int pitch,  const WgPixelFormat& pixelFormat )
{
	assert(glGetError() == 0);
   _setPixelDetails(type);

	m_size	= size;
    m_pitch = ((size.w*m_pixelFormat.bits/8)+3)&0xFFFFFFFC;
    m_pBlob = WgBlob::Create(m_pitch*m_size.h);
    
    m_pPixels = (uint8_t *) m_pBlob->Content();
    _copyFrom( &pixelFormat, pPixels, pitch, size, size );
    m_pPixels = 0;
    
    
    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->Content() );
    
 	assert( glGetError() == 0);
}


WgGlSurface::WgGlSurface( WgSurface * pOther )
{
	assert(glGetError() == 0);
    _setPixelDetails(pOther->PixelFormat()->type);

	m_size	= pOther->PixelSize();
    m_pitch = m_size.w * m_pixelSize;
    m_pBlob = WgBlob::Create(m_pitch*m_size.h);
    
    m_pPixels = (uint8_t *) m_pBlob->Content();
    _copyFrom( pOther->PixelFormat(), (uint8_t*)pOther->Pixels(), pOther->Pitch(), m_size, m_size );
    m_pPixels = 0;
    
    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->Content() );
    
	assert( glGetError() == 0);
}



void WgGlSurface::_setPixelDetails( WgPixelType type )
{
    if( type == WG_PIXEL_BGR_8 )
    {
        m_internalFormat = GL_RGB8;
        m_accessFormat = GL_BGR;
        m_pixelSize = 3;
    }
    else
    {
        m_internalFormat = GL_RGBA8;
        m_accessFormat = GL_BGRA;
        m_pixelSize = 4;
    }
    
    WgUtil::PixelTypeToFormat(type, m_pixelFormat);
}


//____ Destructor ______________________________________________________________

WgGlSurface::~WgGlSurface()
{
	assert(glGetError() == 0);
    // Free the stuff

    glDeleteTextures( 1, &m_texture );
	assert(glGetError() == 0);
}

//____ Type() __________________________________________________________________

const char * WgGlSurface::Type() const
{
    return GetClass();
}

//____ GetClass() _____________________________________________________________

const char * WgGlSurface::GetClass()
{
    return c_surfaceType;
}


//____ setScaleMode() __________________________________________________________

void WgGlSurface::setScaleMode( WgScaleMode mode )
{
	switch( mode )
	{
		case WG_SCALEMODE_INTERPOLATE:
			glBindTexture( GL_TEXTURE_2D, m_texture );
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			break;
			
		case WG_SCALEMODE_NEAREST:
		default:
			glBindTexture( GL_TEXTURE_2D, m_texture );
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			break;
	}
	
	WgSurface::setScaleMode(mode);
}

//____ IsOpaque() ______________________________________________________________

bool WgGlSurface::IsOpaque() const
{
    if( m_internalFormat == GL_RGB )
        return true;

    return false;
}

//____ Lock() __________________________________________________________________

void * WgGlSurface::Lock( WgAccessMode mode )
{
    if( m_accessMode != WG_NO_ACCESS || mode == WG_NO_ACCESS )
        return 0;

    m_pPixels = (uint8_t*) m_pBlob->Content();
    m_lockRegion = WgRect(0,0,m_size);
    m_accessMode = mode;
    return m_pPixels;
}

//____ LockRegion() __________________________________________________________________

void * WgGlSurface::LockRegion( WgAccessMode mode, const WgRect& region )
{
    if( m_accessMode != WG_NO_ACCESS || mode == WG_NO_ACCESS )
        return 0;

    if( region.x + region.w > m_size.w || region.y + region.w > m_size.h || region.x < 0 || region.y < 0 )
        return 0;

    m_pPixels = (uint8_t*) m_pBlob->Content();
    m_lockRegion = region;
    m_accessMode = mode;
    return m_pPixels += (m_size.w*region.y+region.x)*m_pixelSize;
}


//____ Unlock() ________________________________________________________________

void WgGlSurface::Unlock()
{
    if(m_accessMode == WG_NO_ACCESS )
        return;

	assert(glGetError() == 0);
	
    if( m_accessMode != WG_READ_ONLY )
    {
        glBindTexture( GL_TEXTURE_2D, m_texture );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_size.w, m_size.h, m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->Content() );
		//		glTexSubImage2D( GL_TEXTURE_2D, 0, m_lockRegion.x, m_lockRegion.y, m_lockRegion.w, m_lockRegion.h, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
		assert(glGetError() == 0);
    }
	
	m_accessMode = WG_NO_ACCESS;
    m_pPixels = 0;
    m_lockRegion.w = 0;
    m_lockRegion.h = 0;
}


//____ GetPixel() ______________________________________________________________

uint32_t WgGlSurface::GetPixel( WgCoord coord ) const
{
    if( m_accessMode != WG_WRITE_ONLY )
    {
        uint32_t val;

		uint8_t * pPixel = (uint8_t*) m_pBlob->Content();

		switch( m_pixelSize )
		{
			case 1:
				val = (uint32_t) *pPixel;
			case 2:
				val = (uint32_t) ((uint16_t*) pPixel)[0];
			case 3:
			   val = ((uint32_t) pPixel[0]) + (((uint32_t) pPixel[1]) << 8) + (((uint32_t) pPixel[2]) << 16);
			default:
				val = *((uint32_t*) pPixel);
		}

        return val;
    }

    return 0;
}



//____ GetOpacity() ____________________________________________________________

uint8_t WgGlSurface::GetOpacity( WgCoord coord ) const
{
	if( m_pixelFormat.type == WG_PIXEL_BGRA_8 )
    {
		uint8_t * p = (uint8_t*) m_pBlob->Content();
        return p[coord.y*m_pitch+coord.x*4+3];            
    }
    else
        return 255;

}


bool WgGlSurface::unload()
{
	if( m_texture == 0 )
		return true;
	
    glDeleteTextures( 1, &m_texture );
	m_texture = 0;
	
	assert(glGetError() == 0);	
    return true;
}

bool WgGlSurface::isLoaded()
{
	return (m_texture == 0);
}

void WgGlSurface::reload()
{
	assert(glGetError() == 0);
 
    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->Content() );
    

	assert( glGetError() == 0);	
}
