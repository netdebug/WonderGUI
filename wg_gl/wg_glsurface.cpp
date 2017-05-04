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
	GLint maxSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	return WgSize(maxSize,maxSize);
}


//____ Constructor _____________________________________________________________


WgGlSurface::WgGlSurface( WgSize size, WgPixelType type )
{
	assert(glGetError() == 0);

	_setPixelDetails(type);

    m_size	= size;
    m_pitch = ((size.w*m_pixelFormat.bits/8)+3)&0xFFFFFFFC;
	
    glGenBuffers( 1, &m_buffer );
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
    glBufferData( GL_PIXEL_UNPACK_BUFFER, m_pitch*size.h, 0, GL_STREAM_DRAW );
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, NULL );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

	m_pAlphaBuffer = 0;


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
    WgBlobPtr pBlob = WgBlob::Create(m_pitch*m_size.h);
    
    m_pPixels = (uint8_t *) pBlob->Content();
    _copyFrom( &pixelFormat, pPixels, pitch, size, size );
    m_pPixels = 0;
    
    glGenBuffers( 1, &m_buffer );
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
    glBufferData( GL_PIXEL_UNPACK_BUFFER, m_pitch*size.h, pBlob->Content(), GL_STREAM_DRAW );
    
    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, NULL );
    
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	assert( glGetError() == 0);

	m_pAlphaBuffer = _genAlphaBuffer( m_size, (uint8_t*) pBlob->Content(), m_pitch, m_pixelFormat );
}


WgGlSurface::WgGlSurface( WgSurface * pOther )
{
	assert(glGetError() == 0);
    _setPixelDetails(pOther->PixelFormat()->type);

	m_size	= pOther->Size();
    m_pitch = m_size.w * m_pixelWgSize;
    WgBlobPtr pBlob = WgBlob::Create(m_pitch*m_size.h);
    
    m_pPixels = (uint8_t *) pBlob->Content();
    _copyFrom( pOther->PixelFormat(), (uint8_t*)pOther->Pixels(), pOther->Pitch(), m_size, m_size );
    m_pPixels = 0;
    
    glGenBuffers( 1, &m_buffer );
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
    glBufferData( GL_PIXEL_UNPACK_BUFFER, m_pitch*m_size.h, pBlob->Content(), GL_STREAM_DRAW );
    
    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, NULL );
    
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

	m_pAlphaBuffer = _genAlphaBuffer( m_size, (uint8_t*) pBlob->Content(), m_pitch, m_pixelFormat );

	assert( glGetError() == 0);
}



void WgGlSurface::_setPixelDetails( WgPixelType type )
{
    if( type == WG_PIXEL_BGR_8 )
    {
        m_internalFormat = GL_RGB8;
        m_accessFormat = GL_BGR;
        m_pixelWgSize = 3;
    }
    else
    {
        m_internalFormat = GL_RGBA8;
        m_accessFormat = GL_BGRA;
        m_pixelWgSize = 4;
    }
    
    WgUtil::PixelTypeToFormat(type, m_pixelFormat);
}

uint8_t * WgGlSurface::_genAlphaBuffer( WgSize size, uint8_t * pPixels, int pitch, const WgPixelFormat& pixelFormat )
{
	if( pixelFormat.type != WG_PIXEL_BGRA_8 )
		return 0;
		
	m_pAlphaBuffer = new uint8_t[size.w*size.h];
	
	uint8_t * p = m_pAlphaBuffer;
	
	for( int y = 0 ; y < size.h ; y++ )
	{
		for( int x = 0 ; x < size.w ; x++ )
			* p++ = pPixels[y*pitch+x*4+3];
	}	
	
	return m_pAlphaBuffer;
}


//____ Destructor ______________________________________________________________

