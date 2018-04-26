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

#include <algorithm>

#include <wg_gfxdevice.h>

#include <wg_gfxdevice.h>
#include <wg_surface.h>
#include <wg_geo.h>
#include <wg_blockset.h>
#include <wg_text.h>
#include <wg_cursorinstance.h>
#include <wg_font.h>
#include <wg_gfxanim.h>
#include <wg_util.h>
#include <wg_pen.h>

int		WgGfxDevice::s_gfxDeviceCount = 0;
int *	WgGfxDevice::s_pCurveTab = nullptr;

//____ Constructor _____________________________________________________________

WgGfxDevice::WgGfxDevice( WgSize canvasSize )
{
	m_pCanvas			= nullptr;
	m_tintColor 		= WgColor(255,255,255);
	m_blendMode 		= WG_BLENDMODE_BLEND;
	m_bSaveDirtyRects 	= false;
	m_renderFlags		= 0;
	m_canvasSize		= canvasSize;

	if (s_gfxDeviceCount == 0)
	{
		_genCurveTab();
	}
	s_gfxDeviceCount++;
}

//____ Destructor _________________________________________________________

WgGfxDevice::~WgGfxDevice()
{
	s_gfxDeviceCount--;
	if (s_gfxDeviceCount == 0)
	{
		delete[] s_pCurveTab;
	}
}

//____ SetSaveDirtyRects() _____________________________________________________

bool WgGfxDevice::SetSaveDirtyRects( bool bSave )
{
	m_bSaveDirtyRects = bSave;
	return true;
}

//____ SetTintColor() __________________________________________________________

void WgGfxDevice::SetTintColor( WgColor color )
{
	m_tintColor = color;
}

//____ SetBlendMode() __________________________________________________________

bool WgGfxDevice::SetBlendMode( WgBlendMode blendMode )
{
	m_blendMode = blendMode;
	return false;				// Not implemented.
}

//______________________________________________________________________________
Uint32 WgGfxDevice::SetRenderFlags( Uint32 flags )
{
	Uint32 oldFlags = m_renderFlags;
	m_renderFlags = flags;
	return oldFlags;
}

//____ BeginRender() ___________________________________________________________

bool WgGfxDevice::BeginRender()
{
	return true;	// Assumed to be ok if device doesn't have its own method.
}

//____ EndRender() _____________________________________________________________

bool WgGfxDevice::EndRender()
{
	return true;	// Assumed to be ok if device doesn't have its own method.
}

//____ ClipDrawLine() _________________________________________________________

// Coordinates for start are considered to be + 0.5 in the width dimension, so they start in the middle of a line/column.
// A one pixel thick line will only be drawn one pixel think, while a two pixels thick line will cover three pixels in thickness,
// where the outer pixels are faded.

void WgGfxDevice::ClipDrawLine(const WgRect& clip, const WgCoord& _begin, WgDirection dir, int length, WgColor _col, float thickness)
{
	if (thickness <= 0.f)
		return;

	WgCoord begin = _begin;

	switch (dir)
	{
	case WG_LEFT:
		begin.x -= length;
	case WG_RIGHT:
	{
		if (begin.x > clip.x + clip.w)
			return;

		if (begin.x < clip.x)
		{
			length -= clip.x - begin.x;
			if (length <= 0)
				return;
			begin.x = clip.x;
		}

		if (begin.x + length > clip.x + clip.w)
		{
			length = clip.x + clip.w - begin.x;
			if (length <= 0)
				return;
		}

		if (thickness <= 1.f)
		{
			if (begin.y < clip.y || begin.y >= clip.y + clip.h)
				return;

			WgColor col = _col;
			col.a = (uint8_t) (thickness * col.a);

			_drawHorrLine(begin, length, col);
		}
		else
		{
			int expanse = (int) 1 + (thickness - 1) / 2;
			WgColor edgeColor( _col.r, _col.g, _col.b, _col.a * ((thickness - 1) / 2 - (expanse - 1)));

			if (begin.y + expanse <= clip.y || begin.y - expanse >= clip.y + clip.h)
				return;

			int beginY = begin.y - expanse;
			int endY = begin.y + expanse+1;

			if (beginY < clip.y)
				beginY = clip.y - 1;
			else
				_drawHorrLine({ begin.x, beginY }, length, edgeColor);

			if (endY > clip.y + clip.h)
				endY = clip.y + clip.h + 1;
			else
				_drawHorrLine({ begin.x, endY-1 }, length, edgeColor);

			Fill({ begin.x, beginY + 1, length, endY - beginY - 2 }, _col );
		}

		break;
	}
	case WG_UP:
		begin.y -= length;
	case WG_DOWN:
		if (begin.y > clip.y + clip.h )
			return;

		if (begin.y < clip.y)
		{
			length -= clip.y - begin.y;
			if (length <= 0)
				return;
			begin.y = clip.y;
		}

		if (begin.y + length > clip.y + clip.h)
		{
			length = clip.y + clip.h - begin.y;
			if (length <= 0)
				return;
		}

		if (thickness <= 1.f)
		{
			if (begin.x < clip.x || begin.x >= clip.x + clip.w)
				return;

			WgColor col = _col;
			col.a = (uint8_t)(thickness * col.a);

			_drawVertLine(begin, length, col);
		}
		else
		{
			int expanse = (int)1 + (thickness - 1) / 2;
			WgColor edgeColor(_col.r, _col.g, _col.b, _col.a * ((thickness - 1) / 2 - (expanse - 1)));

			if (begin.x + expanse <= clip.x || begin.x - expanse >= clip.x + clip.w)
				return;

			int beginX = begin.x - expanse;
			int endX = begin.x + expanse+1;

			if (beginX < clip.x)
				beginX = clip.x - 1;
			else
				_drawVertLine({ beginX, begin.y }, length, edgeColor);

			if (endX > clip.x + clip.w)
				endX = clip.x + clip.w + 1;
			else
				_drawVertLine({ endX-1, begin.y }, length, edgeColor);

			Fill({ beginX + 1, begin.y, endX - beginX - 2, length }, _col);
		}

		break;
	}
}


