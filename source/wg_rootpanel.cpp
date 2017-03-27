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

#include <wg_rootpanel.h>
#include <new>

#ifndef WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif

#include <wg_eventhandler.h>
#include <wg_util.h>
#include <wg_geometrics.h>

static const char	c_hookType[] = {"RootHook"};


//____ Constructor ____________________________________________________________

WgRootPanel::WgRootPanel()
{
	m_bVisible = true;
	m_canvasScale = 1.f;
	m_pGfxDevice = 0;
	m_pEventHandler = new WgEventHandler(this);
	m_hook.m_pRoot = this;

}


WgRootPanel::WgRootPanel( WgGfxDevice * pGfxDevice )
{
	m_bVisible = true;
	m_canvasScale = 1.f;
	m_pGfxDevice = pGfxDevice;
	m_pEventHandler = new WgEventHandler(this);
	m_hook.m_pRoot = this;

	_updateGeoAndCanvas();
}

//____ Destructor _____________________________________________________________

WgRootPanel::~WgRootPanel()
{
	delete m_pEventHandler;
}

//____ SetGfxDevice() _________________________________________________________

void WgRootPanel::SetGfxDevice( WgGfxDevice * pDevice )
{
	m_pGfxDevice = pDevice;
	_updateGeoAndCanvas();
}

//_____ SetLayoutGeo() _____________________________________________________________

void WgRootPanel::SetLayoutGeo( const WgSize& geo )
{
	m_setLayoutGeo = geo;
	_updateGeoAndCanvas();
}

//____ SetCanvas() _____________________________________________________________

void WgRootPanel::SetCanvas( const WgRect& canvas )
{
	m_setCanvas = canvas;
	_updateGeoAndCanvas();
}

//____ _updateGeoAndCanvas() ___________________________________________________

void WgRootPanel::_updateGeoAndCanvas()
{
	WgRect oldCanvas = m_canvas;
	WgSize oldGeo = m_layoutGeo;

	WgRect canvas;

	if( m_setCanvas.IsEmpty() && m_pGfxDevice )
		canvas = m_pGfxDevice->CanvasSize();
	else
		canvas = m_setCanvas;

	if( m_setLayoutGeo.w > 0 && m_setLayoutGeo.h > 0 )
		m_layoutGeo = m_setLayoutGeo;
	else 
		m_layoutGeo = canvas;

	
	float scale = WgMin( canvas.w / (float) m_layoutGeo.w, canvas.h / (float) m_layoutGeo.h );

	m_canvas = WgRect( canvas.x, canvas.y, m_layoutGeo.w*scale, m_layoutGeo.h*scale );
	m_canvasScale = scale;

	if( m_hook.Widget() )
	{
		if( m_layoutGeo != oldGeo )
			m_hook.Widget()->_onNewSize( m_layoutGeo );
		else if( m_canvas != oldCanvas )
			AddDirtyPatch( m_canvas );
	}
}

//____ LayoutGeo() __________________________________________________________________

WgSize WgRootPanel::LayoutGeo() const
{
	return m_layoutGeo;
}

//____ Canvas() __________________________________________________________________

WgRect WgRootPanel::Canvas() const
{
	return m_canvas;
}


//____ SetChild() _____________________________________________________________

