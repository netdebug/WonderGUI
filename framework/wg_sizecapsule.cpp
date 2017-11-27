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

#include <wg_sizecapsule.h>

static const char	c_widgetType[] = {"SizeCapsule"};

//____ Constructor ____________________________________________________________

WgSizeCapsule::WgSizeCapsule() : m_pointsMax(INT_MAX,INT_MAX), m_pixelsMax(INT_MAX,INT_MAX)
{
}

//____ Destructor _____________________________________________________________

WgSizeCapsule::~WgSizeCapsule()
{
}

//____ Type() _________________________________________________________________

const char * WgSizeCapsule::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgSizeCapsule::GetClass()
{
	return c_widgetType;
}

//____ SetPreferredSize() _____________________________________________________

void WgSizeCapsule::SetPreferredSize( WgSize _size )
{
    
	if( _size != m_pixelsPreferred )
	{
		m_pointsPreferred = _size;
        m_pixelsPreferred = WgSize(_size.w*m_scale>>WG_SCALE_BINALS, _size.h*m_scale>>WG_SCALE_BINALS);
		_requestResize();
	}
}

//____ SetSizes() _____________________________________________________________

void WgSizeCapsule::SetSizes( WgSize _min, WgSize _preferred, WgSize _max )
{

	m_pointsMin = _min;
	m_pointsPreferred = _preferred;
	m_pointsMax = _max;

    m_pixelsMin = WgSize(_min.w*m_scale>>WG_SCALE_BINALS, _min.h*m_scale>>WG_SCALE_BINALS);
    m_pixelsPreferred = WgSize(_preferred.w*m_scale>>WG_SCALE_BINALS, _preferred.h*m_scale>>WG_SCALE_BINALS);
    m_pixelsMax = WgSize(_max.w*m_scale>>WG_SCALE_BINALS, _max.h*m_scale>>WG_SCALE_BINALS);
    _requestResize();
}

//____ SetMinSize() ___________________________________________________________

void WgSizeCapsule::SetMinSize( WgSize _size )
{

    if( _size != m_pixelsMin )
	{
		m_pointsMin = _size;
        m_pixelsMin = WgSize(_size.w*m_scale>>WG_SCALE_BINALS, _size.h*m_scale>>WG_SCALE_BINALS);
		_requestResize();
	}
}

//____ SetMaxSize() ___________________________________________________________

void WgSizeCapsule::SetMaxSize( WgSize _size )
{

    if( _size != m_pixelsMax )
	{
		m_pointsMax = _size;
        m_pixelsMax =  WgSize(_size.w*m_scale>>WG_SCALE_BINALS, _size.h*m_scale>>WG_SCALE_BINALS);
		_requestResize();
	}
}

//____ PreferredSize() ________________________________________________________

WgSize WgSizeCapsule::PreferredSize() const 
{

	if( m_hook.Widget() )
	{
		WgSize pref = m_hook.Widget()->PreferredSize();

		if( m_pixelsPreferred.w != 0 )
		{
			pref.w = m_pixelsPreferred.w;
		}
		if( m_pixelsPreferred.h != 0 )
		{
			pref.h = m_pixelsPreferred.h;
		}

		// Constrain against min/max, taking WidthForHeight/HeightForWidth into account.
		//TODO: Check so we don't have any corner cases that breaks the constraints and
		// and that priorities between preferred height/width are reasonable.

		WgSize min = MinSize();
		WgSize max = MaxSize();

		if( pref.w < min.w )
		{
			pref.w = min.w;
			pref.h = HeightForWidth(pref.w);
		}

		if( pref.h < min.h )
		{
			pref.h = min.h;
			pref.w = WidthForHeight(pref.h);
			if( pref.w < min.w )
				pref.w = min.w;
		}

		if( pref.w > max.w )
		{
			if( pref.h > max.h )
				pref = max;
			else
			{
				pref.w = max.w;
				pref.h = HeightForWidth(pref.w);
				if( pref.h > max.h )
					pref.h = max.h;
			}
		}
		else if( pref.h > max.h )
		{
			if( pref.w > max.w )
				pref = max;
			else
			{
				pref.h = max.h;
				pref.w = WidthForHeight(pref.h);
				if( pref.w > max.w )
					pref.w = max.w;
			}
		}



		return pref;
	}
	else
	{
		WgSize pref = m_pixelsPreferred;
		return pref;
	}
}

//____ MinSize() ______________________________________________________________

WgSize WgSizeCapsule::MinSize() const 
{

	if( m_hook.Widget() )
		return WgSize::Max(m_pixelsMin,m_hook.Widget()->MinSize());
	else
		return m_pixelsMin;
}

//____ MaxSize() ______________________________________________________________

WgSize WgSizeCapsule::MaxSize() const 
{
	if( m_hook.Widget() )
		return WgSize::Min(m_pixelsMax,m_hook.Widget()->MaxSize());
	else
		return m_pixelsMax;
}

//____ HeightForWidth() _______________________________________________________

int WgSizeCapsule::HeightForWidth( int width ) const
{
	if( m_pixelsPreferred.h != 0 )
	{
		int h = m_pixelsPreferred.h;
		
		if( m_hook.Widget() )
		{
			int max = m_hook.Widget()->MaxSize().h;
			int min = m_hook.Widget()->MinSize().h;
			WG_LIMIT( h, min, max );
		}
		return h;
	}
	else if( m_hook.Widget() )
	{
		int h = m_hook.Widget()->HeightForWidth(width);
		WG_LIMIT( h, m_pixelsMin.h, m_pixelsMax.h );
		return h;
	}
	else
		return m_pixelsMin.h;
}

//____ WidthForHeight() _______________________________________________________

int WgSizeCapsule::WidthForHeight( int height ) const
{
	if( m_pixelsPreferred.w != 0 )
	{
		int w = m_pixelsPreferred.w;

		if( m_hook.Widget() )
		{
			int max = m_hook.Widget()->MaxSize().w;
			int min = m_hook.Widget()->MinSize().w;
			WG_LIMIT( w, min, max );
		}
		return w;
	}
	else if( m_hook.Widget() )
	{
		int w = m_hook.Widget()->WidthForHeight(height);
		WG_LIMIT( w, m_pixelsMin.w, m_pixelsMax.w );
		return w;
	}
	else
		return m_pixelsMin.w;
}

//____ _setScale() ______________________________________________________

void WgSizeCapsule::_setScale( int scale )
{
    WgCapsule::_setScale(scale);
    
    m_pixelsMin = WgSize(m_pointsMin.w*m_scale>>WG_SCALE_BINALS, m_pointsMin.h*m_scale>>WG_SCALE_BINALS);
    m_pixelsPreferred = WgSize(m_pointsPreferred.w*m_scale>>WG_SCALE_BINALS, m_pointsPreferred.h*m_scale>>WG_SCALE_BINALS);

    if( m_pointsMax.w == INT_MAX )
        m_pixelsMax.w = INT_MAX;
    else
        m_pixelsMax.w = m_pointsMax.w*m_scale>>WG_SCALE_BINALS;
    
    if( m_pointsMax.h == INT_MAX )
        m_pixelsMax.h = INT_MAX;
    else
        m_pixelsMax.h = m_pointsMax.h*m_scale>>WG_SCALE_BINALS;

    _requestResize();
}
