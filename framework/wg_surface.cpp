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
#include <wg_surface.h>

using namespace std;

//____ WgSurface() ____________________________________________________________

WgSurface::WgSurface()
{
	m_accessMode	= WG_NO_ACCESS;
	m_pPixels		= 0;
	m_pMetaData		= 0;
	m_nMetaBytes	= 0;
    m_scaleMode		= WG_SCALEMODE_NEAREST; //WG_SCALEMODE_INTERPOLATE;		// For Softube...
    m_scaleFactor   = WG_SCALE_BASE;
    m_iResource      = -1;                           // handy for debugging

	memset( &m_pixelFormat, 0, sizeof(WgPixelFormat) );
}

//____ ~WgSurface() ____________________________________________________________

WgSurface::~WgSurface()
{
	if( m_pMetaData )
		delete [] (uint8_t*)m_pMetaData;
}

//____ Width() ________________________________________________________________

int WgSurface::Width() const
{
	return Size().w;
}

//____ Height() _______________________________________________________________

int WgSurface::Height() const
{
	return Size().h;
}


//____ Col2Pixel() ____________________________________________________________

Uint32 WgSurface::Col2Pixel( const WgColor& col ) const
{
	Uint32 pix = ((col.r << m_pixelFormat.R_shift) & m_pixelFormat.R_mask) |
				 ((col.g << m_pixelFormat.G_shift) & m_pixelFormat.G_mask) |
				 ((col.b << m_pixelFormat.B_shift) & m_pixelFormat.B_mask) |
				 ((col.a << m_pixelFormat.A_shift) & m_pixelFormat.A_mask);

	return pix;
}

//____ Pixel2Col() ____________________________________________________________

WgColor WgSurface::Pixel2Col( Uint32 pixel ) const
{
	WgColor col( (pixel & m_pixelFormat.R_mask) >> m_pixelFormat.R_shift,
				 (pixel & m_pixelFormat.G_mask) >> m_pixelFormat.G_shift,
				 (pixel & m_pixelFormat.B_mask) >> m_pixelFormat.B_shift,
				 (pixel & m_pixelFormat.A_mask) >> m_pixelFormat.A_shift );

	return col;
}

//____ _lockAndAdjustRegion() __________________________________________________

WgRect WgSurface::_lockAndAdjustRegion( WgAccessMode modeNeeded, const WgRect& region )
{
	if( m_accessMode == WG_NO_ACCESS )
	{
		Lock( modeNeeded );
		return region;
	}
	else if( m_accessMode != WG_READ_WRITE && m_accessMode != modeNeeded )
		return WgRect(0,0,0,0);

	if( !m_lockRegion.Contains( region ) )
		return WgRect(0,0,0,0);

	return region - m_lockRegion.Pos();
}

//____ Fill() _________________________________________________________________

bool WgSurface::Fill( WgColor col )
{
	return Fill( col, WgRect(0,0,Size()) );
}

bool WgSurface::Fill( WgColor col, const WgRect& _rect )
{

	WgAccessMode oldMode = m_accessMode;
	WgRect rect = _lockAndAdjustRegion(WG_WRITE_ONLY,_rect);

	if( rect.w == 0 )
		return false;

	//


	Uint32 pixel = Col2Pixel( col );
	int width = rect.w;
	int height = rect.h;
	int pitch = Pitch();
	Uint8 * pDest = m_pPixels + rect.y * pitch + rect.x*m_pixelFormat.bits/8;

	bool ret = true;
	switch( m_pixelFormat.bits )
	{
		case 8:
			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
					pDest[x] = (Uint8) pixel;
				pDest += pitch;
			}
			break;
		case 16:
			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
					((Uint16*)pDest)[x] = (Uint16) pixel;
				pDest += pitch;
			}
			break;
		case 24:
		{
			Uint8 one = (Uint8) pixel;
			Uint8 two = (Uint8) (pixel>>8);
			Uint8 three = (Uint8) (pixel>>16);

			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
				{
					pDest[x++] = one;
					pDest[x++] = two;
					pDest[x++] = three;
				}
				pDest += pitch - width*3;
			}
			break;
		}
		case 32:
			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
					((Uint32*)pDest)[x] = pixel;
				pDest += pitch;
			}
			break;
		default:
			ret = false;
	}

	//

	if( oldMode == WG_NO_ACCESS )
		Unlock();

	return ret;
}

//_____ SetMetaData() _________________________________________________________

void WgSurface::SetMetaData( void * pData, int bytes )
{
	if( m_pMetaData )
		delete [] (uint8_t*)m_pMetaData;

	if( pData == 0 )
	{
		m_pMetaData = 0;
		m_nMetaBytes = 0;
	}
	else
	{
		m_pMetaData = new uint8_t[bytes];
		memcpy(m_pMetaData, pData, bytes);
		m_nMetaBytes = bytes;
	}
}

//_____ ClearMetaData() _________________________________________________________

void WgSurface::ClearMetaData()
{
	if( m_pMetaData )
		delete [] (uint8_t*)m_pMetaData;

	m_pMetaData = 0;
	m_nMetaBytes = 0;
}

