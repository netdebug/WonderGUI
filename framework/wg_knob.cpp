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
//#include <wg_color.h>
#include "Debug.h"
#include <wg_event.h>
#include <wg_eventhandler.h>

static const char	c_widgetType[] = {"Knob"};

#include <cmath>
#include <algorithm>

//____ Constructor ____________________________________________________________

WgKnob::WgKnob()
{
	m_bOpaque = false;
    m_iNextPixel = 0;
    m_lineColor = WgColor( 38,  169, 224, 255 ); //WgColor::white;
    m_fValue = 0.0f;
    m_preferredSize = WgSize(45,45);

}

WgKnob::WgKnob(WgSurfaceFactory * pFactory)
{
    WgKnob();
    m_bOpaque = false;
    m_iNextPixel = 0;
    m_lineColor = WgColor::white;
    m_fValue = 0.0f;
    m_preferredSize = WgSize(100,100);
    
    m_pSurf = pFactory->CreateSurface(PixelSize()*m_iOversampleX, WG_PIXEL_BGRA_8);
    m_pSurf->Fill(WgColor::transparent);
    m_pSurfaceFactory = pFactory;

    m_size = PixelSize();

    // Initialize arrays
    SetNumSteps(m_iNumSteps);

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
/*
//____ PreferredPixelSize() __________________________________________________________

void WgKnob::SetPreferredPixelSize(WgSize size)
{
    m_preferredSize = size;
}
*/
WgSize WgKnob::PreferredPixelSize() const
{
    return m_preferredSize; //*m_scale/WG_SCALE_BASE;
}

void WgKnob::SetNumSteps(int steps)
{
    if(m_iNumSteps == steps)
        return;
    
    int maxNumSteps = 11;
    int minNumSteps = 2;
    m_iNumSteps = steps;
    if(steps < minNumSteps || steps > maxNumSteps)
        return;
    
    // Idea:
    // 2 steps: two large segments  ([0.3 0.45], [0.55 0.7])
    // 3 steps: [
    m_AngleStart.resize(steps);
    m_AngleEnd.resize(steps);
    const float PI = 3.141592653f;
    
    float divs = (float)(m_iNumSteps * 2 - 1);
    //float length = (m_fAngleEnd - m_fAngleStart)*steps/maxNumSteps;
    
    float weight = float(steps - minNumSteps)/float(maxNumSteps - minNumSteps);
    float length = (m_fAngleEnd - m_fAngleStart)*weight + 0.25f*(1.0f-weight);
    float start = 0.5f - length/2.0f; // start the sections
    
    for(int i=0;i<steps;i++)
    {
        m_AngleStart[i] = start + 2*i*length/divs;
        m_AngleEnd[i]   = start + (2*i+1)*length/divs;
        m_AngleStart[i] *= 2*PI;
        m_AngleEnd[i] *= 2*PI;
    }
}

void WgKnob::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
    switch( pEvent->Type() )
    {
        case    WG_EVENT_MOUSE_ENTER:
            m_bPointerInside = true;
            _requestRender();
            break;
            
        case    WG_EVENT_MOUSE_LEAVE:
            m_bPointerInside = false;
            _requestRender();
            break;
            
        case WG_EVENT_MOUSEBUTTON_PRESS:
        {
            int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
            if( button == 1 )
                m_bPressed = true;
            
            _requestRender();
            break;
        }
        case WG_EVENT_MOUSEBUTTON_RELEASE:
        {
            int button = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->Button();
            if( button == 1 )
                m_bPressed = false;
            
            _requestRender();
            break;
        }
        default:
            break;
            
    }
    pHandler->ForwardEvent( pEvent );

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
    if(!m_pSurf)
        return;

    // TODO: Is this the place to create a new surface? Or should that be made in
    // "on resize" or something?
    const int w = std::min(PixelSize().w, PixelSize().h);
    const int h = w;
    if(m_size != PixelSize())
    {
        delete m_pSurf;
        m_pSurf = m_pSurfaceFactory->CreateSurface(WgSize(w,h)*m_iOversampleX, WG_PIXEL_BGRA_8);

    }

    // TODO:
    // Better calculation of angles, without atan and div for every pixel (polygon?)
    
    const float PI_HALF = PI/2.0f;
    
    const int oversampling = m_iOversampleX;
    
    const float xd = 2.0f/(float)(w*oversampling);
    const float yd = 2.0f/(float)(h*oversampling);
    const float rd = 3.0*2.0/(float)(w*oversampling); // should be sqrt(w+h);
    const float rd_inv = 1.0f/rd;
    
    const float r_outer = 1.0f-rd;
    const float r_inner = 1.0f-rd-m_iWidth;

    const float a_start = m_fAngleStart*2*PI;
    const float a_end = m_fAngleEnd*2*PI;
    const float a_value = (a_end-a_start)*m_fValue + a_start;

    const int value_index = GetValueIndex();
    
    // Set up colors
    WgColor kForeground = m_kForeground;
    WgColor kBackground = m_kBackground;
    
/*    if(m_bPressed)
        kForeground = Blend(m_kForeground, WgColor::white, 0.1f);
    else if(m_bPointerInside)
        kForeground = Blend(m_kForeground, WgColor::white, 0.5f);
  */
    WgColor kBackTransp = m_kBackground;
    kBackTransp.a = 0;
    
    int background = (m_kBackground.b) | ((m_kBackground.g)<<8) | ((m_kBackground.r)<<16) | ((m_kBackground.a)<<24);
    int foreground = (kForeground.b) | ((kForeground.g)<<8) | ((kForeground.r)<<16) | ((kForeground.a)<<24);
    int backtransp = (kBackTransp.b) | ((kBackTransp.g)<<8) | ((kBackTransp.r)<<16) | ((kBackTransp.a)<<24);
    
    WgColor col;
    

    unsigned char* dest = (unsigned char*)m_pSurf->Lock(WG_WRITE_ONLY);
    unsigned int *ddest = (unsigned int *)dest;

    float x=0.0f, y=0.0f;
    float y_inv = 0.0f;
    int color = 0;
    float r, in, out, weight, R, a;
    for(int yc=0;yc<h*oversampling;yc++)
    {
        // [-1, 1] coordinates
        y = (float)yc*yd - 1.0f;
        y_inv = 1.0f/std::max(std::abs(y), 0.0001f);
        
        for(int xc=0;xc<w*oversampling;xc++)
        {
            // [-1, 1] coordinates
            x = (float)xc*xd - 1.0f;
            
            // Radius
            r = x*x + y*y;
            
            // Reset color
            color = 0;
            
            // Calc boundaries for AA
            in = r-r_inner; // in > 0
            out = r_outer-r; // out > 0
            weight = 0.0f;
            
            R = 0.0f;
            a = 0.0f;

            const bool USE_ANGLE_AA = true;
            const float ANGLE_AA_WIDTH = 0.05f; // this should be dependent of the widget size.


            if((in > -rd) && (out > -rd))
            {
                // Calculate anti-aliasing on the inner and outer circle diameters ("CIRCLE AA")
                if(in < 0 && in > -rd)
                    weight = 1.0f + in*rd_inv;
                else if(out < 0 && out > -rd)
                    weight = 1.0f + out*rd_inv;
                else
                    weight = 1.0f;
                
                // Calculate angle.
                // We compare atan(-x/y) > a_start. Find equivalent -x/y > tan(a_start)?
                R = -x * y_inv;
                a = std::atan(R);

                if(y < 0.0f)
                    a = PI - a;
                
                if(a < 0.0f)
                    a += 2*PI;
                
                if(a > 2*PI)
                    a -= 2*PI;
                
                if(m_iNumSteps < 2 || m_iNumSteps > 11)
                {
                    // Continuous version
                    if(((a > a_start) && (a < a_value)))
                    {
                        if(m_bPressed | m_bPointerInside)
                            col = Blend(WgColor(255,255,255,255), kForeground, (a-a_start)/(a_value-a_start));
                        else
                            col = kForeground;

                        if(USE_ANGLE_AA && (a < (a_start + ANGLE_AA_WIDTH)))
                        {
                            float ww = (a-a_start) * (1.0f/ANGLE_AA_WIDTH);
                            col = Blend(col, kBackTransp, ww);
                        }

                        col = Blend(col, kBackTransp, weight); // CIRCLE AA
                    }
                    else if(USE_ANGLE_AA && (a >= a_value) && (a < a_value + ANGLE_AA_WIDTH))
                    {
                        // This is an example of a quick and dirty anti-aliasing
                        // that works pretty good.

                        float ww = (a-a_value)*(1.0f/ANGLE_AA_WIDTH); // 20 is 1/0.05, which is the angle that's being AAd.
                        if(m_bPressed | m_bPointerInside)
                            col = WgColor(255,255,255,255);
                        else
                            col = kForeground;

                        col = Blend(m_kBackground, col, ww);
                        col = Blend(col, kBackTransp, weight); // CIRCLE AA

                    }
                    else if((a >= a_value) && (a < a_end))
                    {
                        col = Blend(m_kBackground, kBackTransp, weight); // CIRCLE AA

                        if(USE_ANGLE_AA && (a > (a_end - ANGLE_AA_WIDTH)))
                        {
                            float ww = (a_end-a)*(1.0f/ANGLE_AA_WIDTH);
                            col = Blend(col, kBackTransp, ww);

                        }
                    }
                    else
                    {
                        col = kBackTransp;
                    }
                }
                else
                {
                    // Discrete version
                    bool colorize = false;
                    WgColor set_col = m_kBackground;
                    for(int i=0;i<m_iNumSteps;i++)
                    {
                        if((a > m_AngleStart[i] && (a < m_AngleEnd[i])))
                        {
                            colorize = true;
                            if(value_index == i)
                                set_col = kForeground;
                        }
                    }
                    if(colorize)
                        col = Blend(set_col, kBackTransp, weight);
                    else
                        col = kBackTransp;
                }
                
                color = (col.b) | ((col.g)<<8) | ((col.r)<<16) | ((col.a)<<24);

            }

            *ddest++ = color;
        }
    }
    
    m_pSurf->Unlock();
    
    // Downsample. Oversampling is not used.
//    _downsample(m_pSurf, m_iOversampleX);

    pDevice->ClipBlit(_clip, m_pSurf,{0,0,w,h},_canvas.x,_canvas.y);
}

