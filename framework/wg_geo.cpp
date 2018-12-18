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


#include <wg_geo.h>
//#include <wg_blockset.h>

#include <utility>
#include <algorithm>
#include <cstdlib>

//____ WgBorders::Scale() ____________________________________________________

WgBorders WgBorders::Scale(int scale) const
{
    return {    (uint8_t)((left*scale)>>WG_SCALE_BINALS),
                (uint8_t)((top*scale)>>WG_SCALE_BINALS),
                (uint8_t)((right*scale)>>WG_SCALE_BINALS),
                (uint8_t)((bottom*scale)>>WG_SCALE_BINALS) };
}


//____ WgRect() _______________________________________________________________


WgRect::WgRect( const WgRect& r1, const WgRect& r2 )
{
	Intersection( r1, r2 );
}

WgRect::WgRect( const WgCoord& p1, const WgCoord& p2 )
{
	if( p1.x < p2.x )
	{
		x = p1.x;
		w = p2.x - p1.x;
	}
	else
	{
		x = p2.x;
		w = p1.x - p2.x;
	}

	if( p1.y < p2.y )
	{
		y = p1.y;
		h = p2.y - p1.y;
	}
	else
	{
		y = p2.y;
		h = p1.y - p2.y;
	}
}


//____ Intersection() ________________________________________________________________

bool WgRect::Intersection( const WgRect& _r1, const WgRect& _r2 )
{
	int		x1, y1;						// So we can use ourself as inparameter.
	int		x2, y2;

	if( _r1.x > _r2.x )
		x1 = _r1.x;
	else
		x1 = _r2.x;

	if( _r1.y > _r2.y )
		y1 = _r1.y;
	else
		y1 = _r2.y;

	if( _r1.x + _r1.w < _r2.x + _r2.w )
		x2 = _r1.x + _r1.w;
	else
		x2 = _r2.x + _r2.w;

	if( _r1.y + _r1.h < _r2.y + _r2.h )
		y2 = _r1.y + _r1.h;
	else
		y2 = _r2.y + _r2.h;

	x = x1;
	y = y1;
	w = x2 - x;
	h = y2 - y;

	if( w <= 0 || h <= 0 )
	{
		w = 0;
		h = 0;
		return	false;
	}

	return	true;
}

//____ Shrink() _____________________________________________________________

void WgRect::Shrink(int left, int top, int right, int bottom)
{
	x += left;
	y += top;
	w -= right + left;
	h -= bottom + top;

	if( w < 0 )
		w = 0;

	if( h < 0 )
		h = 0;
}

void WgRect::Shrink(const WgBorders &_borders)
{
	x += _borders.left;
	y += _borders.top;
	w -= _borders.Width();
	h -= _borders.Height();

	if( w < 0 )
		w = 0;

	if( h < 0 )
		h = 0;
}

//____ Grow() _____________________________________________________________

void WgRect::Grow(int left, int top, int right, int bottom)
{
	x -= left;
	y -= top;
	w += right + left;
	h += bottom + top;
}

void WgRect::Grow(const WgBorders &_borders)
{
	x -= _borders.left;
	y -= _borders.top;
	w += _borders.Width();
	h += _borders.Height();
}

//____ GrowToContain() _______________________________________________________

void WgRect::GrowToContain( int _x, int _y )
{
	if( _x < x )
	{
		w += x - _x;
		x = _x;
	}
	else if( _x > x + w )
	{
		w = _x - x;
	}

	if( _y < y )
	{
		h += y - _y;
		y = _y;
	}
	else if( _y > y + h )
	{
		h = _y - y;
	}
}

void WgRect::GrowToContain( const WgCoord& _coord )
{
	if( _coord.x < x )
	{
		w += x - _coord.x;
		x = _coord.x;
	}
	else if( _coord.x > x + w )
	{
		w = _coord.x - x;
	}

	if( _coord.y < y )
	{
		h += y - _coord.y;
		y = _coord.y;
	}
	else if( _coord.y > y + h )
	{
		h = _coord.y - y;
	}
}


void WgRect::GrowToContain( const WgRect& r )
{
	if( r.x < x )
	{
		w += x - r.x;
		x = r.x;
	}

	if( r.y < y )
	{
		h += y - r.y;
		y = r.y;
	}

	if( r.x + r.w > x + w )
	{
		w = r.x + r.w - x;
	}

	if( r.y + r.h > y + h )
	{
		h = r.y + r.h - y;
	}
}


WgRectF::WgRectF( const WgRectF& r1, const WgRectF& r2 )
{
	Intersection( r1, r2 );
}

//____ Intersection() ________________________________________________________________

