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

#include <wg_scrollpanel.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_patches.h>
#include <wg_event.h>
#include <wg_eventhandler.h>

#include <algorithm>



static const char	c_widgetType[] = {"ScrollPanel"};
static const char	c_hookType[] = {"ScrollHook"};


//____ Constructor ____________________________________________________________

WgScrollPanel::WgScrollPanel()
{
	m_sliderTargets[0].m_bHorizontal = false;
	m_sliderTargets[0].m_pParent = this;

	m_sliderTargets[1].m_bHorizontal = true;
	m_sliderTargets[1].m_pParent = this;

	m_bgColor = WgColor::White;
	m_contentOrigo = WG_NORTHWEST;
	m_widthPolicy = WG_DEFAULT;
	m_heightPolicy = WG_DEFAULT;


	m_stepSizeX		= 1;
	m_stepSizeY		= 1;
	m_jumpSizeX		= 0.75f;
	m_jumpSizeY		= 0.75f;

	m_viewPixOfs.x	= 0;
	m_viewPixOfs.y	= 0;

	m_bAutoHideSliderX = false;
	m_bAutoHideSliderY = false;

	m_bSliderBottom	= true;
	m_bSliderRight	= true;

	m_bAutoScrollX		= false;
	m_bAutoScrollY		= false;
	
	m_wheelForScrollX	= 2;
	m_wheelForScrollY	= 1;

	m_elements[WINDOW]._setParent(this);
	m_elements[XDRAG]._setParent(this);
	m_elements[YDRAG]._setParent(this);

	_updateElementGeo( WgSize(256,256) );
}

//____ ~WgScrollPanel() __________________________________________________

WgScrollPanel::~WgScrollPanel()
{
}

//____ Type() _________________________________________________________________

const char * WgScrollPanel::Type() const
{
	return GetClass();
}

const char * WgScrollPanel::GetClass( void )
{
	return c_widgetType;
}


//____ StepUp() _______________________________________________________________

bool WgScrollPanel::StepUp()
{
	int ofs = m_viewPixOfs.y - ((m_stepSizeY*m_scale) >> WG_SCALE_BINALS);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsY( ofs );
}

//____ StepDown() _____________________________________________________________
bool WgScrollPanel::StepDown()
{
	return SetViewPixelOfsY( m_viewPixOfs.y + ((m_stepSizeY*m_scale) >> WG_SCALE_BINALS));
}

//____ StepLeft() _____________________________________________________________
bool WgScrollPanel::StepLeft()
{
	int ofs = m_viewPixOfs.x - ((m_stepSizeX*m_scale) >> WG_SCALE_BINALS);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsX( ofs );
}

//____ StepRight() ____________________________________________________________

bool WgScrollPanel::StepRight()
{
	return SetViewPixelOfsX( m_viewPixOfs.x + ((m_stepSizeX*m_scale) >> WG_SCALE_BINALS) );
}


//____ JumpUp() _______________________________________________________________

bool WgScrollPanel::JumpUp()
{
	int ofs = m_viewPixOfs.y - (Uint32)(ViewPixelLenY() * m_jumpSizeY);
	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsY( (Uint32) ofs );
}

//____ JumpDown() _____________________________________________________________

bool WgScrollPanel::JumpDown()
{
	return SetViewPixelOfsY( (Uint32)(m_viewPixOfs.y + ViewPixelLenY() * m_jumpSizeY) );
}

//____ JumpLeft() _____________________________________________________________

bool WgScrollPanel::JumpLeft()
{
	int ofs = m_viewPixOfs.x - (Uint32)(ViewPixelLenX() * m_jumpSizeX);
	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsX( (Uint32) ofs );
}

//____ JumpRight() ____________________________________________________________

bool WgScrollPanel::JumpRight()
{
	return SetViewPixelOfsX( (Uint32)(m_viewPixOfs.x + ViewPixelLenX() * m_jumpSizeX) );
}

//____ _wheelRollX() ___________________________________________________________

bool WgScrollPanel::_wheelRollX(int distance)
{
	int ofs = m_viewPixOfs.x - (((m_stepSizeX*m_scale) >> WG_SCALE_BINALS) * distance * 3);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsX( ofs );
}

//____ _wheelRollY() ___________________________________________________________

bool WgScrollPanel::_wheelRollY(int distance)
{
	int ofs = m_viewPixOfs.y - (((m_stepSizeY*m_scale) >> WG_SCALE_BINALS) * distance * 3);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsY( ofs );
}

//____ SetJumpSizeX() _________________________________________________________

void WgScrollPanel::SetJumpSizeX( float viewFraction )
{
	if( viewFraction < 0.f )
		viewFraction = 0.f;
	if( viewFraction > 1.f )
		viewFraction = 1.f;

	m_jumpSizeX = viewFraction;
}

//____ SetJumpSizeY() _________________________________________________________

void WgScrollPanel::SetJumpSizeY( float viewFraction )
{
	if( viewFraction < 0.f )
		viewFraction = 0.f;
	if( viewFraction > 1.f )
		viewFraction = 1.f;

	m_jumpSizeY = viewFraction;
}

//____ IsHSliderVisible() ____________________________________________________

bool WgScrollPanel::IsHSliderVisible()
{
	return m_elements[XDRAG].m_bVisible;
}

//____ IsVSliderVisible() ____________________________________________________

bool WgScrollPanel::IsVSliderVisible()
{
	return m_elements[YDRAG].m_bVisible;
}


//____ ViewPixelLenX() ________________________________________________________

Uint32 WgScrollPanel::ViewPixelLenX()
{
	return m_elements[WINDOW].m_windowGeo.w;
}

//____ ViewPixelLenY() ________________________________________________________

Uint32 WgScrollPanel::ViewPixelLenY()
{
	return m_elements[WINDOW].m_windowGeo.h;
}

//____ ViewPointLenX() ________________________________________________________

Uint32 WgScrollPanel::ViewPointLenX()
{
    return (m_elements[WINDOW].m_windowGeo.w << WG_SCALE_BINALS) / m_scale;
}

//____ ViewPointLenY() ________________________________________________________

Uint32 WgScrollPanel::ViewPointLenY()
{
    return (m_elements[WINDOW].m_windowGeo.h << WG_SCALE_BINALS) / m_scale;
}


