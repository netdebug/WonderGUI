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



#include <wg_gizmo_button.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_key.h>
#include <wg_event.h>

static const char	c_gizmoType[] = {"Button"};

//____ Constructor ____________________________________________________________

WgGizmoButton::WgGizmoButton()
{
	m_pText = &m_text;
	m_text.setAlignment( WgOrigo::midCenter() );
	m_text.setLineWidth(Size().w);					// We start with no textborders...

 	m_mode				= WG_MODE_NORMAL;

	m_bDownOutside		= false;
	m_aDisplace[0].x	= m_aDisplace[0].y = 0;
	m_aDisplace[1]		= m_aDisplace[2] = m_aDisplace[3] = m_aDisplace[0];

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
	{
		m_bRenderDown[i] = 0;
		m_bPressedInside[i] = 0;
	}
	m_bRenderDown[0] = true;			// Default is first mouse button...

	m_iconOrigo			= WgOrigo::midLeft();
	m_iconOfs.x			= 0;
	m_iconOfs.y			= 0;

	m_bReturnPressed = false;
	m_bPointerInside = false;
}

//____ Destructor _____________________________________________________________

WgGizmoButton::~WgGizmoButton()
{
}

//____ Type() _________________________________________________________________

const char * WgGizmoButton::Type( void ) const
{
	return GetMyType();
}

//____ GetMyType() ____________________________________________________________

const char * WgGizmoButton::GetMyType()
{
	return c_gizmoType;
}

//____ SetSource() ____________________________________________________________

bool WgGizmoButton::SetSource( const WgBlockSetPtr& pGfx )
{
	m_pBgGfx = pGfx;

	if( pGfx && pGfx->IsOpaque() )
		m_bOpaque = true;
	else
		m_bOpaque = false;

	RequestRender();
	return true;
}

//____ GetTextAreaWidth() _____________________________________________________

Uint32 WgGizmoButton::GetTextAreaWidth()
{
	int w = Size().w;

	if( m_pBgGfx )
		w -= m_pBgGfx->GetContentBorders().width();

	return w;
}

//____ SetIcon() ______________________________________________________________

void WgGizmoButton::SetIcon( const WgBlockSetPtr& pGfx, const WgOrigo& origo, Sint8 xOfs, Sint8 yOfs )
{
	m_pIconGfx = pGfx;

	m_iconOrigo = origo;
	m_iconOfs.x = xOfs;
	m_iconOfs.y = yOfs;

	RequestRender();
}

//____ SetIconAlignment() _____________________________________________________

void WgGizmoButton::SetIconAlignment( WgOrigo alignment )
{
	m_iconOrigo = alignment;
	RequestRender();
}

//____ SetIconOffset() ________________________________________________________

void WgGizmoButton::SetIconOffset( Sint8 xOfs, Sint8 yOfs )
{
	m_iconOfs.x = xOfs;
	m_iconOfs.y = yOfs;
	RequestRender();
}

//____ SetDisplacement() _______________________________________________________

bool WgGizmoButton::SetDisplacement( Sint8 _xUp, Sint8 _yUp, Sint8 _xOver, Sint8 _yOver, Sint8 _xDown, Sint8 _yDown )
{
  m_aDisplace[WG_MODE_NORMAL].x = _xUp;
  m_aDisplace[WG_MODE_NORMAL].y = _yUp;

  m_aDisplace[WG_MODE_MARKED].x = _xOver;
  m_aDisplace[WG_MODE_MARKED].y = _yOver;

  m_aDisplace[WG_MODE_SELECTED].x = _xDown;
  m_aDisplace[WG_MODE_SELECTED].y = _yDown;

  return  true;
}

void WgGizmoButton::GetDisplacement( Sint8& xUp, Sint8& yUp, Sint8& xOver, Sint8& yOver, Sint8& xDown, Sint8& yDown ) const
{
	xUp = m_aDisplace[WG_MODE_NORMAL].x;
	yUp = m_aDisplace[WG_MODE_NORMAL].y;

	xOver = m_aDisplace[WG_MODE_MARKED].x;
	yOver = m_aDisplace[WG_MODE_MARKED].y;

	xDown = m_aDisplace[WG_MODE_SELECTED].x;
	yDown = m_aDisplace[WG_MODE_SELECTED].y;
}

//____ BestSize() _____________________________________________________________

