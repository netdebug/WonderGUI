
#include <wg_simplevolumemeter.h>
#include <wg_gfxdevice.h>

static const char	c_widgetType[] = {"SimpleVolumeMeter"};


//____ Constructor ____________________________________________________________

WgSimpleVolumeMeter::WgSimpleVolumeMeter()
{
	m_direction = WG_UP;

	m_sectionColors[0] = WgColor::green;
	m_sectionColors[1] = WgColor::yellow;
	m_sectionColors[2] = WgColor::red;
	
	m_fSectionHeight[0] = 0.75f;
	m_fSectionHeight[1] = 0.18f;
	m_fSectionHeight[2] = 0.07f;
	
	m_fHoldHeight = 0.10f;
	m_bStereo = false;
	m_fPeak[0] = 0.f;
	m_fPeak[1] = 0.f;
	m_fHold[0] = 0.f;
	m_fHold[1] = 0.f;

    m_fGap = 0.1f;
    m_fSidePadding = 0.1f;
    
	_updateIValues( WgSize(0,0) );
}

//____ Destructor _____________________________________________________________

WgSimpleVolumeMeter::~WgSimpleVolumeMeter()
{
}

//____ Type() _________________________________________________________________

const char * WgSimpleVolumeMeter::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgSimpleVolumeMeter::GetClass()
{
	return c_widgetType;
}

//____ SetColors() ___________________________________________________________

void WgSimpleVolumeMeter::SetColors( WgColor bottom, WgColor middle, WgColor top )
{
	if( bottom != m_sectionColors[0] || middle != m_sectionColors[1] || top != m_sectionColors[2] )
	{
		m_sectionColors[0] = bottom;
		m_sectionColors[1] = middle;
		m_sectionColors[2] = top;
		_requestRender();
	}
}

//____ SetSections() _________________________________________________________

void WgSimpleVolumeMeter::SetSections( float bottomFraction, float topFraction )
{
	WG_LIMIT( bottomFraction, 0.f, 1.f );
	WG_LIMIT( topFraction, 0.f, 1.f - bottomFraction );
		
	float middleFraction = 1.f - bottomFraction - topFraction;
	
	if( bottomFraction != m_fSectionHeight[0] || topFraction != m_fSectionHeight[2] )
	{
		m_fSectionHeight[0] = bottomFraction;
		m_fSectionHeight[1] = middleFraction;
		m_fSectionHeight[2] = topFraction;
		_updateIValues( PixelSize() );
		_requestRender();
	}
}

//____ SetHoldHeight() ________________________________________________________

void WgSimpleVolumeMeter::SetHoldHeight( float fraction )
{
	WG_LIMIT( fraction, 0.f, 0.25f );
	
	if( m_fHoldHeight != fraction )
	{
		m_fHoldHeight = fraction;
        _updateIValues( PixelSize() );
		_requestRender();
	}
}

//____ SetValue() ______________________________________________________________

void WgSimpleVolumeMeter::SetValue( float peak, float hold )
{
	WG_LIMIT( peak, 0.f, 1.f );
	WG_LIMIT( hold, 0.f, 1.f );

 	m_fPeak[0] = peak;
	m_fHold[0] = hold;

	WgSize sz = PixelSize();

    int length = (m_direction == WG_LEFT || m_direction == WG_RIGHT) ? sz.w : sz.h;

	int	iPeak = peak * length;
	int iHold = _calcIHold(hold, sz);
	
	if( m_bStereo )
	{
		m_bStereo = false;
		_requestRender();
	}
	else if( m_iPeak[0] != iPeak || m_iHold[0] != iHold )
	{
		_requestRenderPartial( sz, iPeak, iHold, 0, 0 );
	}

	m_iPeak[0] = iPeak;
	m_iHold[0] = iHold;
}

