//
//  wg_simpleline.cpp
//
//  Created by Patrik Holmström on 11/17/15.
//
//

#include <wg_simpleline.h>

#include <wg_image.h>
#include <wg_surface_soft.h>
#include <wg_gfxdevice.h>

static const char	c_widgetType[] = {"Softube/SimpleLine"};

#include <iostream>
#include <iomanip>
#include <math.h>
#define round(x) floorf((x)+0.5f)



//#include "debug.h"

#define round(x) floorf((x)+0.5f)

using namespace std;

//____ Constructor ____________________________________________________________

WgSimpleLine::WgSimpleLine()
{
    m_pAngleLineSurf = 0;
    m_src	= WgRect( 0, 0, 0, 0 );
    
    m_iAlloc = 128;
    
    m_pixelX.resize(m_iAlloc);
    m_pixelY.resize(m_iAlloc);
    m_pixelC.resize(m_iAlloc);
    m_erasePixelC.resize(m_iAlloc);
    m_iLen = 0;
    
    m_kColor = WgColor::white;
    
    /*
    m_fValue = 0.0f;
    m_iLength = 0;
    m_iID = -1;
     */
    
    m_lineThickness = 2.f;
    
    int m_iX1 = 0;
    int m_iY1 = 0;
    int m_iX2 = 10;
    int m_iY2 = 10;
    
}

//____ Destructor _____________________________________________________________

WgSimpleLine::~WgSimpleLine()
{
    if(m_pAngleLineSurf != 0)
        delete m_pAngleLineSurf;
}

//____ Type() _________________________________________________________________

