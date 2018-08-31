/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_popuplayer.h>
#include <wg_util.h>
#include <wg_patches.h>
#include <wg_panel.h>
#include <wg_base.h>
#include <wg_gfxdevice.h>
#include <wg_eventhandler.h>

#include <algorithm>


static const char	c_widgetType[] = { "PopupLayer" };
static const char	c_hookType[] = { "PopupHook" };
static const char	c_basehookType[] = { "PopupLayerBasehook" };


//_____________________________________________________________________________
WgPopupLayer* WgPopupHook::Parent() const
{
	return m_pParent;
}


//_____________________________________________________________________________
const char * WgPopupHook::Type(void) const
{
	return ClassType();
}

//_____________________________________________________________________________
const char * WgPopupHook::ClassType()
{
	return c_hookType;
}


//_____________________________________________________________________________
void WgPopupHook::_requestRender()
{
	m_pParent->_onRequestRender(m_geo, this);
}

//_____________________________________________________________________________
void WgPopupHook::_requestRender(const WgRect& rect)
{
	m_pParent->_onRequestRender(rect + m_geo.Pos(), this);
}

//_____________________________________________________________________________
void WgPopupHook::_requestResize()
{
	m_pParent->_childRequestResize(this);
}

//_____________________________________________________________________________
WgHook * WgPopupHook::_prevHook() const
{
	WgPopupHook * p = _prev();

	if (p)
		return p;
	else if (m_pParent->m_baseHook.Widget())
		return &m_pParent->m_baseHook;
	else
		return 0;
}

//_____________________________________________________________________________
WgHook * WgPopupHook::_nextHook() const
{
	WgPopupHook * p = _next();

	// We have multiple inheritance, so lets make the cast in a safe way, preserving NULL-pointer as NULL.

	if (p)
		return p;
	else
		return 0;
}

//_____________________________________________________________________________
WgContainer * WgPopupHook::_parent() const
{
	return m_pParent;
}
	

//____ Constructor ____________________________________________________________
	
WgPopupLayer::WgPopupLayer()
{
}
	
//____ Destructor _____________________________________________________________
	
WgPopupLayer::~WgPopupLayer()
{
	// In contrast to all other panels we only delete our base child on exit.
	// Menus don't belong to us, we just display them, so they are not ours to delete.

	WgPopupHook * pHook = m_popupHooks.First();
	while (pHook)
	{
		pHook->_releaseWidget();
		pHook = pHook->_next();
	}
}
	
//____ Type() _________________________________________________________________

const char *WgPopupLayer::Type(void) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgPopupLayer::GetClass()
{
	return c_widgetType;
}

//____ NbPopups() _____________________________________________________________

int WgPopupLayer::NbPopups() const
{
	return m_popupHooks.Size();
}

//____ Push() _________________________________________________________________

void WgPopupLayer::Push(WgWidget * pPopup, WgWidget * pOpener, const WgRect& launcherGeo, WgOrigo attachPoint, bool bAutoClose, WgSize maxSize)
{
	_addSlot(pPopup, pOpener, launcherGeo, attachPoint, bAutoClose, maxSize);
}

//____ Pop() __________________________________________________________________

void WgPopupLayer::Pop(int nb)
{
	if (nb <= 0)
		return;

	nb = std::min(nb, m_popupHooks.Size());

	_removeSlots(nb);
}

//____ Pop() __________________________________________________________________

void WgPopupLayer::Pop(WgWidget * pPopup)
{
	int index = 0;
	auto p = m_popupHooks.First();

	while (p && p->m_pWidget != pPopup)
	{
		p = p->Next();
		index++;
	}


	if (p)
		_removeSlots(index + 1);

}

//____ Clear() ________________________________________________________________

void WgPopupLayer::Clear()
{
	if (m_popupHooks.IsEmpty())
		return;

	_removeSlots(m_popupHooks.Size());
}


//____ _updateGeo() __________________________________________________________
	
