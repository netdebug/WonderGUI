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

#include <wg_oscilloscope.h>
#include <wg_gfxdevice.h>
#include <wg_base.h>


#include "Debug.h"

static const char	c_widgetType[] = {"Oscilloscope"};

#include <stdlib.h>
#include <math.h>
#define round(x) floorf((x)+0.5f)

//____ Constructor ____________________________________________________________

WgOscilloscope::WgOscilloscope()
{
	m_gridColor = WgColor::black;
	m_lineColor = WgColor::white;
	m_lineThickness = 2.f;

	m_nVGridLines = 0;
	m_pVGridLines = 0;
	m_nHGridLines = 0;
	m_pHGridLines = 0;

	m_nLinePoints = 0;
	m_pLinePoints = 0;

	m_nMarkers = 0;
	m_pMarkers = 0;
}

//____ Destructor _____________________________________________________________

WgOscilloscope::~WgOscilloscope()
{
	delete [] m_pVGridLines;
	delete [] m_pHGridLines;
	delete [] m_pLinePoints;
	delete [] m_pMarkers;
}

//____ Type() _________________________________________________________________

const char * WgOscilloscope::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgOscilloscope::GetClass()
{
	return c_widgetType;
}

//____ PreferredSize() ________________________________________________________

WgSize WgOscilloscope::PreferredSize() const
{
	return WgSize(80,64);
}


//____ SetBackground() ________________________________________________________

void WgOscilloscope::SetBackground( const WgSkinPtr& skin )
{
	if( m_pBG != skin )
	{
		m_pBG = skin;
		if( m_pBG )
			m_bOpaque = m_pBG->IsOpaque();
		else
			m_bOpaque = false;

		_requestRender();
	}
}

//____ SetGridColor() _________________________________________________________

void WgOscilloscope::SetGridColor( WgColor color )
{
	if( m_gridColor != color )
	{
		m_gridColor = color;
		if( m_nVGridLines != 0 || m_nHGridLines != 0 )
			_requestRender();
	}
}

//____ SetVGridLines() ________________________________________________________

void WgOscilloscope::SetVGridLines( int nLines, float pLines[] )
{
	if( nLines != m_nVGridLines )
	{
		delete [] m_pVGridLines;
		if( nLines == 0 )
			m_pVGridLines = 0;
		else
			m_pVGridLines = new float[nLines];
		m_nVGridLines = nLines;
	}

	for( int i = 0 ; i < nLines ; i++ )
		m_pVGridLines[i] = pLines[i];

	_requestRender();
}

//____ SetHGridLines() ________________________________________________________

void WgOscilloscope::SetHGridLines( int nLines, float pLines[] )
{
	if( nLines != m_nHGridLines )
	{
		delete [] m_pHGridLines;
		if( nLines == 0 )
			m_pHGridLines = 0;
		else
			m_pHGridLines = new float[nLines];
		m_nHGridLines = nLines;
	}

	for( int i = 0 ; i < nLines ; i++ )
		m_pHGridLines[i] = pLines[i];

	_requestRender();
}

//____ SetLineColor() ________________________________________________________

void WgOscilloscope::SetLineColor( WgColor color )
{
	if( color != m_lineColor )
	{
		m_lineColor = color;
		_requestRender();
	}
}

//____ SetLineThickness() _____________________________________________________

void WgOscilloscope::SetLineThickness( float thickness )
{
	WG_LIMIT( thickness, 1.f, 2.f );
	if( thickness != m_lineThickness )
	{
		m_lineThickness = thickness;
		_requestRender();
	}
}

//____ SetLinePoints() ________________________________________________________

