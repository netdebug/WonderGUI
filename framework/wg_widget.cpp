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

#include <wg_widget.h>
#include <wg_types.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>

#	include <wg_rootpanel.h>
#	include <wg_eventhandler.h>


//____ Constructor ____________________________________________________________

WgWidget::WgWidget():m_id(0), m_pHook(0), m_pointerStyle(WG_POINTER_DEFAULT),
					m_markOpacity( 1 ), m_bEnabled(true), m_bOpaque(false),
					m_bFocused(false), m_bSelected(false), m_bTabLock(false), m_bReceiveTick(false), m_scale(WG_SCALE_BASE)
{
#ifdef WG_DEBUG_DIRTY_RECTS
    update = false;
#endif
}

//____ Destructor _____________________________________________________________

WgWidget::~WgWidget()
{
	if( m_pHook )
	{
		m_pHook->Parent()->ReleaseChild(this);
	}
}


//____ GetPointerStyle() ________________________________________

WgPointerStyle WgWidget::GetPointerStyle() const
{
	return m_pointerStyle;
}

//____ SetEnabled() _______________________________________________________________

void WgWidget::SetEnabled( bool bEnabled )
{
	if( m_bEnabled != bEnabled || IsContainer() )
	{
		m_bEnabled = bEnabled;
        m_state.setEnabled(bEnabled);
		if( bEnabled )
			_onEnable();
		else
			_onDisable();
	}
}

//____ MarkTest() _____________________________________________________________

bool WgWidget::MarkTest( const WgCoord& ofs )
{
	switch( m_markOpacity )
	{
	case 0:
		return true;
	case 256:
		return false;
	default:
		return _onAlphaTest(ofs);
	}
}

//____ CloneContent() _________________________________________________________

bool WgWidget::CloneContent( const WgWidget * _pOrg )
{
	if( _pOrg->Type() != Type() )
		return false;

	m_id			= _pOrg->m_id;

	m_pointerStyle 	= _pOrg->m_pointerStyle;

	m_tooltip		= _pOrg->m_tooltip;
	m_markOpacity	= _pOrg->m_markOpacity;

	m_bEnabled		= _pOrg->m_bEnabled;
	m_bOpaque		= _pOrg->m_bOpaque;
	m_bTabLock		= _pOrg->m_bTabLock;

	_onCloneContent( _pOrg );
	return true;
}

//____ _onNewHook() ___________________________________________________________

void WgWidget::_onNewHook( WgHook * pHook )
{
	if( pHook )
	{
		WgRootPanel * pNewRoot = pHook->Root(); 
		WgRootPanel * pOldRoot = m_pHook ? m_pHook->Root() : 0;


		if( pNewRoot && pNewRoot != pOldRoot )
			_onNewRoot( pNewRoot );
	}

    m_pHook = pHook;

    if( pHook )
    {
        int scale;
        WgContainer * p =  pHook->_parent();
        if( p )
            scale = p->m_scale;
        else
            scale = pHook->Root()->Scale();
        
        if( scale != m_scale )
            _setScale( scale );
    }

}

//____ _onNewRoot() ___________________________________________________________

void WgWidget::_onNewRoot( WgRootPanel * pRoot )
{
	if( m_bReceiveTick && pRoot )
		pRoot->EventHandler()->_addTickReceiver(this);
}

//____ _startReceiveTicks() ___________________________________________________

void WgWidget::_startReceiveTicks()
{
	if( !m_bReceiveTick )
	{
		m_bReceiveTick = true;

		if( m_pHook )
		{
			WgRootPanel * pRoot = m_pHook->Root();
			if( pRoot )
				pRoot->EventHandler()->_addTickReceiver(this);
		}
	}
}

//____ _stopReceiveTicks() ____________________________________________________

void WgWidget::_stopReceiveTicks()
{
	m_bReceiveTick = false;
}

//____ Local2absPixel() ____________________________________________________________

WgCoord WgWidget::Local2absPixel( const WgCoord& cord ) const
{
	WgCoord c = ScreenPixelPos();
	c.x += cord.x;
	c.y += cord.y;
	return c;
}

//____ Abs2localPixel() ____________________________________________________________

WgCoord WgWidget::Abs2localPixel( const WgCoord& cord ) const
{
	WgCoord c = ScreenPixelPos();
	return WgCoord( cord.x - c.x, cord.y - c.y );
}

//____ Local2absPoint() ____________________________________________________________

WgCoord WgWidget::Local2absPoint( const WgCoord& cord ) const
{
    return (Local2absPixel(cord*m_scale/WG_SCALE_BASE)*WG_SCALE_BASE) / m_scale;
}

//____ Abs2localPoint() ____________________________________________________________

WgCoord WgWidget::Abs2localPoint( const WgCoord& cord ) const
{
    return (Abs2localPixel(cord*m_scale/WG_SCALE_BASE)*WG_SCALE_BASE) / m_scale;
}


//____ _eventHandler() __________________________________________________________