bool WgPopupLayer::_updateGeo(WgPopupHook* pSlot, bool bInitialUpdate )
{
	// Get size of parent and correct launcherGeo
	
	
	//
	
	WgRect geo(0,0,WgSize::Min(pSlot->m_pWidget->PreferredPixelSize(),WgSize::Min(pSlot->maxSize,m_size)));
	
	switch( pSlot->attachPoint )
	{
		case WgOrigo::WG_NORTHEAST:					// Right side of launcherGeo, going down.
		{
			geo.x = pSlot->launcherGeo.Right();
			geo.y = pSlot->launcherGeo.Top();
			break;
		}
	
		case WgOrigo::WG_SOUTHEAST:					// Right side of launcherGeo, going up.
		{
			geo.x = pSlot->launcherGeo.Right();
			geo.y = pSlot->launcherGeo.Bottom() - geo.h;
			break;
		}
	
		case WgOrigo::WG_NORTHWEST:					// Left-aligned above launcher.
		{
			geo.x = pSlot->launcherGeo.Left();
			geo.y = pSlot->launcherGeo.Top() - geo.h;
			break;
		}
	
		case WgOrigo::WG_SOUTHWEST:					// Left-aligned below launcher.
		{
			geo.x = pSlot->launcherGeo.Left();
			geo.y = pSlot->launcherGeo.Bottom();
			break;
		}
	
		case WgOrigo::WG_WEST:						// Centered left of launcherGeo.
		{
			geo.x = pSlot->launcherGeo.Left() - geo.w;
			geo.y = pSlot->launcherGeo.Top() + pSlot->launcherGeo.h/2 - geo.h/2;
			break;
		}
	
		case WgOrigo::WG_NORTH:						// Centered above launcherGeo.
		{
			geo.x = pSlot->launcherGeo.Left() + pSlot->launcherGeo.w/2 + geo.w/2;
			geo.y = pSlot->launcherGeo.Top() - geo.h;
			break;
		}
	
		case WgOrigo::WG_EAST:						// Centered right of launcherGeo.
		{
			geo.x = pSlot->launcherGeo.Right();
			geo.y = pSlot->launcherGeo.Top() + pSlot->launcherGeo.h/2 - geo.h/2;
			break;
		}
	
		case WgOrigo::WG_SOUTH:						// Centered below launcherGeo.
		{
			geo.x = pSlot->launcherGeo.Left() + pSlot->launcherGeo.w/2 + geo.w/2;
			geo.y = pSlot->launcherGeo.Bottom();
			break;
		}
	
	}
	
	// Adjust geometry to fit inside parent.
	
	if( geo.Right() > m_size.w )
	{
		if( geo.Left() == pSlot->launcherGeo.Right() )
		{
			if( pSlot->launcherGeo.Left() > m_size.w - pSlot->launcherGeo.Right() )
			{
				geo.x = pSlot->launcherGeo.Left() - geo.w;
				if( geo.x < 0 )
				{
					geo.x = 0;
					geo.w = pSlot->launcherGeo.Left();
				}
			}
			else
				geo.w = m_size.w - geo.x;
		}
		else
			geo.x = m_size.w - geo.w;
	}
	
	if( geo.Left() < 0 )
	{
		if( geo.Right() == pSlot->launcherGeo.Left() )
		{
			if( pSlot->launcherGeo.Left() < m_size.w - pSlot->launcherGeo.Right() )
			{
				geo.x = pSlot->launcherGeo.Right();
				if( geo.Right() > m_size.w )
					geo.w = m_size.w - geo.x;
			}
			else
			{
				geo.x = 0;
				geo.w = pSlot->launcherGeo.Left();
			}
	
		}
		else
			geo.x = 0;
	}
	
	if( geo.Bottom() > m_size.h )
	{
		if( geo.Top() == pSlot->launcherGeo.Bottom() )
		{
			if( pSlot->launcherGeo.Top() > m_size.h - pSlot->launcherGeo.Bottom() )
			{
				geo.y = pSlot->launcherGeo.Top() - geo.h;
				if( geo.y < 0 )
				{
					geo.y = 0;
					geo.h = pSlot->launcherGeo.Top();
				}
			}
			else
				geo.h = m_size.h - geo.y;
		}
		else
			geo.y = m_size.h - geo.h;
	}
	
	if( geo.Top() < 0 )
	{
		if( geo.Bottom() == pSlot->launcherGeo.Top() )
		{
			if( pSlot->launcherGeo.Top() < m_size.h - pSlot->launcherGeo.Bottom() )
			{
				geo.y = pSlot->launcherGeo.Bottom();
				if( geo.Bottom() > m_size.h )
					geo.h = m_size.h - geo.y;
			}
			else
			{
				geo.y = 0;
				geo.h = pSlot->launcherGeo.Bottom();
			}
		}
		else
			geo.y = 0;
	}
	
	// Update geometry if it has changed.
	
	if( geo != pSlot->m_geo )
	{
		if( !bInitialUpdate )
			_onRequestRender(pSlot->m_geo,pSlot);	
		pSlot->m_geo = geo;
		_onRequestRender(pSlot->m_geo,pSlot);	

		if( pSlot->m_pWidget->PixelSize() != geo.Size() )
			pSlot->m_pWidget->_onNewSize(geo.Size());

		return true;
	}
	else
		return false;
}