const char * WgSimpleLine::Type( void ) const
{
    return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgSimpleLine::GetClass()
{
    return c_widgetType;
}

//____ PreferredSize() __________________________________________________________

WgSize WgSimpleLine::PreferredSize() const
{
    return m_src.Size();
}

//____ SetSurfaceSize() _______________________________________________________

void WgSimpleLine::SetSurfaceSize( int iWidth, int iHeight )
{
    if(m_pAngleLineSurf != 0) delete m_pAngleLineSurf;
    
    m_pAngleLineSurf = new WgSurfaceSoft( WgSize(iWidth, iHeight), WG_PIXEL_BGRA_8 );
    
    //NO With WgColor::transparent we'll get a white VU in GrandChannel. The transparency
    //   must go towards black. Not white.
    m_pAngleLineSurf->Fill( WgColor(0,0,0,0)); //WgColor::transparent );
    
    m_bOpaque = false;
    
    m_src.x = 0;
    m_src.y = 0;
    m_src.w = iWidth;
    m_src.h = iHeight;
    
    _requestRender();
    _requestResize();
}

inline int ipart(double x)
{
    return (int)floor(x);
}

inline float fpart(float x)
{
    return fabsf(x) - ipart(x);
}

inline float rfpart(float x)
{
    return 1.0f - fpart(x);
}

void WgSimpleLine::SetLine( int iX1, int iY1, int iX2, int iY2 )
{
    // Early out if no change
    if( (m_iX1==iX1) && (m_iY1==iY1) && (m_iX2==iX2) && (m_iY2==iY2) )
        return;
    
    
    // Used to keep track of which rectangle to render
    int iRenderTopLeftX, iRenderTopLeftY, iRenderBottomRightX, iRenderBottomRightY;
    float x1, y1, x2, y2;
    
    // Check old line (to be erased)
    x1 = (float)m_iX1;
    y1 = (float)m_iY1;
    
    x2 = (float)m_iX2;
    y2 = (float)m_iY2;
    
    if(x1 < x2)
    {
        iRenderTopLeftX = ipart(x1)-1;
        iRenderBottomRightX = ipart(x2)+2;
    }
    else
    {
        iRenderTopLeftX = ipart(x2)-1;
        iRenderBottomRightX = ipart(x1)+2;
    }
    
    if(y1 < y2)
    {
        iRenderTopLeftY = ipart(y1)-1;
        iRenderBottomRightY = ipart(y2)+2;
    }
    else
    {
        iRenderTopLeftY = ipart(y2)-1;
        iRenderBottomRightY = ipart(y1)+2;
    }
    
    m_iX1 = (float)iX1;
    m_iY1 = (float)iY1;
    
    m_iX2 = (float)iX2;
    m_iY2 = (float)iY2;
    
    // Check new line (to be drawn)
    x1 = (float)m_iX1;
    y1 = (float)m_iY1;
    
    x2 = (float)m_iX2;
    y2 = (float)m_iY2;
    
    if(x1 < x2)
    {
        if(ipart(x1)-1 < iRenderTopLeftX)
            iRenderTopLeftX = ipart(x1)-1;
        if(ipart(x2)+2 > iRenderBottomRightX)
            iRenderBottomRightX = ipart(x2)+2;
    }
    else
    {
        if(ipart(x2)-1 < iRenderTopLeftX)
            iRenderTopLeftX = ipart(x2)-1;
        if(ipart(x1)+2 > iRenderBottomRightX)
            iRenderBottomRightX = ipart(x1)+2;
    }
    
    if(y1 < y2)
    {
        if(ipart(y1)-1 < iRenderTopLeftY)
            iRenderTopLeftY = ipart(y1)-1;
        if(ipart(y2)+2 > iRenderBottomRightY)
            iRenderBottomRightY = ipart(y2)+2;
    }
    else
    {
        if(ipart(y2)-1 < iRenderTopLeftY)
            iRenderTopLeftY = ipart(y2)-1;
        if(ipart(y1)+2 > iRenderBottomRightY)
            iRenderBottomRightY = ipart(y1)+2;
    }
    
    _requestRender( WgRect(iRenderTopLeftX, iRenderTopLeftY, iRenderBottomRightX-iRenderTopLeftX, iRenderBottomRightY-iRenderTopLeftY) );
    
}

//ToDo: Is RemoveLine() needed?
void WgSimpleLine::RemoveLine()
{
    // Used to keep track of which rectangle to render
    int iRenderTopLeftX, iRenderTopLeftY, iRenderBottomRightX, iRenderBottomRightY;
    float x1, y1, x2, y2;
    
    // Check old line (to be erased)
    x1 = (float)m_iX1;
    y1 = (float)m_iY1;
    
    x2 = (float)m_iX2;
    y2 = (float)m_iY2;
    
    if(x1 < x2)
    {
        iRenderTopLeftX = ipart(x1)-1;
        iRenderBottomRightX = ipart(x2)+2;
    }
    else
    {
        iRenderTopLeftX = ipart(x2)-1;
        iRenderBottomRightX = ipart(x1)+2;
    }
    
    if(y1 < y2)
    {
        iRenderTopLeftY = ipart(y1)-1;
        iRenderBottomRightY = ipart(y2)+2;
    }
    else
    {
        iRenderTopLeftY = ipart(y2)-1;
        iRenderBottomRightY = ipart(y1)+2;
    }
    
    //ToDo: SET SIZE AND VISIBILITY!!!
    
    _requestRender( WgRect(iRenderTopLeftX, iRenderTopLeftY, iRenderBottomRightX-iRenderTopLeftX, iRenderBottomRightY-iRenderTopLeftY) );
}

//____ SetLineColor() ________________________________________________________

void WgSimpleLine::SetColor( WgColor color )
{
    if( color != m_kColor )
    {
        m_kColor = color;
        _requestRender();
    }
}

/*
//____ SetLineThickness() ________________________________________________________
void WgSimpleLine::SetLineThickness( float thickness )
{
    WG_LIMIT( thickness, 1.f, 2.f );
    if( thickness != m_lineThickness )
    {
        m_lineThickness = thickness;
        _requestRender();
    }
}*/

//____ _onCloneContent() ______________________________________________________

void WgSimpleLine::_onCloneContent( const WgWidget * _pOrg )
{
    
}


//___ _onAlphaTest() __________________________________________________________

bool WgSimpleLine::_onAlphaTest( const WgCoord& ofs )
{
    if( (m_pAngleLineSurf != 0) && m_bEnabled) {
        if( m_pAngleLineSurf->GetOpacity( WgCoord(m_src.x + (ofs.x % m_src.w), m_src.y + (ofs.y % m_src.h) )) )
            return true; // Pixel was visible (even if maybe just barely).
    }
    
    return false;
}

//____ _onRender() ____________________________________________________________

void WgSimpleLine::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
    drawLine(_clip);
    
    // Blit AngleLine
    if(m_pAngleLineSurf != 0)
        pDevice->ClipTileBlit( _clip, m_pAngleLineSurf, m_src, _window );
}


void WgSimpleLine::plot(int x, int y, double alpha, const WgRect& _clip)
{
    WgColor col;
    WgCoord abs = Local2abs( WgCoord(x,y) );
    
    alpha *= 1;
    
    WG_LIMIT(alpha, -1, 1);
    
    if ( (abs.y >= _clip.y) && (abs.y < _clip.y + _clip.h) && (abs.x >= _clip.x) && (abs.x < _clip.x + _clip.w) )
    {
        m_pixelX[m_iLen] = x;
        m_pixelY[m_iLen] = y;
        
        WgColor col1(0);
        
        col1.r = m_kColor.r;
        col1.g = m_kColor.g;
        col1.b = m_kColor.b;
        col1.a = (Uint8)ipart(alpha*(double)m_kColor.a);
        
        m_pixelC[m_iLen] = col1.argb;
        
        col1.a = 0x00;
        
        m_erasePixelC[m_iLen] = col1.argb;
        
        m_iLen++;
        
        if(m_iLen >= m_iAlloc)
        {
            m_iAlloc = 2*m_iAlloc;
            
            m_pixelX.resize(m_iAlloc);
            m_pixelY.resize(m_iAlloc);
            m_pixelC.resize(m_iAlloc);
            m_erasePixelC.resize(m_iAlloc);
        }
    }
    
    return;
}

