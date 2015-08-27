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

#include <wg_softgfxdevice.h>
#include <math.h>
#include <algorithm>

#include <assert.h>

namespace wg 
{
	
	#define NB_CURVETAB_ENTRIES	1024
	
	const char SoftGfxDevice::CLASSNAME[] = {"SoftGfxDevice"};
	
	//____ create() _______________________________________________________________
	
	SoftGfxDevice_p SoftGfxDevice::create()
	{
		return SoftGfxDevice_p(new SoftGfxDevice());
	}
	
	SoftGfxDevice_p SoftGfxDevice::create( const SoftSurface_p& pCanvas )
	{
		return SoftGfxDevice_p(new SoftGfxDevice(pCanvas));
	}
	
	
	//____ Constructor _____________________________________________________________
	
	SoftGfxDevice::SoftGfxDevice() : GfxDevice(Size(0,0))
	{
		m_bBilinearFiltering = true;
		m_pCanvas = 0;
		_initTables();
		_genCurveTab();
	}
	
	SoftGfxDevice::SoftGfxDevice( const SoftSurface_p& pCanvas ) : GfxDevice( pCanvas?pCanvas->size():Size() )
	{
		m_bBilinearFiltering = true;
		m_pCanvas = pCanvas;
		_initTables();
		_genCurveTab();
	}
	
	//____ Destructor ______________________________________________________________
	
	SoftGfxDevice::~SoftGfxDevice()
	{
		delete [] m_pDivTab;
		delete [] m_pCurveTab;
	}
	
	//____ isInstanceOf() _________________________________________________________
	
