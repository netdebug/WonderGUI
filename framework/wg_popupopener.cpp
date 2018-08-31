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



#include <wg_popupopener.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_key.h>
#include <wg_event.h>
#include <wg_eventhandler.h>
#include <wg_container.h>
#include <wg_popuplayer.h>

static const char	c_widgetType[] = {"PopupOpener"};

//____ Constructor ____________________________________________________________

WgPopupOpener::WgPopupOpener() : m_attachPoint(WG_SOUTHWEST), m_bOpenOnHover(false), m_bOpen(false)
{


	m_pText = &m_text;
	m_text.setAlignment( WG_CENTER );
	m_text.setLineWidth(PixelSize().w);					// We start with no textborders...
	m_text.SetAutoEllipsis(IsAutoEllipsisDefault());


	m_bReturnPressed = false;
}

//____ Destructor _____________________________________________________________

WgPopupOpener::~WgPopupOpener()
{
}

//____ Type() _________________________________________________________________

const char * WgPopupOpener::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgPopupOpener::GetClass()
{
	return c_widgetType;
}

//____ SetSkin() ______________________________________________________________

void WgPopupOpener::SetSkin(const WgSkinPtr& pSkin)
{
	if (pSkin != m_pSkin)
	{
		m_pSkin = pSkin;
		_requestResize();
		_requestRender();
	}
}


//____ SetIcon() ______________________________________________________________

void WgPopupOpener::SetIcon( const WgBlocksetPtr& pIconGfx )
{
	m_pIconGfx = pIconGfx;
	_iconModified();
}

bool WgPopupOpener::SetIcon( const WgBlocksetPtr& pIconGfx, WgOrigo origo, WgBorders borders, float scale, bool bPushText )
{
	if( scale < 0 || scale > 1.f )
		return false;

	m_pIconGfx = pIconGfx;
	m_iconOrigo = origo;
	m_iconBorders = borders;
	m_iconScale = scale;
	m_bIconPushText = bPushText;

	_iconModified();
	return true;
}


//____ SetPopup() _________________________________________________________

void WgPopupOpener::SetPopup(WgWidget * pPopup)
{
	m_pPopup = pPopup;
}

//____ SetOpenOnHover() ___________________________________________________

void WgPopupOpener::SetOpenOnHover(bool bOpen)
{
	m_bOpenOnHover = bOpen;
}

//____ SetAttachPoint() ___________________________________________________

void WgPopupOpener::SetAttachPoint(WgOrigo attachPoint)
{
	m_attachPoint = attachPoint;
}


//____ GetTextAreaWidth() _____________________________________________________

Uint32 WgPopupOpener::GetTextAreaWidth()
{
	WgRect	contentRect(0,0, PixelSize());

	if( m_pSkin )
		m_pSkin->ContentRect(contentRect, WG_STATE_NORMAL, m_scale);

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconGfx, m_scale ) );

	return textRect.w;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgPopupOpener::MatchingPixelHeight( int width ) const
{
	int height = 0;

	if( m_pSkin )
		height = m_pSkin->PreferredSize(m_scale).h;

	if( m_text.nbChars() != 0 )
	{
		WgSize padding;

		if (m_pSkin)
			padding = m_pSkin->ContentPadding(m_scale);

		int heightForText = m_text.heightForWidth(width-padding.w) + padding.h;
		if( heightForText > height )
			height = heightForText;
	}

	//TODO: Take icon into account.

	return height;
}


//____ PreferredPixelSize() _____________________________________________________________

WgSize WgPopupOpener::PreferredPixelSize() const
{
	WgSize bestSize;

	if (m_pSkin)
		bestSize = m_pSkin->PreferredSize(m_scale);

	if( m_text.nbChars() != 0 )
	{
		WgSize textSize = m_text.unwrappedSize();

		if( m_pSkin )
			textSize += m_pSkin->ContentPadding(m_scale);

		if( textSize.w > bestSize.w )
			bestSize.w = textSize.w;

		if( textSize.h > bestSize.h )
			bestSize.h = textSize.h;
	}

	//TODO: Take icon into account.

	return bestSize;
}


//____ _setState() ________________________________________________________

void WgPopupOpener::_setState(WgState state)
{
	if (m_bOpen)
	{
		m_closeState = state;
		state.setPressed(true);			// Force pressed state when popup is open.
	}
//	WgWidget::_setState(state);

	WgMode mode = WG_MODE_NORMAL;

	if (!m_state.isEnabled())
		mode = WG_MODE_DISABLED;
	else if (m_state.isPressed())
		mode = WG_MODE_SELECTED;
	else if (m_state.isHovered())
		mode = WG_MODE_MARKED;

	m_text.setMode(mode);
	_requestRender(); //TODO: Only requestRender if text appearance has changed (let m_text.setState() return if rendering is needed)
}

//____ _onEnable() _____________________________________________________________

void WgPopupOpener::_onEnable()
{
	m_state.setEnabled(true);
	_requestRender();
}

//____ _onDisable() ____________________________________________________________

void WgPopupOpener::_onDisable()
{
	m_state.setEnabled(false);
	_requestRender();
}

//____ _onNewSize() ____________________________________________________________

void WgPopupOpener::_onNewSize( const WgSize& size )
{
	WgRect	contentRect(0,0, PixelSize());

	if (m_pSkin)
		contentRect = m_pSkin->ContentRect(contentRect, WG_STATE_NORMAL, m_scale);

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconGfx, m_scale ) );

	m_text.setLineWidth(textRect.w);
}

//____ _setScale() _____________________________________________________________

void WgPopupOpener::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	m_text.SetScale(scale);
    
    _requestResize();
}


//____ _onRender() _____________________________________________________________