//____ ViewOfsX() _____________________________________________________________

float WgScrollPanel::ViewOfsX()
{
	Uint32 size = m_contentSize.w - ViewPixelLenX();
	if( size <= 0 )
		return 0.f;
	else
		return m_viewPixOfs.x / (float) size;

}

//____ ViewOfsY() _____________________________________________________________

float WgScrollPanel::ViewOfsY()
{
	Uint32 size = m_contentSize.h - ViewPixelLenY();
	if( size <= 0 )
		return 0.f;
	else
		return m_viewPixOfs.y / (float) size;
}

//____ ViewLenX() _____________________________________________________________

float WgScrollPanel::ViewLenX()
{
	if( m_contentSize.w == 0 )
		return 1.0f;

	float len = ViewPixelLenX()/(float)m_contentSize.w;

	if( len > 1.f )
		len = 1.f;

	return len;
}

//____ ViewLenY() _____________________________________________________________

float WgScrollPanel::ViewLenY()
{
	if( m_contentSize.h == 0 )
		return 1.0f;

	float len = ViewPixelLenY()/(float)m_contentSize.h;

	if( len > 1.f )
		len = 1.f;

	return len;
}

//____ SetViewPixelOfs() ______________________________________________________

bool WgScrollPanel::SetViewPixelOfs( int x, int y )
{
	if( x == m_viewPixOfs.x && y == m_viewPixOfs.y )
		return true;

	int		pixLenX = ViewPixelLenX();
	int		pixLenY = ViewPixelLenY();
	bool	bChangedX = false;
	bool	bChangedY = false;

	bool retVal = true;
    if( y < 0 )
    {
        y = 0;
        retVal = false;
    }
    if( y > m_contentSize.h - pixLenY )
	{
		y = m_contentSize.h - pixLenY;
		retVal = false;
	}

    if( x < 0 )
    {
        x = 0;
        retVal = false;
    }
	if( x > m_contentSize.w - pixLenX )
	{
		x = m_contentSize.w - pixLenX;
		retVal = false;
	}

	if( x != m_viewPixOfs.x )
	{
		m_viewPixOfs.x = x;
		bChangedX = true;
	}

	if( y != m_viewPixOfs.y )
	{
		m_viewPixOfs.y = y;
		bChangedY = true;
	}

	float	ofsX = ViewOfsX();
	float	ofsY = ViewOfsY();

	if( bChangedX )
	{
		m_sliderTargets[1]._updateSlider( ViewOfsX(), ViewLenX() );
	}

	if( bChangedY )
	{
		m_sliderTargets[0]._updateSlider( ViewOfsY(), ViewLenY() );
	}

	if( bChangedX || bChangedY )
	{
		m_elements[WINDOW].m_canvasGeo = _genContentCanvasGeo( m_elements[WINDOW].m_windowGeo, m_contentSize, m_contentOrigo, m_viewPixOfs );
		_requestRender( m_elements[WINDOW].m_windowGeo );
	}

	return retVal;
}

//____ SetViewPixelOfsX() _____________________________________________________

bool WgScrollPanel::SetViewPixelOfsX( int x )
{
	return SetViewPixelOfs(x, m_viewPixOfs.y);
}

//____ SetViewPixelOfsY() _____________________________________________________

bool WgScrollPanel::SetViewPixelOfsY( int y )
{
	return SetViewPixelOfs(m_viewPixOfs.x, y);
}


//____ SetViewPointOfs() _____________________________________________________

bool WgScrollPanel::SetViewPointOfs( int x, int y )
{
    return SetViewPixelOfs((x*m_scale)>>WG_SCALE_BINALS, (y*m_scale)>>WG_SCALE_BINALS);
}


//____ SetViewPointOfsX() _____________________________________________________

bool WgScrollPanel::SetViewPointOfsX( int x )
{
    return SetViewPixelOfs((x*m_scale)>>WG_SCALE_BINALS, m_viewPixOfs.y);
}

//____ SetViewPointOfsY() _____________________________________________________

bool WgScrollPanel::SetViewPointOfsY( int y )
{
    return SetViewPixelOfs(m_viewPixOfs.x,(y*m_scale)>>WG_SCALE_BINALS);
}


//____ SetViewOfs() ___________________________________________________________

bool WgScrollPanel::SetViewOfs( float x, float y )
{
	if( x < 0.f )
		x = 0;
	if( x > 1.f )
		x = 1.f;

	if( y < 0.f )
		y = 0;
	if( y > 1.f )
		y = 1.f;

	int	width = m_contentSize.w - ViewPixelLenX();
	int	height = m_contentSize.h - ViewPixelLenY();

	if( width < 0 )
		width = 0;

	if( height < 0 )
		height = 0;


	return SetViewPixelOfs( (Uint32)((width*x)+0.5f), (Uint32)((height*y)+0.5f) );
}

//____ SetViewOfsX() __________________________________________________________

bool WgScrollPanel::SetViewOfsX( float x )
{
	if( x < 0.f )
		x = 0;
	if( x > 1.f )
		x = 1.f;

	int	width = m_contentSize.w - ViewPixelLenX();
	if( width < 0 )
		width = 0;

	return SetViewPixelOfsX( (Uint32)((width*x)+0.5f) );
}

//____ SetViewOfsY() __________________________________________________________

bool WgScrollPanel::SetViewOfsY( float y )
{
	if( y < 0.f )
		y = 0;
	if( y > 1.f )
		y = 1.f;

	int	height = m_contentSize.h - ViewPixelLenY();
	if( height < 0 )
		height = 0;

	return SetViewPixelOfsY( (Uint32)((height*y)+0.5f) );
}

//____ SetContent() ___________________________________________________________