bool WgRootPanel::SetChild( WgWidget * pWidget )
{
	if( !pWidget )
		return false;

	m_hook._attachWidget(pWidget);
	m_hook.Widget()->_onNewSize(m_layoutGeo);

	WgGeometrics geometrics( m_layoutGeo, m_layoutGeo, m_canvasScale, m_canvas.Pos() );
	m_hook.Widget()->_onCollectPatches( m_dirtyPatches, geometrics, m_canvas );

	return true;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgRootPanel::ReleaseChild()
{
	return m_hook._releaseWidget();
}

WgWidget * WgRootPanel::ReleaseChild( WgWidget * pWidget )
{
	if( pWidget == m_hook.Widget() )
		return ReleaseChild();

	return 0;
}


//____ DeleteChild() __________________________________________________________

bool WgRootPanel::DeleteChild( WgWidget * pWidget )
{
	if( pWidget == m_hook.Widget() )
		return SetChild(0);

	return false;
}

//____ DeleteAllChildren() ______________________________________________________

bool WgRootPanel::DeleteAllChildren()
{
	DeleteChild();
	return true;
}

//____ ReleaseAllChildren() _____________________________________________________

bool WgRootPanel::ReleaseAllChildren()
{
	return ReleaseChild()==0?false:true;
}

//____ SetVisible() ___________________________________________________________

bool WgRootPanel::SetVisible( bool bVisible )
{
	if( bVisible != m_bVisible )
	{
		m_bVisible = bVisible;
		AddDirtyPatch( m_canvas );
	}
	return true;
}

//____ SetUpdatedRectOverlay() _________________________________________________

void WgRootPanel::SetUpdatedRectOverlay( const WgSkinPtr& pUpdatedRectOverlay, int afterglowFrames )
{
	m_pUpdatedRectOverlay = pUpdatedRectOverlay;
	m_afterglowFrames = afterglowFrames;
}



//____ Render() _______________________________________________________________

bool WgRootPanel::Render()
{
	return Render( m_canvas );
}

bool WgRootPanel::Render( const WgRect& clip )
{
	if( !BeginRender() )
		return false;

	if( !RenderSection(clip) )
	{
		EndRender();
		return false;
	}

	if( !EndRender() )
		return false;

	return true;
}

//____ BeginRender() __________________________________________________________

bool WgRootPanel::BeginRender()
{
	if( !m_pGfxDevice || !m_hook.Widget() )
		return false;						// No GFX-device or no widgets to render.


	if( m_pUpdatedRectOverlay )
	{
		// Remove from afterglow queue patches that are overlapped by our new dirty patches.

		for( std::deque<WgPatches>::iterator it = m_afterglowRects.begin() ; it != m_afterglowRects.end() ; ++it )
			it->Sub(&m_dirtyPatches);

		// Add our new dirty patches to the top of the afterglow queue.
		
		
		m_afterglowRects.push_front(WgPatches());
		m_afterglowRects.front().Add(&m_dirtyPatches);
		
		// Possibly remove overlays from the back, put them into dirty rects for re-render
		
		while( m_afterglowRects.size() > m_afterglowFrames+1 )
		{
			m_dirtyPatches.Add( &m_afterglowRects.back() );
			m_afterglowRects.pop_back();
		}
		
		// Re-render graphics behind overlays that go from state FOCUSED to NORMAL

		if( m_afterglowRects.size() > 1 )
		{
			m_dirtyPatches.Add( &m_afterglowRects[1] );
		}
	}
	
	// Initialize GfxDevice

	return m_pGfxDevice->BeginRender();
}


//____ RenderSection() __________________________________________________________

bool WgRootPanel::RenderSection( const WgRect& _clip )
{
	if( !m_pGfxDevice || !m_hook.Widget() )
		return false;						// No GFX-device or no widgets to render.

	// Make sure we have a vaild clip rectangle (doesn't go outside our geometry and has an area)
 
	WgRect canvas = m_canvas;
	WgRect clip( _clip, canvas );
	if( clip.w == 0 || clip.h == 0 )
		return false;						// Invalid rect area.

	// Nothing to render if our only child is hidden

	if( !m_bVisible )
		return true;						// Not an error, just hidden.

	// Copy and clip our dirty patches
	// TODO: Optimize when clip rectangle equals canvas

	WgPatches dirtyPatches( m_dirtyPatches.Size() );

	WgRect clipped;
	for( const WgRect * pRect = m_dirtyPatches.Begin() ; pRect != m_dirtyPatches.End() ; pRect++ )
	{
		if( clipped.Intersection( *pRect, clip ) )
			dirtyPatches.Push( clipped );
	}

	// Render the dirty patches recursively

	WgGeometrics geometrics( m_layoutGeo, m_layoutGeo, m_canvasScale, m_canvas.Pos() );

	m_hook.Widget()->_renderPatches( m_pGfxDevice, geometrics, &dirtyPatches );

	// Handle updated rect overlays
	
	if( m_pUpdatedRectOverlay )
	{
		// Render our new overlays
		
		for( const WgRect * pRect = m_afterglowRects[0].Begin() ; pRect != m_afterglowRects[0].End() ; pRect++ )
		{
			m_pUpdatedRectOverlay->Render( m_pGfxDevice, WG_STATE_FOCUSED, *pRect, clip );
		}		

		// Render overlays that have turned into afterglow

		if( m_afterglowRects.size() > 1 )
		{
			for( const WgRect * pRect = m_afterglowRects[1].Begin() ; pRect != m_afterglowRects[1].End() ; pRect++ )
			{
				m_pUpdatedRectOverlay->Render( m_pGfxDevice, WG_STATE_NORMAL, *pRect, clip );
			}		
		}

	}
	
	return true;
}

//____ EndRender() ____________________________________________________________

bool WgRootPanel::EndRender( void )
{
	if( !m_pGfxDevice || !m_hook.Widget() )
		return false;						// No GFX-device or no widgets to render.


	// Turn dirty patches into updated patches
	//TODO: Optimize by just making a swap.

	m_updatedPatches.Clear();
	m_updatedPatches.Add(&m_dirtyPatches);
	m_dirtyPatches.Clear();

	return m_pGfxDevice->EndRender();
}


//____ FindWidget() _____________________________________________________________

WgWidget * WgRootPanel::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	if( ofs.x >= m_layoutGeo.w || ofs.y >= m_layoutGeo.h || !m_hook.Widget() )
		return 0;

	if( m_hook.Widget() && m_hook.Widget()->IsContainer() )
		return m_hook.Widget()->CastToContainer()->FindWidget( ofs, mode );

	return m_hook.Widget();
}


