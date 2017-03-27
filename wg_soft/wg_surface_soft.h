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
#ifndef WG_SURFACE_SOFT_DOT_H
#define WG_SURFACE_SOFT_DOT_H

#ifndef WG_SURFACE_DOT_H
#	include <wg_surface.h>
#endif


//____ Class WgSurfaceSoft _____________________________________________________________________

class WgSurfaceSoft : public WgSurface
{
	friend class WgGfxDeviceSoft;

 public:
	WgSurfaceSoft( WgSize size, WgPixelType type = WG_PIXEL_BGRA_8 );
	WgSurfaceSoft( WgSize size, WgPixelType type, Uint8 * pPixels, int pitch );
    WgSurfaceSoft( WgSize size, WgPixelType type, uint8_t * pPixels, int pitch, const WgPixelFormat& pixelFormat );
	WgSurfaceSoft( const WgSurfaceSoft * pOther );
	~WgSurfaceSoft();

	const char *Type() const;
	static const char * GetClass();

	WgSize		Size() const;
	bool		IsOpaque() const;

	Uint32		GetPixel( WgCoord coord ) const;
	Uint8		GetOpacity( WgCoord coord ) const;

	void *		Lock( WgAccessMode mode );
	void *		LockRegion( WgAccessMode mode, const WgRect& region );
	void		Unlock();

	inline float ScaleAlpha() { return m_fScaleAlpha; }
	void        SetScaleAlpha(float fScaleAlpha);

	static WgSize	MaxSize();
protected:

	void        _copy(const WgSurfaceSoft * pOther);

	WgSize		m_size;
	float    	m_fScaleAlpha;
	bool		m_bOwnsData;
	Uint8 *		m_pData;
};


//========================================================================================
#endif // WG_SURFACE_SOFT_DOT_H