//_____ ClipBlitFromCanvas() ______________________________________________________

void WgGfxDevice::ClipBlitFromCanvas(const WgRect& clip, const WgSurface* pSrc, const WgRect& src, int dx, int dy)
{
	ClipBlit(clip, pSrc, src, dx, dy);		// Default is a normal blit, only OpenGL needs to flip (until that has been fixed)
}


//____ Blit() __________________________________________________________________

void WgGfxDevice::Blit( const WgSurface* pSrc )
{
	Blit( pSrc, WgRect( 0, 0, pSrc->Width(), pSrc->Height() ), 0, 0 );
}

void WgGfxDevice::Blit( const WgSurface* pSrc, int dx, int dy )
{
	Blit( pSrc, WgRect( 0, 0, pSrc->Width(), pSrc->Height() ), dx, dy );
}

//____ StretchBlit() ___________________________________________________________

void WgGfxDevice::StretchBlit( const WgSurface * pSrc, bool bTriLinear, float mipmapBias )
{
	StretchBlit( pSrc, WgRect(0, 0, pSrc->Width(),pSrc->Height()), WgRect(0,0,m_canvasSize.w,m_canvasSize.h), bTriLinear, mipmapBias );
}

void WgGfxDevice::StretchBlit( const WgSurface * pSrc, const WgRect& dest, bool bTriLinear, float mipmapBias )
{
	StretchBlit( pSrc, WgRect(0, 0, pSrc->Width(),pSrc->Height()), dest, bTriLinear, mipmapBias );
}

void WgGfxDevice::StretchBlit( const WgSurface * pSrc, const WgRect& src, const WgRect& dest, bool bTriLinear, float mipmapBias )
{
	float srcW = (float) src.w;
	float srcH = (float) src.h;

	float destW = (float) dest.w;
	float destH = (float) dest.h;

	if( pSrc->scaleMode() == WG_SCALEMODE_INTERPOLATE )
	{
		if( srcW < destW )
			srcW--;

		if( srcH < destH )
			srcH--;
	}

	StretchBlitSubPixel( pSrc, (float) src.x, (float) src.y, srcW, srcH, (float) dest.x, (float) dest.y, destW, destH, bTriLinear, mipmapBias );
}

//____ TileBlit() ______________________________________________________________

void WgGfxDevice::TileBlit( const WgSurface* _pSrc )
{
	TileBlit( _pSrc, WgRect( 0, 0, _pSrc->Width(), _pSrc->Height() ), WgRect(0,0,m_canvasSize) );
}

void WgGfxDevice::TileBlit( const WgSurface* _pSrc, const WgRect& _dest )
{
	TileBlit( _pSrc, WgRect( 0, 0, _pSrc->Width(), _pSrc->Height() ), _dest );
}

void WgGfxDevice::TileBlit( const WgSurface* _pSrc, const WgRect& _src, const WgRect& _dest )
{
	if( !_pSrc || _dest.h == 0 || _dest.w == 0 )
		return;

	WgRect	r = _src;
	WgRect	r2 = _src;

	int nCol = _dest.w / _src.w;
	r2.w = _dest.w % _src.w;

	int nRow = (_dest.h+(_src.h-1))/ _src.h;	// Including any cut row....

	int		destX = _dest.x;
	int		destY = _dest.y;

	for( int row = 1 ; row <= nRow ; row++ )
	{
		// Possibly cut the height if this is the last row...

		if( row == nRow )
		{
			r.h = _dest.y + _dest.h - destY;
			r2.h = r.h;
		}

		// Blit a row.

		for( int col = 0 ; col < nCol ; col++ )
		{
			Blit( _pSrc, r, destX, destY );
			destX += r.w;
		}

		// Blit any left over part at end of row.

		if( r2.w > 0 )
			Blit( _pSrc, r2, destX, destY );

		destX = _dest.x;
		destY += _src.h;
	}
	return;
}


//____ ClipFill() ______________________________________________________________

void WgGfxDevice::ClipFill( const WgRect& _clip, const WgRect& _rect, const WgColor& _col )
{
	Fill( WgRect( _clip, _rect ), _col );
}

//____ ClipBlit() ______________________________________________________________

void WgGfxDevice::ClipBlit( const WgRect& clip, const WgSurface* pSrc )
{
	ClipBlit( clip, pSrc, WgRect(0,0,pSrc->Width(),pSrc->Height()), 0, 0 );
}

void WgGfxDevice::ClipBlit( const WgRect& clip, const WgSurface* pSrc, int dx, int dy  )
{
	ClipBlit( clip, pSrc, WgRect(0,0,pSrc->Width(),pSrc->Height()), dx, dy );
}

void WgGfxDevice::ClipBlit( const WgRect& clip, const WgSurface* pSrc, const WgRect& srcRect, int dx, int dy  )
{
	if( (clip.x <= dx) && (clip.x + clip.w > dx + srcRect.w) &&
      (clip.y <= dy) && (clip.y + clip.h > dy + srcRect.h) )
	{
		Blit( pSrc, srcRect, dx, dy );														// Totally inside clip-rect.
		return;
	}

	if( (clip.x > dx + srcRect.w) || (clip.x + clip.w < dx) ||
      (clip.y > dy + srcRect.h) || (clip.y + clip.h < dy) )
		return;																						// Totally outside clip-rect.

	// Do Clipping

	WgRect	newSrc = srcRect;

	if( dx < clip.x )
	{
		newSrc.w -= clip.x - dx;
		newSrc.x += clip.x - dx;
		dx = clip.x;
	}

	if( dy < clip.y )
	{
		newSrc.h -= clip.y - dy;
		newSrc.y += clip.y - dy;
		dy = clip.y;
	}

	if( dx + newSrc.w > clip.x + clip.w )
		newSrc.w = (clip.x + clip.w) - dx;

	if( dy + newSrc.h > clip.y + clip.h )
		newSrc.h = (clip.y + clip.h) - dy;


	Blit( pSrc, newSrc, dx, dy );
}

