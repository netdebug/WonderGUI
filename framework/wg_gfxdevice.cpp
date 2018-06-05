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

#include <wg_versionbridge.h>

//____ Constructor _____________________________________________________________


//____ Destructor _________________________________________________________

WgGfxDevice::~WgGfxDevice()
{
}

//____ SetSaveDirtyRects() _____________________________________________________
/*
bool WgGfxDevice::SetSaveDirtyRects( bool bSave )
{
	m_bSaveDirtyRects = bSave;
	return true;
}
*/
//____ SetTintColor() __________________________________________________________

void WgGfxDevice::SetTintColor( WgColor color )
{
	m_pRealDevice->setTintColor( _convert(color) );
}

//____ SetBlendMode() __________________________________________________________

bool WgGfxDevice::SetBlendMode( WgBlendMode blendMode )
{
	return m_pRealDevice->setBlendMode(_convert(blendMode));
	return false;				// Not implemented.
}

//____ GetTintColor() _________________________________________________________

const WgColor WgGfxDevice::GetTintColor() const 
{ 
	return _convert(m_pRealDevice->tintColor()); 
}

//____ GetBlendMode() _________________________________________________________

WgBlendMode WgGfxDevice::GetBlendMode() const 
{ 
	return _convert(m_pRealDevice->blendMode()); 
}

//____ SetCanvas() ____________________________________________________________

bool WgGfxDevice::SetCanvas(WgSurface * pCanvas)
{
    wg::Surface * pSurf = nullptr;
    
    if( pCanvas )
        pSurf = pCanvas->m_pRealSurface;
        
	bool ret = m_pRealDevice->setCanvas(pSurf);
	if (ret)
		m_pCanvas = pCanvas;
	return ret;
}

//____ Canvas() _______________________________________________________________

WgSurface *	 WgGfxDevice::Canvas() const 
{ 
	return m_pCanvas; 
}

//____ CanvasSize() ___________________________________________________________

WgSize WgGfxDevice::CanvasSize() const 
{ 
	return _convert(m_pRealDevice->canvasSize()); 
}



//____ BeginRender() ___________________________________________________________

bool WgGfxDevice::BeginRender()
{
	return m_pRealDevice->beginRender();
}

//____ EndRender() _____________________________________________________________

bool WgGfxDevice::EndRender()
{
	return m_pRealDevice->endRender();
}

//____ Fill() _________________________________________________________________

void WgGfxDevice::Fill(const WgRect& rect, const WgColor& col)
{
	m_pRealDevice->fill(_convert(rect), _convert(col));
}


//____ ClipDrawLine() _________________________________________________________

// Coordinates for start are considered to be + 0.5 in the width dimension, so they start in the middle of a line/column.
// A one pixel thick line will only be drawn one pixel think, while a two pixels thick line will cover three pixels in thickness,
// where the outer pixels are faded.

void WgGfxDevice::ClipDrawLine(const WgRect& clip, const WgCoord& _begin, WgDirection dir, int length, WgColor _col, float thickness)
{
	m_pRealDevice->clipDrawLine(_convert(clip), _convert(_begin), _convert(dir), length, _convert(_col), thickness);
}

//____ ClipPlotPixels() _______________________________________________________

void WgGfxDevice::ClipPlotPixels(const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor * colors)
{
	m_pRealDevice->clipPlotPixels(_convert(clip), nCoords, (const wg::Coord*) pCoords, (const wg::Color*) colors);
}

//____ DrawLine() _____________________________________________________________

void WgGfxDevice::DrawLine(WgCoord begin, WgCoord end, WgColor color, float thickness)
{
	m_pRealDevice->drawLine(_convert(begin), _convert(end), _convert(color), thickness);
}

//____ ClipDrawLine() _________________________________________________________

void WgGfxDevice::ClipDrawLine(const WgRect& clip, WgCoord begin, WgCoord end, WgColor color, float thickness)
{
	m_pRealDevice->clipDrawLine(_convert(clip), _convert(begin), _convert(end), _convert(color), thickness);
}

//____ ClipDrawHorrWave() _____________________________________________________

void WgGfxDevice::ClipDrawHorrWave(const WgRect& clip, WgCoord begin, int length, const WgWaveLine& topBorder, const WgWaveLine& bottomBorder, WgColor frontFill, WgColor backFill)
{
	m_pRealDevice->clipDrawHorrWave(_convert(clip), _convert(begin), length, (const wg::WaveLine*) &topBorder, (const wg::WaveLine*) &bottomBorder, _convert(frontFill), _convert(backFill));
}

//_____ ClipBlitFromCanvas() ______________________________________________________

void WgGfxDevice::ClipBlitFromCanvas(const WgRect& clip, const WgSurface* pSrc, const WgRect& src, int dx, int dy)
{
	m_pRealDevice->clipBlitFromCanvas(_convert(clip), pSrc->m_pRealSurface, _convert(src), wg::Coord(dx, dy));
}


//____ Blit() __________________________________________________________________

void WgGfxDevice::Blit( const WgSurface* pSrc )
{
	m_pRealDevice->blit(pSrc->m_pRealSurface);
}

void WgGfxDevice::Blit( const WgSurface* pSrc, int dx, int dy )
{
	m_pRealDevice->blit(pSrc->m_pRealSurface, wg::Coord(dx, dy));
}

void WgGfxDevice::Blit(const WgSurface* pSrc, const WgRect& src, int dx, int dy)
{
	m_pRealDevice->blit(pSrc->m_pRealSurface, _convert(src), wg::Coord(dx, dy));
}


//____ StretchBlit() ___________________________________________________________