WgColor WgKnob::Blend( const WgColor& start, const WgColor& dest, float grade )
{
    WgColor col;

    // Yes, it's backwards. Sue me.
    col.r = (Uint8)( (float)start.r * grade + (1.0f-grade) * (float)dest.r );
    col.g = (Uint8)( (float)start.g * grade + (1.0f-grade) * (float)dest.g );
    col.b = (Uint8)( (float)start.b * grade + (1.0f-grade) * (float)dest.b );
    col.a = (Uint8)( (float)start.a * grade + (1.0f-grade) * (float)dest.a );

    return col;
}

void WgKnob::_downsample(WgSurface* pSurf, const int oversample)
{
    if(oversample == 1)
        return;
    
    int w = pSurf->PixelSize().w; //Width();
    int h = pSurf->PixelSize().h; //Height();
    unsigned int col = (255) | (2<<8) | (1<<16) | (192<<24);

    unsigned int* data = (unsigned int*)pSurf->Lock(WG_READ_WRITE);
    int i=0, j=0;
    
    // Loop over small size
    for(int y=0; y<h/oversample; y++)
    {
        for(int x=0; x<w/oversample; x++)
        {
            i = y*w+x; // every pixel
            j = y*oversample*w + x*oversample; // every other pixel

            data[i] = 0; // clear pixel. Wrong for (0,0).
            uint8_t* qd = (uint8_t*)(&(data[i]));
            
            for(int ys=0; ys<oversample;ys++)
            {
                for(int xs=0; xs<oversample;xs++)
                {
                    uint8_t* qs = (uint8_t*)(&(data[j + xs + ys*w]));
                    qd[0] += qs[0]>>oversample;
                    qd[1] += qs[1]>>oversample;
                    qd[2] += qs[2]>>oversample;
                    qd[3] += qs[3]>>oversample;
                }
            }
        }
    }
    
    
    pSurf->Unlock();
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
    
    quarter = std::round(rx2 / sqrt(rx2 + ry2));
    for (int x=0; x<=quarter; x++) {
        float y = radY * sqrt(1.0-x*x/rx2);
        float e = y - floor(y);
        plot4(centerX, centerY, x, -floor(y), e);
        plot4(centerX, centerY, x, -floor(y)+1, 1.0-e);
    }
    
    quarter = std::round(ry2 / sqrt(rx2 + ry2));
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
        
    xend = std::round(x0);
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
    xend = std::round(x1);
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
    m_pAACol[m_iNextPixel].a = (Uint8)std::round((float)m_lineColor.a * alpha);
    
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