bool WgScrollPanel::SetContent( WgWidget * pContent )
{
	m_elements[WINDOW]._attachWidget(pContent);

	_updateElementGeo( PixelSize() );
	_requestRender( m_elements[WINDOW].m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	_requestResize();
	return true;

}

//____ SetHSlider() ________________________________________________________

bool WgScrollPanel::SetHSlider( WgHSlider* pSlider )
{
	// Remove us as target target from current Slider (if we have any)

	if( m_elements[XDRAG].Widget() )
		((WgWidgetSlider*)m_elements[XDRAG].Widget())->SetSliderTarget(0);

	//

	m_elements[XDRAG]._attachWidget(pSlider);

	if( pSlider )
		pSlider->SetSliderTarget( &m_sliderTargets[1] );

	_updateElementGeo( PixelSize() );
    if( pSlider )
        pSlider->SetSlider( ViewOfsX(), ViewLenX() );
	_requestRender( m_elements[XDRAG].m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	return true;
}

//____ SetVSlider() ________________________________________________________

bool WgScrollPanel::SetVSlider( WgVSlider* pSlider )
{
	// Remove us as target target from current Slider (if we have any)

	if( m_elements[YDRAG].Widget() )
		((WgWidgetSlider*)m_elements[YDRAG].Widget())->SetSliderTarget(0);

	//

	m_elements[YDRAG]._attachWidget(pSlider);

	//

	if( pSlider )
		pSlider->SetSliderTarget( &m_sliderTargets[0] );

	_updateElementGeo( PixelSize() );
    if(pSlider)
        pSlider->SetSlider( ViewOfsY(), ViewLenY() );
	_requestRender( m_elements[YDRAG].m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	return true;
}

//____ ReleaseContent() _______________________________________________________

WgWidget* WgScrollPanel::ReleaseContent()
{
	WgWidget * p = m_elements[WINDOW]._releaseWidget();
	_updateElementGeo( PixelSize() );
	return p;
}

//____ ReleaseHSlider() ____________________________________________________

WgHSlider* WgScrollPanel::ReleaseHSlider()
{
	WgHSlider * p = (WgHSlider*) m_elements[XDRAG]._releaseWidget();
	_updateElementGeo( PixelSize() );
	return p;
}

//____ ReleaseVSlider() ____________________________________________________

WgVSlider* WgScrollPanel::ReleaseVSlider()
{
	WgVSlider * p = (WgVSlider*) m_elements[YDRAG]._releaseWidget();
	_updateElementGeo( PixelSize() );
	return p;
}

//____ DeleteChild() __________________________________________________________

bool WgScrollPanel::DeleteChild( WgWidget * pWidget )
{
	if( pWidget == m_elements[XDRAG].Widget() )
		return SetHSlider(0);
	else if( pWidget == m_elements[YDRAG].Widget() )
		return SetVSlider(0);
	else if( pWidget == m_elements[WINDOW].Widget() )
		return SetContent(0);

	return false;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgScrollPanel::ReleaseChild( WgWidget * pWidget )
{
	if( pWidget == m_elements[XDRAG].Widget() )
		return ReleaseHSlider();
	else if( pWidget == m_elements[YDRAG].Widget() )
		return ReleaseVSlider();
	else if( pWidget == m_elements[WINDOW].Widget() )
		return ReleaseContent();

	return 0;
}

//____ DeleteAllChildren() ______________________________________________________

bool WgScrollPanel::DeleteAllChildren()
{
	SetHSlider(0);
	SetVSlider(0);
	SetContent(0);
	return true;
}

//____ ReleaseAllChildren() _____________________________________________________

bool WgScrollPanel::ReleaseAllChildren()
{
	ReleaseHSlider();
	ReleaseVSlider();
	ReleaseContent();
	return true;
}



//____ SetSliderAutoHide() _________________________________________________

void WgScrollPanel::SetSliderAutoHide( bool bHideHSlider, bool bHideVSlider )
{
	if( bHideHSlider == m_bAutoHideSliderX && bHideVSlider == m_bAutoHideSliderY )
		return;

	bool	bWasVisibleX = IsHSliderVisible();
	bool	bWasVisibleY = IsVSliderVisible();

	m_bAutoHideSliderX = bHideHSlider;
	m_bAutoHideSliderY = bHideVSlider;

	// Force a refresh of our subclass if its geometry has been affected.

	if( IsHSliderVisible() != bWasVisibleX || IsVSliderVisible() != bWasVisibleY )
		_updateElementGeo( PixelSize() );
}

//____ SetSliderPositions() ________________________________________________

void WgScrollPanel::SetSliderPositions( bool bBottom, bool bRight )
{
	if( bBottom == m_bSliderBottom && bRight == m_bSliderRight )
		return;

	m_bSliderBottom	= bBottom;
	m_bSliderRight	= bRight;

	_updateElementGeo( PixelSize() );
}

//____ SetFillerBlocks() ______________________________________________________

void WgScrollPanel::SetFillerBlocks( const WgBlocksetPtr& pBlocks )
{
	m_pFillerBlocks = pBlocks;
	_requestRender( m_geoFiller );
}

//____ FindWidget() ____________________________________________________________

WgWidget * WgScrollPanel::FindWidget( const WgCoord& pos, WgSearchMode mode )
{
	// Check XDRAG

	WgScrollHook * p = &m_elements[XDRAG];
	if( p->IsVisible() && p->m_pWidget && p->m_windowGeo.Contains( pos ) )
	{
		if( mode != WG_SEARCH_MARKPOLICY || p->m_pWidget->MarkTest( pos - p->m_windowGeo.Pos() ) )
			return p->m_pWidget;
	}

	// Check YDRAG

	p = &m_elements[YDRAG];
	if( p->IsVisible() && p->m_pWidget && p->m_windowGeo.Contains( pos ) )
	{
		if( mode != WG_SEARCH_MARKPOLICY || p->m_pWidget->MarkTest( pos - p->m_windowGeo.Pos() ) )
			return p->m_pWidget;
	}

	// Check WINDOW

	p = &m_elements[WINDOW];
	if( p->IsVisible() && p->m_pWidget && p->m_windowGeo.Contains( pos ) && p->m_canvasGeo.Contains( pos ) )
	{
		if( p->m_pWidget->IsContainer() )
		{
			WgWidget * pFound = p->m_pWidget->CastToContainer()->FindWidget( pos - p->m_canvasGeo.Pos(), mode );
			if( pFound )
				return pFound;
		}
		else if( mode != WG_SEARCH_MARKPOLICY || p->m_pWidget->MarkTest( pos - p->m_canvasGeo.Pos() ) )
			return p->m_pWidget;
	}

	// Check window background color

	if( m_bgColor.a != 0 && p->m_windowGeo.Contains( pos ) )
		return this;


	// Check our little corner square and geometry

	if( mode == WG_SEARCH_GEOMETRY || MarkTest( pos ) )
		return this;

	//

	return 0;
}

//____ PreferredPixelSize() ___________________________________________________________

WgSize WgScrollPanel::PreferredPixelSize() const
{
	WgSize sz;

	if (m_elements[WINDOW].m_pWidget)
		sz = m_elements[WINDOW]._paddedPreferredPixelSize(m_scale);

	if (m_elements[XDRAG].m_pWidget && !m_bAutoHideSliderX)
	{
		WgSize scrollbar = m_elements[XDRAG]._paddedPreferredPixelSize(m_scale);
		sz.h += scrollbar.h;
		if (scrollbar.w > sz.w)
			sz.w = scrollbar.w;
	}

	if (m_elements[YDRAG].m_pWidget && !m_bAutoHideSliderY)
	{
		WgSize scrollbar = m_elements[YDRAG]._paddedPreferredPixelSize(m_scale);
		sz.w += scrollbar.w;
		if (scrollbar.h > sz.h)
			sz.h = scrollbar.h;
	}

	return sz;
}

//____ MatchingPixelHeight() __________________________________________________

int WgScrollPanel::MatchingPixelHeight(int pixelWidth) const
{
	int height = 0;

	// If YDRAG needs to be present, we need to remove its width and consider its height.

	if (m_elements[YDRAG].m_pWidget && !m_bAutoHideSliderY)
	{
		WgSize sz = m_elements[YDRAG]._paddedPreferredPixelSize(m_scale);
		height = sz.h;
		pixelWidth -= sz.w;
	}

	int contentWidth = 0;
	if (m_elements[WINDOW].m_pWidget)
	{
		height = std::max(m_elements[WINDOW]._paddedMatchingPixelHeight(pixelWidth,m_scale), height);
		contentWidth = m_elements[WINDOW]._paddedPreferredPixelSize(m_scale).w;
	}

	// If XDRAG needs to be displayed at this width, we need to include its height.

	if (m_elements[XDRAG].m_pWidget && (!m_bAutoHideSliderX || contentWidth > pixelWidth) )
		height += m_elements[XDRAG]._paddedPreferredPixelSize(m_scale).h;


	return height;
}

//____ MatchingPixelWidth() ___________________________________________________

int WgScrollPanel::MatchingPixelWidth(int pixelHeight) const
{
	//TODO: Implement!!!

	return PreferredPixelSize().w;
}



//____ SetBgColor() ____________________________________________________________

void  WgScrollPanel::SetBgColor( WgColor color )
{
	if( color != m_bgColor )
	{
		m_bgColor = color;
		_requestRender( m_elements[WINDOW].m_windowGeo );
	}
}

//____ SetContentOrigo() _________________________________________________

void  WgScrollPanel::SetContentOrigo( WgOrigo origo )
{
	m_contentOrigo = origo;

	WgScrollHook * p = &m_elements[WINDOW];
	WgRect geo = _genContentCanvasGeo( p->m_windowGeo, m_contentSize, m_contentOrigo, m_viewPixOfs );

	if( geo != p->m_canvasGeo )
	{
		p->m_canvasGeo = geo;
		_requestRender( p->m_windowGeo );
	}
}

//____ SetContentSizePolicy() __________________________________________________

void  WgScrollPanel::SetContentSizePolicy( WgSizePolicy widthPolicy, WgSizePolicy heightPolicy )
{
	if( widthPolicy != m_widthPolicy || heightPolicy != m_heightPolicy )
	{
		m_widthPolicy = widthPolicy;
		m_heightPolicy = heightPolicy;

		_updateElementGeo( PixelSize() );
	}
}

//____ _calcContentSize() ______________________________________________________

WgSize WgScrollPanel::_calcContentSize( WgSize mySize )
{
	WgWidget * pContent = m_elements[WINDOW].Widget();

	if( !pContent )
		return WgSize(0,0);

	//

	if( !m_bAutoHideSliderX && m_elements[XDRAG].Widget() )
		mySize.h -= m_elements[XDRAG].Widget()->PreferredPixelSize().h;

	if( !m_bAutoHideSliderY && m_elements[YDRAG].Widget() )
		mySize.w -= m_elements[YDRAG].Widget()->PreferredPixelSize().w;

	//

	WgSize contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );

	if( contentSize.h > mySize.h && m_bAutoHideSliderY && m_elements[YDRAG].Widget() )
	{
		mySize.w -= m_elements[YDRAG].Widget()->PreferredPixelSize().w;
		contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );

		if( contentSize.w > mySize.w && m_bAutoHideSliderX && m_elements[XDRAG].Widget() )
		{
			mySize.h -= m_elements[XDRAG].Widget()->PreferredPixelSize().h;
			contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );
		}
	}
	else if( contentSize.w > mySize.w && m_bAutoHideSliderX && m_elements[XDRAG].Widget() )
	{
		mySize.h -= m_elements[XDRAG].Widget()->PreferredPixelSize().h;
		contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );

		if( contentSize.h > mySize.h && m_bAutoHideSliderY && m_elements[YDRAG].Widget() )
		{
			mySize.w -= m_elements[YDRAG].Widget()->PreferredPixelSize().w;
			contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );
		}
	}

	return contentSize;
}