void WgGfxDevice::StretchBlit( const WgSurface * pSrc, bool bTriLinear, float mipmapBias )
{
	m_pRealDevice->stretchBlit(pSrc->m_pRealSurface);
}

void WgGfxDevice::StretchBlit( const WgSurface * pSrc, const WgRect& dest, bool bTriLinear, float mipmapBias )
{
	m_pRealDevice->stretchBlit(pSrc->m_pRealSurface, _convert(dest));
}

void WgGfxDevice::StretchBlit( const WgSurface * pSrc, const WgRect& src, const WgRect& dest, bool bTriLinear, float mipmapBias )
{
	m_pRealDevice->stretchBlit(pSrc->m_pRealSurface, _convert(src), _convert(dest));
}

//____ TileBlit() ______________________________________________________________

void WgGfxDevice::TileBlit( const WgSurface* _pSrc )
{
	m_pRealDevice->tileBlit(_pSrc->m_pRealSurface);
}

void WgGfxDevice::TileBlit( const WgSurface* _pSrc, const WgRect& _dest )
{
	m_pRealDevice->tileBlit(_pSrc->m_pRealSurface, _convert(_dest));
}

void WgGfxDevice::TileBlit( const WgSurface* _pSrc, const WgRect& _src, const WgRect& _dest )
{
	m_pRealDevice->tileBlit(_pSrc->m_pRealSurface, _convert(_src), _convert(_dest));
}


//____ ClipFill() ______________________________________________________________

void WgGfxDevice::ClipFill( const WgRect& _clip, const WgRect& _rect, const WgColor& _col )
{
	m_pRealDevice->clipFill(_convert(_clip), _convert(_rect), _convert(_col));
}

//____ ClipBlit() ______________________________________________________________

void WgGfxDevice::ClipBlit( const WgRect& clip, const WgSurface* pSrc )
{
	m_pRealDevice->clipBlit(_convert(clip), pSrc->m_pRealSurface);
}

void WgGfxDevice::ClipBlit( const WgRect& clip, const WgSurface* pSrc, int dx, int dy  )
{
	m_pRealDevice->clipBlit(_convert(clip), pSrc->m_pRealSurface, wg::Coord(dx, dy));
}

void WgGfxDevice::ClipBlit( const WgRect& clip, const WgSurface* pSrc, const WgRect& srcRect, int dx, int dy  )
{
	m_pRealDevice->clipBlit(_convert(clip), pSrc->m_pRealSurface, _convert(srcRect), wg::Coord(dx, dy));
}

//____ ClipStretchBlit() _______________________________________________________

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, bool bTriLinear, float mipBias )
{
	m_pRealDevice->clipStretchBlit(_convert(clip), pSrc->m_pRealSurface);
}

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, const WgRect& dest, bool bTriLinear, float mipBias )
{
	m_pRealDevice->clipStretchBlit(_convert(clip), pSrc->m_pRealSurface, _convert(dest));
}

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, const WgRect& src, const WgRect& dest, bool bTriLinear, float mipBias )
{
	m_pRealDevice->clipStretchBlit(_convert(clip), pSrc->m_pRealSurface, _convert(src), _convert(dest));
}

void WgGfxDevice::ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias)
{
	m_pRealDevice->clipStretchBlit(_convert(clip), pSrc->m_pRealSurface, wg::RectF(sx, sy, sw, sh), wg::Rect((int)dx, (int)dy, (int)dw, (int)dh));
}

//____ ClipTileBlit() __________________________________________________________

void WgGfxDevice::ClipTileBlit( const WgRect& clip, const WgSurface* pSrc )
{
	m_pRealDevice->clipTileBlit(_convert(clip), pSrc->m_pRealSurface);
}

void WgGfxDevice::ClipTileBlit( const WgRect& clip, const WgSurface* pSrc,
								  const WgRect& dest )
{
	m_pRealDevice->clipTileBlit(_convert(clip), pSrc->m_pRealSurface, _convert(dest));
}


void WgGfxDevice::ClipTileBlit( const WgRect& _clip, const WgSurface* _pSrc, const WgRect& _src, const WgRect& _dest )
{
	m_pRealDevice->clipTileBlit(_convert(_clip), _pSrc->m_pRealSurface, _convert(_src), _convert(_dest));
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
	WgColor baseCol	= _convert(m_pRealDevice->tintColor());
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

	if( GetTintColor() != baseCol )
		SetTintColor(baseCol);
}


//____ _printEllipsisTextSpan() ________________________________________________________

void WgGfxDevice::_printEllipsisTextSpan( WgPen& pen, const WgText * pText, int ofs, int len, int endX )
{
	WgColor baseCol	= _convert(m_pRealDevice->tintColor());
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

	if( GetTintColor() != baseCol )
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

	WgColor baseCol	= _convert( m_pRealDevice->tintColor() );
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

	if( GetTintColor() != baseCol )
		SetTintColor(baseCol);
}

//____ FillSubPixel() _________________________________________________________

void WgGfxDevice::FillSubPixel(const WgRectF& rect, const WgColor& col)
{
	m_pRealDevice->fillSubPixel(_convert(rect), _convert(col));
}

//____ StretchBlitSubPixel() __________________________________________________

void WgGfxDevice::StretchBlitSubPixel(const WgSurface * pSrc, float sx, float sy, float sw, float sh,
		float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias)
{
	m_pRealDevice->stretchBlit(pSrc->m_pRealSurface, wg::RectF(sx, sy, sw, sh), wg::Rect((int)dx, (int)dy, (int)dw, (int)dh));
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
