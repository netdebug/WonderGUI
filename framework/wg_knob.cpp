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


#include <wg_knob.h>
#include <wg_gfxdevice.h>
#include <wg_base.h>

#include "Debug.h"

static const char	c_widgetType[] = {"Knob"};

#include <math.h>
#define round(x) floorf((x)+0.5f)

//____ Constructor ____________________________________________________________

WgKnob::WgKnob()
{
	m_bOpaque = false;
    m_iNextPixel = 0;
    m_lineColor = WgColor::white;
    m_fValue = 0.0f;
    m_preferredSize = WgSize(45,45);

}

//____ Destructor _____________________________________________________________

WgKnob::~WgKnob()
{
}

//____ Type() _________________________________________________________________

const char * WgKnob::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgKnob::GetClass()
{
	return c_widgetType;
}

//____ SetValue() _____________________________________________________________

void WgKnob::SetValue( float fValue )
{
    if(m_fValue == fValue)
        return;
    
    m_fValue = fValue;
  	_requestRender();
}

//____ PreferredPixelSize() __________________________________________________________
void WgKnob::SetPreferredPixelSize(WgSize size)
{
    m_preferredSize = size;
}

WgSize WgKnob::PreferredPixelSize() const
{
	return m_preferredSize;
}


//____ _onCloneContent() _______________________________________________________

void WgKnob::_onCloneContent( const WgWidget * _pOrg )
{
	const WgKnob * pOrg = static_cast<const WgKnob*>(_pOrg);
}

//____ _onRender() _____________________________________________________________
#define PI 3.141592653f
void WgKnob::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
//    pDevice->Fill( _clip, WgColor::black );

    int rad = (WgMin( _canvas.w,_canvas.h ) - 1) / 2;
    int sz = 2*rad + 1; // Rad is the radius (even). Total size will be 2*radius + 1 middle point
    
	if( sz > 1 )
	{
        m_iNextPixel = 0;
        drawCircle(_canvas.x + sz/2, _canvas.y + sz/2, sz/2, sz/2);
        pDevice->ClipPlotPixels(_clip, m_iNextPixel, m_pAAPix, m_pAACol);
        
        const float d = 45.0f;
        // Draw the index line
        const float x = -sin(d*PI/180.0f + m_fValue*(360.0f-d*2)*PI/180.0f)*(0.92f*float(sz)/2.0f);
        const float y =  cos(d*PI/180.0f + m_fValue*(360.0f-d*2)*PI/180.0f)*(0.92f*float(sz)/2.0f);

        m_iNextPixel = 0;
        drawLine(_canvas.x + rad, _canvas.y + rad, _canvas.x + rad + x, _canvas.y + rad + y);
        pDevice->ClipPlotPixels(_clip, m_iNextPixel, m_pAAPix, m_pAACol);
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgKnob::_onAlphaTest( const WgCoord& ofs )
{
	return true;
}


//____ _onEnable() _____________________________________________________________

void WgKnob::_onEnable()
{
    _requestRender();
}

//____ _onDisable() ____________________________________________________________

void WgKnob::_onDisable()
{
    _requestRender();
}

void WgKnob::drawCircle(const int centerX, const int centerY, const float radX, const float radY)
{
    const float rx2 = radX*radX;
    const float ry2 = radY*radY;
    int quarter;
    
    quarter = round(rx2 / sqrt(rx2 + ry2));
    for (int x=0; x<=quarter; x++) {
        float y = radY * sqrt(1.0-x*x/rx2);
        float e = y - floor(y);
        plot4(centerX, centerY, x, -floor(y), e);
        plot4(centerX, centerY, x, -floor(y)+1, 1.0-e);
    }
    
    quarter = round(ry2 / sqrt(rx2 + ry2));
    for (int y=0; y<=quarter; y++) {
        float x = radX * sqrt(1.0-y*y/ry2);
        float e = x - floor(x);
        plot4(centerX, centerY, -floor(x),   y, e);
        plot4(centerX, centerY, -floor(x)+1, y, 1.0-e);
    }
}

// Xiaolin Wu's line algorithm
void WgKnob::drawLine(float x0, float y0, float x1, float y1)
{
	float dx;
	float dy;
    float intery, gradient;
    int xpxl1, xpxl2;
    int ypxl1, ypxl2;
    float xgap;
    float xend;
    float yend;
        
    bool steep = fabsf(y1 - y0) > fabs(x1 - x0);
        
    if(steep) {
        std::swap(x0,y0);
        std::swap(x1,y1);
    }
    
    if (x0>x1) {
        std::swap(x0,x1);
        std::swap(y0,y1);
    }
        
    dx = (float)x1 - (float)x0;
    dy = (float)y1 - (float)y0;
    gradient = dy/dx;
        
    xend = round(x0);
    yend = y0 + gradient * (xend - x0);
    xgap = rfpart(x0 + 0.5);
    xpxl1 = xend;
    ypxl1 = ipart(yend);
        
    if (steep) {
        plot(ypxl1    , xpxl1, rfpart(yend) * xgap);
        plot(ypxl1 + 1, xpxl1,  fpart(yend) * xgap);
    } else {
        plot(xpxl1, ypxl1,     rfpart(yend) * xgap);
        plot(xpxl1, ypxl1 + 1,  fpart(yend) * xgap);
    }
    intery = yend + gradient;
        
    // Second end point
    xend = round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5);
    xpxl2 = xend;
    ypxl2 = ipart(yend);
        
    if (steep) {
        plot(ypxl2    , xpxl2, rfpart(yend) * xgap);
        plot(ypxl2 + 1, xpxl2,  fpart(yend) * xgap);
    } else {
        plot(xpxl2, ypxl2,     rfpart(yend) * xgap);
        plot(xpxl2, ypxl2 + 1,  fpart(yend) * xgap);
    }
        
    for (int x=xpxl1+1; x<xpxl2; x++) {
        if (steep) {
            plot(ipart(intery)    , x, rfpart(intery));
            plot(ipart(intery) + 1, x,  fpart(intery));
        } else {
            plot(x, ipart (intery),  rfpart(intery));
            plot(x, ipart (intery)+1, fpart(intery));
        }
        intery = intery + gradient;
    }
    
	return;
}


void WgKnob::plot(const int x, const int y, const float alpha)
{
    m_pAAPix[m_iNextPixel] = WgCoord(x, y);
    m_pAACol[m_iNextPixel] = m_lineColor;
    m_pAACol[m_iNextPixel].a = (Uint8)round((float)m_lineColor.a * alpha);
    
    ++m_iNextPixel;
    
    DBG_ASSERT(m_iNextPixel < WG_KNOB_PIXEL_BUFFER_SIZE);
}

void WgKnob::plot4(const int centerX, const int centerY, const int deltaX, const int deltaY, const float alpha)
{
    plot(centerX+deltaX, centerY+deltaY, alpha);
    plot(centerX-deltaX, centerY+deltaY, alpha);
    plot(centerX+deltaX, centerY-deltaY, alpha);
    plot(centerX-deltaX, centerY-deltaY, alpha);
}