//____ _updateElementGeo() _____________________________________________________

void WgScrollPanel::_updateElementGeo( WgSize mySize )
{

	WgSize newContentSize = _calcContentSize( mySize );

	WgRect	newDragX, newDragY, newWindow, newFiller;
	bool	bShowDragX = false, bShowDragY = false;

	// First get "default geometry" for them all, ignoring overlaps.

	newWindow = mySize;

	if( m_elements[XDRAG].Widget() )
	{
		newDragX.h = m_elements[XDRAG].Widget()->PreferredPixelSize().h;
		newDragX.w = newWindow.w;
		newDragX.x = 0;

		if( m_bSliderBottom )
			newDragX.y = newWindow.h - newDragX.h;
		else
			newDragX.y = 0;
	}

	if( m_elements[YDRAG].Widget() )
	{
		newDragY.w = m_elements[YDRAG].Widget()->PreferredPixelSize().w;
		newDragY.h = newWindow.h;
		newDragY.y = 0;

		if( m_bSliderRight )
			newDragY.x = newWindow.w - newDragY.w;
		else
			newDragY.x = 0;
	}

	// Determine which dragbars we need to show, using basic rules

	if( m_elements[XDRAG].Widget() && (newContentSize.w > newWindow.w || !m_bAutoHideSliderX) )
		bShowDragX = true;

	if( m_elements[YDRAG].Widget() && (newContentSize.h > newWindow.h || !m_bAutoHideSliderY) )
		bShowDragY = true;

	// See if showing one forces us to show the other

	if( bShowDragY && m_elements[XDRAG].Widget() && newContentSize.w > (newWindow.w - newDragY.w) )
		bShowDragX = true;

	if( bShowDragX && m_elements[YDRAG].Widget() && newContentSize.h > (newWindow.h - newDragY.h) )
		bShowDragY = true;


	// Adjust view layout to accomodate visible dragbars

	if( bShowDragY )
	{
		newWindow.w -= newDragY.w;
		if( !m_bSliderRight )
			newWindow.x += newDragY.w;
	}

	if( bShowDragX )
	{
		newWindow.h -= newDragX.h;
		if( !m_bSliderBottom )
			newWindow.y += newDragX.h;
	}

	// If both dragbars are visible we need to avoid overlap
	// and include a filler for the empty square.

	if( bShowDragX && bShowDragY )
	{
		newDragX.x = newWindow.x;
		newDragX.w = newWindow.w;

		newDragY.y = newWindow.y;
		newDragY.h = newWindow.h;

		newFiller.x = newDragY.x;
		newFiller.y = newDragX.y;
		newFiller.w = newDragY.w;
		newFiller.h = newDragX.h;
	}

	// Determine changes to views position and size over content before
	// we overwrite old values, so we can emit right signals once all is updated.
	// Update m_viewPixOfs at the same time.

	bool	bNewOfsX = false;
	bool	bNewOfsY = false;
	bool	bNewWidth = false;
	bool	bNewHeight = false;

	if( newWindow.h != m_elements[WINDOW].m_windowGeo.h )
		bNewHeight = true;

	if( newWindow.w != m_elements[WINDOW].m_windowGeo.w )
		bNewWidth = true;


	if( m_bAutoScrollX )
	{
		if( m_viewPixOfs.x + m_elements[WINDOW].m_windowGeo.w >= m_contentSize.w && newWindow.w < newContentSize.w )
		{
			bNewOfsX = true;
			m_viewPixOfs.x = newContentSize.w - newWindow.w;
		}
	}

	if( m_bAutoScrollY )
	{
		if( m_viewPixOfs.y + m_elements[WINDOW].m_windowGeo.h >= m_contentSize.h && newWindow.h < newContentSize.h )
		{
			bNewOfsY = true;
			m_viewPixOfs.y = newContentSize.h - newWindow.h;
		}
	}

	if( m_viewPixOfs.x + newWindow.w > newContentSize.w )
	{
		int x = newContentSize.w - newWindow.w;
		if( x < 0 )
			x = 0;
		if( x != m_viewPixOfs.x )
		{
			bNewOfsX = true;
			m_viewPixOfs.x = x;
		}
	}

	if( m_viewPixOfs.y + newWindow.h > newContentSize.h )
	{
		int y = newContentSize.h - newWindow.h;
		if( y < 0 )
			y = 0;
		if( y != m_viewPixOfs.y )
		{
			bNewOfsY = true;
			m_viewPixOfs.y = y;
		}
	}

	// If something visible has changed we need to update element geometry and request render.
	// This is more optimized than it looks like...

	if( newWindow != m_elements[WINDOW].m_windowGeo || bShowDragX != m_elements[XDRAG].m_bVisible || bShowDragY != m_elements[YDRAG].m_bVisible || newContentSize != m_contentSize )
	{
		m_elements[WINDOW].m_windowGeo = newWindow;
		m_elements[XDRAG].m_windowGeo = newDragX;
		m_elements[YDRAG].m_windowGeo = newDragY;
		m_elements[WINDOW].m_canvasGeo = _genContentCanvasGeo( newWindow, newContentSize, m_contentOrigo, m_viewPixOfs );
		m_elements[XDRAG].m_canvasGeo = newDragX;
		m_elements[YDRAG].m_canvasGeo = newDragY;
		m_elements[XDRAG].m_bVisible = bShowDragX;
		m_elements[YDRAG].m_bVisible = bShowDragY;

		_requestRender();

		// Notify sliders of their new size.

		if( bShowDragX )
			m_elements[XDRAG].Widget()->_onNewSize(newDragX.Size());
		if( bShowDragY )
			m_elements[YDRAG].Widget()->_onNewSize(newDragY.Size());
	}

	// If contentSize has changed we save changes and set flags

	bool bNewContentWidth = false;
	bool bNewContentHeight = false;

	if( newContentSize.w != m_contentSize.w )
		bNewContentWidth = true;

	if( newContentSize.h != m_contentSize.h )
		bNewContentHeight = true;

	if( bNewContentWidth || bNewContentHeight )
	{
		m_contentSize = newContentSize;

		// Notify content of its new size.

		if( m_elements[WINDOW].Widget() )
			m_elements[WINDOW].Widget()->_onNewSize(newContentSize);
	}

	// Notify sliders of any change to content size, view size or view offset.

	if( bNewOfsX || bNewWidth || bNewContentWidth )
		m_sliderTargets[1]._updateSlider( ViewOfsX(), ViewLenX() );

	if( bNewOfsY || bNewHeight || bNewContentHeight )
		m_sliderTargets[0]._updateSlider( ViewOfsY(), ViewLenY() );

/*
	// Send signals if views size or position over content has changed
	// or contents size has changed.

	if( bNewOfsX || bNewContentWidth )
		Emit( ViewPosX(), ViewOfsX() );
	if( bNewOfsX )
		Emit( ViewPosPixelX(), m_viewPixOfs.x );

	if( bNewOfsY || bNewContentHeight )
		Emit( ViewPosY(), ViewOfsY() );
	if( bNewOfsY )
		Emit( ViewPosPixelY(), m_viewPixOfs.y );

	if( bNewWidth || bNewContentWidth )
		Emit( ViewSizeX(), ViewLenX() );
	if( bNewWidth )
		Emit( ViewSizePixelX(), ViewPixelLenX() );

	if( bNewHeight || bNewContentHeight )
		Emit( ViewSizeY(), ViewLenY() );
	if( bNewHeight )
		Emit( ViewSizePixelY(), ViewPixelLenY() );

	if( bNewOfsX || bNewWidth || bNewContentWidth )
		Emit( ViewPosSizeX(), ViewOfsX(), ViewLenX() );
	if( bNewOfsX || bNewWidth )
		Emit( ViewPosSizePixelX(), m_viewPixOfs.x, ViewPixelLenX() );

	if( bNewOfsY || bNewHeight || bNewContentHeight )
		Emit( ViewPosSizeY(), ViewOfsY(), ViewLenY() );
	if( bNewOfsY || bNewHeight )
		Emit( ViewPosSizePixelY(), m_viewPixOfs.y, ViewPixelLenY() );

	if( bNewOfsX || bNewOfsY || bNewContentHeight || bNewContentWidth )
		Emit( ViewPos(), ViewOfsX(), ViewOfsY() );
	if( bNewOfsX || bNewOfsY )
		Emit( ViewPosPixel(), m_viewPixOfs.x, m_viewPixOfs.y );

	if( bNewWidth || bNewHeight || bNewContentHeight || bNewContentWidth )
		Emit( ViewSize(), ViewLenX(), ViewLenY() );
	if( bNewWidth || bNewHeight )
		Emit( ViewSizePixel(), ViewPixelLenX(), ViewPixelLenY() );
*/
}