void WgOscilloscope::SetLinePoints( int nPoints, float pPointValues[] )
{
//    DBGM(DBG_GUI, ("DBG_GUI WgOscilloscope::SetLinePoints( nPoints=%d, pPointValues[0]=%f ) width=%d", nPoints, pPointValues[0], Size().w));
    
    
    // TODO: Den här koden var helt åt skogen förut. Nu verkar det funka, men det behövs kollas igenom.
    // TODO: Eftersom SetLinePoints kallas vid varje uppdatering verkar det ju fånigt att skapa en ny array varje gång?
    
    int sz = 0;
    if( nPoints == 0 )
    {
        delete [] m_pLinePoints;
        m_pLinePoints = 0;
        m_nLinePoints = 0;
        _requestRender();

        return;
    }

    
	if( nPoints != m_nLinePoints )
	{
		
        // What is bigger, the data or window?
        if(nPoints > Size().w)
            sz = nPoints;
        else
            sz = Size().w;
        
        // Set up array
        if(m_nLinePoints != sz)
        {
            delete [] m_pLinePoints;
            m_pLinePoints = new float[sz+2]; // Need two extra points for anti-aliasing
            m_nLinePoints = sz;
        }
	}
    
    // "resample"
    int ip = 0;
    float lam = 0;
    float point = 0;
    
    if (nPoints != Size().w)
    {
        float ratio = (float)nPoints/(float)Size().w;
        
        for(int i=0; i < m_nLinePoints; i++)
        {
            point = (float)i * ratio;
            ip = (int)floorf(point);
            lam = point - (float)ip;
            
            if(ip+1 < nPoints)
                m_pLinePoints[i] = pPointValues[ip] * (1-lam) + pPointValues[ip+1] * lam;
            else
                m_pLinePoints[i] = pPointValues[nPoints-1];
            
            // m_pLinePoints[i] = pPointValues[(int)((float)i*ratio)];
        }
//        m_nLinePoints = Size().w;
        nPoints = m_nLinePoints;
    }
    else
    {
        // Truncation
        for( int i = 0 ; i < nPoints ; i++ )
            m_pLinePoints[i] = pPointValues[i];
    }


    
	m_pLinePoints[m_nLinePoints] = m_pLinePoints[m_nLinePoints-1];
    m_pLinePoints[m_nLinePoints+1] = m_pLinePoints[m_nLinePoints-1];
    
	_requestRender();
}

//____ ClearMarkers() _________________________________________________________

void WgOscilloscope::ClearMarkers()
{
	delete [] m_pMarkers;
	m_pMarkers = 0;
	m_nMarkers = 0;
	_requestRender();
}

//____ AddMarker() ____________________________________________________________

void WgOscilloscope::AddMarker( int xOfs, float yOfs )
{
	Marker * pNew = new Marker[m_nMarkers+1];
	for( int i = 0 ; i < m_nMarkers ; i++ )
		pNew[i] = m_pMarkers[i];
	delete [] m_pMarkers;

	pNew[m_nMarkers].x = xOfs;
	pNew[m_nMarkers].y = yOfs;

	m_nMarkers++;
	m_pMarkers = pNew;
	_requestRender();
}

//____ SetMarkerGfx() _________________________________________________________

void WgOscilloscope::SetMarkerGfx( const WgBlocksetPtr& pBlockset )
{
	if( m_pMarkerGfx != pBlockset )
	{
		m_pMarkerGfx = pBlockset;
		_requestRender();
	}
}



//____ _onCloneContent() ______________________________________________________

void WgOscilloscope::_onCloneContent( const WgWidget * _pOrg )
{

}

//____ _onRender() ____________________________________________________________

void WgOscilloscope::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
    

    
	// Render background
	if( m_pBG )
		m_pBG->Render( pDevice, WG_STATE_NORMAL, _canvas, _clip );

	float centerX = _canvas.x + _canvas.w/2.f;
	float centerY = _canvas.y + _canvas.h/2.f;
	float scaleX = (_canvas.w-1)/2.f;
	float scaleY = (_canvas.h-1)/2.f;

	// Draw HGridLines
	for( int i = 0; i < m_nHGridLines; i++ )
	{
		int ofsY = (int) (m_pHGridLines[i] * scaleY + centerY);
		pDevice->ClipDrawHorrLine( _clip, WgCoord(_canvas.x,ofsY), _canvas.w, m_gridColor );
	}

	// Draw VGridLines
	for( int i = 0; i < m_nVGridLines; i++ )
	{
		int ofsX = (int) (m_pVGridLines[i] * scaleX + centerX);
		pDevice->ClipDrawVertLine( _clip, WgCoord(ofsX,_canvas.y), _canvas.h, m_gridColor );
	}

    // Nothing to draw (yet)
    if(m_nLinePoints == 0)
        return;

    
	// Draw the oscilloscope line
	const int nPoints = m_nLinePoints > _canvas.w ? _canvas.w : m_nLinePoints;

	// Need two extra point for anti-aliasing
	const int allocSize = sizeof(float)*(nPoints+1);

	float* pYval = reinterpret_cast<float*>(WgBase::MemStackAlloc(allocSize));
	
	for( int i = 0; i < nPoints+1; i++ )
	{
		pYval[i] = centerY + scaleY*m_pLinePoints[i];
	}

	m_iNextPixel = 0;
	antiAlias(nPoints, _canvas.x, pYval);

	pDevice->ClipPlotPixels(_clip, m_iNextPixel, m_pAAPix, m_lineColor, m_pAACol);

	WgBase::MemStackRelease(allocSize);

	// Blit markers

	for( int i = 0; i < m_nMarkers; i++ )
	{
		WgRect dest;

		int x = m_pMarkers[i].x;
		int y = (int) (m_pMarkers[i].y*scaleY+centerY);

		dest.SetSize( m_pMarkerGfx->Size() );
		dest.x = x - dest.w / 2;
		dest.y = y - dest.h / 2;

		pDevice->ClipBlitBlock( _clip, m_pMarkerGfx->GetBlock(WG_MODE_NORMAL), dest );
	}

}

