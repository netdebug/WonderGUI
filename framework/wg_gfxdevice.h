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

#ifndef	WG_GFXDEVICE_DOT_H
#define WG_GFXDEVICE_DOT_H

#include <climits>

#ifndef WG_TYPES_DOT_H
#	include <wg_types.h>
#endif

#ifndef	WG_COLOR_DOT_H
#	include <wg_color.h>
#endif

#ifndef WG_TEXTPROP_DOT_H
#	include <wg_textprop.h>
#endif


#ifndef WG_GEO_DOT_H
#	include <wg_geo.h>
#endif
/*
#ifndef WG_SURFACE_DOT_H
#	include <wg_surface.h>
#endif

#ifndef WG_BLOCKSET_DOT_H
#	include <wg_blockset.h>
#endif
*/

#include <wg3_gfxdevice.h>

//____ WgWaveLine ___________________________________________________________

struct WgWaveLine
{
	int			length;
	float		thickness;
	WgColor		color;
	int *		pWave;			// Pixel offset in 24.8 format.
	int			hold;			// Value for extending the line if it is too short (or completely missing).
};

class	WgBlock;
class	WgRect;
class	WgBorders;
class	WgSurface;
class	WgText;
class	WgCursorInstance;
class 	WgPen;

class WgGfxDevice
{
public:

	enum WgRenderFlags
	{
		WG_ORIENT_NORMAL				= 0x0,
		WG_ORIENT_ROTATE_CW90			= 0x1,
		WG_ORIENT_ROTATE_180			= 0x2,
		WG_ORIENT_ROTATE_CCW90			= 0x3,
		WG_ORIENT_MIRROR_X				= 0x4,
		WG_ORIENT_MIRROR_X_ROTATE_CW90	= 0x5,
		WG_ORIENT_MIRROR_X_ROTATE_180	= 0x6,
		WG_ORIENT_MIRROR_X_ROTATE_CCW90	= 0x7,
		WG_ORIENT_MASK					= 0x7,
	};

	virtual ~WgGfxDevice();

	void			SetTintColor( WgColor color );
	bool			SetBlendMode( WgBlendMode blendMode );
//	virtual Uint32	SetRenderFlags( Uint32 flags );
//	virtual bool	SetSaveDirtyRects( bool bSave );

	const WgColor		GetTintColor() const;
	WgBlendMode 	GetBlendMode() const;
//	inline Uint32		GetRenderFlags() const { return m_renderFlags; }
//	inline bool			GetSaveDirtyRects() const { return m_bSaveDirtyRects; }

	// Geometry related methods.

	bool			SetCanvas(WgSurface * pCanvas);
	WgSurface *		Canvas() const;

	WgSize			CanvasSize() const;

	// Begin/end render methods.

	bool	BeginRender();
	bool	EndRender();


	// Special blit methods

	 void	ClipBlitFromCanvas(const WgRect& clip, const WgSurface* pSrc, const WgRect& src, int dx, int dy);	// Blit from surface that has been used as canvas. Will flip Y on OpenGL.


	// Low level draw methods.

	 void	Fill( const WgRect& rect, const WgColor& col );