//____ _genContentCanvasGeo() __________________________________________________

WgRect WgScrollPanel::_genContentCanvasGeo( const WgRect& window, WgSize contentSize, WgOrigo origo, WgCoord viewOfs )
{
	WgRect	out( window.Pos() - viewOfs, contentSize);

	if( window.w > contentSize.w )
	{
		WgRect r = WgUtil::OrigoToRect( origo, WgSize(window.w,1), WgSize(contentSize.w,1) );
		out.x = window.x + r.x;
	}

	if( window.h > contentSize.h )
	{
		WgRect r = WgUtil::OrigoToRect( origo, WgSize(1,window.h), WgSize(1,contentSize.h) );
		out.y = window.y + r.y;
	}

	return out;
}

//____ _onNewSize() ____________________________________________________________

void WgScrollPanel::_onNewSize( const WgSize& size )
{
	_updateElementGeo( size );
}

//____ _onEvent() ______________________________________________________________

void WgScrollPanel::_onEvent( const WgEvent::Event * _pEvent, WgEventHandler * pHandler )
{
	switch( _pEvent->Type() )
	{
		case WG_EVENT_MOUSEWHEEL_ROLL:
		{			
			const WgEvent::MouseWheelRoll * pEvent = static_cast<const WgEvent::MouseWheelRoll*>(_pEvent);

			if( m_elements[WINDOW].m_windowGeo.Contains(pEvent->PointerPixelPos()) )
			{
				int wheel = pEvent->Wheel();

				if( wheel == m_wheelForScrollY )
					_wheelRollY( pEvent->Distance() );
				else if( wheel == m_wheelForScrollX )
					_wheelRollX( pEvent->Distance() );
			}
		}
		break;

		default:
			pHandler->ForwardEvent(_pEvent);
		break;
	}
}