//____ setScaleMode() __________________________________________________________

void WgSurface::setScaleMode( WgScaleMode mode )
{
	m_scaleMode = mode;
}


//_____ CopyFrom() _____________________________________________________________

bool WgSurface::CopyFrom( WgSurface * pSrcSurface, WgCoord dst )
{
	if( pSrcSurface == 0 )
		return false;

	return CopyFrom( pSrcSurface, WgRect(0,0,pSrcSurface->Size()), dst );
}

bool WgSurface::CopyFrom( WgSurface * pSrcSurface, const WgRect& _srcRect, WgCoord _dst )
{
    if( !pSrcSurface || pSrcSurface->m_pixelFormat.type == WG_PIXEL_UNKNOWN || m_pixelFormat.type == WG_PIXEL_UNKNOWN )
        return false;
    
    // Save old locks and lock the way we want.
    
    WgAccessMode 	dstOldMode 		= m_accessMode;
    WgAccessMode 	srcOldMode 		= pSrcSurface->GetLockStatus();
    
    WgRect srcRect = pSrcSurface->_lockAndAdjustRegion( WG_READ_ONLY, _srcRect );
    WgRect dstRect = _lockAndAdjustRegion( WG_WRITE_ONLY, WgRect(_dst.x,_dst.y,srcRect.w,srcRect.h) );
    
    // Do the copying
    
    bool retVal = _copyFrom( pSrcSurface->PixelFormat(), (uint8_t*) pSrcSurface->Pixels(), pSrcSurface->Pitch(), srcRect, dstRect );
    
    // Release any temporary locks
    
    if( dstOldMode == WG_NO_ACCESS )
        Unlock();
    
    if( srcOldMode == WG_NO_ACCESS )
        pSrcSurface->Unlock();
    
    return retVal;
}

//____ _copyFrom() _________________________________________________________