bool WgRectF::Intersection( const WgRectF& _r1, const WgRectF& _r2 )
{
	float	x1, y1;						// So we can use ourself as inparameter.
	float	x2, y2;
	
	if( _r1.x > _r2.x )
		x1 = _r1.x;
	else
		x1 = _r2.x;
	
	if( _r1.y > _r2.y )
		y1 = _r1.y;
	else
		y1 = _r2.y;
	
	if( _r1.x + _r1.w < _r2.x + _r2.w )
		x2 = _r1.x + _r1.w;
	else
		x2 = _r2.x + _r2.w;
	
	if( _r1.y + _r1.h < _r2.y + _r2.h )
		y2 = _r1.y + _r1.h;
	else
		y2 = _r2.y + _r2.h;
	
	x = x1;
	y = y1;
	w = x2 - x;
	h = y2 - y;
	
	if( w <= 0.f || h <= 0.f )
	{
		w = 0.f;
		h = 0.f;
		return	false;
	}
	
	return	true;
}


//____ IntersectsWithOrContains() _______________________________________________________

bool WgRect::IntersectsWithOrContains(WgCoord p1, WgCoord p2, int precision) const
{
	// NOTE: This needs to be kept in sync with ClipLine() so they always return same result for same input.

	int x1 = x;
	int x2 = x + w;

	// We go from left to right when working with the X coordinates

	if (p2.x < p1.x)
		std::swap(p1, p2);

	// Verify that x-coordinates are within range

	if (p1.x >= x2 || p2.x <= x1)
		return false;

	// Calculate angles now before we destroy any values

	int angleX = ((p2.y - p1.y) << precision) / (1 + p2.x - p1.x);		// Change in Y for each increase of X.

	// Clip line so no coord is outside rectangles x-dimensions

	if (p1.x < x1)
		p1.y = p1.y + ((x1 - p1.x)*angleX >> precision);

	if (p2.x > x2)
		p2.y = p2.y - ((p2.x - x2)*angleX >> precision);

	// We go from top to bottom when working with the Y coordinates

	if (p2.y < p1.y)
		std::swap(p1.y, p2.y);

	// Verify that clipped line intersects in y-dimension.

	if (p1.y >= y+h || p2.y <= y)
		return false;

	return true;
}

//____ ClipLine() _____________________________________________________________

bool WgRect::ClipLine(WgCoord * p1, WgCoord* p2, int precision) const
{
	// NOTE: This needs to be kept in sync with IntersectsOrContains() so they always return same result for same input.

	int x1 = x;
	int x2 = x + w;
	int y1 = y;
	int y2 = y + h;

	// We go from left to right when working with the X coordinates

	if (p2->x < p1->x)
		std::swap(p1, p2);

	// Verify that x-coordinates are within range

	if (p1->x >= x2 || p2->x <= x1)
		return false;

	// Calculate both angles now before we destroy precision

	int angleX = ((p2->y - p1->y) << precision) / (1 + p2->x - p1->x);		// Change in Y for each increase of X.
	int angleY = ((p2->x - p1->x) << precision) / (1 + std::abs(p2->y - p1->y));		// Change in X for each increase of Y.

	// Clip line so no coord is outside rectangles x-dimensions

	if (p1->x < x1)
	{
		p1->y = p1->y + ((x1 - p1->x)*angleX >> precision);
		p1->x = x2;
	}

	if (p2->x > x2)
	{
		p2->y = p2->y - ((p2->x - x2)*angleX >> precision);
		p2->x = x2;
	}

	// We go from top to bottom when working with the Y coordinates

	if (p2->y < p1->y)
	{
		std::swap(p1, p2);
		angleY = -angleY;
	}

	// Verify that clipped line intersects in y-dimension.

	if (p1->y >= y2 || p2->y <= y1)
		return false;

	// Clip line so no coord is outside rectangles y-dimensions

	if (p1->y < y1)
	{
		p1->x = p1->x + ((y1 - p1->y)*angleY >> precision);
		p1->y = y1;
	}

	if (p2->y > y2)
	{
		p2->x = p2->x - ((p2->y - y2)*angleY >> precision);
		p2->y = y2;
	}

	return true;
}

//____ Distance() ________________________________________________________

WgCoord WgRect::Distance( WgCoord coord ) const
{
    int dx = coord.x - x;
    int dy = coord.y - y;
    
    if( dx > 0 )
    {
        dx -= w;
        if( dx < 0 )
            dx = 0;
    }
    
    if( dy > 0 )
    {
        dy -= h;
        if( dy < 0 )
            dy = 0;
    }
    
    return { dx, dy};
}