//____ ClipStretchBlit() _______________________________________________________

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, bool bTriLinear, float mipBias )
{
	ClipStretchBlit( clip, pSrc, WgRect(0,0,pSrc->Width(), pSrc->Height()), WgRect( 0,0,m_canvasSize), bTriLinear, mipBias );
}

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, const WgRect& dest, bool bTriLinear, float mipBias )
{
	ClipStretchBlit( clip, pSrc, WgRect(0,0,pSrc->Width(), pSrc->Height()), dest, bTriLinear, mipBias );
}

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, const WgRect& src, const WgRect& dest, bool bTriLinear, float mipBias )
{
	ClipStretchBlit( clip, pSrc, (float)src.x, (float)src.y, (float)src.w, (float)src.h, (float)dest.x, (float)dest.y, (float)dest.w, (float)dest.h, false );
}

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias)
{
	if( pSrc->scaleMode() == WG_SCALEMODE_INTERPOLATE )
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

	StretchBlitSubPixel( pSrc, sx, sy, sw, sh, cx, cy, cw, ch, bTriLinear, mipBias );
}

//____ ClipTileBlit() __________________________________________________________

void WgGfxDevice::ClipTileBlit( const WgRect& clip, const WgSurface* pSrc )
{
	ClipTileBlit( clip, pSrc, WgRect( 0,0,pSrc->Width(),pSrc->Height() ),
				  WgRect(0,0,m_canvasSize) );
}

void WgGfxDevice::ClipTileBlit( const WgRect& clip, const WgSurface* pSrc,
								  const WgRect& dest )
{
	ClipTileBlit( clip, pSrc, WgRect(0,0,pSrc->Width(),pSrc->Height()), dest );
}


void WgGfxDevice::ClipTileBlit( const WgRect& _clip, const WgSurface* _pSrc, const WgRect& _src, const WgRect& _dest )
{
	if( !_pSrc )
		return;

	WgRect	myRect;

	WgRect	clip;
	if( !clip.Intersection( _dest, _clip ) )
		return;

	// Take care of start-offset change caused by clipping.

	int		xStart = (clip.x - _dest.x) % _src.w;
	if( xStart < 0 )
		xStart += _src.w;
	xStart += _src.x;

	int		yStart = (clip.y - _dest.y) % _src.h;
	if( yStart < 0 )
		yStart += _src.h;
	yStart += _src.y;


	int		destY = clip.y, destX;

	myRect.y = yStart;
	myRect.h =_src.y + _src.h - yStart;

	while( destY < clip.y + clip.h )
	{
		if( myRect.h > clip.y + clip.h - destY )
			myRect.h = clip.y + clip.h - destY;

		myRect.x = xStart;
		myRect.w = _src.x + _src.w - xStart;
		if( myRect.w > clip.w )
			myRect.w = clip.w;


		// Blit a row.

		destX = clip.x;
		Blit( _pSrc, myRect, destX, destY );
		destX += myRect.w;
		myRect.x = _src.x;
		myRect.w = _src.w;

		while( destX <= clip.x + clip.w - _src.w )
		{
			Blit( _pSrc, myRect, destX, destY );
			destX += myRect.w;
		}
		myRect.w = clip.x + clip.w - destX;
		if( myRect.w > 0 )
			Blit( _pSrc, myRect, destX, destY );

		destY += myRect.h;
		myRect.y = _src.y;
		myRect.h = _src.h;
	}
	return;
}

//____ BlitBlock() ____________________________________________________________

void WgGfxDevice::BlitBlock( const WgBlock& _block, const WgRect& _dest2, bool bTriLinear, float mipmapbias )
{
	// Isn't worth the time to reimplement, just call the slightly slower version.

	ClipBlitBlock( _dest2, _block, _dest2, bTriLinear, mipmapbias );
}


//____ ClipBlitBlock() ________________________________________________________

void WgGfxDevice::ClipBlitBlock( const WgRect& _clip, const WgBlock& _block, const WgRect& _canvas, bool bTriLinear, float mipmapbias)
{
	if( !_block.Surface() )
		return;

	if( _block.IsSkipable() )
		return;

    const WgBorders&    sourceBorders = _block.SourceFrame();
    const WgBorders     canvasBorders = _block.CanvasFrame();

	// Shortcuts & optimizations for common special cases.

	WgRect canvas = _canvas;
	WgUtil::AdjustScaledArea(_block, canvas);

	const WgRect& src = _block.Rect();
	const WgSurface * pSurf = _block.Surface();

	if( sourceBorders == canvasBorders )
	{
		if( src.w == canvas.w && src.h == canvas.h )
		{
			ClipBlit( _clip, pSurf, src, canvas.x, canvas.y );
			return;
		}

		if( !_block.HasBorders() )
		{
	        ClipStretchBlit( _clip, pSurf, src, canvas );
			return;
		}
	    
		if( src.w == canvas.w )
		{
			ClipBlitVertStretchBar( _clip, pSurf, src, sourceBorders, canvas, canvasBorders );
			return;
		}
	}

	// Render upper row (top-left corner, top stretch area and top-right corner)
	
	if( canvasBorders.top > 0 )
	{
		WgRect sourceRect( src.x, src.y, src.w, sourceBorders.top );
        WgRect destRect( canvas.x, canvas.y, canvas.w, canvasBorders.top );

        ClipBlitHorrStretchBar( _clip, pSurf, sourceRect, sourceBorders, destRect, canvasBorders );
	}

    // Render mid row (left and right stretch area and middle section)
    
    if( canvas.h - canvasBorders.Height() > 0 )
    {
        WgRect sourceRect( src.x, src.y + sourceBorders.top, src.w, src.h - sourceBorders.Height() );
        WgRect destRect( canvas.x, canvas.y + canvasBorders.top, canvas.w, canvas.h - canvasBorders.Height() );
        
        ClipBlitHorrStretchBar( _clip, pSurf, sourceRect, sourceBorders, destRect, canvasBorders );
    }
    
	// Render lowest row (bottom-left corner, bottom stretch area and bottom-right corner)
	
	if( canvasBorders.bottom > 0 )
	{
		WgRect sourceRect( src.x, src.y + src.h - sourceBorders.bottom, src.w, sourceBorders.bottom );
        WgRect destRect( canvas.x, canvas.y + canvas.h - canvasBorders.bottom, canvas.w, canvasBorders.bottom );
	
        ClipBlitHorrStretchBar( _clip, pSurf, sourceRect, sourceBorders, destRect, canvasBorders );
	}

}