//____ FindWidget() ____________________________________________________________
	
WgWidget *  WgPopupLayer::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	// MenuPanel has its own _findWidget() method since we need special treatment of
	// searchmode ACTION_TARGET when a menu is open.
	
	if( mode == WgSearchMode::WG_SEARCH_ACTION_TARGET && !m_popupHooks.IsEmpty() )
	{
		// In search mode ACTION_TARGET we limit our target to us, our menu-branches and the menu-opener if a menu is open.
	
		WgPopupHook * pSlot = m_popupHooks.First();
		WgWidget * pResult = 0;
	
		while( pSlot && !pResult )
		{
			if( pSlot->m_geo.Contains( ofs ) )
			{
				if( pSlot->m_pWidget->IsContainer() )
					pResult = static_cast<WgContainer*>(pSlot->m_pWidget)->FindWidget( ofs - pSlot->m_geo.Pos(), mode );
				else if( pSlot->m_pWidget->MarkTest( ofs - pSlot->m_geo.Pos() ) )
					pResult = pSlot->m_pWidget;
			}
			pSlot = pSlot->Next();
		}
	
		if( pResult == 0 )
		{
			// Check the root opener
				
			WgPopupHook * pSlot = m_popupHooks.Last();
			if( pSlot->pOpener )
			{
				WgWidget * pOpener = pSlot->pOpener.GetRealPtr();
	
				WgCoord 	absPos 		= ofs + ScreenPixelPos();
				WgRect	openerGeo 	= pOpener->ScreenPixelGeo();
	
				if( openerGeo.Contains(absPos) && pOpener->MarkTest( absPos - openerGeo.Pos() ) )
					pResult = pOpener;
			}
				
			// Fall back to us.
				
			if( pResult == 0 )
				pResult = this;
		}
		return pResult;
	}
	else
	{
		// For the rest of the modes we can rely on the default method.
	
		return WgContainer::FindWidget( ofs, mode );
	}
}
	
//____ _onRequestRender() _____________________________________________________
	
void WgPopupLayer::_onRequestRender( const WgRect& rect, const WgPopupHook * pSlot )
{
	// Don not render anything if not visible

	if (pSlot && pSlot->state == WgPopupHook::State::OpeningDelay)
		return;

	// Clip our geometry and put it in a dirtyrect-list
	
	WgPatches patches;
	patches.Add( WgRect( rect, WgRect(0,0,m_size)) );
	
	// Remove portions of dirty rect that are covered by opaque upper siblings,
	// possibly filling list with many small dirty rects instead.

	if( !m_popupHooks.IsEmpty() )
	{
		WgPopupHook * pCover;

		if (pSlot)
			pCover = pSlot->Prev();
		else
			pCover = m_popupHooks.Last();

		while (pCover)
		{
			if (pCover->m_geo.IntersectsWith(rect) && pCover->state != WgPopupHook::State::OpeningDelay && pCover->state != WgPopupHook::State::Opening && pCover->state != WgPopupHook::State::Closing)
				pCover->m_pWidget->_onMaskPatches(patches, pCover->m_geo, WgRect(0, 0, INT_MAX, INT_MAX), _getBlendMode());

			pCover = pCover->Prev();
		}
	}
	// Make request render calls
	
	for( const WgRect * pRect = patches.Begin() ; pRect < patches.End() ; pRect++ )
		_requestRender( * pRect );
}