	 void	ClipDrawLine(const WgRect& clip, const WgCoord& begin, WgDirection dir, int length, WgColor col, float thickness = 1.f);

//	 void	ClipDrawHorrLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col ) = 0;
//	 void	ClipDrawVertLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col ) = 0;
	 void    ClipPlotPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor * colors);

	 void	DrawLine( WgCoord begin, WgCoord end, WgColor color, float thickness = 1.f );
	 void	ClipDrawLine( const WgRect& clip, WgCoord begin, WgCoord end, WgColor color, float thickness = 1.f );

	 void	ClipDrawHorrWave(const WgRect& clip, WgCoord begin, int length, const WgWaveLine& topBorder, const WgWaveLine& bottomBorder, WgColor frontFill, WgColor backFill);

	 void	Blit( const WgSurface* pSrc );
	 void	Blit( const WgSurface* pSrc, int dx, int dy );
	 void	Blit( const WgSurface* pSrc, const WgRect& src, int dx, int dy );

	 void	StretchBlit( const WgSurface * pSrc, bool bTriLinear = false, float mipmapBias = 0.f );
	 void	StretchBlit( const WgSurface * pSrc, const WgRect& dest, bool bTriLinear = false, float mipmapBias = 0.f );
	 void	StretchBlit( const WgSurface * pSrc, const WgRect& src, const WgRect& dest, bool bTriLinear = false, float mipmapBias = 0.f );

	 void	TileBlit( const WgSurface* pSrc );
	 void	TileBlit( const WgSurface* pSrc, const WgRect& dest );
	 void	TileBlit( const WgSurface* pSrc, const WgRect& src, const WgRect& dest );


	 void	ClipFill( const WgRect& clip, const WgRect& rect, const WgColor& col );

	 void	ClipBlit( const WgRect& clip, const WgSurface* src );
	 void	ClipBlit( const WgRect& clip, const WgSurface* src, int dx, int dy  );
	 void	ClipBlit( const WgRect& clip, const WgSurface* src,
							  const WgRect& srcrect, int dx, int dy  );

	 void	ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, bool bTriLinear = false, float mipBias = 0.f );
	 void	ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, const WgRect& dest, bool bTriLinear = false, float mipBias = 0.f );
	 void	ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, const WgRect& src, const WgRect& dest, bool bTriLinear = false, float mipBias = 0.f );
	 void	ClipStretchBlit( const WgRect& clip, const WgSurface * pSrc, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias = 0.f);

	void			ClipTileBlit( const WgRect& clip, const WgSurface* src );
	void			ClipTileBlit( const WgRect& clip, const WgSurface* src,
								  const WgRect& dest );
	void			ClipTileBlit( const WgRect& clip, const WgSurface* src,
								  const WgRect& srcrect, const WgRect& dest );


	// High-level draw methods
	void			ClipBlitBlock(	const WgRect& clip, const WgBlock& block, const WgRect& dest, bool bTriLinear = false, float mipmapbias = 0.f );

	void			BlitBlock(		const WgBlock& block, const WgRect& dest, bool bTriLinear = false, float mipmapbias = 0.f );


	// Mid-level draw methods

    void		ClipBlitHorrStretchBar(const WgRect& _clip, const WgSurface * _pSurf,
                                        const WgRect& _src, const WgBorders& _srcBorders,
                                        const WgRect& _dest, const WgBorders& _destBorders );

    void		ClipBlitVertStretchBar(const WgRect& _clip, const WgSurface * _pSurf,
                                        const WgRect& _src, const WgBorders& _srcBorders,
                                        const WgRect& _dest, const WgBorders& _destBorders );
    
    
	void		ClipBlitHorrBar(	const WgRect& _clip, const WgSurface * _pSurf, const WgRect& _src,
										const WgBorders& _borders, bool _bTile,
										int _dx, int _dy, int _len );

	void		ClipBlitVertBar(	const WgRect& _clip, const WgSurface * _pSurf, const WgRect& _src,
										const WgBorders& _borders, bool _bTile,
										int _dx, int _dy, int _len );


	void		BlitHorrBar(		const WgSurface * _pSurf, const WgRect& _src,
										const WgBorders& _borders, bool _bTile,
										int _dx, int _dy, int _len );

	void		BlitVertBar(		const WgSurface * _pSurf, const WgRect& _src,
										const WgBorders& _borders, bool _bTile,
										int _dx, int _dy, int _len );

	// High-level print methods

	bool		PrintText( const WgRect& clip, const WgText * pText, const WgRect& dest );

	// Low-level print methods

	void		PrintLine( WgPen& pen, const WgTextAttr& baseAttr, const WgChar * _pLine, int maxChars = INT_MAX, WgMode mode = WG_MODE_NORMAL );

	void	FillSubPixel( const WgRectF& rect, const WgColor& col );
	void	StretchBlitSubPixel( const WgSurface * pSrc, float sx, float sy, float sw, float sh,
						   		 float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias = 0.f );

	
protected:

	void _printTextSpan(WgPen& pen, const WgText * pText, int ofs, int len, bool bLineEnding);
	void _printEllipsisTextSpan(WgPen& pen, const WgText * pText, int ofs, int len, int endX);
	void _drawTextBg(const WgRect& _clip, const WgText * pText, const WgRect& dest);
	void _drawTextSectionBg(const WgRect& clip, const WgText * pText, const WgRect& dstRect,
		int iStartOfs, int iEndOfs, WgColor color);
	void _drawUnderline(const WgRect& clip, const WgText * pText, int _x, int _y, int ofs, int maxChars);


	wg::GfxDevice_p m_pRealDevice;
	WgSurface *		m_pCanvas = nullptr;		// We need to save pointer to WgSurface since we can't retrieve it from our real canvas (wg::Surface).

};

#endif	// WG_GFXDEVICE_DOT_H