	bool SoftGfxDevice::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return GfxDevice::isInstanceOf(pClassName);
	}
	
	//____ className() ____________________________________________________________
	
	const char * SoftGfxDevice::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	//____ cast() _________________________________________________________________
	
	SoftGfxDevice_p SoftGfxDevice::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return SoftGfxDevice_p( static_cast<SoftGfxDevice*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ setCanvas() _______________________________________________________________
	
	void SoftGfxDevice::setCanvas( const SoftSurface_p& pCanvas )
	{
		m_pCanvas = pCanvas;
		if( pCanvas )
			m_canvasSize = pCanvas->size();
		else
			m_canvasSize = Size();
	}
	
	//____ fill() ____________________________________________________________________
	
	void SoftGfxDevice::fill( const Rect& rect, const Color& col )
	{
		if( !m_pCanvas || !m_pCanvas->m_pData )
			return;
	
		Color fillColor = col * m_tintColor;
	
		// Skip calls that won't affect destination
	
		if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
			return;
	
		// Optimize calls
	
		WgBlendMode blendMode = m_blendMode;
		if( blendMode == WG_BLENDMODE_BLEND && fillColor.a == 255 )
			blendMode = WG_BLENDMODE_OPAQUE;
	
		//
	
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
		Uint8 * pDst = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch + rect.x * pixelBytes;
	
	
	
	
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
			{
				for( int y = 0 ; y < rect.h ; y++ )
				{
					for( int x = 0 ; x < rect.w*pixelBytes ; x+=pixelBytes )
					{
						pDst[x] = fillColor.b;
						pDst[x+1] = fillColor.g;
						pDst[x+2] = fillColor.r;
					}
					pDst += m_pCanvas->m_pitch;
				}
			}
			break;
			case WG_BLENDMODE_BLEND:
			{
				int storedRed = ((int)fillColor.r) * fillColor.a;
				int storedGreen = ((int)fillColor.g) * fillColor.a;
				int storedBlue = ((int)fillColor.b) * fillColor.a;
				int invAlpha = 255-fillColor.a;
	
				for( int y = 0 ; y < rect.h ; y++ )
				{
					for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
					{
						pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
						pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
						pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
					}
					pDst += m_pCanvas->m_pitch;
				}
				break;
			}
			case WG_BLENDMODE_ADD:
			{
				int storedRed = (int) m_pDivTab[fillColor.r * fillColor.a];
				int storedGreen = (int) m_pDivTab[fillColor.g * fillColor.a];
				int storedBlue = (int) m_pDivTab[fillColor.b * fillColor.a];
	
				if( storedRed + storedGreen + storedBlue == 0 )
					return;
	
				for( int y = 0 ; y < rect.h ; y++ )
				{
					for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
					{
						pDst[x] = m_limitTable[pDst[x] + storedBlue];
						pDst[x+1] = m_limitTable[pDst[x+1] + storedGreen];
						pDst[x+2] = m_limitTable[pDst[x+2] + storedRed];
					}
					pDst += m_pCanvas->m_pitch;
				}
				break;
			}
			case WG_BLENDMODE_MULTIPLY:
			{
				int storedRed = (int)fillColor.r;
				int storedGreen = (int)fillColor.g;
				int storedBlue = (int)fillColor.b;
	
				for( int y = 0 ; y < rect.h ; y++ )
				{
					for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
					{
						pDst[x] = m_pDivTab[pDst[x] * storedBlue];
						pDst[x+1] = m_pDivTab[pDst[x+1] * storedGreen];
						pDst[x+2] = m_pDivTab[pDst[x+2] * storedRed];
					}
					pDst += m_pCanvas->m_pitch;
				}
				break;
			}
			case WG_BLENDMODE_INVERT:
			{
				int storedRed = (int)fillColor.r;
				int storedGreen = (int)fillColor.g;
				int storedBlue = (int)fillColor.b;
	
				int invertRed = 255 - (int)fillColor.r;
				int invertGreen = 255 - (int)fillColor.g;
				int invertBlue = 255 - (int)fillColor.b;
	
	
				for( int y = 0 ; y < rect.h ; y++ )
				{
					for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
					{
						pDst[x] = m_pDivTab[(255-pDst[x]) * storedBlue + pDst[x] * invertBlue];
						pDst[x+1] = m_pDivTab[(255-pDst[x+1]) * storedGreen + pDst[x+1] * invertGreen];
						pDst[x+2] = m_pDivTab[(255-pDst[x+2]) * storedRed + pDst[x+2] * invertRed];
					}
					pDst += m_pCanvas->m_pitch;
				}
				break;
			}
			default:
				break;
		}
	}
	
	//____ fillSubPixel() ____________________________________________________________________
	
	void SoftGfxDevice::fillSubPixel( const RectF& rect, const Color& col )
	{
		if( !m_pCanvas || !m_pCanvas->m_pData )
			return;
	
		Color fillColor = col * m_tintColor;
	
		// Skip calls that won't affect destination
	
		if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
			return;
	
		// Fill all but anti-aliased edges
	
		int x1 = (int) (rect.x + 0.999f);
		int y1 = (int) (rect.y + 0.999f);
		int x2 = (int) (rect.x + rect.w);
		int y2 = (int) (rect.y + rect.h);
	
		fill( Rect( x1,y1,x2-x1,y2-y1 ), col );
	
		// Optimize calls
	
		WgBlendMode blendMode = m_blendMode;
		if( blendMode == WG_BLENDMODE_BLEND && fillColor.a == 255 )
			blendMode = WG_BLENDMODE_OPAQUE;
	
		// Draw the sides
	
		int aaLeft = (256 - (int)(rect.x * 256)) & 0xFF;
		int aaTop = (256 - (int)(rect.y * 256)) & 0xFF;
		int aaRight = ((int)((rect.x + rect.w) * 256)) & 0xFF;
		int aaBottom = ((int)((rect.y + rect.h) * 256)) & 0xFF;
	
		if( aaTop != 0 )
			_drawHorrVertLineAA( x1, (int) rect.y, x2-x1, fillColor, blendMode, aaTop, WG_HORIZONTAL );
	
		if( aaBottom != 0 )
			_drawHorrVertLineAA( x1, (int) y2, x2-x1, fillColor, blendMode, aaBottom, WG_HORIZONTAL );
	
		if( aaLeft != 0 )
			_drawHorrVertLineAA( (int) rect.x, y1, y2-y1, fillColor, blendMode, aaLeft, WG_VERTICAL );
	
		if( aaRight != 0 )
			_drawHorrVertLineAA( (int) x2, y1, y2-y1, fillColor, blendMode, aaRight, WG_VERTICAL );
	
		// Draw corner pieces
	
		int aaTopLeft = aaTop * aaLeft / 256;
		int aaTopRight = aaTop * aaRight / 256;
		int aaBottomLeft = aaBottom * aaLeft / 256;
		int aaBottomRight = aaBottom * aaRight / 256;
	
		if( aaTopLeft != 0 )
			_plotAA( (int) rect.x, (int) rect.y, fillColor, blendMode, aaTopLeft );
	
		if( aaTopRight != 0 )
			_plotAA( x2, (int) rect.y, fillColor, blendMode, aaTopRight );
	
		if( aaBottomLeft != 0 )
			_plotAA( (int) rect.x, y2, fillColor, blendMode, aaBottomLeft );
	
		if( aaBottomRight != 0 )
			_plotAA( x2, y2, fillColor, blendMode, aaBottomRight );
	}
	
	//____ clipDrawHorrLine() _____________________________________________________
	
	void SoftGfxDevice::clipDrawHorrLine( const Rect& clip, const Coord& start, int length, const Color& col )
	{
		if( start.y < clip.y || start.y >= clip.y + clip.h || start.x >= clip.x + clip.w || start.x + length <= clip.x )
			return;
	
		int x = start.x;
	
		if( x < clip.x )
		{
			length = start.x + length - clip.x;
			x = clip.x;
		}
	
		if( x + length > clip.x + clip.w )
			length = clip.x + clip.w - x;
	
		_drawHorrVertLine( x, start.y, length, col, WG_HORIZONTAL );
	}
	
	//____ clipDrawVertLine() _____________________________________________________
	
	void SoftGfxDevice::clipDrawVertLine( const Rect& clip, const Coord& start, int length, const Color& col )
	{
		if( start.x < clip.x || start.x >= clip.x + clip.w || start.y >= clip.y + clip.h || start.y + length <= clip.y )
			return;
	
		int y = start.y;
	
		if( y < clip.y )
		{
			length = start.y + length - clip.y;
			y = clip.y;
		}
	
		if( y + length > clip.y + clip.h )
			length = clip.y + clip.h - y;
	
		_drawHorrVertLine( start.x, y, length, col, WG_VERTICAL );
	}
	
	//____ clipPlotSoftPixels() _______________________________________________________
	
	void SoftGfxDevice::clipPlotSoftPixels( const Rect& clip, int nCoords, const Coord * pCoords, const Color& col, float thickness )
	{
		int pitch = m_pCanvas->m_pitch;
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	
		int offset[4];
	
		offset[0] = -pixelBytes;
		offset[1] = -pitch;
		offset[2] = pixelBytes;
		offset[3] = pitch;
	
		int alpha = (int) (256*(thickness - 1.f)/2);
	
		int storedRed = ((int)col.r) * alpha;
		int storedGreen = ((int)col.g) * alpha;
		int storedBlue = ((int)col.b) * alpha;
		int invAlpha = 255-alpha;
	
		int yp = pCoords[0].y;
	
		for( int i = 0 ; i < nCoords ; i++ )
		{
			int x = pCoords[i].x;
			int begY;
			int endY;
	
			if( yp > pCoords[i].y )
			{
				begY = pCoords[i].y;
				endY = yp-1;
			}
			else if( pCoords[i].y > yp )
			{
				begY = yp+1;
				endY = pCoords[i].y;
			}
			else
			{
				begY = endY = yp;
			}
	
			for( int y = begY ; y <= endY ; y++ )
			{
				Uint8 * pDst = m_pCanvas->m_pData + y * m_pCanvas->m_pitch + pCoords[i].x * pixelBytes;
	
				if( y > clip.y && y < clip.y + clip.h -1 && x > clip.x && x < clip.x + clip.w -1 )
				{
					pDst[0] = col.b;
					pDst[1] = col.g;
					pDst[2] = col.r;
	
					for( int x = 0 ; x < 4 ; x++ )
					{
						int ofs = offset[x];
						pDst[ofs] = m_pDivTab[pDst[ofs]*invAlpha + storedBlue];
						pDst[ofs+1] = m_pDivTab[pDst[ofs+1]*invAlpha + storedGreen];
						pDst[ofs+2] = m_pDivTab[pDst[ofs+2]*invAlpha + storedRed];
					}
				}
			}
	
			yp = pCoords[i].y;
		}
	}
	/*
	void SoftGfxDevice::clipPlotSoftPixels( const Rect& clip, int nCoords, const Coord * pCoords, const Color& col, float thickness )
	{
		int pitch = m_pCanvas->m_pitch;
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	
		int offset[4];
	
		offset[0] = -pixelBytes;
		offset[1] = -pitch;
		offset[2] = pixelBytes;
		offset[3] = pitch;
	
		int alpha = (int) (256*(thickness - 1.f)/2);
	
		int storedRed = ((int)col.r) * alpha;
		int storedGreen = ((int)col.g) * alpha;
		int storedBlue = ((int)col.b) * alpha;
		int invAlpha = 255-alpha;
	
	
		for( int i = 0 ; i < nCoords ; i++ )
		{
			Uint8 * pDst = m_pCanvas->m_pData + pCoords[i].y * m_pCanvas->m_pitch + pCoords[i].x * pixelBytes;
	
			pDst[0] = col.b;
			pDst[1] = col.g;
			pDst[2] = col.r;
	
			for( int x = 0 ; x < 4 ; x++ )
			{
				int ofs = offset[x];
				pDst[ofs] = (Uint8) ((pDst[ofs]*invAlpha + storedBlue) >> 8);
				pDst[ofs+1] = (Uint8) ((pDst[ofs+1]*invAlpha + storedGreen) >> 8);
				pDst[ofs+2] = (Uint8) ((pDst[ofs+2]*invAlpha + storedRed) >> 8);
			}
		}
	}
	*/
	
	//____ _drawHorrVertLine() ________________________________________________
	
	void SoftGfxDevice::_drawHorrVertLine( int _x, int _y, int _length, const Color& _col, WgOrientation orientation  )
	{
		if( !m_pCanvas || !m_pCanvas->m_pData || _length <= 0  )
			return;
	
		Color fillColor = _col * m_tintColor;
	
		// Skip calls that won't affect destination
	
		if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
			return;
	
		// Optimize calls
	
		WgBlendMode blendMode = m_blendMode;
		if( blendMode == WG_BLENDMODE_BLEND && fillColor.a == 255 )
			blendMode = WG_BLENDMODE_OPAQUE;
	
		//
	
		int pitch = m_pCanvas->m_pitch;
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
		Uint8 * pDst = m_pCanvas->m_pData + _y * m_pCanvas->m_pitch + _x * pixelBytes;
	
		int inc;
	
		if( orientation == WG_HORIZONTAL )
			inc = pixelBytes;
		else
			inc = pitch;
	
		//
	
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
			{
				for( int x = 0 ; x < _length*inc ; x+=inc )
				{
					pDst[x] = fillColor.b;
					pDst[x+1] = fillColor.g;
					pDst[x+2] = fillColor.r;
				}
			}
			break;
			case WG_BLENDMODE_BLEND:
			{
				int storedRed = ((int)fillColor.r) * fillColor.a;
				int storedGreen = ((int)fillColor.g) * fillColor.a;
				int storedBlue = ((int)fillColor.b) * fillColor.a;
				int invAlpha = 255-fillColor.a;
	
				for( int x = 0 ; x < _length*inc ; x+=inc )
				{
					pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
					pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
					pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
				}
	
				break;
			}
			case WG_BLENDMODE_ADD:
			{
				int storedRed = m_pDivTab[fillColor.r * (int) fillColor.a];
				int storedGreen = m_pDivTab[fillColor.g * (int) fillColor.a];
				int storedBlue = m_pDivTab[fillColor.b * (int) fillColor.a];
	
				if( storedRed + storedGreen + storedBlue == 0 )
					return;
	
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = m_limitTable[pDst[x] + storedBlue];
					pDst[x+1] = m_limitTable[pDst[x+1] + storedGreen];
					pDst[x+2] = m_limitTable[pDst[x+2] + storedRed];
				}
				break;
			}
			case WG_BLENDMODE_MULTIPLY:
			{
				int storedRed = (int)fillColor.r;
				int storedGreen = (int)fillColor.g;
				int storedBlue = (int)fillColor.b;
	
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = m_pDivTab[pDst[x] * storedBlue];
					pDst[x+1] = m_pDivTab[pDst[x+1] * storedGreen];
					pDst[x+2] = m_pDivTab[pDst[x+2] * storedRed];
				}
				break;
			}
			case WG_BLENDMODE_INVERT:
			{
				int storedRed = (int)fillColor.r;
				int storedGreen = (int)fillColor.g;
				int storedBlue = (int)fillColor.b;
	
				int invertRed = 255 - (int)fillColor.r;
				int invertGreen = 255 - (int)fillColor.g;
				int invertBlue = 255 - (int)fillColor.b;
	
	
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = m_pDivTab[(255-pDst[x]) * storedBlue + pDst[x] * invertBlue];
					pDst[x+1] = m_pDivTab[(255-pDst[x+1]) * storedGreen + pDst[x+1] * invertGreen];
					pDst[x+2] = m_pDivTab[(255-pDst[x+2]) * storedRed + pDst[x+2] * invertRed];
				}
				break;
			}
			default:
				break;
		}
	}
	
	//____ _drawHorrVertLineAA() ________________________________________________
	
	void SoftGfxDevice::_drawHorrVertLineAA( int _x, int _y, int _length, const Color& _col, WgBlendMode blendMode, int _aa, WgOrientation orientation )
	{
		int pitch = m_pCanvas->m_pitch;
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
		Uint8 * pDst = m_pCanvas->m_pData + _y * m_pCanvas->m_pitch + _x * pixelBytes;
	
		int inc;
		if( orientation == WG_HORIZONTAL )
			inc = pixelBytes;
		else
			inc = pitch;
	
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
			{
				int storedRed = ((int)_col.r) * _aa;
				int storedGreen = ((int)_col.g) * _aa;
				int storedBlue = ((int)_col.b) * _aa;
				int invAlpha = 255- _aa;
	
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
					pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
					pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
				}
				break;
			}
			case WG_BLENDMODE_BLEND:
			{
				int aa = m_pDivTab[_col.a * _aa];
				
				int storedRed = _col.r * aa;
				int storedGreen = _col.g * aa;
				int storedBlue = _col.b * aa;
				int invAlpha = 255-aa;
	
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
					pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
					pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
				}
				break;
			}
			case WG_BLENDMODE_ADD:
			{
				int aa = m_pDivTab[_col.a * _aa];
				
				int storedRed = m_pDivTab[_col.r * aa];
				int storedGreen = m_pDivTab[_col.g * aa];
				int storedBlue = m_pDivTab[_col.b * aa];
				
				if( storedRed + storedGreen + storedBlue == 0 )
					return;
	
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = m_limitTable[pDst[x] + storedBlue];
					pDst[x+1] = m_limitTable[pDst[x+1] + storedGreen];
					pDst[x+2] = m_limitTable[pDst[x+2] + storedRed];
				}
				break;
			}
			case WG_BLENDMODE_MULTIPLY:
			{
				int storedRed = (int) m_pDivTab[_col.r*_aa];
				int storedGreen = (int) m_pDivTab[_col.g*_aa];
				int storedBlue = (int) m_pDivTab[_col.b*_aa];
				
				int invAlpha = 255 - _aa;
				
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = m_pDivTab[(pDst[x]*invAlpha) + (pDst[x] * storedBlue)];
					pDst[x+1] = m_pDivTab[(pDst[x+1]*invAlpha) + (pDst[x+1] * storedGreen)];
					pDst[x+2] = m_pDivTab[(pDst[x+2]*invAlpha) + (pDst[x+2] * storedRed)];
				}
				break;
			}
			case WG_BLENDMODE_INVERT:
			{
				//TODO: Translate to use m_pDivTab
	
				int storedRed = (int)_col.r;
				int storedGreen = (int)_col.g;
				int storedBlue = (int)_col.b;
	
				int invertRed = 255 - (int)_col.r;
				int invertGreen = 255 - (int)_col.g;
				int invertBlue = 255 - (int)_col.b;
	
				int invAlpha = (255 - _aa) << 8;
	
				for( int x = 0 ; x < _length*inc ; x+= inc )
				{
					pDst[x] = ( (pDst[x]*invAlpha) + _aa * ((255-pDst[x]) * storedBlue + pDst[x] * invertBlue) ) >> 16;
					pDst[x+1] = ( (pDst[x+1]*invAlpha) + _aa * ((255-pDst[x+1]) * storedGreen + pDst[x+1] * invertGreen) )  >> 16;
					pDst[x+2] = ( (pDst[x+2]*invAlpha) + _aa * ((255-pDst[x+2]) * storedRed + pDst[x+2] * invertRed) ) >> 16;
				}
				break;
			}
			default:
				break;
		}
	}
	
	//____ _plotAA() ________________________________________________
	
	void SoftGfxDevice::_plotAA( int _x, int _y, const Color& _col, WgBlendMode blendMode, int _aa )
	{
		//TODO: Translate to use m_pDivTab
	
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
		Uint8 * pDst = m_pCanvas->m_pData + _y * m_pCanvas->m_pitch + _x * pixelBytes;
	
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
			{
				int storedRed = ((int)_col.r) * _aa;
				int storedGreen = ((int)_col.g) * _aa;
				int storedBlue = ((int)_col.b) * _aa;
				int invAlpha = 255- _aa;
	
				pDst[0] = (Uint8) ((pDst[0]*invAlpha + storedBlue) >> 8);
				pDst[1] = (Uint8) ((pDst[1]*invAlpha + storedGreen) >> 8);
				pDst[2] = (Uint8) ((pDst[2]*invAlpha + storedRed) >> 8);
				break;
			}
			case WG_BLENDMODE_BLEND:
			{
				int aa = _col.a * _aa;
	
				int storedRed = (((int)_col.r) * aa) >> 8;
				int storedGreen = (((int)_col.g) * aa) >> 8;
				int storedBlue = (((int)_col.b) * aa) >> 8;
				int invAlpha = 255-(aa>>8);
	
				pDst[0] = (Uint8) ((pDst[0]*invAlpha + storedBlue) >> 8);
				pDst[1] = (Uint8) ((pDst[1]*invAlpha + storedGreen) >> 8);
				pDst[2] = (Uint8) ((pDst[2]*invAlpha + storedRed) >> 8);
				break;
			}
			case WG_BLENDMODE_ADD:
			{
				int aa = _col.a * _aa;
	
				int storedRed = (((int)_col.r) * aa) >> 16;
				int storedGreen = (((int)_col.g) * aa) >> 16;
				int storedBlue = (((int)_col.b) * aa) >> 16;
	
				if( storedRed + storedGreen + storedBlue == 0 )
					return;
	
				pDst[0] = m_limitTable[pDst[0] + storedBlue];
				pDst[1] = m_limitTable[pDst[1] + storedGreen];
				pDst[2] = m_limitTable[pDst[2] + storedRed];
				break;
			}
			case WG_BLENDMODE_MULTIPLY:
			{
				int storedRed = (int)_col.r;
				int storedGreen = (int)_col.g;
				int storedBlue = (int)_col.b;
	
				int invAlpha = (255 - _aa) << 8;
	
				pDst[0] = ( (pDst[0]*invAlpha) + (_aa * pDst[0] * storedBlue) ) >> 16;
				pDst[1] = ( (pDst[1]*invAlpha) + (_aa * pDst[1] * storedGreen) ) >> 16;
				pDst[2] = ( (pDst[2]*invAlpha) + (_aa * pDst[2] * storedRed) ) >> 16;
				break;
			}
			case WG_BLENDMODE_INVERT:
			{
				int storedRed = (int)_col.r;
				int storedGreen = (int)_col.g;
				int storedBlue = (int)_col.b;
	
				int invertRed = 255 - (int)_col.r;
				int invertGreen = 255 - (int)_col.g;
				int invertBlue = 255 - (int)_col.b;
	
				int invAlpha = (255 - _aa) << 8;
	
				pDst[0] = ( (pDst[0]*invAlpha) + _aa * ((255-pDst[0]) * storedBlue + pDst[0] * invertBlue) ) >> 16;
				pDst[1] = ( (pDst[1]*invAlpha) + _aa * ((255-pDst[1]) * storedGreen + pDst[1] * invertGreen) )  >> 16;
				pDst[2] = ( (pDst[2]*invAlpha) + _aa * ((255-pDst[2]) * storedRed + pDst[2] * invertRed) ) >> 16;
				break;
			}
			default:
				break;
		}
	}
	
	
	//____ _genCurveTab() ___________________________________________________________
	
	void SoftGfxDevice::_genCurveTab()
	{
		m_pCurveTab = new int[NB_CURVETAB_ENTRIES];
	
		double factor = 3.14159265 / (2.0 * NB_CURVETAB_ENTRIES);
	
		for( int i = 0 ; i < NB_CURVETAB_ENTRIES ; i++ )
		{
			double y = 1.f - i/(double)NB_CURVETAB_ENTRIES;
			m_pCurveTab[i] = (int) (sqrt(1.f - y*y)*65536.f);
		}
	}
	
	
	//____ drawElipse() _______________________________________________________________
	
	void SoftGfxDevice::drawElipse( const Rect& rect, Color color )
	{
		if( rect.h < 2 || rect.w < 1 )
			return;
	
		int sectionHeight = rect.h/2;
		int maxWidth = rect.w/2;
	
		Uint8 * pLineBeg = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch;
		int pitch = m_pCanvas->m_pitch;
	
		int center = (rect.x + rect.w/2) << 8;
	
		int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / sectionHeight;
		int sinOfs = 0;
	
		int begOfs = 0;
		int peakOfs = 0;
		int endOfs = 0;
	
		for( int i = 0 ; i < sectionHeight ; i++ )
		{
			peakOfs = ((m_pCurveTab[sinOfs>>16] * maxWidth) >> 8);
			endOfs = (m_pCurveTab[(sinOfs+(sinOfsInc-1))>>16] * maxWidth) >> 8;
	
			_drawHorrFadeLine( pLineBeg + i*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, color );
			_drawHorrFadeLine( pLineBeg + i*pitch, center - endOfs, center - peakOfs, center - begOfs +256, color );
	
			_drawHorrFadeLine( pLineBeg + (sectionHeight*2-i-1)*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, color );
			_drawHorrFadeLine( pLineBeg + (sectionHeight*2-i-1)*pitch, center - endOfs, center - peakOfs, center - begOfs +256, color );
	
			begOfs = peakOfs;
			sinOfs += sinOfsInc;
		}
	}
	
	//____ _clipDrawHorrFadeLine() _______________________________________________________________
	
	void SoftGfxDevice::_clipDrawHorrFadeLine( int clipX1, int clipX2, Uint8 * pLineStart, int begOfs, int peakOfs, int endOfs, Color color )
	{
		//TODO: Translate to use m_pDivTab
	
		int pitch = m_pCanvas->m_pitch;
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
		Uint8 * p = pLineStart + (begOfs>>8) * pixelBytes;
		Uint8 * pClip1 = pLineStart + clipX1*pixelBytes;
		Uint8 * pClip2 = pLineStart + clipX2*pixelBytes;
	
		int alphaInc, alpha, len;
	
		if( (peakOfs>>8) == (begOfs>>8) )
		{
			alphaInc = 0;
			alpha = (256-(peakOfs&0xff) + (peakOfs-begOfs)/2) << 14;
			len = 1;
		}
		else
		{
			alphaInc = (255 << 22) / (peakOfs - begOfs);			// alpha inc per pixel with 14 binals.
			alpha = ((256 - (begOfs&0xff)) * alphaInc) >> 9;		// alpha for ramp up start pixel with 14 binals.
			len = ((peakOfs+256) >> 8) - (begOfs >> 8);
		}
	
		for( int i = 0 ; i < len ; i++ )
		{
			if( p >= pClip1 && p < pClip2 )
			{
				int invAlpha = (255 << 14) - alpha;
	
				p[0] = ((color.b * alpha) + (p[0]*invAlpha)) >> 22;
				p[1] = ((color.g * alpha) + (p[1]*invAlpha)) >> 22;
				p[2] = ((color.r * alpha) + (p[2]*invAlpha)) >> 22;
			}
			alpha += alphaInc;
			if( alpha > 255 << 14 )
				alpha = 255 << 14;
	
			p += pixelBytes;
		}
	
		if( (endOfs>>8) == ((peakOfs + 256)>>8) )
		{
			alphaInc = 0;
			alpha = ((peakOfs&0xff)+(endOfs-peakOfs-256)/2) << 14;
			len = 1;
		}
		else
		{
			alphaInc = (255 << 22) / (endOfs - (peakOfs+256));						// alpha dec per pixel with 14 binals.
			alpha = (255 << 14) - (((256 - (peakOfs&0xff)) * alphaInc) >> 9);	// alpha for ramp down start pixel with 14 binals.
			len = (endOfs >> 8) - ((peakOfs+256) >> 8);
			alphaInc = -alphaInc;
		}
	
		for( int i = 0 ; i < len ; i++ )
		{
			if( p >= pClip1 && p < pClip2 )
			{
				int invAlpha = (255 << 14) - alpha;
	
				p[0] = ((color.b * alpha) + (p[0]*invAlpha)) >> 22;
				p[1] = ((color.g * alpha) + (p[1]*invAlpha)) >> 22;
				p[2] = ((color.r * alpha) + (p[2]*invAlpha)) >> 22;
			}
			alpha += alphaInc;
			p += pixelBytes;
		}
	}
	
	
	//____ _drawHorrFadeLine() _______________________________________________________________
	
	void SoftGfxDevice::_drawHorrFadeLine( Uint8 * pLineStart, int begOfs, int peakOfs, int endOfs, Color color )
	{
		//TODO: Translate to use m_pDivTab
	
		int pitch = m_pCanvas->m_pitch;
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
		Uint8 * p = pLineStart + (begOfs>>8) * pixelBytes;
	
		int alphaInc, alpha, len;
	
		if( (peakOfs>>8) == (begOfs>>8) )
		{
			alphaInc = 0;
			alpha = (256-(peakOfs&0xff) + (peakOfs-begOfs)/2) << 14;
			len = 1;
		}
		else
		{
			alphaInc = (255 << 22) / (peakOfs - begOfs);			// alpha inc per pixel with 14 binals.
			alpha = ((256 - (begOfs&0xff)) * alphaInc) >> 9;		// alpha for ramp up start pixel with 14 binals.
			len = ((peakOfs+256) >> 8) - (begOfs >> 8);
		}
	
		for( int i = 0 ; i < len ; i++ )
		{
			int invAlpha = (255 << 14) - alpha;
	
			p[0] = ((color.b * alpha) + (p[0]*invAlpha)) >> 22;
			p[1] = ((color.g * alpha) + (p[1]*invAlpha)) >> 22;
			p[2] = ((color.r * alpha) + (p[2]*invAlpha)) >> 22;
			alpha += alphaInc;
			if( alpha > 255 << 14 )
				alpha = 255 << 14;
	
			p += pixelBytes;
		}
	
		if( (endOfs>>8) == ((peakOfs + 256)>>8) )
		{
			alphaInc = 0;
			alpha = ((peakOfs&0xff)+(endOfs-peakOfs-256)/2) << 14;
			len = 1;
		}
		else
		{
			alphaInc = (255 << 22) / (endOfs - (peakOfs+256));						// alpha dec per pixel with 14 binals.
			alpha = (255 << 14) - (((256 - (peakOfs&0xff)) * alphaInc) >> 9);	// alpha for ramp down start pixel with 14 binals.
			len = (endOfs >> 8) - ((peakOfs+256) >> 8);
			alphaInc = -alphaInc;
		}
	
		for( int i = 0 ; i < len ; i++ )
		{
			int invAlpha = (255 << 14) - alpha;
	
			p[0] = ((color.b * alpha) + (p[0]*invAlpha)) >> 22;
			p[1] = ((color.g * alpha) + (p[1]*invAlpha)) >> 22;
			p[2] = ((color.r * alpha) + (p[2]*invAlpha)) >> 22;
			alpha += alphaInc;
			p += pixelBytes;
		}
	
	}
	
	//____ clipDrawElipse() _______________________________________________________________
	
	void SoftGfxDevice::clipDrawElipse( const Rect& clip, const Rect& rect, Color color )
	{
		if( rect.h < 2 || rect.w < 1 )
			return;
	
		if( !rect.intersectsWith(clip) )
			return;
	
		if( clip.contains(rect) )
			return drawElipse(rect,color);
	
		int sectionHeight = rect.h/2;
		int maxWidth = rect.w/2;
	
		Uint8 * pLineBeg = m_pCanvas->m_pData + rect.y*m_pCanvas->m_pitch;
		int pitch = m_pCanvas->m_pitch;
	
		int center = (rect.x + rect.w/2) << 8;
	
		int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / sectionHeight;
		int sinOfs = 0;
	
		int begOfs = 0;
		int peakOfs = 0;
		int endOfs = 0;
	
		for( int i = 0 ; i < sectionHeight ; i++ )
		{
			peakOfs = ((m_pCurveTab[sinOfs>>16] * maxWidth) >> 8);
			endOfs = (m_pCurveTab[(sinOfs+(sinOfsInc-1))>>16] * maxWidth) >> 8;
	
			if( rect.y + i >= clip.y && rect.y + i < clip.y + clip.h )
			{
				_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + i*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, color );
				_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + i*pitch, center - endOfs, center - peakOfs, center - begOfs +256, color );
			}
	
			int y2 = sectionHeight*2-i-1;
			if( rect.y + y2 >= clip.y && rect.y + y2 < clip.y + clip.h )
			{
				_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + y2*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, color );
				_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + y2*pitch, center - endOfs, center - peakOfs, center - begOfs +256, color );
			}
	
			begOfs = peakOfs;
			sinOfs += sinOfsInc;
		}
	}
	
	//____ drawFilledElipse() _____________________________________________________
	
	void SoftGfxDevice::drawFilledElipse( const Rect& rect, Color color )
	{
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	
		Uint8 * pLineCenter = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch + (rect.x+rect.w/2) * pixelBytes;
	
		int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / (rect.h/2);
		int sinOfs = sinOfsInc >> 1;
	
		for( int j = 0 ; j < 2 ; j++ )
		{
			for( int i = 0 ; i < rect.h/2 ; i++ )
			{
				int lineLen = ((m_pCurveTab[sinOfs>>16] * rect.w/2 + 32768)>>16)*pixelBytes;
				Uint8 * pLineBeg = pLineCenter - lineLen;
				Uint8 * pLineEnd = pLineCenter + lineLen;
	
				for( Uint8 * p = pLineBeg ; p < pLineEnd ; p += pixelBytes )
				{
					p[0] = color.b;
					p[1] = color.g;
					p[2] = color.r;
				}
	
				sinOfs += sinOfsInc;
				pLineCenter += m_pCanvas->m_pitch;
			}
			sinOfsInc = -sinOfsInc;
			sinOfs = (NB_CURVETAB_ENTRIES << 16) + (sinOfsInc >> 1);
		}
	}
	
	//____ clipDrawFilledElipse() _____________________________________________________
	
	void SoftGfxDevice::clipDrawFilledElipse( const Rect& clip, const Rect& rect, Color color )
	{
		if( !rect.intersectsWith(clip) )
			return;
	
		if( clip.contains(rect) )
			return drawFilledElipse(rect,color);
	
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	
		Uint8 * pLine = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch;
	
		int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / (rect.h/2);
		int sinOfs = sinOfsInc >> 1;
	
		for( int j = 0 ; j < 2 ; j++ )
		{
			for( int i = 0 ; i < rect.h/2 ; i++ )
			{
				if( rect.y + j*(rect.h/2) + i >= clip.y && rect.y + j*(rect.h/2) + i < clip.y + clip.h )
				{
					int lineLen = ((m_pCurveTab[sinOfs>>16] * rect.w/2 + 32768)>>16);
	
					int beg = rect.x + rect.w/2 - lineLen;
					int end = rect.x + rect.w/2 + lineLen;
	
					if( beg < clip.x )
						beg = clip.x;
	
					if( end > clip.x + clip.w )
						end = clip.x + clip.w;
	
					if( beg < end )
					{
						Uint8 * pLineBeg = pLine + beg * pixelBytes;
						Uint8 * pLineEnd = pLine + end * pixelBytes;
	
						for( Uint8 * p = pLineBeg ; p < pLineEnd ; p += pixelBytes )
						{
							p[0] = color.b;
							p[1] = color.g;
							p[2] = color.r;
						}
					}
				}
	
				sinOfs += sinOfsInc;
				pLine += m_pCanvas->m_pitch;
			}
	
			sinOfsInc = -sinOfsInc;
			sinOfs = (NB_CURVETAB_ENTRIES << 16) + (sinOfsInc >> 1);
		}
	}
	
	
	//____ drawArcNE() ____________________________________________________________
	
	void SoftGfxDevice::drawArcNE( const Rect& rect, Color color )
	{
		int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	
		Uint8 * pLineBeg = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch + rect.x * pixelBytes;
	
		int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / rect.h;
		int sinOfs = sinOfsInc >> 1;
	
		for( int i = 0 ; i < rect.h ; i++ )
		{
			Uint8 * pLineEnd = pLineBeg + ((m_pCurveTab[sinOfs>>16] * rect.w + 32768)>>16)*pixelBytes;
	
			for( Uint8 * p = pLineBeg ; p < pLineEnd ; p += pixelBytes )
			{
				p[0] = color.b;
				p[1] = color.g;
				p[2] = color.r;
			}
	
			sinOfs += sinOfsInc;
			pLineBeg += m_pCanvas->m_pitch;
		}
	
	}
	
	//____ clipDrawArcNE() _________________________________________________________
	
	void SoftGfxDevice::clipDrawArcNE( const Rect& clip, const Rect& rect, Color color )
	{
		//TODO: Implement!!!
	}
	
	
	//____ blit() __________________________________________________________________
	
	void SoftGfxDevice::blit( const Surface_p& pSrcSurf, const Rect& srcrect, int dx, int dy  )
	{
		Surface * pSrc = pSrcSurf.rawPtr();
	
		if( m_tintColor.argb == 0xFFFFFFFF )
			_blit( pSrc, srcrect, dx, dy );
		else
			_tintBlit( pSrc, srcrect, dx, dy );
	}
	
	//____ _blit() _____________________________________________________________
	
	void SoftGfxDevice::_blit( const Surface* _pSrcSurf, const Rect& srcrect, int dx, int dy  )
	{
		if( !_pSrcSurf || !m_pCanvas || !_pSrcSurf->isInstanceOf(SoftSurface::CLASSNAME) )
			return;
	
		SoftSurface * pSrcSurf = (SoftSurface*) _pSrcSurf;
	
		if( !m_pCanvas->m_pData || !pSrcSurf->m_pData )
			return;
	
		int srcPixelBytes = pSrcSurf->m_pixelFormat.bits/8;
		int dstPixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	
		int	srcPitchAdd = pSrcSurf->m_pitch - srcrect.w*srcPixelBytes;
		int	dstPitchAdd = m_pCanvas->m_pitch - srcrect.w*dstPixelBytes;
	
		Uint8 * pDst = m_pCanvas->m_pData + dy * m_pCanvas->m_pitch + dx * dstPixelBytes;
		Uint8 * pSrc = pSrcSurf->m_pData + srcrect.y * pSrcSurf->m_pitch + srcrect.x * srcPixelBytes;
	
		WgBlendMode		blendMode = m_blendMode;
		if( srcPixelBytes == 3 && blendMode == WG_BLENDMODE_BLEND )
			blendMode = WG_BLENDMODE_OPAQUE;
	
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
			{
				if( srcPixelBytes == 4 && dstPixelBytes == 4 )
				{
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							* ((Uint32*)pDst) = * ((Uint32*)pSrc) & 0x00FFFFFF;
							pSrc += 4;
							pDst += 4;
						}
	
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				else
				{
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							pDst[0] = pSrc[0];
							pDst[1] = pSrc[1];
							pDst[2] = pSrc[2];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
	
				break;
			}
			case WG_BLENDMODE_BLEND:
			{
				if( srcPixelBytes == 4 )
				{
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							int alpha = pSrc[3];
							int invAlpha = 255-alpha;
	
							pDst[0] = m_pDivTab[pDst[0]*invAlpha + pSrc[0]*alpha];
							pDst[1] = m_pDivTab[pDst[1]*invAlpha + pSrc[1]*alpha];
							pDst[2] = m_pDivTab[pDst[2]*invAlpha + pSrc[2]*alpha];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				else
				{
					// Should never get here, skips to blendmode opaque instead.
				}
				break;
			}
			case WG_BLENDMODE_ADD:
			{
				if( srcPixelBytes == 4 )
				{
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							int alpha = pSrc[3];
	
							pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[pSrc[0]*alpha] ];
							pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[pSrc[1]*alpha] ];
							pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[pSrc[2]*alpha] ];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				else
				{
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							pDst[0] = m_limitTable[pDst[0] + pSrc[0]];
							pDst[1] = m_limitTable[pDst[1] + pSrc[1]];
							pDst[2] = m_limitTable[pDst[2] + pSrc[2]];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				break;
			}
			case WG_BLENDMODE_MULTIPLY:
			{
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						pDst[0] = m_pDivTab[ pDst[0]*pSrc[0] ];
						pDst[1] = m_pDivTab[ pDst[1]*pSrc[1] ];
						pDst[2] = m_pDivTab[ pDst[2]*pSrc[2] ];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
				break;
			}
			case WG_BLENDMODE_INVERT:
			{
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						pDst[0] = m_pDivTab[pSrc[0]*(255-pDst[0]) + pDst[0]*(255-pSrc[0])];
						pDst[1] = m_pDivTab[pSrc[1]*(255-pDst[1]) + pDst[1]*(255-pSrc[0])];
						pDst[2] = m_pDivTab[pSrc[2]*(255-pDst[2]) + pDst[2]*(255-pSrc[0])];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
				break;
			}
			default:
				break;
		}
	}
	
	
	
	//____ _tintBlit() _____________________________________________________________
	
	void SoftGfxDevice::_tintBlit( const Surface* _pSrcSurf, const Rect& srcrect, int dx, int dy  )
	{
		if( !_pSrcSurf || !m_pCanvas || !_pSrcSurf->isInstanceOf(SoftSurface::CLASSNAME) )
			return;
	
		SoftSurface * pSrcSurf = (SoftSurface*) _pSrcSurf;
	
		if( !m_pCanvas->m_pData || !pSrcSurf->m_pData )
			return;
	
		int srcPixelBytes = pSrcSurf->m_pixelFormat.bits/8;
		int dstPixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	
		int	srcPitchAdd = pSrcSurf->m_pitch - srcrect.w*srcPixelBytes;
		int	dstPitchAdd = m_pCanvas->m_pitch - srcrect.w*dstPixelBytes;
	
		Uint8 * pDst = m_pCanvas->m_pData + dy * m_pCanvas->m_pitch + dx * dstPixelBytes;
		Uint8 * pSrc = pSrcSurf->m_pData + srcrect.y * pSrcSurf->m_pitch + srcrect.x * srcPixelBytes;
	
		WgBlendMode		blendMode = m_blendMode;
		if( srcPixelBytes == 3 && blendMode == WG_BLENDMODE_BLEND && m_tintColor.a == 255 )
			blendMode = WG_BLENDMODE_OPAQUE;
	
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
			{
				int tintRed = (int) m_tintColor.r;
				int tintGreen = (int) m_tintColor.g;
				int tintBlue = (int) m_tintColor.b;
	
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						pDst[0] = m_pDivTab[pSrc[0]*tintBlue];
						pDst[1] = m_pDivTab[pSrc[1]*tintGreen];
						pDst[2] = m_pDivTab[pSrc[2]*tintRed];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
				break;
			}
			case WG_BLENDMODE_BLEND:
			{
				if( srcPixelBytes == 4 )
				{
					int tintAlpha = (int) m_tintColor.a;
					int tintRed = (int) m_tintColor.r;
					int tintGreen = (int) m_tintColor.g;
					int tintBlue = (int) m_tintColor.b;
	
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							int alpha = m_pDivTab[pSrc[3]*tintAlpha];
							int invAlpha = 255-alpha;
	
							int srcBlue		= m_pDivTab[pSrc[0] * tintBlue];
							int srcGreen	= m_pDivTab[pSrc[1] * tintGreen];
							int srcRed		= m_pDivTab[pSrc[2] * tintRed];
							
	
							pDst[0] = m_pDivTab[ pDst[0]*invAlpha + srcBlue*alpha ];
							pDst[1] = m_pDivTab[ pDst[1]*invAlpha + srcGreen*alpha ];
							pDst[2] = m_pDivTab[ pDst[2]*invAlpha + srcRed*alpha ];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				else
				{
					int tintAlpha = (int) m_tintColor.a;
					int tintRed = (int) m_pDivTab[ m_tintColor.r * tintAlpha ];
					int tintGreen = (int) m_pDivTab[ m_tintColor.g * tintAlpha ];
					int tintBlue = (int) m_pDivTab[ m_tintColor.b * tintAlpha ];
					int invAlpha = 255-tintAlpha;
	
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							pDst[0] = m_pDivTab[ pDst[0]*invAlpha + pSrc[0]*tintBlue ];
							pDst[1] = m_pDivTab[ pDst[1]*invAlpha + pSrc[1]*tintGreen ];
							pDst[2] = m_pDivTab[ pDst[2]*invAlpha + pSrc[2]*tintRed ];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				break;
			}
			case WG_BLENDMODE_ADD:
			{
				if( srcPixelBytes == 4 )
				{
					int tintAlpha = (int) m_tintColor.a;
					int tintRed = (int) m_tintColor.r;
					int tintGreen = (int) m_tintColor.g;
					int tintBlue = (int) m_tintColor.b;
	
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							int alpha = m_pDivTab[ pSrc[3]*tintAlpha ];
	
							int srcBlue		= m_pDivTab[pSrc[0] * tintBlue];
							int srcGreen	= m_pDivTab[pSrc[1] * tintGreen];
							int srcRed		= m_pDivTab[pSrc[2] * tintRed];
	
							pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*alpha] ];
							pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*alpha] ];
							pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[ srcRed*alpha] ];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				else
				{
					int tintAlpha = (int) m_tintColor.a;
					int tintRed = (int) m_pDivTab[m_tintColor.r * tintAlpha];
					int tintGreen = (int) m_pDivTab[m_tintColor.g * tintAlpha];
					int tintBlue = (int) m_pDivTab[m_tintColor.b * tintAlpha];
	
					for( int y = 0 ; y < srcrect.h ; y++ )
					{
						for( int x = 0 ; x < srcrect.w ; x++ )
						{
							pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[pSrc[0]*tintBlue] ];
							pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[pSrc[1]*tintGreen] ];
							pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[pSrc[2]*tintRed] ];
							pSrc += srcPixelBytes;
							pDst += dstPixelBytes;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
				}
				break;
			}
			case WG_BLENDMODE_MULTIPLY:
			{
				int tintRed = (int) m_tintColor.r;
				int tintGreen = (int) m_tintColor.g;
				int tintBlue = (int) m_tintColor.b;
	
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						int srcBlue		= m_pDivTab[pSrc[0] * tintBlue];
						int srcGreen	= m_pDivTab[pSrc[1] * tintGreen];
						int srcRed		= m_pDivTab[pSrc[2] * tintRed];
	
	
						pDst[0] = m_pDivTab[srcBlue*pDst[0]];
						pDst[1] = m_pDivTab[srcGreen*pDst[1]];
						pDst[2] = m_pDivTab[srcRed*pDst[2]];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
				break;
			}
			case WG_BLENDMODE_INVERT:
			{
				int tintRed = (int) m_tintColor.r;
				int tintGreen = (int) m_tintColor.g;
				int tintBlue = (int) m_tintColor.b;
	
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						int srcBlue = m_pDivTab[tintBlue*pSrc[0]];
						int srcGreen = m_pDivTab[tintGreen*pSrc[1]];
						int srcRed = m_pDivTab[tintRed*pSrc[2]];
	
						pDst[0] = m_pDivTab[srcBlue*(255-pDst[0]) + pDst[0]*(255-srcBlue)];
						pDst[1] = m_pDivTab[srcGreen*(255-pDst[1]) + pDst[1]*(255-srcGreen)];
						pDst[2] = m_pDivTab[srcRed*(255-pDst[2]) + pDst[2]*(255-srcRed)];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
				break;
			}
			default:
				break;
		}
	}
	
	//____ stretchBlit() ___________________________________________________________
	
	void SoftGfxDevice::stretchBlit( const Surface_p& pSrc, bool bTriLinear, float mipmapBias )
	{
		stretchBlit( pSrc, Rect(0, 0, pSrc->width(),pSrc->height()), Rect(0,0,m_canvasSize.w,m_canvasSize.h), bTriLinear, mipmapBias );
	}
	
	void SoftGfxDevice::stretchBlit( const Surface_p& pSrc, const Rect& dest, bool bTriLinear, float mipmapBias )
	{
		stretchBlit( pSrc, Rect(0, 0, pSrc->width(),pSrc->height()), dest, bTriLinear, mipmapBias );
	}
	
	void SoftGfxDevice::stretchBlit( const Surface_p& pSrc, const Rect& src, const Rect& dest, bool bTriLinear, float mipmapBias )
	{
		float srcW = (float) src.w;
		float srcH = (float) src.h;
	
		float destW = (float) dest.w;
		float destH = (float) dest.h;
	
		if( m_bBilinearFiltering )
		{
			if( srcW < destW )
				srcW--;
	
			if( srcH < destH )
				srcH--;
		}
	
		stretchBlitSubPixel( pSrc, (float) src.x, (float) src.y, srcW, srcH, (float) dest.x, (float) dest.y, destW, destH, bTriLinear, mipmapBias );
	}
	
	//____ clipStretchBlit() _______________________________________________________
	
	void SoftGfxDevice::clipStretchBlit( const Rect& clip, const Surface_p& pSrc, bool bTriLinear, float mipBias )
	{
		clipStretchBlit( clip, pSrc, Rect(0,0,pSrc->width(), pSrc->height()), Rect( 0,0,m_canvasSize), bTriLinear, mipBias );
	}
	
	void SoftGfxDevice::clipStretchBlit( const Rect& clip, const Surface_p& pSrc, const Rect& dest, bool bTriLinear, float mipBias )
	{
		clipStretchBlit( clip, pSrc, Rect(0,0,pSrc->width(), pSrc->height()), dest, bTriLinear, mipBias );
	}
	
	void SoftGfxDevice::clipStretchBlit( const Rect& clip, const Surface_p& pSrc, const Rect& src, const Rect& dest, bool bTriLinear, float mipBias )
	{
		clipStretchBlit( clip, pSrc, (float)src.x, (float)src.y, (float)src.w, (float)src.h, (float)dest.x, (float)dest.y, (float)dest.w, (float)dest.h, false );
	}
	
	void SoftGfxDevice::clipStretchBlit( const Rect& clip, const Surface_p& pSrc, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias)
	{
		if( m_bBilinearFiltering )
		{
			if( sw < dw )
				sw--;
	
			if( sh < dh )
				sh--;
		}
	
		float cx = std::max(float(clip.x), dx);
		float cy = std::max(float(clip.y), dy);
		float cw = std::min(float(clip.x + clip.w), dx + dw) - cx;
		float ch = std::min(float(clip.y + clip.h), dy + dh) - cy;
	
		if(cw <= 0 || ch <= 0)
			return;
	
		if( dw > cw )
		{
			float	sdxr = sw / dw;			// Source/Destination X Ratio.
	
			sw = sdxr * cw;
	
			if( dx < cx )
				sx += sdxr * (cx - dx);
		}
	
		if( dh > ch )
		{
			float	sdyr = sh / dh;			// Source/Destination Y Ratio.
	
			sh = sdyr * ch;
	
			if( dy < cy )
				sy += sdyr * (cy - dy);
		}
	
		stretchBlitSubPixel( pSrc, sx, sy, sw, sh, cx, cy, cw, ch, bTriLinear, mipBias );
	}
	
	
	//____ stretchBlitSubPixel() ___________________________________________________
	
	void SoftGfxDevice::stretchBlitSubPixel( const Surface_p& _pSrcSurf, float sx, float sy, float sw, float sh,
							   		 float _dx, float _dy, float _dw, float _dh, bool bTriLinear, float mipBias )
	{
		if( !_pSrcSurf || !m_pCanvas || !_pSrcSurf->isInstanceOf(SoftSurface::CLASSNAME) )
			return;
	
		SoftSurface * pSrcSurf = (SoftSurface*) _pSrcSurf.rawPtr();
	
		if( !m_pCanvas->m_pData || !pSrcSurf->m_pData )
			return;
	
		int dx = (int) _dx;
		int dy = (int) _dy;
		int dw = (int) _dw;
		int dh = (int) _dh;
	
		WgBlendMode		blendMode = m_blendMode;
		if( pSrcSurf->m_pixelFormat.bits == 24 && blendMode == WG_BLENDMODE_BLEND && m_tintColor.a == 255 )
			blendMode = WG_BLENDMODE_OPAQUE;
	
		if( m_tintColor == 0xFFFFFFFF )
		{
			switch( blendMode )
			{
				case WG_BLENDMODE_OPAQUE:
					_stretchBlitOpaque( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_BLEND:
					if( pSrcSurf->m_pixelFormat.bits == 24 )
						assert(0);							// SHOULD NEVER GET HERE!
					else
						_stretchBlitBlend32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_ADD:
					if( pSrcSurf->m_pixelFormat.bits == 24 )
						_stretchBlitAdd24( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					else
						_stretchBlitAdd32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_MULTIPLY:
					_stretchBlitMultiply( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_INVERT:
					_stretchBlitInvert( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
			}
		}
		else
		{
			switch( blendMode )
			{
				case WG_BLENDMODE_OPAQUE:
					_stretchBlitTintedOpaque( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_BLEND:
					if( pSrcSurf->m_pixelFormat.bits == 24 )
						_stretchBlitTintedBlend24( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					else
						_stretchBlitTintedBlend32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_ADD:
					if( pSrcSurf->m_pixelFormat.bits == 24 )
						_stretchBlitTintedAdd24( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					else
						_stretchBlitTintedAdd32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_MULTIPLY:
					_stretchBlitTintedMultiply( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
				case WG_BLENDMODE_INVERT:
					_stretchBlitTintedInvert( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
					break;
			}
		}
	}
	
	
	#define STRETCHBLIT( _bReadAlpha_, _init_, _loop_ )										\
	{																						\
		int srcPixelBytes = pSrcSurf->m_pixelFormat.bits/8;									\
		int dstPixelBytes = m_pCanvas->m_pixelFormat.bits/8;								\
																							\
		int	srcPitch = pSrcSurf->m_pitch;													\
		int	dstPitch = m_pCanvas->m_pitch;													\
																							\
		_init_																				\
																							\
		if( m_bBilinearFiltering )															\
		{																					\
			int ofsY = (int) (sy*32768);		/* We use 15 binals for all calculations */	\
			int incY = (int) (sh*32768/dh);													\
																							\
			for( int y = 0 ; y < dh ; y++ )													\
			{																				\
				int fracY2 = ofsY & 0x7FFF;													\
				int fracY1 = 32768 - fracY2;												\
																							\
				int ofsX = (int) (sx*32768);												\
				int incX = (int) (sw*32768/dw);												\
																							\
				Uint8 * pDst = m_pCanvas->m_pData + (dy+y) * m_pCanvas->m_pitch + dx * dstPixelBytes;	\
				Uint8 * pSrc = pSrcSurf->m_pData + (ofsY>>15) * pSrcSurf->m_pitch;						\
																							\
				for( int x = 0 ; x < dw ; x++ )												\
				{																			\
					int fracX2 = ofsX & 0x7FFF;												\
					int fracX1 = 32768 - fracX2;											\
																							\
					Uint8 * p = pSrc + (ofsX >> 15)*srcPixelBytes;							\
																							\
					int mul11 = fracX1*fracY1 >> 15;										\
					int mul12 = fracX2*fracY1 >> 15;										\
					int mul21 = fracX1*fracY2 >> 15;										\
					int mul22 = fracX2*fracY2 >> 15;										\
																							\
					int srcBlue = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15; 	\
					p++;																												\
					int srcGreen = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15;	\
					p++;																												\
					int srcRed = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15;		\
					int srcAlpha;																										\
					if( _bReadAlpha_ )																									\
					{																													\
						p++;																											\
						srcAlpha = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15;	\
					}																		\
																							\
					_loop_																	\
																							\
					ofsX += incX;															\
					pDst += dstPixelBytes;													\
				}																			\
				ofsY += incY;																\
			}																				\
		}																					\
		else	/* UNFILTERED */															\
		{																					\
			int ofsY = (int) (sy*32768);		/* We use 15 binals for all calculations */	\
			int incY = (int) (sh*32768/dh);													\
																							\
			for( int y = 0 ; y < dh ; y++ )													\
			{																				\
				int ofsX = (int) (sx*32768);												\
				int incX = (int) (sw*32768/dw);												\
																							\
				Uint8 * pDst = m_pCanvas->m_pData + (dy+y) * m_pCanvas->m_pitch + dx * dstPixelBytes;	\
				Uint8 * pSrc = pSrcSurf->m_pData + (ofsY>>15) * pSrcSurf->m_pitch;						\
																							\
				for( int x = 0 ; x < dw ; x++ )												\
				{																			\
					Uint8 * p = pSrc + (ofsX >> 15)*srcPixelBytes;							\
																							\
					int srcBlue = p[0]; 													\
					int srcGreen = p[1];													\
					int srcRed = p[2];														\
					int srcAlpha;															\
					if( _bReadAlpha_ )														\
						srcAlpha = p[3];													\
																							\
					_loop_																	\
																							\
					ofsX += incX;															\
					pDst += dstPixelBytes;													\
				}																			\
				ofsY += incY;																\
			}																				\
		}																					\
	}																						\
	
	
	
	//____ _stretchBlitTintedOpaque() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitTintedOpaque( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
															int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		int tintRed = (int) m_tintColor.r;
		int tintGreen = (int) m_tintColor.g;
		int tintBlue = (int) m_tintColor.b;
	
		,
	
		pDst[0] = m_pDivTab[srcBlue*tintBlue];
		pDst[1] = m_pDivTab[srcGreen*tintGreen];
		pDst[2] = m_pDivTab[srcRed*tintRed];
	
		)
	}
	
	//____ _stretchBlitTintedBlend32() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitTintedBlend32( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
															 int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( true,
	
		int tintAlpha = (int) m_tintColor.a;
		int tintRed = (int) m_tintColor.r;
		int tintGreen = (int) m_tintColor.g;
		int tintBlue = (int) m_tintColor.b;
	
		,
	
		int alpha = m_pDivTab[srcAlpha*tintAlpha];
		int invAlpha = 255-alpha;
	
		srcBlue = m_pDivTab[srcBlue * tintBlue];
		srcGreen = m_pDivTab[srcGreen * tintGreen];
		srcRed = m_pDivTab[srcRed * tintRed];
	
	
		pDst[0] = m_pDivTab[pDst[0]*invAlpha + srcBlue*alpha];
		pDst[1] = m_pDivTab[pDst[1]*invAlpha + srcGreen*alpha];
		pDst[2] = m_pDivTab[pDst[2]*invAlpha + srcRed*alpha];
	
		)
	}
	
	//____ _stretchBlitTintedBlend24() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitTintedBlend24( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
															 int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		int tintAlpha = (int) m_tintColor.a;
		int tintRed = (int) m_pDivTab[m_tintColor.r * tintAlpha];
		int tintGreen = (int) m_pDivTab[m_tintColor.g * tintAlpha];
		int tintBlue = (int) m_pDivTab[m_tintColor.b * tintAlpha];
		int invAlpha = 255-tintAlpha;
	
		,
	
		pDst[0] = m_pDivTab[pDst[0]*invAlpha + srcBlue*tintBlue];
		pDst[1] = m_pDivTab[pDst[1]*invAlpha + srcGreen*tintGreen];
		pDst[2] = m_pDivTab[pDst[2]*invAlpha + srcRed*tintRed];
	
		)
	}
	
	
	//____ _stretchBlitTintedAdd32() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitTintedAdd32( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
															int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( true,
	
		int tintAlpha = (int) m_tintColor.a;
		int tintRed = (int) m_tintColor.r;
		int tintGreen = (int) m_tintColor.g;
		int tintBlue = (int) m_tintColor.b;
	
		,
	
		int alpha = m_pDivTab[srcAlpha*tintAlpha];
	
		srcBlue = m_pDivTab[srcBlue * tintBlue];
		srcGreen = m_pDivTab[srcGreen * tintGreen];
		srcRed = m_pDivTab[srcRed * tintRed];
	
		pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*alpha] ];
		pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*alpha] ];
		pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[srcRed*alpha] ];
	
		)
	}
	
	
	//____ _stretchBlitTintedAdd24() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitTintedAdd24( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
															int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		int tintAlpha = (int) m_tintColor.a;
		int tintRed = (int) m_pDivTab[m_tintColor.r * tintAlpha];
		int tintGreen = (int) m_pDivTab[m_tintColor.g * tintAlpha];
		int tintBlue = (int) m_pDivTab[m_tintColor.b * tintAlpha];
	
		,
	
		pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*tintBlue]];
		pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*tintGreen]];
		pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[srcRed*tintRed]];
	
		)
	}
	
	
	//____ _stretchBlitTintedMultiply() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitTintedMultiply( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
															  int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		int tintRed = (int) m_tintColor.r;
		int tintGreen = (int) m_tintColor.g;
		int tintBlue = (int) m_tintColor.b;
	
		,
	
		srcBlue = m_pDivTab[srcBlue * tintBlue];
		srcGreen = m_pDivTab[srcGreen * tintGreen];
		srcRed = m_pDivTab[srcRed * tintRed];
	
		pDst[0] = m_pDivTab[pDst[0]*srcBlue];
		pDst[1] = m_pDivTab[pDst[1]*srcGreen];
		pDst[2] = m_pDivTab[pDst[2]*srcRed];
	
		)
	}
	
	//____ _stretchBlitTintedInvert() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitTintedInvert( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
												      int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		int tintRed = (int) m_tintColor.r;
		int tintGreen = (int) m_tintColor.g;
		int tintBlue = (int) m_tintColor.b;
	
		,
	
		srcBlue = m_pDivTab[srcBlue * tintBlue];
		srcGreen = m_pDivTab[srcGreen * tintGreen];
		srcRed = m_pDivTab[srcRed * tintRed];
	
		pDst[0] = m_pDivTab[srcBlue*(255-pDst[0]) + pDst[0]*(255-srcBlue)];
		pDst[1] = m_pDivTab[srcGreen*(255-pDst[1]) + pDst[1]*(255-srcGreen)];
		pDst[2] = m_pDivTab[srcRed*(255-pDst[2]) + pDst[2]*(255-srcRed)];
	
		)
	}
	
	
	//____ _stretchBlitOpaque() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitOpaque( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
													  int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		,
	
		pDst[0] = srcBlue;
		pDst[1] = srcGreen;
		pDst[2] = srcRed;
	
		)
	}
	
	//____ _stretchBlitBlend32() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitBlend32( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
													   int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( true,
	
		,
	
		int invAlpha = 255-srcAlpha;
	
		pDst[0] = m_pDivTab[pDst[0]*invAlpha + srcBlue*srcAlpha];
		pDst[1] = m_pDivTab[pDst[1]*invAlpha + srcGreen*srcAlpha];
		pDst[2] = m_pDivTab[pDst[2]*invAlpha + srcRed*srcAlpha];
	
		)
	}
	
	//____ _stretchBlitAdd32() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitAdd32( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
													 int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( true,
	
		,
	
		pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*srcAlpha] ];
		pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*srcAlpha] ];
		pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[srcRed*srcAlpha] ];
	
		)
	}
	
	
	//____ _stretchBlitAdd24() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitAdd24( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
													 int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		,
	
		pDst[0] = m_limitTable[pDst[0] + srcBlue];
		pDst[1] = m_limitTable[pDst[1] + srcGreen];
		pDst[2] = m_limitTable[pDst[2] + srcRed];
	
		)
	}
	
	
	//____ _stretchBlitMultiply() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitMultiply( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
													    int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		,
	
		pDst[0] = m_pDivTab[pDst[0]*srcBlue];
		pDst[1] = m_pDivTab[pDst[1]*srcGreen];
		pDst[2] = m_pDivTab[pDst[2]*srcRed];
	
		)
	}
	
	//____ _stretchBlitInvert() ____________________________________________
	
	void SoftGfxDevice::_stretchBlitInvert( const SoftSurface * pSrcSurf, float sx, float sy, float sw, float sh,
												      int dx, int dy, int dw, int dh )
	{
		STRETCHBLIT( false,
	
		,
	
		pDst[0] = m_pDivTab[(srcBlue*(255-pDst[0]) + pDst[0]*(255-srcBlue))];
		pDst[1] = m_pDivTab[(srcGreen*(255-pDst[1]) + pDst[1]*(255-srcGreen))];
		pDst[2] = m_pDivTab[(srcRed*(255-pDst[2]) + pDst[2]*(255-srcRed))];
	
		)
	}
	
	
	
	//____ _initTables() ___________________________________________________________
	
	void SoftGfxDevice::_initTables()
	{
		// Init limitTable
	
		for( int i = 0 ; i < 256 ; i++ )
			m_limitTable[i] = i;
	
		for( int i = 256 ; i < 512 ; i++ )
			m_limitTable[i] = 255;
	
		// Init divTable
	
		m_pDivTab = new Uint8[65536];
	
		for( int i = 0 ; i < 65536 ; i++ )
			m_pDivTab[i] = i / 255;
	
	}

} // namespace wg