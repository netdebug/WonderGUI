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
#include <wg_container.h>
#include <wg_popuplayer.h>
#include <wg_msg.h>
#include <wg_inputhandler.h>

namespace wg
{

	const char PopupOpener::CLASSNAME[] = { "PopupOpener" };

	//____ Constructor ____________________________________________________________

	PopupOpener::PopupOpener() : m_text(this), m_icon(this), label(&m_text), icon(&m_icon), m_attachPoint(Origo::SouthWest), m_bOpenOnHover(false), m_bOpen(false)
	{
		m_bSelectable = false;
	}

	//____ Destructor _____________________________________________________________

	PopupOpener::~PopupOpener()
	{
	}

	//____ isInstanceOf() _________________________________________________________

	bool PopupOpener::isInstanceOf(const char * pClassName) const
	{
		if (pClassName == CLASSNAME)
			return true;

		return Widget::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * PopupOpener::className(void) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	PopupOpener_p PopupOpener::cast(Object * pObject)
	{
		if (pObject && pObject->isInstanceOf(CLASSNAME))
			return PopupOpener_p(static_cast<PopupOpener*>(pObject));

		return 0;
	}

	//____ setPopup() _________________________________________________________

	void PopupOpener::setPopup(Widget * pPopup)
	{
		m_pPopup = pPopup;
	}

	//____ setOpenOnHover() ___________________________________________________

	void PopupOpener::setOpenOnHover(bool bOpen)
	{
		m_bOpenOnHover = bOpen;
	}

	//____ setAttachPoint() ___________________________________________________

	void PopupOpener::setAttachPoint(Origo attachPoint)
	{
		m_attachPoint = attachPoint;
	}

	//____ _cloneContent() ____________________________________________________

	void PopupOpener::_cloneContent(const Widget * _pOrg)
	{

	}

	//____ _render() __________________________________________________________

	void PopupOpener::_render(GfxDevice * pDevice, const Rect& _canvas, const Rect& _window)
	{
		Widget::_render(pDevice, _canvas, _window);

		Rect	contentRect = _canvas;

		if (m_pSkin)
			contentRect = m_pSkin->contentRect(_canvas, m_state);

		// Get icon and text rect from content rect

		Rect iconRect = m_icon.getIconRect(contentRect);
		Rect textRect = m_icon.getTextRect(contentRect, iconRect);

		// Render icon

		if (m_icon.skin())
			m_icon.skin()->render(pDevice, iconRect, m_state);

		// Print text

		if (!m_text.isEmpty())
			m_text.render(pDevice, textRect);
	}


	//____ _setSize() ____________________________________________________________

	void PopupOpener::_setSize(const Size& _size)
	{
		Widget::_setSize(_size);

		Rect	contentRect(0, 0, _size);

		if (m_pSkin)
			contentRect -= m_pSkin->contentPadding();

		Rect textRect = m_icon.getTextRect(contentRect, m_icon.getIconRect(contentRect));

		m_text.setSize(textRect);
	}

	//____ _refresh() _________________________________________________________

	void PopupOpener::_refresh()
	{
		Widget::_refresh();
		m_text.refresh();

		//TODO: Handling of icon and text.
	}

	//____ _receive() _________________________________________________________

	void PopupOpener::_receive(Msg * pMsg)
	{
		Widget::_receive(pMsg);

		switch (pMsg->type())
		{
			case MsgType::PopupClosed:
			{
				m_bOpen = false;
				_setState(m_closeState);
				break;
			}

			case MsgType::MouseEnter:
			{
				if (m_bOpenOnHover && !m_bOpen)
					_open();
				break;
			}

			case MsgType::MousePress:
			{
				if (!m_bOpenOnHover && MouseButtonMsg::cast(pMsg)->button() == MouseButton::Left)
				{
					if (m_bOpen)
					{
						_close();
					}
					else
					{
						_open();
						Base::inputHandler()->_yieldButtonEvents(MouseButton::Left, this, _parent()->_getPopupLayer());
						m_bPressed = false;		// We have yielded our press...
					}
				}
				break;
			}
/*
			case MsgType::MouseRelease:
			{
				auto pButtonMsg = MouseReleaseMsg::cast(pMsg);
				if (!m_bOpenOnHover && pButtonMsg->button() == MouseButton::Left && !pButtonMsg->releaseInside() )
					_close();
				break;
			}
*/

			default:
				break;
		}

		if (pMsg->isMouseButtonMsg() && MouseButtonMsg::cast(pMsg)->button() == MouseButton::Left)
			pMsg->swallow();

	}

	//____ matchingHeight() _______________________________________________________

	int PopupOpener::matchingHeight(int width) const
	{
		int height = 0;

		if (m_pSkin)
			height = m_pSkin->preferredSize().h;

		if (!m_text.isEmpty())
		{
			Size padding;

			if (m_pSkin)
				padding = m_pSkin->contentPadding();

			int heightForText = m_text.matchingHeight(width - padding.w) + padding.h;
			if (heightForText > height)
				height = heightForText;
		}

		//TODO: Take icon into account.

		return height;
	}


	//____ preferredSize() _____________________________________________________________

	Size PopupOpener::preferredSize() const
	{
		Size preferred;

		if (!m_text.isEmpty())
			preferred = m_text.preferredSize();

		if (m_pSkin)
			preferred = m_pSkin->sizeForContent(preferred);

		//TODO: Take icon into account.

		return preferred;
	}

	//____ _setState() ________________________________________________________

	void PopupOpener::_setState(State state)
	{
		if (m_bOpen)
		{
			m_closeState = state;
			state.setPressed(true);			// Force pressed state when popup is open.
		}
		Widget::_setState(state);
		m_text.setState(state);
		_requestRender(); //TODO: Only requestRender if text appearance has changed (let m_text.setState() return if rendering is needed)
	}

	//____ _setSkin() _________________________________________________________

	void PopupOpener::_setSkin(Skin * pSkin)
	{
		Widget::_setSkin(pSkin);
	}

	//____ _open() ____________________________________________________________

	void PopupOpener::_open()
	{
		auto pLayer = _parent()->_getPopupLayer();
		if (pLayer && m_pPopup)
		{
			pLayer->popups.push(m_pPopup, this, globalGeo(), m_attachPoint, m_bOpenOnHover );
			m_bOpen = true;
			m_closeState = m_state;
		}
	}

	//____ _close() ___________________________________________________________

	void PopupOpener::_close()
	{
		auto pLayer = _parent()->_getPopupLayer();
		if (pLayer && m_pPopup)
		{
			pLayer->popups.pop(m_pPopup.rawPtr());
		}
	}

	//____ _componentPos() ______________________________________________________________

	Coord PopupOpener::_componentPos(const Component * pComponent) const
	{
		Rect	contentRect = m_size;

		if (m_pSkin)
			contentRect = m_pSkin->contentRect(contentRect, m_state);

		// Get icon and text rect from content rect

		Rect iconRect = m_icon.getIconRect(contentRect);

		if (pComponent == &m_icon)
			return iconRect.pos();

		Rect textRect = m_icon.getTextRect(contentRect, iconRect);
		return textRect.pos();
	}

	//____ _componentSize() ______________________________________________________________

	Size PopupOpener::_componentSize(const Component * pComponent) const
	{
		Size	sz = m_size;

		if (m_pSkin)
			sz -= m_pSkin->contentPadding();

		Rect iconRect = m_icon.getIconRect(sz);

		if (pComponent == &m_icon)
			return iconRect.size();

		Rect textRect = m_icon.getTextRect(sz, iconRect);
		return textRect.size();

	}

	//____ _componentGeo() ______________________________________________________________

	Rect PopupOpener::_componentGeo(const Component * pComponent) const
	{
		Rect	contentRect = m_size;

		if (m_pSkin)
			contentRect = m_pSkin->contentRect(contentRect, m_state);

		// Get icon and text rect from content rect

		Rect iconRect = m_icon.getIconRect(contentRect);

		if (pComponent == &m_icon)
			return iconRect;

		Rect textRect = m_icon.getTextRect(contentRect, iconRect);
		return textRect;
	}



}	// namespace wg