//____ _focusRequested() _______________________________________________________

bool WgRootPanel::_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting )
{
	if( m_pEventHandler )
		return m_pEventHandler->SetKeyboardFocus(pWidgetRequesting);
	else
		return false;
}

//____ _focusReleased() ________________________________________________________

bool WgRootPanel::_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing )
{
	if( m_pEventHandler )
		return m_pEventHandler->SetKeyboardFocus(0);
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////

WgRootPanel::Hook::~Hook()
{
}

const char * WgRootPanel::Hook::Type( void ) const
{
	return ClassType();
}

const char * WgRootPanel::Hook::ClassType()
{
	return c_hookType;
}

WgCoord WgRootPanel::Hook::Pos() const
{
	return WgCoord();
}

WgSize WgRootPanel::Hook::Size() const
{
	return m_pRoot->LayoutGeo();
}

WgRect WgRootPanel::Hook::Geo() const
{
	return m_pRoot->LayoutGeo();
}

WgCoord WgRootPanel::Hook::ScreenPos() const
{
	return WgCoord();
}

WgRect WgRootPanel::Hook::ScreenGeo() const
{
	return m_pRoot->LayoutGeo();
}

WgRootPanel* WgRootPanel::Hook::Root() const
{
	return m_pRoot;
}

void WgRootPanel::Hook::_requestRender()
{
	if( m_pRoot->m_bVisible )
		m_pRoot->AddDirtyPatch( m_pRoot->Canvas() );
}

void WgRootPanel::Hook::_requestRender( const WgRect& rect )
{
	if( m_pRoot->m_bVisible )
		m_pRoot->AddDirtyPatch( WgUtil::layoutToCanvas(rect, m_pRoot->CanvasScale() ) + m_pRoot->Canvas().Pos() );
}

void WgRootPanel::Hook::_requestResize()
{
	// Do nothing, root ignores size requests.
}

WgHook * WgRootPanel::Hook::_prevHook() const
{
	return 0;
}

WgHook * WgRootPanel::Hook::_nextHook() const
{
	return 0;
}

WgWidgetHolder * WgRootPanel::Hook::_holder() const
{
	return m_pRoot;
}


WgContainer * WgRootPanel::Hook::_parent() const
{
	return 0;
}