//____ ClipBlitHorrStretchBar() ______________________________________________________

void WgGfxDevice::ClipBlitHorrStretchBar(	const WgRect& _clip, const WgSurface * _pSurf,
                                            const WgRect& _src, const WgBorders& _srcBorders,
                                            const WgRect& _dest, const WgBorders& _destBorders )
{
    /*
     This can be optimized by handling clipping directly instead of calling clipBlit().
     */
    
    // Blit left edge
    
    WgRect	src( _src.x, _src.y, _srcBorders.left, _src.h );
    WgRect  dest( _dest.x, _dest.y, _destBorders.left, _dest.h );

    ClipStretchBlit( _clip, _pSurf, src, dest );

    src.x += src.w;
    src.w = _src.w - _srcBorders.Width();
    dest.x += dest.w;
    dest.w = _dest.w - _destBorders.Width();
    
    ClipStretchBlit( _clip, _pSurf, src, dest );
    
    src.x += src.w;
    src.w = _srcBorders.right;
    dest.x += dest.w;
    dest.w = _destBorders.right;

    ClipStretchBlit( _clip, _pSurf, src, dest );
}

//____ ClipBlitVertStretchBar() ______________________________________________________

void WgGfxDevice::ClipBlitVertStretchBar(	const WgRect& _clip, const WgSurface * _pSurf,
                                         const WgRect& _src, const WgBorders& _srcBorders,
                                         const WgRect& _dest, const WgBorders& _destBorders )
{
    /*
     This can be optimized by handling clipping directly instead of calling clipBlit().
     */
    
    // Blit left edge
    
    WgRect	src( _src.x, _src.y, _src.w, _srcBorders.top );
    WgRect  dest( _dest.x, _dest.y, _dest.w, _destBorders.top );
    
    ClipStretchBlit( _clip, _pSurf, src, dest );
    
    src.y += src.h;
    src.h = _src.h - _srcBorders.Height();
    dest.y += dest.h;
    dest.h = _dest.h - _destBorders.Height();
    
    ClipStretchBlit( _clip, _pSurf, src, dest );
    
    src.y += src.h;
    src.h = _srcBorders.bottom;
    dest.y += dest.h;
    dest.h = _destBorders.bottom;
    
    ClipStretchBlit( _clip, _pSurf, src, dest );
}


//____ ClipBlitHorrBar() ______________________________________________________

void WgGfxDevice::ClipBlitHorrBar(	const WgRect& _clip, const WgSurface * _pSurf,
								  	const WgRect& _src, const WgBorders& _borders,
								  	bool _bTile, int _dx, int _dy, int _len )
{
	/*
		This can be optimized by handling clipping directly instead of calling clipBlit().
	*/

	// Blit left edge

	WgRect	r( _src.x, _src.y, _borders.left, _src.h );
	ClipBlit( _clip, _pSurf, r, _dx, _dy );

	_len -= _borders.Width();			// Remove left and right edges from len.
	_dx += _borders.left;

	// Blit tiling part

	r.x += _borders.left;
	r.w = _src.w - _borders.Width();

	if( _bTile )
	{
		while( _len > r.w )
		{
			ClipBlit( _clip, _pSurf, r, _dx, _dy );
			_len -= r.w;
			_dx += r.w;
		}
		if( _len != 0 )
		{
			r.w = _len;
			ClipBlit( _clip, _pSurf, r, _dx, _dy );
			_dx += _len;
		}
	}
	else
	{
		ClipStretchBlit( _clip, _pSurf, r, WgRect( _dx, _dy, _len, r.h ) );
		_dx += _len;
	}

	// Blit right edge

	r.x = _src.x + _src.w - _borders.right;
	r.w = _borders.right;
	ClipBlit( _clip, _pSurf, r, _dx, _dy );
}

//____ ClipBlitVertBar() ______________________________________________________

void WgGfxDevice::ClipBlitVertBar(	const WgRect& _clip, const WgSurface * _pSurf,
								  	const WgRect& _src, const WgBorders& _borders,
								  	bool _bTile, int _dx, int _dy, int _len )
{
	/*
		This can be optimized by handling clipping directly instead of calling clipBlit().
	*/

	// Blit top edge

	WgRect	r( _src.x, _src.y, _src.w, _borders.top );
	ClipBlit( _clip, _pSurf, r, _dx, _dy );

	_len -= _borders.Height();			// Remove top and bottom edges from len.
	_dy += _borders.top;

	// Blit tiling part

	r.y += _borders.top;
	r.h = _src.h - _borders.Height();

	if( _bTile )
	{
		while( _len > r.h )
		{
			ClipBlit( _clip, _pSurf, r, _dx, _dy );
			_len -= r.h;
			_dy += r.h;
		}
		if( _len != 0 )
		{
			r.h = _len;
			ClipBlit( _clip, _pSurf, r, _dx, _dy );
			_dy += _len;
		}
	}
	else
	{
		ClipStretchBlit( _clip, _pSurf, r, WgRect( _dx, _dy, r.w, _len ) );
		_dy += _len;
	}

	// Blit bottom edge

	r.y = _src.y + _src.h - _borders.bottom;
	r.h = _borders.bottom;
	ClipBlit( _clip, _pSurf, r, _dx, _dy );
}