WgEventHandler * WgWidget::_eventHandler() const
{
	if( m_pHook )
	{
		WgRootPanel * pRoot = m_pHook->Root();
		if( pRoot )
			return pRoot->EventHandler();
	}
	return 0;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgWidget::MatchingPixelHeight( int width ) const
{
	return PreferredPixelSize().h;		// Default is to stick with best height no matter what width.
}

//____ MatchingPixelWidth() _______________________________________________________

int WgWidget::MatchingPixelWidth( int height ) const
{
	return PreferredPixelSize().w;		// Default is to stick with best width no matter what height.
}

//____ MinPixelSize() ______________________________________________________________

WgSize WgWidget::MinPixelSize() const
{
	return WgSize(0,0);
}

//____ MaxPixelSize() ______________________________________________________________

WgSize WgWidget::MaxPixelSize() const
{
	return WgSize(2<<24,2<<24);
}

//____ SetMarked() ____________________________________________________________

bool WgWidget::SetMarked()
{
	return false;
}

//____ SetSelected() __________________________________________________________

bool WgWidget::SetSelected()
{
	if (m_bEnabled)
	{
		m_bSelected = true;

		if (!m_state.isSelected())
		{
			m_state.setSelected(true);
			_requestRender();
		}
		return true;
	}
	else
		return false;
}

//____ SetNormal() ____________________________________________________________

bool WgWidget::SetNormal()
{
	if (m_bEnabled)
	{
		m_bSelected = false;
		if (m_state.isSelected())
		{
			m_state.setSelected(false);
			_requestRender();
		}
		return true;
	}
	else
		return false;
}

//____ SetSelectable() ________________________________________________________

void WgWidget::SetSelectable(bool bSelectable)
{
	m_bSelectable = bSelectable;
}


//____ Mode() _________________________________________________________________

WgMode WgWidget::Mode() const
{
    return WgUtil::StateToMode(m_state);

//	if (m_bSelected)
//		return WG_MODE_SELECTED;
//	else if( m_bEnabled )
//		return WG_MODE_NORMAL;
//	else
//		return WG_MODE_DISABLED;
}

//____ PreferredPointSize() ___________________________________________________

WgSize WgWidget::PreferredPointSize() const 
{ 
	WgSize sz = PreferredPixelSize()*WG_SCALE_BASE; 

	sz.w += m_scale - 1;		// Preferred size should be rounded up.
	sz.h += m_scale - 1;

	return sz / m_scale; 
};

//____ MinPointSize() ___________________________________________________

WgSize WgWidget::MinPointSize() const
{ 
	WgSize sz = MinPixelSize()*WG_SCALE_BASE;

	sz.w += m_scale - 1;		// Min size should be rounded up.
	sz.h += m_scale - 1;

	return sz / m_scale; 
};


//____ MaxPointSize() ___________________________________________________

WgSize WgWidget::MaxPointSize() const 
{ 
	WgSize sz = MaxPixelSize(); 
	return sz*WG_SCALE_BASE / m_scale; 
};



//____ _getBlendMode() _________________________________________________________

WgBlendMode WgWidget::_getBlendMode() const
{
	WgContainer * pParent = Parent();
	if( pParent )
		return pParent->_getBlendMode();
	else
		return WG_BLENDMODE_BLEND;		// We always start out with WG_BLENDMODE_BLEND.
}

//____ _queueEvent() __________________________________________________________

void WgWidget::_queueEvent( WgEvent::Event * pEvent )
{
	if( m_pHook )
	{
		WgRootPanel * pRoot = m_pHook->Root();
		if( pRoot )
		{
			pRoot->EventHandler()->QueueEvent(pEvent);
			return;
		}
	}

	delete pEvent;		// Can't queue event, silently delete it.
}


//____ _renderPatches() ________________________________________________________

void WgWidget::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	for( const WgRect * pRect = _pPatches->Begin() ; pRect != _pPatches->End() ; pRect++ )
	{
		WgRect clip( _window, *pRect );
		if( clip.w > 0 && clip.h > 0 ) {
			_onRender( pDevice, _canvas, _window, clip );
#ifdef WG_DEBUG_DIRTY_RECTS
            if(update)
                pDevice->Fill(_canvas, WgColor(255,255,0,16));
            else
                pDevice->Fill(_canvas, WgColor(255,255,0,0));
            
            update = !update;
#endif
        }
	}
}

//____ Fillers _______________________________________________________________

void WgWidget::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
		container.Add( WgRect( geo, clip ) );
}

void WgWidget::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	if( (m_bOpaque && blendMode == WG_BLENDMODE_BLEND) || blendMode == WG_BLENDMODE_OPAQUE )
	{
		patches.Sub( WgRect( geo, clip ) );
	}
}

void WgWidget::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	if (m_pSkin)
	{
//		WgState	state;
//		state.setFocused(m_bFocused);
//		state.setSelected(m_bSelected);
//		state.setEnabled(m_bEnabled);

		m_pSkin->Render(pDevice, m_state, _canvas, _clip, m_scale);
	}
}

void WgWidget::_onNewSize( const WgSize& size )
{
	_requestRender();
}

void WgWidget::_setScale( int scale )
{
	m_scale = scale;
}

void WgWidget::_onRefresh()
{
	_requestRender();
}

void WgWidget::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	pHandler->ForwardEvent( pEvent );
}

bool WgWidget::_onAlphaTest( const WgCoord& ofs )
{
	if (m_pSkin)
		return m_pSkin->MarkTest(ofs, PixelSize(), WG_STATE_NORMAL, m_markOpacity, m_scale);
	else
		return false;
}

void WgWidget::_onEnable()
{
	_requestRender();
}

void WgWidget::_onDisable()
{
	_requestRender();
}

void WgWidget::_onGotInputFocus()
{
	m_bFocused = true;
    m_state.setFocused(true);
	_queueEvent(new WgEvent::FocusGained(this));
}

void WgWidget::_onLostInputFocus()
{
	m_bFocused = false;
    m_state.setFocused(false);
	_queueEvent(new WgEvent::FocusLost(this));
}

bool WgWidget::TempIsInputField() const
{
	return false;
}

Wg_Interface_TextHolder* WgWidget::TempGetText()
{
	return 0;
}