WgGlSurface::~WgGlSurface()
{
	assert(glGetError() == 0);
    // Free the stuff

    if( m_buffer )
        glDeleteBuffers ( 1, &m_buffer );

	if( m_pAlphaBuffer )
		delete [] m_pAlphaBuffer;

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

//____ Size() ______________________________________________________________

WgSize WgGlSurface::Size() const
{
    return m_size;
}

//____ IsOpaque() ______________________________________________________________

bool WgGlSurface::IsOpaque() const
{
    if( m_buffer && m_internalFormat == GL_RGB )
        return true;

    return false;
}

//____ Lock() __________________________________________________________________

void * WgGlSurface::Lock( WgAccessMode mode )
{
    if( m_accessMode != WG_NO_ACCESS || mode == WG_NO_ACCESS )
        return 0;

	assert(glGetError() == 0);
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );

    switch( mode )
    {
        case WG_READ_ONLY:
            m_pPixels = (uint8_t*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY );
            break;
        case WG_WRITE_ONLY:
            m_pPixels = (uint8_t*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY );
            break;
        case WG_READ_WRITE:
            m_pPixels = (uint8_t*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE );
            break;
        default:
            break;	// Should never happen, just here to avoid compiler warnings...
    }

    m_lockRegion = WgRect(0,0,m_size);
    m_accessMode = mode;
	assert(glGetError() == 0);
    return m_pPixels;
}

//____ LockRegion() __________________________________________________________________

void * WgGlSurface::LockRegion( WgAccessMode mode, const WgRect& region )
{
    if( m_accessMode != WG_NO_ACCESS || mode == WG_NO_ACCESS )
        return 0;

    if( region.x + region.w > m_size.w || region.y + region.w > m_size.h || region.x < 0 || region.y < 0 )
        return 0;

	assert(glGetError() == 0);
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );

    switch( mode )
    {
        case WG_READ_ONLY:
            m_pPixels = (uint8_t*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY );
            break;
        case WG_WRITE_ONLY:
            m_pPixels = (uint8_t*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY );
            break;
        case WG_READ_WRITE:
            m_pPixels = (uint8_t*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE );
            break;
        default:
            break;	// Should never happen, just here to avoid compiler warnings...
    }

    m_lockRegion = region;
    m_accessMode = mode;
	assert(glGetError() == 0);
    return m_pPixels += (m_size.w*region.y+region.x)*m_pixelWgSize;
}


//____ Unlock() ________________________________________________________________

void WgGlSurface::Unlock()
{
    if(m_accessMode == WG_NO_ACCESS )
        return;

	assert(glGetError() == 0);
	
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
	glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );
	
    if( m_accessMode != WG_READ_ONLY )
    {
        glBindTexture( GL_TEXTURE_2D, m_texture );
		//		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_size.w, m_size.h, m_accessFormat, GL_UNSIGNED_BYTE, 0 );
		//		glTexSubImage2D( GL_TEXTURE_2D, 0, m_lockRegion.x, m_lockRegion.y, m_lockRegion.w, m_lockRegion.h, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    }
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	assert(glGetError() == 0);
	
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
		assert(glGetError() == 0);
        uint32_t val;

        if( m_accessMode == WG_NO_ACCESS )
        {
            // Quick lock of surface

            glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
            uint8_t * pPixel = (uint8_t*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY );

            //

             pPixel += (m_size.w*coord.y+coord.x)*m_pixelWgSize;
            switch( m_pixelWgSize )
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

            // Quick unlock of surface

            glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );
            glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
        }
        else
        {
            uint8_t * pPixel = m_pPixels + (m_size.w*coord.y+coord.x)*m_pixelWgSize;
            switch( m_pixelWgSize )
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
        }

		assert(glGetError() == 0);
        return val;
    }

    return 0;
}



//____ GetOpacity() ____________________________________________________________

uint8_t WgGlSurface::GetOpacity( WgCoord coord ) const
{
	if( m_pAlphaBuffer )
    {
        return m_pAlphaBuffer[m_size.w*coord.y+coord.x];            
    }
    else
        return 255;

}