//____ BlitHorrBar() __________________________________________________________

void WgGfxDevice::BlitHorrBar(	const WgSurface * _pSurf, const WgRect& _src,
								const WgBorders& _borders, bool _bTile,
								int _dx, int _dy, int _len )
{
	// Blit left edge

	WgRect	r( _src.x, _src.y, _borders.left, _src.h );
	Blit( _pSurf, r, _dx, _dy );

	_len -= _borders.Width();			// Remove left and right edges from len.
	_dx += _borders.left;

	// Blit tiling part

	r.x += _borders.left;
	r.w = _src.w - _borders.Width();

	if( _bTile )
	{
		while( _len > r.w )
		{
			Blit( _pSurf, r, _dx, _dy );
			_len -= r.w;
			_dx += r.w;
		}
		if( _len != 0 )
		{
			r.w = _len;
			Blit( _pSurf, r, _dx, _dy );
			_dx += _len;
		}
	}
	else
	{
		StretchBlit( _pSurf, r, WgRect( _dx, _dy, _len, r.h ) );
		_dx += _len;
	}

	// Blit right edge

	r.x = _src.x + _src.w - _borders.right;
	r.w = _borders.right;
	Blit( _pSurf, r, _dx, _dy );
}

//____ BlitVertBar() __________________________________________________________

void WgGfxDevice::BlitVertBar(	const WgSurface * _pSurf, const WgRect& _src,
								const WgBorders& _borders, bool _bTile,
								int _dx, int _dy, int _len )
{
	// Blit top edge

	WgRect	r( _src.x, _src.y, _src.w, _borders.top );
	Blit( _pSurf, r, _dx, _dy );

	_len -= _borders.Height();			// Remove top and bottom borders from len.
	_dy += _borders.top;

	// Blit tiling part

	r.y += _borders.top;
	r.h = _src.h - _borders.Height();

	if( _bTile )
	{
		while( _len > r.h )
		{
			Blit( _pSurf, r, _dx, _dy );
			_len -= r.h;
			_dy += r.h;
		}
		if( _len != 0 )
		{
			r.h = _len;
			Blit( _pSurf, r, _dx, _dy );
			_dy += _len;
		}
	}
	else
	{
		StretchBlit( _pSurf, r, WgRect( _dx, _dy, r.w, _len ) );
		_dy += _len;
	}

	// Blit bottom edge

	r.y = _src.y + _src.h - _borders.bottom;
	r.h = _borders.bottom;
	Blit( _pSurf, r, _dx, _dy );
}

//____ PrintText() ____________________________________________________________

bool WgGfxDevice::PrintText( const WgRect& clip, const WgText * pText, const WgRect& dest )
{
	if( !pText || dest.w <= 0 )
		return false;

	_drawTextBg(clip, pText, dest);

	WgPen pen;
	pen.SetDevice( this );
	pen.SetTextNode( pText->getNode() );
	pen.SetScale( pText->Scale() );

	WgTextAttr	attr;
	pText->GetBaseAttr(attr);

	if( attr.pFont == 0 )
		return false;

	pen.SetAttributes(attr);

	WgSize	textSize( pText->width(), pText->height() );

	if( dest.h < (int) textSize.h || dest.w < (int) textSize.w || !clip.Contains( dest ) || pText->isCursorShowing() )
		pen.SetClipRect( clip );

	const WgCursorInstance* pCursor = 0;
	int cursLine = -1, cursCol = -1;

	if( pText->isCursorShowing() )
	{
		pCursor = pText->GetCursor();
		pCursor->getSoftPos( cursLine, cursCol );
	}

	bool bEllipsisActive = false;
	if( pText->IsAutoEllipsis() && !pText->isCursorShowing() && (textSize.w > dest.w || textSize.h > dest.h) )
		bEllipsisActive = true;

	WgCoord	pos;
	pos.y = pText->LineStartY( 0, dest ) + pText->getSoftLine(0)->baseline;

	int					nLines = pText->nbSoftLines();
	const WgTextLine *	pLines = pText->getSoftLines();

	for( int i = 0 ; i < nLines ; i++ )
	{
		pos.x = pText->LineStartX( i, dest );
		pen.SetOrigo( pos );		// So tab positions will start counting from start of line.
		pen.SetPos( pos );
		pen.FlushChar();			// Remove kerning info for previous char.

		bool bLastFullyVisibleLine = false;
		if( (i < nLines-1) && (pos.y + pLines[i].lineSpacing + pLines[i+1].height - pLines[i+1].baseline > dest.y + dest.h) )
			bLastFullyVisibleLine = true;

		if( cursLine == i )
		{
			// Draw line parts, make space for cursor.

			_printTextSpan( pen, pText, pLines[i].ofs, cursCol, false );
			WgCoord cursorPos = pen.GetPos();
			pen.AdvancePosCursor( *pCursor );
			_printTextSpan( pen, pText, pLines[i].ofs + cursCol, pLines[i].nChars - cursCol, true );

			// Blit the cursor

			WgCoord restorePos = pen.GetPos();
			pen.SetPos( cursorPos );
			pen.BlitCursor( *pCursor );
			pen.SetPos( restorePos );
		}
		else
		{
			if( bEllipsisActive && (bLastFullyVisibleLine || pLines[i].width > dest.w) )
			{
				_printEllipsisTextSpan( pen, pText, pLines[i].ofs, pLines[i].nChars, dest.x + dest.w );
				break;
			}
			else
				_printTextSpan( pen, pText, pLines[i].ofs, pLines[i].nChars, true );
		}

		pos.y += pLines[i].lineSpacing;
	}

	if( dest.w >= textSize.w && (dest.h >= textSize.h || nLines == 1) )
		return true;
	else
		return false;
}

//____ _printTextSpan() ________________________________________________________