//____ _renderPatches() ________________________________________________________

void WgScrollPanel::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{

	// We start by eliminating dirt outside our geometry

	WgPatches 	patches( _pPatches->Size() );								// TODO: Optimize by pre-allocating?

	for( const WgRect * pRect = _pPatches->Begin() ; pRect != _pPatches->End() ; pRect++ )
	{
		if( _window.IntersectsWith( *pRect ) )
			patches.Push( WgRect(*pRect,_window) );
	}

	//

	WgRect	dirtBounds = patches.Union();

	// Render Window background color

	if( m_bgColor.a != 0 )
	{
		WgRect window = m_elements[WINDOW].m_windowGeo + _canvas.Pos();

		for( const WgRect * pRect = patches.Begin() ; pRect != patches.End() ; pRect++ )
			pDevice->ClipFill(*pRect, window, m_bgColor );
	}

	//

	if( m_elements[WINDOW].Widget() )
	{
		WgRect canvas = m_elements[WINDOW].m_canvasGeo + _canvas.Pos();
		WgRect window( canvas, m_elements[WINDOW].m_windowGeo + _canvas.Pos() );	// Use intersection in case canvas is smaller than window.

		if( window.IntersectsWith(dirtBounds) )
			m_elements[WINDOW].Widget()->_renderPatches( pDevice, canvas, window, &patches );
	}
	if( m_elements[XDRAG].m_bVisible )
	{
		WgRect canvas = m_elements[XDRAG].m_windowGeo + _canvas.Pos();
		if( canvas.IntersectsWith(dirtBounds) )
			m_elements[XDRAG].Widget()->_renderPatches( pDevice, canvas, canvas, &patches );
	}
	if( m_elements[YDRAG].m_bVisible )
	{
		WgRect canvas = m_elements[YDRAG].m_windowGeo + _canvas.Pos();
		if( canvas.IntersectsWith(dirtBounds) )
			m_elements[YDRAG].Widget()->_renderPatches( pDevice, canvas, canvas, &patches );
	}

	WgMode mode = m_bEnabled?WG_MODE_NORMAL:WG_MODE_DISABLED;
	if( m_pFillerBlocks && m_geoFiller.w != 0 && m_geoFiller.h != 0 )
	{
		WgRect canvas = m_geoFiller + _canvas.Pos();

		for( const WgRect * pRect = patches.Begin() ; pRect != patches.End() ; pRect++ )
		{
			WgRect clip( canvas, *pRect );
			if( clip.w > 0 || clip.h > 0 )
				pDevice->ClipBlitBlock( clip, m_pFillerBlocks->GetBlock( mode, m_scale ), canvas );
		}

	}
}