WgSize WgGizmoButton::BestSize() const
{
	WgSize best( m_text.width(), m_text.height() );

	if( m_pBgGfx )
	{
		WgSize gfxSize = m_pBgGfx->GetSize();
		WgBorders textBorders = m_pBgGfx->GetContentBorders();
		WgSize	textArea = gfxSize - textBorders;

		if( best.w > textArea.w )
			best.w += textBorders.width();

		if( best.h > textArea.h )
			best.h += textBorders.height();

		if( best.w < gfxSize.w )
			best.w = gfxSize.w;

		if( best.h < gfxSize.h )
			best.h = gfxSize.h;
	}

	//TODO: Take icon with origo, offset and size into account.

	return best;
}


//____ _onEnable() _____________________________________________________________

void WgGizmoButton::_onEnable()
{
	m_mode = WG_MODE_NORMAL;
	RequestRender();
}

//____ _onDisable() ____________________________________________________________

void WgGizmoButton::_onDisable()
{
	m_mode = WG_MODE_DISABLED;
	RequestRender();
}

//____ _onNewSize() ____________________________________________________________

void WgGizmoButton::_onNewSize( const WgSize& size )
{
	Uint32 w = size.w;

	if( m_pBgGfx )
		w -= m_pBgGfx->GetContentBorders().width();

	m_text.setLineWidth(w);
}


//____ _onRender() _____________________________________________________________

void WgGizmoButton::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip, Uint8 _layer )
{
	WgRect cli = _clip;
	WgRect can = _canvas;
	WgRect win = _window;

	// Render background

	if( m_pBgGfx )
		pDevice->ClipBlitBlock( _clip, m_pBgGfx->GetBlock(m_mode, _canvas), _canvas );

	// Get displacement offset

    int   xOfs = m_aDisplace[m_mode].x;
    int   yOfs = m_aDisplace[m_mode].y;

	// Render icon (with displacement).

	if( m_pIconGfx )
	{
		int w = m_pIconGfx->GetWidth();
		int h = m_pIconGfx->GetHeight();

		int dx = (int)( m_iconOrigo.anchorX() * _canvas.w - m_iconOrigo.hotspotX() * w + m_iconOfs.x + xOfs );
		int dy = (int)( m_iconOrigo.anchorY() * _canvas.h - m_iconOrigo.hotspotY() * h + m_iconOfs.y + yOfs );

		WgRect dest( _canvas.x + dx, _canvas.y + dy, w, h );
		pDevice->ClipBlitBlock( _clip, m_pIconGfx->GetBlock(m_mode, dest.size()), dest );
	}

	// Print text (with displacement).

 	if( m_text.nbLines()!= 0 )
	{
		m_text.setMode(m_mode);

		if( m_pBgGfx )
			m_text.SetBgBlockColors( m_pBgGfx->GetTextColors() );

		WgRect printWindow( _canvas.x + xOfs, _canvas.y + yOfs, _canvas.w, _canvas.h );

		if( m_pBgGfx )
			printWindow.shrink( m_pBgGfx->GetContentBorders() );

		WgRect c = _clip;
		WgRect printW = printWindow;

		pDevice->PrintText( _clip, &m_text, printWindow );
	}
}

//____ _onEvent() ______________________________________________________________

void WgGizmoButton::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case	WG_EVENT_KEY_PRESS:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
				m_bReturnPressed = true;
			break;
		}

		case	WG_EVENT_KEY_RELEASE:
		{
			if( static_cast< const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
				m_bReturnPressed = false;
			break;

			//TODO: Send signal!
		}

		case	WG_EVENT_POINTER_ENTER:
			m_bPointerInside = true;
			break;

		case	WG_EVENT_POINTER_EXIT:
			m_bPointerInside = false;
			break;

		case WG_EVENT_BUTTON_PRESS:
		{
			int button = static_cast<const WgEvent::ButtonPress*>(pEvent)->Button();

			m_bPressedInside[button-1] = true;
			break;
		}
		case WG_EVENT_BUTTON_RELEASE:
		{
			const WgEvent::ButtonRelease* pEv = static_cast<const WgEvent::ButtonRelease*>(pEvent);
			int button = pEv->Button();

			if( pEv->ReleaseInside() )
			{
				//TODO: Send signal if right button was released!
			}

			m_bPressedInside[button-1] = false;
			break;
		}
        default:
            break;

	}

	WgMode newMode = GetRenderMode();
	if( newMode != m_mode )
	{
		m_mode = newMode;
		RequestRender();
	}

}


//____ _onAction() _____________________________________________________________