void WgPopupOpener::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	if (m_pSkin)
    {
        m_pSkin->Render(pDevice, m_state, _canvas, _clip, m_scale);
    }

    WgRect cli = _clip;
    WgRect can = _canvas;
    WgRect win = _window;

    WgBlock    block;

    WgRect contentRect = _canvas;

    if (m_pSkin)
    {
        contentRect = m_pSkin->ContentRect(_canvas, m_state, m_scale);
    }

	// Get icon and text rect from content rect

	WgRect iconRect = _getIconRect( contentRect, m_pIconGfx, m_scale );
	WgRect textRect = _getTextRect( contentRect, iconRect );

	// Convert WgState to WgMode

	WgMode mode = WG_MODE_NORMAL;

	if (!m_state.isEnabled())
		mode = WG_MODE_DISABLED;
	else if (m_state.isPressed())
		mode = WG_MODE_SELECTED;
	else if (m_state.isHovered())
		mode = WG_MODE_MARKED;

	// Render icon

	if( m_pIconGfx )
		pDevice->ClipBlitBlock( _clip, m_pIconGfx->GetBlock(mode, m_scale), iconRect );

	// Print text

 	if( !m_text.IsEmpty() )
	{
		m_text.setMode(mode);

		WgRect clip(textRect,_clip);
		pDevice->PrintText( clip, &m_text, textRect );
	}
}

//____ _onEvent() ______________________________________________________________

void WgPopupOpener::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	WgState oldState = m_state;

	switch( pEvent->Type() )
	{
/*
		case	WG_EVENT_KEY_PRESS:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
				m_bReturnPressed = true;
			else
				pHandler->ForwardEvent( pEvent );
			break;
		}

		case	WG_EVENT_KEY_REPEAT:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
			{
			}
			else
				pHandler->ForwardEvent( pEvent );
			break;
		}


		case	WG_EVENT_KEY_RELEASE:
		{
			if( static_cast< const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
			{
				m_bReturnPressed = false;
				pHandler->QueueEvent( new WgEvent::ButtonPress(this) );
			}
			else
				pHandler->ForwardEvent( pEvent );
			break;
		}
|*/

		case	WG_EVENT_MOUSE_ENTER:
		{
			if (m_bOpenOnHover && !m_bOpen)
				_open();
	
			m_state.setHovered(true);
			break;
		}
		
		case	WG_EVENT_MOUSE_LEAVE:
			m_state.setHovered(false);
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
			if (button == 1)
				m_state.setPressed(true);
			else
				pHandler->ForwardEvent(pEvent);

			if (!m_bOpenOnHover && button == 1)
			{
				if (m_bOpen)
				{
					_close();
				}
				else
				{
					_open();
//					Base::inputHandler()->_yieldButtonEvents(MouseButton::Left, this, _parent()->_getPopupLayer());
				}
			}
			break;
		}
		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			int button = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->Button();
			if( button == 1 )
				m_state.setPressed(false);
			else
				pHandler->ForwardEvent( pEvent );
			break;
		}

		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			int button = static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button();
			if( button != 1 )
				pHandler->ForwardEvent( pEvent );
			break;
		}

		case WG_EVENT_POPUP_CLOSED:
		{
			m_bOpen = false;

			break;
		}

        default:
			pHandler->ForwardEvent( pEvent );
            break;

	}

	if( oldState != m_state )
		_requestRender();
}


//____ _onRefresh() ____________________________________________________________

void WgPopupOpener::_onRefresh( void )
{
	if( m_pSkin )
	{
		if( m_pSkin->IsOpaque() )
			m_bOpaque = true;
		else
			m_bOpaque = false;

		_requestRender();
	}
}

//____ _onCloneContent() _______________________________________________________

void WgPopupOpener::_onCloneContent( const WgWidget * _pOrg )
{

	WgPopupOpener * pOrg = (WgPopupOpener *) _pOrg;

	pOrg->Wg_Interface_TextHolder::_cloneInterface( this );
	WgIconHolder::_onCloneContent( pOrg );

	m_text.setText(&pOrg->m_text);
	m_pText = &m_text;
	m_text.setHolder( this );

	m_pIconGfx		= pOrg->m_pIconGfx;
//	m_state			= pOrg->m_state;
}

//____ _onAlphaTest() ___________________________________________________________

bool WgPopupOpener::_onAlphaTest( const WgCoord& ofs )
{
	if (m_pSkin)
		return WgWidget::_onAlphaTest(ofs);


	return	false;
}

//____ _onGotInputFocus() ______________________________________________________

void WgPopupOpener::_onGotInputFocus()
{
	WgWidget::_onGotInputFocus();
	_requestRender();
}

//____ _onLostInputFocus() _____________________________________________________

void WgPopupOpener::_onLostInputFocus()
{
	WgWidget::_onLostInputFocus();
	m_bReturnPressed = false;
	_requestRender();
}


//____ _textModified() __________________________________________________________

void WgPopupOpener::_textModified()
{
	_requestRender();
}

//____ _iconModified() __________________________________________________________

void WgPopupOpener::_iconModified()
{
	//TODO: Should possibly refresh size too.
	_requestRender();
}

//____ _open() ____________________________________________________________

void WgPopupOpener::_open()
{
	auto pLayer = Parent()->_getPopupLayer();
	if (pLayer && m_pPopup)
	{
		pLayer->Push(m_pPopup.GetRealPtr(), this, ScreenPixelGeo(), m_attachPoint, m_bOpenOnHover);
		m_bOpen = true;
		m_closeState = m_state;
	}
}

//____ _close() ___________________________________________________________

void WgPopupOpener::_close()
{
	auto pLayer = Parent()->_getPopupLayer();
	if (pLayer && m_pPopup)
	{
		pLayer->Pop(m_pPopup.GetRealPtr());
	}
}