//____ _renderPatches() ___________________________________________________

class WidgetRenderContext
{
public:
	WidgetRenderContext() : pSlot(0) {}
	WidgetRenderContext(WgPopupHook * pSlot, const WgRect& geo) : pSlot(pSlot), geo(geo) {}

	WgPopupHook *	pSlot;
	WgRect			geo;
	WgPatches		patches;
};

void WgPopupLayer::_renderPatches(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{

	// We start by eliminating dirt outside our geometry

	WgPatches 	patches(_pPatches->Size());								// TODO: Optimize by pre-allocating?

	for (const WgRect * pRect = _pPatches->Begin(); pRect != _pPatches->End(); pRect++)
	{
		if (_canvas.IntersectsWith(*pRect))
			patches.Push(WgRect(*pRect, _canvas));
	}


	// Render container itself

	for (const WgRect * pRect = patches.Begin(); pRect != patches.End(); pRect++)
		_onRender(pDevice, _canvas, _window, *pRect);


	// Render children

	WgRect	dirtBounds = patches.Union();

	// Create WidgetRenderContext's for popups that might get dirty patches

	std::vector<WidgetRenderContext> renderList;

	auto pSlot = m_popupHooks.First();

	while (pSlot != nullptr )
	{
		WgRect geo = pSlot->m_geo + _canvas.Pos();

		if (geo.IntersectsWith(dirtBounds) && pSlot->state != WgPopupHook::State::OpeningDelay)
			renderList.push_back(WidgetRenderContext(pSlot, geo));

		pSlot = pSlot->Next();
	}

	// Go through WidgetRenderContexts, push and mask dirt

	for (unsigned int i = 0; i < renderList.size(); i++)
	{
		WidgetRenderContext * p = &renderList[i];

		p->patches.Push(&patches);
		if( p->pSlot->state != WgPopupHook::State::Opening && p->pSlot->state != WgPopupHook::State::Closing )
			p->pSlot->m_pWidget->_onMaskPatches(patches, p->geo, p->geo, pDevice->GetBlendMode());		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

		if (patches.IsEmpty())
			break;
	}

	// Any dirt left in patches is for base child, lets render that first


	if (!patches.IsEmpty())
		m_baseHook.Widget()->_renderPatches(pDevice, _canvas, _window, &patches);


	// Go through WidgetRenderContexts and render the patches in reverse order (topmost popup rendered last).

	for (int i = renderList.size() - 1; i >= 0; i--)
	{
		WidgetRenderContext * p = &renderList[i];

		WgColor tint = WgColor::white;

		if (p->pSlot->state == WgPopupHook::State::Opening)
			tint.a = 255 * p->pSlot->stateCounter / m_openingFadeMs;

		if (p->pSlot->state == WgPopupHook::State::Closing)
			tint.a = 255 - (255 * p->pSlot->stateCounter / m_closingFadeMs);

		if (tint.a == 255)
			p->pSlot->m_pWidget->_renderPatches(pDevice, p->geo, p->geo, &p->patches);
		else
		{
			WgColor oldTint = pDevice->GetTintColor();
			pDevice->SetTintColor(oldTint*tint);
			p->pSlot->m_pWidget->_renderPatches(pDevice, p->geo, p->geo, &p->patches);
			pDevice->SetTintColor(oldTint);
		}
	}
}


//____ _maskPatches() _____________________________________________________

//	void WgPopupLayer::_maskPatches(Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode)
//	{
//		Need to except children that are in states OpeningDelay, Opening and Closing.
//	}

//____ _collectPatches() ___________________________________________________

//	void WgPopupLayer::_collectPatches(Patches& container, const Rect& geo, const Rect& clip)
//	{
//		Need to make sure patches are not collected for children in mode "OpeningDelay"
//		This might be handled by slotWithGeo() methods, depending on how what we choose.
//	}



//____ _onNewSize() ___________________________________________________________
	
void WgPopupLayer::_onNewSize( const WgSize& sz )
{
	m_size = sz;

	WgLayer::_onNewSize(sz);
}
	
//____ _onCloneContent() ______________________________________________________
	
void WgPopupLayer::_onCloneContent( const WgWidget * _pOrg )
{
	WgLayer::_onCloneContent( (WgContainer*) _pOrg );
}
	
//____ _onEvent() ______________________________________________________________
	
void WgPopupLayer::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	WgLayer::_onEvent(pEvent, pHandler);
		

	switch( pEvent->Type() )
	{
		case WG_EVENT_TICK:
		{
			// Update state for all open popups

			int ms = static_cast<const WgEvent::Tick*>(pEvent)->Millisec();

			WgPopupHook * pHook = m_popupHooks.First();
			while (pHook)
			{
				switch (pHook->state)
				{
				case WgPopupHook::State::OpeningDelay:
					if (pHook->stateCounter + ms < m_openingDelayMs)
					{
						pHook->stateCounter += ms;
						break;
					}
					else
					{
						pHook->state = WgPopupHook::State::Opening;
						pHook->stateCounter -= m_openingDelayMs;
						// No break here, let's continue down to opening...
					}
				case WgPopupHook::State::Opening:
					pHook->stateCounter += ms;
					_requestRender(pHook->m_geo);
					if (pHook->stateCounter >= m_openingFadeMs)
					{
						pHook->stateCounter = 0;
						pHook->state = pHook->bAutoClose ? WgPopupHook::State::PeekOpen : WgPopupHook::State::FixedOpen;
					}
					break;

				case WgPopupHook::State::ClosingDelay:
					if (pHook->stateCounter + ms < m_closingDelayMs)
					{
						pHook->stateCounter += ms;
						break;
					}
					else
					{
						pHook->state = WgPopupHook::State::Closing;
						pHook->stateCounter -= m_closingDelayMs;
						// No break here, let's continue down to closing...
					}
				case WgPopupHook::State::Closing:
					pHook->stateCounter += ms;
					_requestRender(pHook->m_geo);
					// Removing any closed popups is done in next loop
					break;
				default:
					break;
				}

				pHook = pHook->_next();
			}

			// Close any popup that is due for closing.

			while (!m_popupHooks.IsEmpty() && m_popupHooks.First()->state == WgPopupHook::State::Closing && m_popupHooks.First()->stateCounter >= m_closingFadeMs)
				_removeSlots(1);
		}
		break;

		case WG_EVENT_MOUSE_ENTER:
		case WG_EVENT_MOUSE_MOVE:
		{
			if (m_popupHooks.IsEmpty())
				break;

			WgCoord 	pointerPos = pEvent->PointerPixelPos() - ScreenPixelPos();

			// Top popup can be in state PeekOpen, which needs special attention.

			WgPopupHook * pHook = m_popupHooks.First();
			if (pHook && pHook->state == WgPopupHook::State::PeekOpen)
			{
				// Promote popup to state WeakOpen if pointer has entered its geo,
				// otherwise begin delayed closing if pointer has left launcherGeo.

				if (pHook->m_geo.Contains(pointerPos))
					pHook->state = WgPopupHook::State::WeakOpen;
				else if (!pHook->launcherGeo.Contains(pointerPos))
				{
					pHook->state = WgPopupHook::State::ClosingDelay;
					pHook->stateCounter = 0;
				}
			}
			
			// A popup in state ClosingDelay should be promoted to
			// state PeekOpen if pointer has entered its launcherGeo and
			// to state WeakOpen if pointer has entered its geo.
			// Promoting to WeakOpen Should also promote any ancestor also in state ClosingDelay.


			pHook = m_popupHooks.First();
			while (pHook)
			{
				if (pHook->state == WgPopupHook::State::ClosingDelay)
				{
					if (pHook->launcherGeo.Contains(pointerPos))
					{
						pHook->state = WgPopupHook::State::PeekOpen;
						pHook->stateCounter = 0;
					}
					else if (pHook->m_geo.Contains(pointerPos))
					{
						WgPopupHook * p = pHook;
						while (p && p->state == WgPopupHook::State::ClosingDelay)
						{
							p->state = WgPopupHook::State::WeakOpen;
							p->stateCounter = 0;
							p = p->_next();
						}
						break;		// Nothing more to do further down.
					}
				}
				pHook = pHook->_next();
			}

			// If pointer has entered a selectable widget of a popup that isn't the top one
			// and all widgets between them have bAutoClose=true, they should all enter
			// state ClosingDelay (unless already in state Closing).


			WgWidget * pTop = m_popupHooks.First()->m_pWidget;
			WgWidget * pMarked = FindWidget(pointerPos, WgSearchMode::WG_SEARCH_ACTION_TARGET);

			if (pMarked != this && pMarked->IsSelectable() && m_popupHooks.First()->bAutoClose)
			{
				// Trace hierarchy from marked to one of our children.

				while (pMarked->Parent() != this)
					pMarked = pMarked->Parent();

				//

				auto p = m_popupHooks.First();
				while (p->bAutoClose && p->m_pWidget != pMarked)
				{
					if (p->state != WgPopupHook::State::Closing && p->state != WgPopupHook::State::ClosingDelay)
					{
						p->state = WgPopupHook::State::ClosingDelay;
						p->stateCounter = 0;
					}
					p--;
				}
			}


		}				
		break;

/*
		case MsgType::MouseLeave:
		{
			// Top popup can be in state PeekOpen, which should begin closing when
			// pointer has left.

			PopupSlot * pSlot = m_popups.first();
			if (pSlot && pSlot->state == PopupSlot::State::PeekOpen)
			{
				pSlot->state = PopupSlot::State::ClosingDelay;
				pSlot->stateCounter = 0;
			}
		}
		break;
*/

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			if (m_popupHooks.IsEmpty())
				break;					// Popup was removed already on the press.

			// Allow us to release the mouse within opener without closing any popups

			WgPopupHook * pSlot = m_popupHooks.First();
			if (pSlot->pOpener)
			{
				WgWidget * pOpener = pSlot->pOpener.GetRealPtr();

				WgCoord absPos = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->PointerPixelPos();
				WgRect	openerGeo = pOpener->ScreenPixelGeo();

				if (pOpener->MarkTest(absPos - openerGeo.Pos()))
					break;
			}
						
			// DON'T BREAK! Continuing down to case MousePress on purpose.
		}
		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			auto pEv = static_cast<const WgEvent::MouseButtonPress*>(pEvent);

			auto pSource = static_cast<WgWidget*>(pEvent->ForwardedFrom() );
			if (!pSource || pSource == this )
				_removeSlots(m_popupHooks.Size());
			else if (pSource->IsSelectable())
			{
				pHandler->QueueEvent(new WgEvent::Selected(pSource));

				_removeSlots(m_popupHooks.Size());
			}
			return;
		}
		break;
	
		case WG_EVENT_KEY_PRESS:
		case WG_EVENT_KEY_REPEAT:
		{
			auto pEv = static_cast<const WgEvent::KeyEvent*>(pEvent);
	
			if( pEv->TranslatedKeyCode() == WgKey::WG_KEY_ESCAPE )
			{
				if( !m_popupHooks.IsEmpty() )
				{
					_removeSlots(1);
					return;
				}
			}
		}
		break;
            
        default:
            break;
	}

	// Final solution: forward to our parent.

	pHandler->ForwardEvent(pEvent);

}
	