void WgSimpleVolumeMeter::SetValue( float leftPeak, float leftHold, float rightPeak, float rightHold )
{
	WG_LIMIT( leftPeak, 0.f, 1.f );
	WG_LIMIT( leftHold, 0.f, 1.f );
	WG_LIMIT( rightPeak, 0.f, 1.f );
	WG_LIMIT( rightHold, 0.f, 1.f );

 	m_fPeak[0] = leftPeak;
	m_fPeak[1] = rightPeak;
	m_fHold[0] = leftHold;
	m_fHold[1] = rightHold;

	WgSize sz = PixelSize();

    int length = (m_direction == WG_LEFT || m_direction == WG_RIGHT) ? sz.w : sz.h;

	int	iPeakL = leftPeak * length;
	int	iPeakR = rightPeak * length;
	int iHoldL = _calcIHold(leftHold, sz);
	int iHoldR = _calcIHold(rightHold, sz);

	if( !m_bStereo )
	{
		m_bStereo = true;
		_requestRender();		
	}
	else if( m_iPeak[0] != iPeakL || m_iHold[0] != iHoldL || m_iPeak[1] != iPeakR || m_iHold[1] != iHoldR )
	{
		_requestRenderPartial( sz, iPeakL, iHoldL, iPeakR, iHoldR );
	}

	m_iPeak[0] = iPeakL;
	m_iPeak[1] = iPeakR;
	m_iHold[0] = iHoldL;
	m_iHold[1] = iHoldR;

}

//____ SetDirection() _________________________________________________________

void WgSimpleVolumeMeter::SetDirection(WgDirection direction)
{
	if (direction != m_direction)
	{
		m_direction = direction;
		_updateIValues(PixelSize());
		_requestResize();
		_requestRender();
	}
}

//____ _requestRenderPartial() _________________________________________________

void WgSimpleVolumeMeter::_requestRenderPartial( WgSize sz, int newLeftPeak, int newLeftHold, int newRightPeak, int newRightHold )
{
	int	beg = INT_MAX, end = INT_MIN;
	
	if( newLeftPeak != m_iPeak[0] )
	{
		beg = WgMin(newLeftPeak,m_iPeak[0]);
		end = WgMax(newLeftPeak,m_iPeak[0]);
	}

	if( m_bStereo && newRightPeak != m_iPeak[1] )
	{
		int b = WgMin(newRightPeak,m_iPeak[1]);
		if(beg>b)
			beg = b;

		int e = WgMax(newRightPeak,m_iPeak[1]);
		if(end<e)
			end = e;
	}

	if( m_iHoldHeight != 0 )
	{
		if( newLeftHold != m_iHold[0] )
		{
			int b = WgMin(newLeftHold,m_iHold[0]) - m_iHoldHeight;
			if(beg>b)
				beg = b;

			int e = WgMax(newLeftHold,m_iHold[0]);
			if(end<e)
				end = e;
		}
		
		if( m_bStereo && newRightHold != m_iHold[1] )
		{
			int b = WgMin(newRightHold,m_iHold[1]) - m_iHoldHeight;
			if(beg>b)
				beg = b;

			int e = WgMax(newRightHold,m_iHold[1]);
			if(end<e)
				end = e;
		}
	}
	
	switch( m_direction )
	{ 
		case WG_UP:
			_requestRender(WgRect(0, sz.h - end, sz.w, end - beg));
			break;
		case WG_DOWN:
			_requestRender(WgRect(0, beg, sz.w, end - beg));
			break;
		case WG_LEFT:
			_requestRender(WgRect(sz.w - end, 0, end - beg, sz.h));
			break;
		case WG_RIGHT:
			_requestRender(WgRect(beg, 0, end - beg, sz.h));
			break;
	}
}


//____ _calcIHold() ____________________________________________________________