//____ _onCollectPatches() _______________________________________________________

void WgScrollPanel::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	container.Add( WgRect(geo,clip) );
}

//____ _onMaskPatches() __________________________________________________________

void WgScrollPanel::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	switch( m_maskOp )
	{
		case WG_MASKOP_RECURSE:
		{
			// Mask against view

			WgScrollHook * p = &m_elements[WINDOW];

			if( m_bgColor.a == 255 )
				patches.Sub( WgRect( p->m_windowGeo + geo.Pos(), clip) );
			else if( p->Widget() )
				p->Widget()->_onMaskPatches( patches, p->m_canvasGeo + geo.Pos(), WgRect(p->m_windowGeo + geo.Pos(), clip), blendMode );

			// Mask against dragbars

			p = &m_elements[XDRAG];
			if( p->IsVisible() )
				p->Widget()->_onMaskPatches( patches, p->m_windowGeo + geo.Pos(), clip, blendMode );

			p = &m_elements[YDRAG];
			if( p->IsVisible() )
				p->Widget()->_onMaskPatches( patches, p->m_windowGeo + geo.Pos(), clip, blendMode );

			// Maska against corner piece

			if( !m_geoFiller.IsEmpty() && m_pFillerBlocks && m_pFillerBlocks->IsOpaque() )
				patches.Sub( WgRect(m_geoFiller + geo.Pos(), clip) );

			break;
		}
		case WG_MASKOP_SKIP:
			break;
		case WG_MASKOP_MASK:
			patches.Sub( WgRect(geo,clip) );
			break;
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgScrollPanel::_onAlphaTest( const WgCoord& ofs )
{
	if( m_pFillerBlocks && m_geoFiller.Contains( ofs ) )
	{
		WgMode mode = m_bEnabled?WG_MODE_NORMAL:WG_MODE_DISABLED;

		if( WgUtil::MarkTestBlock( ofs, m_pFillerBlocks->GetBlock(mode, m_scale), m_geoFiller, m_markOpacity ) )
			return true;
	}

	return false;
}

//____ _onCloneContent() _______________________________________________________

void WgScrollPanel::_onCloneContent( const WgWidget * _pOrg )
{
	WgScrollPanel* pOrg = (WgScrollPanel*)_pOrg;

	WgPanel::_onCloneContent(pOrg);
	

	m_contentSize = pOrg->m_contentSize;

	m_stepSizeX = pOrg->m_stepSizeX;
	m_stepSizeY = pOrg->m_stepSizeY;
	m_jumpSizeX = pOrg->m_jumpSizeX;
	m_jumpSizeY = pOrg->m_jumpSizeY;

	m_viewPixOfs = pOrg->m_viewPixOfs;

	m_bAutoHideSliderX = pOrg->m_bAutoHideSliderX;
	m_bAutoHideSliderY = pOrg->m_bAutoHideSliderY;

	m_bSliderBottom = pOrg->m_bSliderBottom;
	m_bSliderRight = pOrg->m_bSliderRight;

	m_bAutoScrollX = pOrg->m_bAutoScrollX;
	m_bAutoScrollY = pOrg->m_bAutoScrollY;
	
	m_bgColor = pOrg->m_bgColor;
	m_widthPolicy = pOrg->m_widthPolicy;
	m_heightPolicy = pOrg->m_heightPolicy;
	m_contentOrigo = pOrg->m_contentOrigo;

	m_pFillerBlocks = pOrg->m_pFillerBlocks;
	m_geoFiller = pOrg->m_geoFiller;

	//

	m_elements[WINDOW].m_windowGeo = pOrg->m_elements[WINDOW].m_windowGeo;
	// Canvas geo for WINDOW should remain as it is since we don't have any content.
	
	//
	
	m_elements[XDRAG].m_windowGeo = pOrg->m_elements[XDRAG].m_windowGeo;
	m_elements[XDRAG].m_canvasGeo = pOrg->m_elements[XDRAG].m_canvasGeo;
	m_elements[XDRAG].m_bVisible = pOrg->m_elements[XDRAG].m_bVisible;
	if( pOrg->m_elements[XDRAG].m_pWidget )
	{
		WgHSlider * pSlider = (WgHSlider*) pOrg->m_elements[XDRAG].m_pWidget->NewOfMyType();
		pSlider->CloneContent( pOrg->m_elements[XDRAG].m_pWidget );
		pSlider->SetSliderTarget(&m_sliderTargets[1]);
		m_elements[XDRAG]._attachWidget(pSlider);		
	}

	//
	
	m_elements[YDRAG].m_windowGeo = pOrg->m_elements[YDRAG].m_windowGeo;
	m_elements[YDRAG].m_canvasGeo = pOrg->m_elements[YDRAG].m_canvasGeo;
	m_elements[YDRAG].m_bVisible = pOrg->m_elements[YDRAG].m_bVisible;
	if( pOrg->m_elements[YDRAG].m_pWidget )
	{
		WgVSlider * pSlider = (WgVSlider*) pOrg->m_elements[YDRAG].m_pWidget->NewOfMyType();
		pSlider->CloneContent( pOrg->m_elements[YDRAG].m_pWidget );
		pSlider->SetSliderTarget(&m_sliderTargets[0]);
		m_elements[YDRAG]._attachWidget(pSlider);
	}

	//
}

//______________________________________________________________________________
bool WgScrollPanel::SetScrollWheels( int wheelForX, int wheelForY )
{
	m_wheelForScrollX = wheelForX;
	m_wheelForScrollY = wheelForY;
	return true;
}

//_____________________________________________________________________________
bool WgScrollPanel::SetAutoscroll( bool bAutoX, bool bAutoY )
{
	m_bAutoScrollX = bAutoX;
	m_bAutoScrollY = bAutoY;
	return true;
}


//_____________________________________________________________________________
WgHook*	WgScrollPanel::_firstHook() const
{
	for( int i = 0 ; i < 3 ; i++ )
		if( m_elements[i].m_pWidget )
			return const_cast<WgScrollHook*>(&m_elements[i]);

	return 0;
}

//_____________________________________________________________________________
WgHook*	WgScrollPanel::_lastHook() const
{
	for( int i = 2 ; i >= 0 ; i++ )
		if( m_elements[i].m_pWidget )
			return const_cast<WgScrollHook*>(&m_elements[i]);

	return 0;
}


//_____________________________________________________________________________
WgHook * WgScrollPanel::_firstHookWithGeo( WgRect& geo ) const
{
	for( int i = 0 ; i < 3 ; i++ )
		if( m_elements[i].m_pWidget )
		{
			geo = m_elements[i].m_canvasGeo;
			return const_cast<WgScrollHook*>(&m_elements[i]);
		}

	return 0;
}

//_____________________________________________________________________________
WgHook * WgScrollPanel::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	const WgScrollHook * pLast = &m_elements[2];
	WgScrollHook * p = (WgScrollHook*) pHook;


	while( p != pLast )
	{
		p++;
		if( p->m_pWidget )
		{
			geo = p->m_canvasGeo;
			return const_cast<WgScrollHook*>(p);
		}
	}
	return 0;
}