//____ _stealKeyboardFocus() _________________________________________________
	
void WgPopupLayer::_stealKeyboardFocus()
{

	// Verify that we have a root
	
	if( !Hook() || !Hook()->Root() )
		return;
	
	// Save old keyboard focus, which we assume belonged to previous menu in hierarchy.
	
	if( m_popupHooks.Size() < 2 )
		m_pKeyFocus = _eventHandler()->KeyboardFocus();
	else
		m_popupHooks.Get(1)->pKeyFocus = _eventHandler()->KeyboardFocus();
	
	// Steal keyboard focus to top menu
	
	WgWidget * pWidget = m_popupHooks.First()->m_pWidget;

	pWidget->GrabFocus();

}
	
//____ _restoreKeyboardFocus() _________________________________________________
	
void WgPopupLayer::_restoreKeyboardFocus()
{

	// Verify that we have a root
	
	if (!Hook() || !Hook()->Root())
		return;
	
	//
	
	if (m_popupHooks.IsEmpty())
	{
		if( m_pKeyFocus )
		m_pKeyFocus->GrabFocus();
	}
	else
		 m_popupHooks.First()->m_pWidget->GrabFocus();

}

//____ _childRequestResize() _______________________________________________

void WgPopupLayer::_childRequestResize(WgPopupHook * pHook)
{
	if( ((WgHook*)pHook) == &m_baseHook )
		_requestResize();
	else
		_updateGeo( pHook );
}

