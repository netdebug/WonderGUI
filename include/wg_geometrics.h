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

#ifndef	WG_GEOMETRICS_DOT_H
#define WG_GEOMETRICS_DOT_H

#include <wg_geo.h>

class WgGeometrics
{
public:
	WgGeometrics() : m_scale(1.f) {}
	WgGeometrics( const WgRect& baseGeo, const WgRect& baseWindow, float scale, WgCoord rootCanvasOfs ) : m_baseGeo(baseGeo), m_baseWindow(baseWindow), m_scale(scale), m_rootCanvasOfs(rootCanvasOfs) {}
	WgGeometrics( const WgRect& baseGeo, const WgRect& baseWindow, const WgGeometrics& stencil ) : m_baseGeo(baseGeo), m_baseWindow(baseWindow), m_scale(stencil.m_scale), m_rootCanvasOfs(stencil.m_rootCanvasOfs) {}
	WgGeometrics( const WgRect& baseGeo, const WgGeometrics& stencil ) : m_baseGeo(baseGeo), m_baseWindow(baseGeo), m_scale(stencil.m_scale), m_rootCanvasOfs(stencil.m_rootCanvasOfs) {}


	inline WgRect	baseGeo() const { return m_baseGeo; }
	inline WgRect	baseWindow() const { return m_baseWindow; }

	inline WgRect 	canvas() const { return scaleToCanvas( m_baseGeo ); }
	inline WgRect 	window() const { return scaleToCanvas( m_baseWindow ); }
	
	inline float	scale() const { return m_scale; }

	inline WgRect	scaleToCanvas( const WgRect& baseRect ) const;
	inline WgCoord	scaleToCanvas( const WgCoord& basePos ) const;
	
	

private:
    WgRect  m_baseGeo;			// Widgets geometry in base coordinates.
    WgRect  m_baseWindow;		// Widgets window in base coordinates.

	float 	m_scale;
	WgCoord m_rootCanvasOfs;	// Coordinate offset for root canvas.

};



inline WgRect WgGeometrics::scaleToCanvas( const WgRect& baseRect ) const
{
	WgRect cnv;

	cnv.x = (int) baseRect.x * m_scale;
	cnv.y = (int) baseRect.y * m_scale;
	cnv.w = (int) (baseRect.x + baseRect.w) * m_scale - cnv.x;
	cnv.h = (int) (baseRect.y + baseRect.h) * m_scale - cnv.y;

	return cnv;		
}

inline WgCoord WgGeometrics::scaleToCanvas( const WgCoord& basePos ) const
{
	WgCoord cnv;
	
	cnv.x = (int) basePos.x * m_scale;
	cnv.y = (int) basePos.y * m_scale;
	return cnv;			
}



#endif //WG_GEOMETRICS_DOT_H