void WgGfxDevice::_printTextSpan( WgPen& pen, const WgText * pText, int ofs, int len, bool bLineEnding )
{
	WgColor baseCol	= m_tintColor;
	WgColor	color	= baseCol;

	const WgChar * pChars = pText->getText();
	Uint16	hProp	= 0xFFFF;		// Setting to impossible value forces setting of properties in first loop.
	WgTextAttr		attr;

	WgRange	selection = pText->getSelection();

	// Print loop

	for( int i = ofs ; i < ofs + len ; i++ )
 	{
		// Act on possible change of character attributes.

		if( pChars[i].PropHandle() != hProp || i == selection.Begin() || i == selection.End() )
		{
			bool bWasUnderlined = attr.bUnderlined;

			hProp = pChars[i].PropHandle();

			pText->GetCharAttr( attr, i );
			pen.SetAttributes(attr);
			if( !pen.GetGlyphset() )
				return;											// Better not to print than to crash...

			// Set tint colors (if changed)

			if( pen.GetColor() != color )
			{
				color = pen.GetColor();
				SetTintColor( baseCol * color );
			}

			// Check if this is start of underlined text and in that case draw the underline.

			if( attr.bUnderlined && (i==0 || !bWasUnderlined) )
			{
				WgRect clip = pen.HasClipRect()?pen.GetClipRect():WgRect(0,0,65535,65535);
				_drawUnderline( clip, pText, pen.GetPosX(), pen.GetPosY(), i, (ofs+len)-i );
			}

		}

		// Calculate position and blit the glyph.

		Uint16 ch = pChars[i].Glyph();

		bool bBlit = pen.SetChar( ch );
		pen.ApplyKerning();
		if( bBlit )
		{

/*			if(selStartX == -1 && i >= iSelStart)
				selStartX = pen.GetBlitPosX();

			if(selStartX >= 0 && i < iSelEnd)
				selEndX = pen.GetBlitPosX();
*/
			pen.BlitChar();
		}

		pen.AdvancePos();

 	}

	// Render line-endings.

	if( bLineEnding )
	{
		// If character after line-end was a WG_HYPHEN_BREAK_PERMITTED we need
		// to render a normal hyphen.

		if( pChars[ofs+len].Glyph() == WG_HYPHEN_BREAK_PERMITTED )
		{
			if( pen.SetChar( '-' ) )
			{
				pen.ApplyKerning();
				pen.BlitChar();
			}
		}

		// TODO: print LF-character if there is one following and properties says it should be displayed.

	}

	// Restore tint color.

	if( m_tintColor != baseCol )
		SetTintColor(baseCol);
}


//____ _printEllipsisTextSpan() ________________________________________________________

void WgGfxDevice::_printEllipsisTextSpan( WgPen& pen, const WgText * pText, int ofs, int len, int endX )
{
	WgColor baseCol	= m_tintColor;
	WgColor	color	= baseCol;

	const WgChar * pChars = pText->getText();
	Uint16	hProp	= 0xFFFF;		// Setting to impossible value forces setting of properties in first loop.
	WgTextAttr		attr;
	WgTextAttr		baseAttr;

	WgRange	selection = pText->getSelection();
	int		ellipsisWidth = 0;


	// Get the width of an ellipsis

	pText->GetBaseAttr( baseAttr );	// Ellipsis are always rendered using the base attributes.
	pen.SetAttributes( baseAttr );

	Uint16	ellipsisChar = WG_ELLIPSIS;
	ellipsisWidth = 0;
	WgGlyphPtr pEllipsis = pen.GetFont()->GetGlyph( WG_ELLIPSIS, pen.GetStyle(), pen.GetSize() );

	if( !pEllipsis )
	{
		pEllipsis = pen.GetFont()->GetGlyph( '.', pen.GetStyle(), pen.GetSize() );
		ellipsisChar = '.';
	}

	if( pEllipsis )
	{
		const WgGlyphBitmap * pBitmap = pEllipsis->GetBitmap();
		if( pBitmap )
		{
			if( ellipsisChar == WG_ELLIPSIS )
				ellipsisWidth = pBitmap->rect.w + pBitmap->bearingX;
			else
				ellipsisWidth = pEllipsis->Advance()*2+pBitmap->rect.w + pBitmap->bearingX;
		}
	}

	// Print loop

	for( int i = ofs ; i < ofs + len ; i++ )
 	{
		// Act on possible change of character attributes.

		if( pChars[i].PropHandle() != hProp || i == selection.Begin() || i == selection.End() )
		{
			bool bWasUnderlined = attr.bUnderlined;

			hProp = pChars[i].PropHandle();

			pText->GetCharAttr( attr, i );
			pen.SetAttributes(attr);
			if( !pen.GetGlyphset() )
				return;											// Better not to print than to crash...

			// Set tint colors (if changed)

			if( pen.GetColor() != color )
			{
				color = pen.GetColor();
				SetTintColor( baseCol * color );
			}

			// Check if this is start of underlined text and in that case draw the underline.

			if( attr.bUnderlined && (i==0 || !bWasUnderlined) )
			{
				WgRect clip = pen.HasClipRect()?pen.GetClipRect():WgRect(0,0,65535,65535);
				_drawUnderline( clip, pText, pen.GetPosX(), pen.GetPosY(), i, (ofs+len)-i );
			}
		}

		// Calculate position and blit the glyph.

		Uint16 ch = pChars[i].Glyph();

		bool bBlit = pen.SetChar( ch );

		WgCoord savedPos = pen.GetPos();
		pen.ApplyKerning();
		WgGlyphPtr pGlyph = pen.GetGlyph();
		if( pen.GetPosX() +  pGlyph->Advance() + ellipsisWidth > endX )
		{
			pen.SetPos( savedPos );
			break;
		}

		if( bBlit )
			pen.BlitChar();

		pen.AdvancePos();

 	}

	// Render ellipsis.

	pen.SetAttributes(baseAttr);		// Ellipsis are always rendered using the base attributes.
	pen.SetChar( ellipsisChar );

	// Set tint colors (if changed)

	if( pen.GetColor() != color )
	{
		color = pen.GetColor();
		SetTintColor( baseCol * color );
	}

	if( ellipsisChar == '.' )
	{
		pen.SetChar( ellipsisChar );		// Set once more to get kerning between dots to work.
		pen.BlitChar();
		pen.AdvancePos();
		pen.ApplyKerning();
		pen.BlitChar();
		pen.AdvancePos();
		pen.ApplyKerning();
		pen.BlitChar();
	}
	else
		pen.BlitChar();						// We could have kerning here but we have screwed up previous glyph...

	// Restore tint color.

	if( m_tintColor != baseCol )
		SetTintColor(baseCol);
}


