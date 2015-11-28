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
#include <assert.h>

#include <wg_surface_gl.h>
#include <wg_util.h>

#ifdef WIN32
#	include <glext.h>
#else
#	include <GL/glext.h>
#	include <GL/glx.h>
#endif
//#include <GL/glu.h>


static const char	c_surfaceType[] = {"OpenGL"};

//____ Constructor _____________________________________________________________

WgSurfaceGL::WgSurfaceGL( WgSize dimensions, WgPixelType pixelType, void * _pPixels )
{
	assert( dimensions.w != 0 && dimensions.h != 0 );
	
	switch( pixelType )
	{
		case WG_PIXEL_BGR_8:
			m_format = GL_BGR;
			m_pixelSize = 3;
		break;
		
		case WG_PIXEL_BGRA_8:
			m_format = GL_BGRA;
			m_pixelSize = 4;
		break;
		
		default:
			assert(pixelType != WG_PIXEL_BGR_8 && pixelType != WG_PIXEL_BGRA_8 );
	}

	WgUtil::PixelTypeToFormat(pixelType, m_pixelFormat);


	m_size	 = dimensions;
	m_pPixels = 0;
	m_buffer = 0;
	m_pitch = dimensions.w*m_pixelFormat.bits/8;

	int size = dimensions.w*dimensions.h*m_pixelSize;
	glGenBuffers( 1, &m_buffer );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
	glBufferData( GL_PIXEL_UNPACK_BUFFER, size, _pPixels, GL_STREAM_DRAW );


	GLenum internalFormat = m_pixelSize == 4 ? GL_RGBA : GL_RGB;

	glGenTextures( 1, &m_texture );
	glBindTexture( GL_TEXTURE_2D, m_texture );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, m_size.w, m_size.h, 0,
		m_format, GL_UNSIGNED_BYTE, NULL );

	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

}

//____ Destructor ______________________________________________________________

WgSurfaceGL::~WgSurfaceGL()
{
	// Free the stuff

	glDeleteBuffers( 1, &m_buffer );
	glDeleteTextures( 1, &m_texture );
}

//____ Type() __________________________________________________________________

const char * WgSurfaceGL::Type() const
{
	return GetClass();
}

//____ GetClass() _____________________________________________________________

const char * WgSurfaceGL::GetClass()
{
	return c_surfaceType;
}


//____ Size() ______________________________________________________________

WgSize WgSurfaceGL::Size() const
{
	return m_size;
}

//____ IsOpaque() ______________________________________________________________

bool WgSurfaceGL::IsOpaque() const
{
	if( m_buffer && m_format == GL_BGRA )
		return false;

	return true;
}

//____ Lock() __________________________________________________________________

void * WgSurfaceGL::Lock( WgAccessMode mode )
{
	if( m_format == 0 || m_accessMode != WG_NO_ACCESS || mode == WG_NO_ACCESS )
		return 0;

	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );

	switch( mode )
	{
		case WG_READ_ONLY:
			m_pPixels = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY );
			break;
		case WG_WRITE_ONLY:
			m_pPixels = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY );
			break;
		case WG_READ_WRITE:
			m_pPixels = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE );
			break;
		default:
			break;	// Should never happen, just here to avoid compiler warnings...
	}

	m_lockRegion = WgRect(0,0,m_size);
	m_accessMode = mode;
	return m_pPixels;
}

//____ LockRegion() __________________________________________________________________

void * WgSurfaceGL::LockRegion( WgAccessMode mode, const WgRect& region )
{
	if( m_format == 0 || m_accessMode != WG_NO_ACCESS || mode == WG_NO_ACCESS )
		return 0;

	if( region.x + region.w > m_size.w || region.y + region.w > m_size.h || region.x < 0 || region.y < 0 )
		return 0;

	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );

	switch( mode )
	{
		case WG_READ_ONLY:
			m_pPixels = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY );
			break;
		case WG_WRITE_ONLY:
			m_pPixels = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY );
			break;
		case WG_READ_WRITE:
			m_pPixels = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE );
			break;
		default:
			break;	// Should never happen, just here to avoid compiler warnings...
	}

	m_lockRegion = region;
	m_accessMode = mode;
	return m_pPixels += (m_size.w*region.y+region.x)*m_pixelSize;
}


//____ Unlock() ________________________________________________________________

void WgSurfaceGL::Unlock()
{
	if(m_accessMode == WG_NO_ACCESS )
		return;

	glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );

	if( m_accessMode != WG_READ_ONLY )
	{
		glBindTexture( GL_TEXTURE_2D, m_texture );
//		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
//		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_size.w, m_size.h, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		glTexSubImage2D( GL_TEXTURE_2D, 0, m_lockRegion.x, m_lockRegion.y, m_lockRegion.w, m_lockRegion.h, GL_BGRA, GL_UNSIGNED_BYTE, 0 );
	}
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	m_accessMode = WG_NO_ACCESS;
	m_pPixels = 0;
	m_lockRegion.w = 0;
	m_lockRegion.h = 0;
}


//____ GetPixel() ______________________________________________________________

Uint32 WgSurfaceGL::GetPixel( WgCoord coord ) const
{
	if( m_accessMode == WG_WRITE_ONLY )
		return 0;

	Uint32 val;

	if( m_accessMode == WG_NO_ACCESS )
	{
		// Quick lock of surface

		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
		Uint8 * pPixel = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY );

		//

		 pPixel += (m_size.w*coord.y+coord.x)*m_pixelSize;
		switch( m_pixelSize )
		{
			case 1:
				val = (Uint32) *pPixel;
			case 2:
				val = (Uint32) ((Uint16*) pPixel)[0];
			case 3:
				val = ((Uint32) pPixel[0]) + (((Uint32) pPixel[1]) << 8) + (((Uint32) pPixel[2]) << 16);
			default:
				val = *((Uint32*) pPixel);
		}

		// Quick unlock of surface

		glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );
		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	}
	else
	{
		Uint8 * pPixel = m_pPixels + (m_size.w*coord.y+coord.x)*m_pixelSize;
		switch( m_pixelSize )
		{
			case 1:
				val = (Uint32) *pPixel;
			case 2:
				val = (Uint32) ((Uint16*) pPixel)[0];
			case 3:
			   val = ((Uint32) pPixel[0]) + (((Uint32) pPixel[1]) << 8) + (((Uint32) pPixel[2]) << 16);
			default:
				val = *((Uint32*) pPixel);
		}
	}

	return val;
}



//____ GetOpacity() ____________________________________________________________

Uint8 WgSurfaceGL::GetOpacity( WgCoord coord ) const
{

	if( m_format == GL_BGRA && m_accessMode != WG_WRITE_ONLY )
	{
		Uint8 a;

		if( m_accessMode == WG_NO_ACCESS )
		{
			// Quick lock of surface

			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_buffer );
			Uint8 * pPixel = (Uint8*) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY );

			//

			a = pPixel[(m_size.w*coord.y+coord.x)*m_pixelSize+3];

			// Quick unlock of surface

			glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

		}
		else
		{
			a = m_pPixels[(m_size.w*coord.y+coord.x)*m_pixelSize+3];
		}

		return a;
	}


	return 255;
}


//____ WgSurfaceFactoryGL::CreateSurface() ___________________________________

WgSurface * WgSurfaceFactoryGL::CreateSurface( const WgSize& size, WgPixelType type ) const
{
	return new WgSurfaceGL( size, type, 0 );
}
