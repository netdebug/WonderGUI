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

	inline WgRect 	canvas() const;
	inline WgRect 	window() const;
	
	inline float	scale() const { return m_scale; }
	

private:
    WgRect  m_baseGeo;			// Widgets geometry in base coordinates.
    WgRect  m_baseWindow;		// Widgets window in base coordinates.

	float 	m_scale;
	WgCoord m_rootCanvasOfs;	// Coordinate offset for root canvas.

};


WgRect WgGeometrics::canvas() const
{
	WgRect cnv;

	cnv.x = (int) m_baseGeo.x * m_scale;
	cnv.y = (int) m_baseGeo.y * m_scale;
	cnv.w = (int) (m_baseGeo.x + m_baseGeo.w) * m_scale - cnv.x;
	cnv.h = (int) (m_baseGeo.y + m_baseGeo.h) * m_scale - cnv.y;

	return cnv;
}

WgRect WgGeometrics::window() const
{
	WgRect win;

	win.x = (int) m_baseWindow.x * m_scale;
	win.y = (int) m_baseWindow.y * m_scale;
	win.w = (int) (m_baseWindow.x + m_baseGeo.w) * m_scale - win.x;
	win.h = (int) (m_baseWindow.y + m_baseGeo.h) * m_scale - win.y;

	return win;
}



#endif //WG_GEOMETRICS_DOT_H