//____ _drawTextBg() ___________________________________________________________

void WgGfxDevice::_drawTextBg( const WgRect& _clip, const WgText * pText, const WgRect& dest )
{
	WgRect		clip(_clip,dest);		// Make sure clipping rect is inside dest.

	WgMode mode = pText->mode();

	// Take care of selection background color (if we have any)

	int selStart, selEnd;

	int startLine, startCol, endLine, endCol;
	pText->getSelection( startLine, startCol, endLine, endCol );
	selStart = pText->LineColToOffset(startLine, startCol);
	selEnd = pText->LineColToOffset(endLine,endCol);

	WgTextpropPtr pSelProp = WgTextTool::GetSelectionProperties(pText);

	if( selStart != selEnd && pSelProp->IsBgColored() )
	{
		_drawTextSectionBg( clip, pText, dest, selStart, selEnd, pSelProp->BgColor(mode) );
	}
	else
	{
		selStart = -1;
		selEnd = -1;
	}

	// Scan through the text, drawing character specific backgrounds
	// (and in the future punching holes in the general background?)

	Uint16	hProp = 0xFFFF;
	WgColor	color;
	int		startOfs = 0;

	const WgChar * pChars = pText->getText();
	int nChars = pText->nbChars();

	for( int ofs = 0 ; ofs < nChars ; ofs++ )
	{
		if( ofs == selStart )
		{
			if( color.a != 0 )
				_drawTextSectionBg( clip, pText, dest, startOfs, ofs, color );

			startOfs = selEnd;			//TODO: We should not skip rendering background color, there migth be on link or character (or selection might not have one).
			ofs = startOfs;
		}

		if( pChars[ofs].PropHandle() != hProp )
		{
			// Update hProp and get background color

			hProp = pChars[ofs].PropHandle();

			WgColor newColor = pText->GetCharBgColor(ofs);

			if( newColor != color )
			{
				// Draw previous bg section which now ended

				if( ofs != startOfs && color.a != 0 )
					_drawTextSectionBg( clip, pText, dest, startOfs, ofs, color );

				// Set start and color of current background section

				color = newColor;
				startOfs = ofs;
			}

		}
	}

	// Draw last background section if it is colored

	if( startOfs != nChars && color.a != 0 )
		_drawTextSectionBg( clip, pText, dest, startOfs, nChars, color );
}

//____ _drawTextSectionBg() ___________________________________________________

void WgGfxDevice::_drawTextSectionBg( const WgRect& clip, const WgText * pText, const WgRect& dstRect,
									  int iStartOfs, int iEndOfs, WgColor color )
{
	const WgTextLine *	pLines = pText->getSoftLines();

	WgTextPos startPos = pText->OfsToPos( iStartOfs );
	WgTextPos endPos = pText->OfsToPos( iEndOfs );

	int xs = pText->PosToCoordX( startPos, dstRect );
	int xe = pText->PosToCoordX( endPos, dstRect );

	int dstPosY = pText->LineStartY( 0, dstRect );

	WgRect r;

	if(startPos.line == endPos.line)
	{
		r.x = xs;
		r.y = dstPosY + pText->getLineOfsY(startPos.line);
		r.w = xe - xs;
		r.h = pLines[startPos.line].height;
		ClipFill(clip, r, color);
	}
	else
	{
		r.x = xs;
		r.y = dstPosY + pText->getLineOfsY(startPos.line);
		r.w = pText->LineStartX(startPos.line, dstRect) + pText->getSoftLineSelectionWidth(startPos.line) - xs;
		r.h = pLines[startPos.line].height;
		ClipFill(clip, r, color);
		r.y += pLines[startPos.line].lineSpacing;

		++startPos.line;
		for(; startPos.line < endPos.line; ++startPos.line)
		{
			r.x = pText->LineStartX( startPos.line, dstRect );
			r.w = pText->getSoftLineSelectionWidth(startPos.line);
			r.h = pLines[startPos.line].height;
			ClipFill(clip, r, color);
			r.y += pLines[startPos.line].lineSpacing;
		}

		r.x = pText->LineStartX( startPos.line, dstRect );
		r.w = xe - r.x;
		r.h = pLines[startPos.line].height;
		ClipFill(clip, r, color);
	}
}

//____ PrintLine() ________________________________________________________