int WgSimpleVolumeMeter::_calcIHold( float holdValue, WgSize canvas )
{
	if( m_iHoldHeight == 0 )
		return 0;					// Should not be visible.

	int height = m_iHoldHeight;
	
	int canvasLength = m_direction == WG_UP || m_direction == WG_DOWN ? canvas.h : canvas.w;

	int ofs = (int) (holdValue * canvasLength);

	if( ofs > m_iSectionHeight[0] )
	{
		if( ofs - height < m_iSectionHeight[0] )
			ofs = m_iSectionHeight[0] + height;
		else if( ofs > m_iSectionHeight[0] + m_iSectionHeight[1] )
		{
			if( ofs - height < m_iSectionHeight[0] + m_iSectionHeight[1] )
				ofs = m_iSectionHeight[0] + m_iSectionHeight[1] + height;
		}
	}

	return ofs;
}

//____ PreferredPixelSize() ________________________________________________________________

WgSize WgSimpleVolumeMeter::PreferredPixelSize() const
{
	return m_direction == WG_UP || m_direction == WG_DOWN ? WgSize(9,20) : WgSize(20,9);
}

//____ _onNewSize() ____________________________________________________________________

void WgSimpleVolumeMeter::_onNewSize( const WgSize& size )
{
	_updateIValues( size );
	_requestRender();
}


//____ _onRender() _____________________________________________________________________

void WgSimpleVolumeMeter::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	if( !m_bEnabled )
		return;

    if( m_bStereo )
	{
		WgRect r = _canvas;

		if (m_direction == WG_UP || m_direction == WG_DOWN)
		{
			r.w = (r.w - m_iGap) / 2 - m_iSidePadding;
			r.x += m_iSidePadding;
		}
		else
		{
			r.h = (r.h - m_iGap) / 2 - m_iSidePadding;
			r.y += m_iSidePadding;
		}

		_renderBar( pDevice, 0, r, _clip );

		if (m_direction == WG_UP || m_direction == WG_DOWN)
			r.x += r.w + m_iGap;
		else
			r.y += r.h + m_iGap;

		_renderBar( pDevice, 1, r, _clip );
		
	}
	else 
	{
        WgRect r = _canvas;

		if (m_direction == WG_UP || m_direction == WG_DOWN)
		{
			r.w = r.w - 2 * m_iSidePadding;
			r.x += m_iSidePadding;
		}
		else
		{
			r.h = r.h - 2 * m_iSidePadding;
			r.y += m_iSidePadding;
		}
		_renderBar( pDevice, 0, r, _clip );
	}
}

//____ _renderBar()_____________________________________________________________

void WgSimpleVolumeMeter::_renderBar( WgGfxDevice * pDevice, int nb, const WgRect& _rect, const WgRect& _clip )
{
	int peakHeight 	= m_iPeak[nb];
	int holdOfs 	= m_iHold[nb];
	
	// Possibly render Hold
	
	if( m_iHoldHeight > 0 )
	{
		if( holdOfs - m_iHoldHeight > peakHeight )				// Render Hold separately if separated from Peak
		{
			WgColor c;
			
			if( holdOfs <= m_iSectionHeight[0] )
				c = m_sectionColors[0];
			else if( holdOfs > m_iSectionHeight[0] + m_iSectionHeight[1] )
				c = m_sectionColors[2];
			else 
				c = m_sectionColors[1];

			WgRect r = _rect;
			switch (m_direction)
			{
			case WG_UP:
				r.y += _rect.h - holdOfs;
				r.h = m_iHoldHeight;
				break;
			case WG_DOWN:
				r.y += holdOfs - m_iHoldHeight;
				r.h = m_iHoldHeight;
				break;
			case WG_LEFT:
				r.x += _rect.w - holdOfs;
				r.w = m_iHoldHeight;
				break;
			case WG_RIGHT:
				r.x += holdOfs - m_iHoldHeight;
				r.w = m_iHoldHeight;
				break;
			}

			pDevice->Fill( WgRect( r, _clip ), c );
		}
		else if( holdOfs > peakHeight )
			peakHeight = m_iHold[nb];							// Hold and Peak are connected, so we let Hold extend the peakHeight.
	}
	
	// Render Peak
		
	int ofs = 0;
	
	for( int i = 0 ; i < 3 ; i++ )
	{
		if( peakHeight <= 0 )
			break;
		
		int sectionHeight = m_iSectionHeight[i];
		if( sectionHeight > peakHeight )
			sectionHeight = peakHeight;
		
		WgRect r = _rect;
		switch (m_direction)
		{
			case WG_UP:
				r.y += _rect.h - ofs - sectionHeight;
				r.h = sectionHeight;
				break;
			case WG_DOWN:
				r.y += ofs;
				r.h = sectionHeight;
				break;
			case WG_LEFT:
				r.x += _rect.w - ofs - sectionHeight;
				r.w = sectionHeight;
				break;
			case WG_RIGHT:
				r.x += ofs;
				r.w = sectionHeight;
				break;
		}

		pDevice->Fill( WgRect( r, _clip ), m_sectionColors[i] );
		
		ofs += sectionHeight;
		peakHeight -= sectionHeight;
	}
		
}