//____ _addSlot() ____________________________________________________________

void WgPopupLayer::_addSlot(WgWidget * _pPopup, WgWidget * _pOpener, const WgRect& _launcherGeo, WgOrigo _attachPoint, bool _bAutoClose, WgSize _maxSize)
{
	WgPopupHook * pHook = new WgPopupHook(this);
	pHook->_attachWidget(_pPopup);
	m_popupHooks.PushFront(pHook);

	pHook->pOpener = _pOpener;
	pHook->launcherGeo = _launcherGeo;
	pHook->attachPoint = _attachPoint;
	pHook->bAutoClose = _bAutoClose;
	pHook->state = WgPopupHook::State::OpeningDelay;
	pHook->stateCounter = 0;
	pHook->maxSize = _maxSize;

	_updateGeo(pHook, true);
	_stealKeyboardFocus();

	if( m_popupHooks.Size() == 1 )
		_startReceiveTicks();
}


//____ _removeSlots() __________________________________________________

void WgPopupLayer::_removeSlots(int nb)
{
	WgEventHandler * pEH = _eventHandler();

	nb = std::min(nb, m_popupHooks.Size());

	WgPopupHook * pHook = (WgPopupHook *) m_popupHooks.First();


	for (int i = 0; i < nb; i++)
	{
		WgPopupHook * p = pHook;
		pHook = pHook->Next();

		if (pEH)
			pEH->QueueEvent(new WgEvent::PopupClosed(p->Widget(), p->pOpener));

		p->_requestRender();
		p->_releaseWidget();
		delete p;
	}
	_restoreKeyboardFocus();

	if (m_popupHooks.IsEmpty() )
		_stopReceiveTicks();
}