void WgGfxDevice::PrintLine( WgPen& pen, const WgTextAttr& baseAttr, const WgChar * _pLine, int maxChars, WgMode mode )
{
	if( !_pLine )
		return;

	WgColor baseCol	= m_tintColor;
	WgColor	color	= baseCol;

	Uint16	hProp				= 0xFFFF;		// Setting to impossible value forces setting of properties in first loop.
	WgTextAttr	attr;

	pen.FlushChar();

	// Print loop

	int i;
	for( i = 0 ; i < maxChars && !_pLine[i].IsEndOfLine(); i++ )
 	{
		// Act on possible change of character attributes.

		if( _pLine[i].PropHandle() != hProp )
		{
			bool bWasUnderlined = attr.bUnderlined;

			attr = baseAttr;

			WgTextTool::AddPropAttributes( attr, _pLine[i].Properties(), mode );

			hProp = _pLine[i].PropHandle();

			pen.SetAttributes( attr );
			if( !pen.GetGlyphset() )
				return;											// No glyphset, better to leave than to crash...

			// Set tint colors (if changed)

			if( pen.GetColor() != color )
			{
				color = pen.GetColor();
				SetTintColor( baseCol * color );
			}

			// Check if this is start of underlined text and in that case draw the underline.
/*
		TODO: Figure out how to do this properly, taking mode and char-props correctly into account.

			if( attr.bUnderlined && (i==0 || !bWasUnderlined) )
			{
				WgRect clip = pen.HasClipRect()?pen.GetClipRect():WgRect(0,0,65535,65535);
				DrawUnderline( clip, pText, pen.GetPosX(), pen.GetPosY(), i, len-i );
			}
*/
		}

		// Calculate position and blit the glyph.

		Uint16 ch = _pLine[i].Glyph();

		bool bBlit = pen.SetChar( ch );
		pen.ApplyKerning();
		if( bBlit )
		{

/*			if(selStartX == -1 && i >= iSelStart)
				selStartX = pen.GetBlitPosX();

			if(selStartX >= 0 && i < iSelEnd)
				selEndX = pen.GetBlitPosX();
*/
			pen.BlitChar();
		}

		pen.AdvancePos();

 	}

	// Restore tint color.

	if( m_tintColor != baseCol )
		SetTintColor(baseCol);
}


//____ _drawUnderline() ________________________________________________________

void WgGfxDevice::_drawUnderline( const WgRect& clip, const WgText * pText, int _x, int _y, int ofs, int maxChars )
{
	Uint32 hProp = 0xFFFF;

	WgPen pen;
	const WgChar * pChars = pText->getText();
	pen.SetTextNode( pText->getNode() );
	pen.SetScale( pText->Scale() );

	for( int i = ofs ; i < ofs + maxChars && !pChars[i].IsEndOfLine() ; i++ )
	{
		if( pChars[i].PropHandle() != hProp )
		{
			WgTextAttr attr;
			pText->GetCharAttr( attr, i );

			if( attr.bUnderlined )
			{
				hProp = pChars[i].PropHandle();
				pen.SetAttributes( attr );
			}
			else
				break;
		}

		pen.SetChar( pChars[i].Glyph() );
		pen.ApplyKerning();
		pen.AdvancePos();
	}

	const WgUnderline * pUnderline = pen.GetFont()->GetUnderline( pen.GetSize() );

	ClipBlitHorrBar( clip, pUnderline->pSurf, pUnderline->rect, WgBorders( pUnderline->leftBorder, pUnderline->rightBorder, 0, 0 ), false,
					_x + pUnderline->bearingX, _y + pUnderline->bearingY, pen.GetPosX() );
}

//____ _genCurveTab() ___________________________________________________________

void WgGfxDevice::_genCurveTab()
{
	s_pCurveTab = new int[c_nCurveTabEntries];

	//		double factor = 3.14159265 / (2.0 * c_nCurveTabEntries);

	for (int i = 0; i < c_nCurveTabEntries; i++)
	{
		double y = 1.f - i / (double)c_nCurveTabEntries;
		s_pCurveTab[i] = (int)(sqrt(1.f - y*y)*65536.f);
	}
}

//____ _traceLine() __________________________________________________________

void WgGfxDevice::_traceLine(int * pDest, int nPoints, const WgWaveLine& wave, int offset)
{
	static const int c_supersamples = 4;

	static int brush[128*c_supersamples];
	static float prevThickness = -1.f;

	float thickness = wave.thickness;
	int brushSteps = (int)(thickness * c_supersamples / 2);

	// Generate brush

	if (thickness != prevThickness)
	{
		int scaledThickness = (int)(thickness / 2 * 256);

		brush[0] = scaledThickness;
		for (int i = 1; i <= brushSteps; i++)
			brush[i] = (scaledThickness * s_pCurveTab[c_nCurveTabEntries - (i*c_nCurveTabEntries) / brushSteps]) >> 16;
		prevThickness = thickness;
	}

	int nTracePoints = WgMax(0, WgMin(nPoints, wave.length - offset));
	int nFillPoints = nPoints - nTracePoints;

	// Trace...

	int * pSrc = wave.pWave + offset;
	for (int i = 0; i < nTracePoints; i++)
	{
		// Start with top and bottom for current point

		int top = pSrc[i] - brush[0];
		int bottom = pSrc[i] + brush[0];

		// Check brush's coverage from previous points

		int end = WgMin(i + 1, brushSteps+1);

		for (int j = 1; j < end; j++)
		{
			int from = pSrc[i - j / c_supersamples];
			int to = pSrc[i - j / c_supersamples - 1];

			int sample = (to - from) * (j%c_supersamples) / c_supersamples + from;

			int topCover = sample - brush[j];
			int bottomCover = sample + brush[j];

			if (topCover < top)
				top = topCover;
			else if (bottomCover > bottom)
				bottom = bottomCover;
		}

		// Check brush's coverage from following points

		end = WgMin(nPoints - i, brushSteps+1);

		for (int j = 1; j < end; j++)
		{
			int from = pSrc[i + j / c_supersamples];
			int to = pSrc[i + j / c_supersamples + 1];

			int sample = (to - from) * (j%c_supersamples) / c_supersamples + from;

			int topCover = sample - brush[j];
			int bottomCover = sample + brush[j];

			if (topCover < top)
				top = topCover;
			else if (bottomCover > bottom)
				bottom = bottomCover;
		}

		// Save traced values

		*pDest++ = top;
		*pDest++ = bottom;
	}

	// Fill...

	if (nFillPoints)
	{
		int top = wave.hold - brush[0];
		int bottom = wave.hold + brush[0];
		for (int i = 0; i < nFillPoints; i++)
		{
			*pDest++ = top;
			*pDest++ = bottom;
		}
	}
}