//____ _updateIValues() ______________________________________________________

void WgSimpleVolumeMeter::_updateIValues( WgSize sz )
{
	int length = sz.h;
	int width = sz.w;

	if (m_direction == WG_LEFT || m_direction == WG_RIGHT)
		std::swap(length, width);

    m_iGap = (int) (width * m_fGap);
    if( m_iGap == 0 && m_fGap > 0.f )
        m_iGap = 1;

    m_iSidePadding = (int) (width * m_fSidePadding);
    if( m_iSidePadding == 0 && m_fSidePadding > 0.f )
        m_iSidePadding = 1;
				
	m_iHoldHeight = m_fHoldHeight * length;
	if( m_iHoldHeight == 0 && m_fHoldHeight > 0.f )
		m_iHoldHeight = 1;

	m_iSectionHeight[0] = (int) (m_fSectionHeight[0] * length + 0.5f);
	m_iSectionHeight[1] =  ((int)((m_fSectionHeight[0] + m_fSectionHeight[1]) * length + 0.5f)) - m_iSectionHeight[0];
	m_iSectionHeight[2] = length - m_iSectionHeight[1] - m_iSectionHeight[0];

	m_iPeak[0] = m_fPeak[0] * length;
	m_iPeak[1] = m_fPeak[1] * length;

    m_iHold[0] = _calcIHold( m_fHold[0], sz );
	m_iHold[1] = _calcIHold( m_fHold[1], sz );
}

//____ _onCloneContent() _________________________________________________________________ 

void WgSimpleVolumeMeter::_onCloneContent( const WgWidget * _pOrg )
{
	const WgSimpleVolumeMeter * pOrg = static_cast<const WgSimpleVolumeMeter*>(_pOrg);

	for( int i = 0 ; i < 3 ; i++ )
	{
		m_sectionColors[i] 	= pOrg->m_sectionColors[i];
		m_fSectionHeight[i] = pOrg->m_fSectionHeight[i];
	}
	
	m_direction = pOrg->m_direction;
	m_fHoldHeight = pOrg->m_fHoldHeight;
	m_fGap = pOrg->m_fGap;
	m_fSidePadding = pOrg->m_fSidePadding;
	m_bStereo = pOrg->m_bStereo;
	m_fPeak[0] = pOrg->m_fPeak[0];
	m_fPeak[1] = pOrg->m_fPeak[1];
	m_fHold[0] = pOrg->m_fHold[0];
	m_fHold[1] = pOrg->m_fHold[1];
	
	_updateIValues( PixelSize() );
}

//____ _onAlphaTest() ____________________________________________________________________

bool WgSimpleVolumeMeter::_onAlphaTest( const WgCoord& ofs )
{
	return false;
}