void WgSimpleLine::drawLine(const WgRect& _clip, bool erase)
{
    // Erase old angle line
    m_pAngleLineSurf->Lock(WG_WRITE_ONLY);
    m_pAngleLineSurf->PutPixels(m_pixelX,m_pixelY,m_erasePixelC,m_iLen,true);
    m_pAngleLineSurf->Unlock();
    
    m_iLen = 0;
    
    if(m_bEnabled)
    {
        float x1, y1, x2, y2;
        
        x1 = (float)m_iX1;
        y1 = (float)m_iY1;
        
        x2 = (float)m_iX2;// + (float)(m_iLength - 1) * cosf(m_fStartAngle + m_fValue * (m_fStopAngle - m_fStartAngle));
        y2 = (float)m_iY2;// - (float)(m_iLength - 1) * sinf(m_fStartAngle + m_fValue * (m_fStopAngle - m_fStartAngle));
        
        // Algorithm: Xiaolin Wu's line algorithm, adapted from http://en.wikipedia.org/wiki/Xiaolin_Wu's_line_algorithm
        bool swap;
        float dx, dy, gradient, xgap, intery, xend, yend;
        int xpxl1, xpxl2, ypxl1, ypxl2;
        dx = x2 - x1;
        dy = y2 - y1;
        
        swap = (fabsf(dx) < fabsf(dy));
        
        if(swap) {
            // handle "vertical" lines
            // Swap x1 <-> y1, x2 <-> y2, dx <-> dy
            float tmp;
            tmp = x1;
            x1 = y1;
            y1 = tmp;
            tmp = x2;
            x2 = y2;
            y2 = tmp;
            tmp = dx;
            dx = dy;
            dy = tmp;
        }
        
        if(x2 < x1) {
            // Swap x1 <-> x2, y1 <-> y2
            float tmp;
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }
        
        if (dx == 0.0f) gradient = 1e12f;
        else gradient = dy / dx;
        
        // handle first endpoint
        xend = round(x1);
        yend = y1 + gradient * (xend - x1);
        xgap = rfpart(x1 + 0.5f);
        xpxl1 = (int)xend;  // this will be used in the main loop
        ypxl1 = (int)ipart(yend);
        if(!swap) {
            plot(xpxl1, ypxl1, rfpart(yend) * xgap, _clip);
            plot(xpxl1, ypxl1 + 1, fpart(yend) * xgap, _clip);
        } else {
            plot(ypxl1, xpxl1, rfpart(yend) * xgap, _clip);
            plot(ypxl1 + 1, xpxl1, fpart(yend) * xgap, _clip);
        }
        intery = yend + gradient; // first y-intersection for the main loop
        
        // handle second endpoint
        xend = round(x2);
        yend = y2 + gradient * (xend - x2);
        xgap = fpart(x2 + 0.5f);
        xpxl2 = (int)xend;  // this will be used in the main loop
        ypxl2 = (int)ipart(yend);
        if(!swap) {
            plot(xpxl2, ypxl2, rfpart(yend) * xgap, _clip);
            plot(xpxl2, ypxl2 + 1, fpart(yend) * xgap, _clip);
        } else {
            plot(ypxl2, xpxl2, rfpart(yend) * xgap, _clip);
            plot(ypxl2 + 1, xpxl2, fpart(yend) * xgap, _clip);
        }
        
        // main loop
        for(int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
            if(!swap) {
                plot(x, ipart(intery), rfpart(intery), _clip);
                plot(x, ipart(intery) + 1, fpart(intery), _clip);
            } else {
                plot(ipart(intery), x, rfpart(intery), _clip);
                plot(ipart(intery) + 1, x, fpart(intery), _clip);
            }				
            intery = intery + gradient;
        }
    }
    
    // Draw new angle line
    m_pAngleLineSurf->Lock( WG_WRITE_ONLY );
    m_pAngleLineSurf->PutPixels(m_pixelX,m_pixelY,m_pixelC,m_iLen,false);
    m_pAngleLineSurf->Unlock();
    
    return;
}