//____ _firstLayerHook() ______________________________________________________

WgLayerHook * WgPopupLayer::_firstLayerHook() const
{
	return m_popupHooks.First();
}

//____ _firstHook() ___________________________________________________________

WgHook* WgPopupLayer::_firstHook() const
{
	if (m_baseHook.Widget())
		return const_cast<BaseHook*>(&m_baseHook);
	else
		return m_popupHooks.First();
}

//____ _lastHook() ____________________________________________________________

WgHook* WgPopupLayer::_lastHook() const
{
	return m_popupHooks.Last();
}

//____ _firstHookWithGeo() _____________________________________________________

WgHook * WgPopupLayer::_firstHookWithGeo(WgRect& geo) const
{
	if (m_baseHook.Widget())
	{
		geo = WgRect(0, 0, m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
	{
		WgPopupHook * p = m_popupHooks.First();
		if (p)
			geo = p->m_geo;

		return p;
	}
}

//____ _nextHookWithGeo() _______________________________________________________

WgHook * WgPopupLayer::_nextHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	WgHook * p = pHook->Next();
	if (p)
		geo = ((WgPopupHook*)p)->m_geo;

	return p;
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook * WgPopupLayer::_lastHookWithGeo(WgRect& geo) const
{
	WgPopupHook * p = m_popupHooks.Last();
	if (p)
	{
		geo = p->m_geo;
		return p;
	}
	else if (m_baseHook.Widget())
	{
		geo = WgRect(0, 0, m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
		return 0;
}

//____ _prevHookWithGeo() _______________________________________________________

WgHook * WgPopupLayer::_prevHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	WgHook * p = pHook->Prev();
	if (p)
		geo = p->PixelGeo();

	return p;
}