bool WgSurface::_copyFrom( const WgPixelFormat * pSrcFormat, uint8_t * pSrcPixels, int srcPitch, const WgRect& srcRect, const WgRect& dstRect )
{
    
    if( srcRect.w <= 0 || dstRect.w <= 0 )
        return false;
    
    const WgPixelFormat * pDstFormat 	= &m_pixelFormat;
    int		dstPitch 				= m_pitch;
    
    const unsigned char *	pSrc = (const unsigned char*) pSrcPixels;
    unsigned char *	pDst = (unsigned char*) m_pPixels;
    
    pSrc += srcRect.y * srcPitch + srcRect.x * pSrcFormat->bits/8;
    pDst += dstRect.y * dstPitch + dstRect.x * pDstFormat->bits/8;
    
    
    if( pSrcFormat->bits == pDstFormat->bits && pSrcFormat->R_mask == pDstFormat->R_mask &&
       pSrcFormat->G_mask == pDstFormat->G_mask && pSrcFormat->B_mask == pDstFormat->B_mask &&
       pSrcFormat->A_mask == pDstFormat->A_mask)
    {
        // We have identical formats so we can do a fast straight copy
        
        int lineLength = srcRect.w * pSrcFormat->bits/8;
        for( int y = 0 ; y < srcRect.h ; y++ )
        {
            memcpy( pDst, pSrc, lineLength );
            pSrc += srcPitch;
            pDst += dstPitch;
        }
    }
    else if( (pSrcFormat->type == WG_PIXEL_BGRA_8 || pSrcFormat->type == WG_PIXEL_BGR_8) &&
            (pDstFormat->type == WG_PIXEL_BGRA_8 || pDstFormat->type == WG_PIXEL_BGR_8) )
    {
        // We are just switching between RGBA_8 and RGB_8, just copy RGB components and skip alpha
        
        int		srcInc = pSrcFormat->bits/8;
        int		dstInc = pDstFormat->bits/8;
        
        int		srcLineInc = srcPitch - srcInc * srcRect.w;
        int		dstLineInc = dstPitch - dstInc * srcRect.w;
        
        for( int y = 0 ; y < srcRect.h ; y++ )
        {
            for( int x = 0 ; x < srcRect.w ; x++ )
            {
                pDst[0] = pSrc[0];
                pDst[1] = pSrc[1];
                pDst[2] = pSrc[2];
                pSrc += srcInc;
                pDst += dstInc;
            }
            pSrc += srcLineInc;
            pDst += dstLineInc;
        }
        
    }
    else
    {
        // We need to fully convert pixels when copying
        
        int		srcInc = pSrcFormat->bits/8;
        int		dstInc = pDstFormat->bits/8;
        
        int		srcLineInc = srcPitch - srcInc * srcRect.w;
        int		dstLineInc = dstPitch - dstInc * srcRect.w;
        
        unsigned int	R_mask = (((0xFFFFFFFF & pSrcFormat->R_mask) >> pSrcFormat->R_shift) << pDstFormat->R_shift) & pDstFormat->R_mask;
        unsigned int	G_mask = (((0xFFFFFFFF & pSrcFormat->G_mask) >> pSrcFormat->G_shift) << pDstFormat->G_shift) & pDstFormat->G_mask;
        unsigned int	B_mask = (((0xFFFFFFFF & pSrcFormat->B_mask) >> pSrcFormat->B_shift) << pDstFormat->B_shift) & pDstFormat->B_mask;
        unsigned int	A_mask = (((0xFFFFFFFF & pSrcFormat->A_mask) >> pSrcFormat->A_shift) << pDstFormat->A_shift) & pDstFormat->A_mask;
        
        
        switch( (pSrcFormat->bits << 8) + pDstFormat->bits )
        {
            case 0x1010:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = * ((unsigned short*)pSrc); pSrc+=2;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        * ((unsigned short*)pDst) = (unsigned short) dstpixel; pDst+=2;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
            case 0x1810:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = pSrc[0] + (((unsigned int)pSrc[1])<<8) + (((unsigned int)pSrc[2])<<16); pSrc+=3;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        * ((unsigned short*)pDst) = (unsigned short) dstpixel; pDst+=2;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
            case 0x2010:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = * ((unsigned int*)pSrc); pSrc+=4;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        * ((unsigned short*)pDst) = (unsigned short) dstpixel; pDst+=2;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
            case 0x1018:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = * ((unsigned short*)pSrc); pSrc+=2;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        pDst[0] = (unsigned char) dstpixel;
                        pDst[1] = (unsigned char) (dstpixel >> 8);
                        pDst[2] = (unsigned char) (dstpixel >> 16);
                        pDst+=3;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                    pDst+=3;
                }
                break;
            case 0x1818:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = pSrc[0] + (((unsigned int)pSrc[1]) << 8) + (((unsigned int)pSrc[2]) << 16); pSrc+=3;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        pDst[0] = (unsigned char) dstpixel;
                        pDst[1] = (unsigned char) (dstpixel >> 8);
                        pDst[2] = (unsigned char) (dstpixel >> 16);
                        pDst+=3;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
            case 0x2018:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = * ((unsigned int*)pSrc); pSrc+=4;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        pDst[0] = (unsigned char) dstpixel;
                        pDst[1] = (unsigned char) (dstpixel >> 8);
                        pDst[2] = (unsigned char) (dstpixel >> 16);
                        pDst+=3;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
            case 0x1020:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = * ((unsigned short*)pSrc); pSrc+=2;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        * ((unsigned int*)pDst) = dstpixel; pDst+=4;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
            case 0x1820:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = pSrc[0] + (((unsigned int)pSrc[1]) << 8) + (((unsigned int)pSrc[2]) << 16); pSrc+=3;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        * ((unsigned int*)pDst) = dstpixel; pDst+=4;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
            case 0x2020:
                for( int y = 0 ; y < srcRect.h ; y++ )
                {
                    for( int x = 0 ; x < srcRect.w ; x++ )
                    {
                        unsigned int srcpixel = * ((unsigned int*)pSrc); pSrc+=4;
                        unsigned int dstpixel = (((srcpixel >> pSrcFormat->R_shift) << pDstFormat->R_shift) & R_mask) |
                        (((srcpixel >> pSrcFormat->G_shift) << pDstFormat->G_shift) & G_mask) |
                        (((srcpixel >> pSrcFormat->B_shift) << pDstFormat->B_shift) & B_mask) |
                        (((srcpixel >> pSrcFormat->A_shift) << pDstFormat->A_shift) & A_mask);
                        * ((unsigned int*)pDst) = dstpixel; pDst+=4;
                    }
                    pSrc += srcLineInc;
                    pDst += dstLineInc;
                }
                break;
                
            default:
                return false;			// Failed to copy
        }
    }
    
    return true;
}


//____ PutPixels() _____________________________________________________________

#define PCLIP(x,y) (((x)>(y))?(y):(x))

void WgSurface::PutPixels(const vector<int> &x, const vector<int> &y, const vector<Uint32> &col, int length, bool replace)
{
	WgColor color1;
	WgColor color2;
	int ind;
	
	switch(m_pixelFormat.type)
	{
		case WG_PIXEL_BGR_8:
			break;
		case WG_PIXEL_BGRA_8:
			for(int n=0; n<length; n++)
			{
				ind = y[n]*m_pitch + x[n]*4;
				if(!replace) {
					// Get old (1) and new (2) pixel
					color1.argb = *((Uint32*)&m_pPixels[ind]);
					color2.argb = col[n];
					// Blend
					color1.r = (Uint8)(PCLIP((int)color1.r + (int)color2.r,0xFF));
					color1.g = (Uint8)(PCLIP((int)color1.g + (int)color2.g,0xFF));
					color1.b = (Uint8)(PCLIP((int)color1.b + (int)color2.b,0xFF));
					color1.a = (Uint8)(PCLIP((int)color1.a + (int)color2.a,0xFF));
					// Store
					*((Uint32*)&m_pPixels[ind]) = color1.argb;
				} else {
					// Overwrite old pixel with new pixel
					*((Uint32*)&m_pPixels[ind]) = col[n];
				}
			}
			break;
		default:
			break;
	}
}