//_____________________________________________________________________________
WgHook * WgScrollPanel::_lastHookWithGeo( WgRect& geo ) const
{
	for( int i = 2 ; i >= 0 ; i++ )
		if( m_elements[i].m_pWidget )
		{
			geo = m_elements[i].m_canvasGeo;
			return const_cast<WgScrollHook*>(&m_elements[i]);
		}

	return 0;
}

//_____________________________________________________________________________
WgHook * WgScrollPanel::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	const WgScrollHook * pFirst = &m_elements[0];
	WgScrollHook * p = (WgScrollHook*) pHook;


	while( p != pFirst )
	{
		p--;
		if( p->m_pWidget )
		{
			geo = p->m_canvasGeo;
			return const_cast<WgScrollHook*>(p);
		}
	}
	return 0;
}



//____ WgScrollHook::Destructor ___________________________________________________

WgScrollHook::~WgScrollHook()
{
}

//____ WgScrollHook::Type _______________________________________________________

const char * WgScrollHook::Type( void ) const
{
	return ClassType();
}

//____ WgScrollHook::ClassType __________________________________________________

const char * WgScrollHook::ClassType()
{
	return c_hookType;
}


//____ WgScrollHook::PixelPos() ________________________________________________________

WgCoord WgScrollHook::PixelPos() const
{
	return m_canvasGeo.Pos();
}

//____ WgScrollHook::PixelSize() _______________________________________________________

WgSize WgScrollHook::PixelSize() const
{
	return m_canvasGeo.Size();
}

//____ WgScrollHook::PixelGeo() ________________________________________________________

WgRect WgScrollHook::PixelGeo() const
{
	return m_canvasGeo;
}

//____ WgScrollHook::ScreenPixelPos() __________________________________________________

WgCoord WgScrollHook::ScreenPixelPos() const
{
	return m_pView->ScreenPixelPos() + m_canvasGeo.Pos();
}

//____ WgScrollHook::ScreenPixelGeo() __________________________________________________

WgRect WgScrollHook::ScreenPixelGeo() const
{
	return m_canvasGeo + m_pView->ScreenPixelPos();
}


//____ WgScrollHook::_requestRender() ______________________________________________

void WgScrollHook::_requestRender()
{
	if( m_bVisible )
		m_pView->_requestRender( m_windowGeo );
}

void WgScrollHook::_requestRender( const WgRect& rect )
{
	if( m_bVisible )
	{
		WgRect r( m_windowGeo, rect + m_canvasGeo.Pos() );

		if( !r.IsEmpty() )
			m_pView->_requestRender( r );
	}
}

//____ WgScrollHook::_requestResize() ______________________________________________

void WgScrollHook::_requestResize()
{
	m_pView->_updateElementGeo( m_pView->PixelSize() );
	m_pView->_requestRender( m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	m_pView->_requestResize();
}

//____ WgScrollHook::_prevHook() ___________________________________________________

WgHook * WgScrollHook::_prevHook() const
{
	const WgScrollHook * pFirst = &m_pView->m_elements[0];
	WgScrollHook * p 		= const_cast<WgScrollHook*>(this);

	while( p != pFirst )
	{
		p--;
		if( p->m_pWidget )
			return p;
	}
	return 0;
}

//____ WgScrollHook::_nextHook() ___________________________________________________

WgHook * WgScrollHook::_nextHook() const
{
	WgScrollHook * pLast = &m_pView->m_elements[2];
	WgScrollHook * p 		= const_cast<WgScrollHook*>(this);

	while( p != pLast )
	{
		p++;
		if( p->m_pWidget )
			return p;
	}
	return 0;
}

//____ WgScrollHook::_parent() ___________________________________________________

WgContainer * WgScrollHook::_parent() const
{
	return m_pView;
}

//____ SliderTarget methods ____________________________________________________

float WgScrollPanel::SliderTarget::_stepFwd()
{
	if( m_bHorizontal )
	{
		m_pParent->StepRight();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->StepDown();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_stepBwd()
{
	if( m_bHorizontal )
	{
		m_pParent->StepLeft();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->StepUp();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_jumpFwd()
{
	if( m_bHorizontal )
	{
		m_pParent->JumpRight();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->JumpDown();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_jumpBwd()
{
	if( m_bHorizontal )
	{
		m_pParent->JumpLeft();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->JumpUp();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_wheelRolled( int distance )
{
	if( m_bHorizontal )
	{
		m_pParent->_wheelRollX(distance);
		return m_pParent->ViewOfsX();
	}
	else
	{	
		m_pParent->_wheelRollY(distance);
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_setPosition( float fraction )
{
	if( m_bHorizontal )
	{
		m_pParent->SetViewOfsX(fraction);
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->SetViewOfsY(fraction);
		return m_pParent->ViewOfsY();
	}
}

WgWidget* WgScrollPanel::SliderTarget::_getWidget()
{
	return m_pParent;
}

float WgScrollPanel::SliderTarget::_getSliderPosition()
{
	if( m_bHorizontal )
		return m_pParent->ViewOfsX();
	else
		return m_pParent->ViewOfsY();
}

float WgScrollPanel::SliderTarget::_getSliderSize()
{
	if( m_bHorizontal )
		return m_pParent->ViewLenX();
	else
		return m_pParent->ViewLenY();
}