void WgOscilloscope::plot(const int x, const int y, const float alpha)
{
  if(m_iNextPixel < WG_OSC_PIXEL_BUFFER_SIZE-1)
  {
      m_pAAPix[m_iNextPixel] = WgCoord(x, y);
      m_pAACol[m_iNextPixel] = m_lineColor;
      m_pAACol[m_iNextPixel].a = (Uint8)255*alpha;
      
      ++m_iNextPixel;
  }
  else
  {
      //DBGM(DBG_FFT, ("-----WARNING!------- m_iNextPixel=%i", m_iNextPixel));
  }
    
  //DBG_ASSERT(m_iNextPixel < WG_OSC_PIXEL_BUFFER_SIZE);
}


// Xiaolin Wu's line algorithm
void WgOscilloscope::antiAlias(const int nPoints, const int x_offset, const float *pYval)
{
    int   x0i,x1i;
	float x0,x1,y0,y1,yprev;
	float dx;
	float dy;
    float intery, gradient;
    int xpxl1, xpxl2;
    int ypxl1, ypxl2;
    float xgap;
    float xend;
    float yend;
    
	yprev = pYval[0];
    
	for(x0i = 0; x0i < nPoints; x0i++)
	{
		x1i = x0i + 1;
		y0 = yprev;
		DBG_ASSERT(x1i <= (nPoints+1));
		y1 = pYval[x1i];
        yprev = y1;
        
        x0 = float(x0i);
        x1 = float(x1i);
        
		DBG_ASSERT( y1 > 0.0f || y1 <= 0.0f );

		// Check for NaN
        if(!( y1 > 0.0f || y1 <= 0.0f ))
            y1 = 0.0f;
        
        
        bool steep = fabsf(y1 - y0) > abs(x1 - x0);
        
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
            plot(ypxl1 +     x_offset, xpxl1, rfpart(yend) * xgap);
            plot(ypxl1 + 1 + x_offset, xpxl1,  fpart(yend) * xgap);
        } else {
            plot(xpxl1 +     x_offset, ypxl1,     rfpart(yend) * xgap);
            plot(xpxl1 +     x_offset, ypxl1 + 1,  fpart(yend) * xgap);
        }
        intery = yend + gradient;
        
        // Second end point
        xend = round(x1);
        yend = y1 + gradient * (xend - x1);
        xgap = fpart(x1 + 0.5);
        xpxl2 = xend;
        ypxl2 = ipart(yend);
        
        if (steep) {
            plot(ypxl2 +     x_offset, xpxl2, rfpart(yend) * xgap);
            plot(ypxl2 + 1 + x_offset, xpxl2,  fpart(yend) * xgap);
        } else {
            plot(xpxl2 +     x_offset, ypxl2,     rfpart(yend) * xgap);
            plot(xpxl2 +     x_offset, ypxl2 + 1,  fpart(yend) * xgap);
        }
        
        for (int x=xpxl1+1; x<xpxl2; x++) {
            if (steep) {
                plot(ipart(intery) +     x_offset, x, rfpart(intery));
                plot(ipart(intery) + 1 + x_offset, x,  fpart(intery));
            } else {
                plot(x + x_offset, ipart (intery),  rfpart(intery));
                plot(x + x_offset, ipart (intery)+1, fpart(intery));
            }
            intery = intery + gradient;
        }
	}
    
	return;
}