void WgGizmoButton::_onAction( WgInput::UserAction action, int button, const WgActionDetails& info, const WgInput& inputObj )
{
//	if( !m_bEnabled )
//		return;

	switch( action )
	{
		case	WgInput::KEY_PRESS:
		{
			if( button == WG_KEY_RETURN )
				m_bReturnPressed = true;
			break;
		}

		case	WgInput::KEY_RELEASE:
		{
			if( button == WG_KEY_RETURN )
				m_bReturnPressed = false;
			break;

			//TODO: Send signal!
		}

		case	WgInput::POINTER_ENTER:
			m_bPointerInside = true;
			break;

		case	WgInput::POINTER_EXIT:
			m_bPointerInside = false;
			break;

		case WgInput::BUTTON_PRESS:
			m_bPressedInside[button-1] = true;
			break;

		case WgInput::BUTTON_RELEASE:
			//TODO: Send signal if right button was released!
		case WgInput::BUTTON_RELEASE_OUTSIDE:
			m_bPressedInside[button-1] = false;
			break;

        default:
            break;

	}

	WgMode newMode = GetRenderMode();
	if( newMode != m_mode )
	{
		m_mode = newMode;
		RequestRender();
	}
}

//_____ GetRenderMode() ________________________________________________________

WgMode WgGizmoButton::GetRenderMode()
{
	if( !IsEnabled() )
		return WG_MODE_DISABLED;

	if( m_bReturnPressed )
		return WG_MODE_SELECTED;

	if( m_bPointerInside || m_bDownOutside )
	{
		for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
		{
			if( m_bRenderDown[i] && m_bPressedInside[i] )
				return WG_MODE_SELECTED;
		}
	}

	if( m_bPointerInside )
		return WG_MODE_MARKED;

	return WG_MODE_NORMAL;
}


//____ _onRefresh() ____________________________________________________________

void WgGizmoButton::_onRefresh( void )
{
	if( m_pBgGfx )
	{
		if( m_pBgGfx->IsOpaque() )
			m_bOpaque = true;
		else
			m_bOpaque = false;

		RequestRender();
	}
}


//____ SetPressAnim() __________________________________________________________

void	WgGizmoButton::SetPressAnim( bool _button1, bool _button2, bool _button3, bool _bDownOutside )
{
		m_bRenderDown[0]	= _button1;
		m_bRenderDown[1]	= _button2;
		m_bRenderDown[2]	= _button3;
		m_bDownOutside		= _bDownOutside;
}

//____ GetPressAnim() _________________________________________________________

void WgGizmoButton::GetPressAnim( bool& button1, bool& button2, bool& button3, bool& bDownWhenMouseOutside )
{
	button1 = m_bRenderDown[0];
	button2 = m_bRenderDown[1];
	button3 = m_bRenderDown[2];
	bDownWhenMouseOutside = m_bDownOutside;
}


//____ _onCloneContent() _______________________________________________________

void WgGizmoButton::_onCloneContent( const WgGizmo * _pOrg )
{

	WgGizmoButton * pOrg = (WgGizmoButton *) _pOrg;

	pOrg->Wg_Interface_TextHolder::CloneInterface( this );

	m_text.setText(&pOrg->m_text);
	m_pText = &m_text;
	m_text.setHolder( this );

	m_pBgGfx		= pOrg->m_pBgGfx;
	m_pIconGfx		= pOrg->m_pIconGfx;
	m_mode			= pOrg->m_mode;

	m_iconOrigo		= pOrg->m_iconOrigo;
	m_iconOfs		= pOrg->m_iconOfs;

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
		m_aDisplace[i]	= pOrg->m_aDisplace[i];

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
	 	m_bRenderDown[i] = pOrg->m_bRenderDown[i];
}

//____ _onAlphaTest() ___________________________________________________________

bool WgGizmoButton::_onAlphaTest( const WgCord& ofs )
{
	if( !m_pBgGfx )
		return false;

	WgSize	sz = Size();

	return	WgUtil::MarkTestBlock( ofs, m_pBgGfx->GetBlock(m_mode,sz), WgRect(0,0,sz) );
}

//____ _onGotInputFocus() ______________________________________________________

void WgGizmoButton::_onGotInputFocus()
{
	m_bFocused = true;
	RequestRender();
}

//____ _onLostInputFocus() _____________________________________________________

void WgGizmoButton::_onLostInputFocus()
{
	m_bFocused = false;
	m_bReturnPressed = false;
	RequestRender();
}


//____ TextModified() __________________________________________________________

void WgGizmoButton::TextModified()
{
	RequestRender();
}
