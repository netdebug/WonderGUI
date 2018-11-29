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


#include <assert.h>
#include <algorithm>
#include <wg_multislider.h>
#include <wg_gfxdevice.h>
#include <wg_eventhandler.h>

static const char	c_widgetType[] = {"MultiSlider"};

//____ Constructor ____________________________________________________________

WgMultiSlider::WgMultiSlider()
{
}

//____ Destructor _____________________________________________________________

WgMultiSlider::~WgMultiSlider()
{
}

//____ Type() _________________________________________________________________

const char * WgMultiSlider::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgMultiSlider::GetClass()
{
	return c_widgetType;
}

//____ PreferredPixelSize() ___________________________________________________

WgSize WgMultiSlider::PreferredPixelSize() const
{
	return WgSize(128, 128);
}

//____ SetDefaults() __________________________________________________________

void WgMultiSlider::SetDefaults(const WgSkinPtr& pSliderBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders handleMarkExtension, WgBorders sliderMarkExtension)
{
	m_pDefaultBgSkin = pSliderBgSkin;
	m_pDefaultHandleSkin = pHandleSkin;
	m_defaultHandleHotspot = handleHotspot;
	m_defaultHandleMarkExtension = handleMarkExtension;
	m_defaultSliderMarkExtension = sliderMarkExtension;
}

//____ SetCallback() __________________________________________________________

void WgMultiSlider::SetCallback(const std::function<void(int sliderId, float value, float value2 )>& callback)
{
	m_callback = callback;
}

//____ SetPassive() ___________________________________________________________

void WgMultiSlider::SetPassive(bool bPassive)
{
	m_bPassive = bPassive;
}

//____ SetDeltaDrag() ___________________________________________________________

void WgMultiSlider::SetDeltaDrag(bool bDeltaDrag)
{
	m_bDeltaDrag = bDeltaDrag;
}

//____ SetGhostHandle() _______________________________________________________

void WgMultiSlider::SetGhostHandle(bool bGhost)
{
	m_bGhostHandle = bGhost;
}

//____ AddSlider() ____________________________________________________________

int WgMultiSlider::AddSlider(	int id, WgDirection dir, SetGeoFunc pSetGeoFunc, float startValue, float minValue, float maxValue, int steps,
								SetValueFunc pSetValueFunc, const WgSkinPtr& pBgSkin,
								const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders handleMarkExtension, WgBorders sliderMarkExtension)
{
	WgOrigo origo;

	switch (dir)
	{
	case WG_UP:
		origo = WG_SOUTH;
		break;
	case WG_DOWN:
		origo = WG_NORTH;
		break;
	case WG_LEFT:
		origo = WG_EAST;
		break;
	case WG_RIGHT:
		origo = WG_WEST;
		break;
	}

	m_sliders.emplace_back();
	Slider& s = m_sliders.back();

	s.id = id;
	s.is2D = false;


	s.value[0] = startValue;
	s.bounds[0].min = minValue;
	s.bounds[0].max = maxValue;
	s.bounds[0].steps = steps;

	s.origo = origo;

	s.handleMarkExtension = handleMarkExtension;
	s.sliderMarkExtension = sliderMarkExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;
	s.pSetValueFunc = pSetValueFunc;

	s.pSetValueFunc2D = nullptr;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.sliderState = WG_STATE_NORMAL;
	s.handleState = WG_STATE_NORMAL;

	_updateHandlePos(s);
	_updateGeo(s);

	return m_sliders.size() - 1;
}

//____ AddSlider2D() __________________________________________________________

int WgMultiSlider::AddSlider2D( int id, WgOrigo origo, SetGeoFunc pSetGeoFunc, float startValueX, float startValueY,
								float minValueX, float maxValueX, int stepsX, float minValueY, float maxValueY, int stepsY,
								SetValueFunc2D pSetValueFunc,
								const WgSkinPtr& pBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders handleMarkExtension, WgBorders sliderMarkExtension)
{
	if (origo != WG_NORTHWEST && origo != WG_NORTHEAST && origo != WG_SOUTHEAST && origo != WG_SOUTHWEST)
		return -1;

	m_sliders.emplace_back();
	Slider& s = m_sliders.back();

	s.id = id;
	s.is2D = true;

	s.value[0] = startValueX;
	s.bounds[0].min = minValueX;
	s.bounds[0].max = maxValueX;
	s.bounds[0].steps = stepsX;

	s.value[1] = startValueY;
	s.bounds[1].min = minValueY;
	s.bounds[1].max = maxValueY;
	s.bounds[1].steps = stepsY;

	s.origo = origo;

	s.handleMarkExtension = handleMarkExtension;
	s.sliderMarkExtension = sliderMarkExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;

	s.pSetValueFunc = nullptr;

	s.pSetValueFunc2D = pSetValueFunc;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.sliderState = WG_STATE_NORMAL;
	s.handleState = WG_STATE_NORMAL;

	_updateHandlePos(s);
	_updateGeo(s);

	return m_sliders.size() - 1;
}

//____ RemoveAllSliders() _______________________________________________________

void WgMultiSlider::RemoveAllSliders()
{
    m_sliders.clear();
    m_selectedSliderHandle = -1;

    _requestRender();
}


//____ SetSliderValue() _______________________________________________________

float WgMultiSlider::SetSliderValue(int id, float value, float value2)
{
	Slider * p = _findSlider(id);
	if (!p || (p->is2D == isnan(value2)) )
		return NAN;

	_invokeSetValueCallback(*p, value, value2);

	return _setValue(*p, value, value2, false);
}

//____ HandlePointPos() ________________________________________________________

WgCoord WgMultiSlider::HandlePointPos( int sliderId )
{
    WgCoord pixelPos = HandlePixelPos(sliderId);

    if(pixelPos.x == -1 && pixelPos.y == -1 )
        return pixelPos;

    return pixelPos*WG_SCALE_BASE / m_scale;
}

//____ HandlePixelPos() ________________________________________________________

WgCoord WgMultiSlider::HandlePixelPos( int sliderId )
{
    WgCoord pos(-1,-1);

    Slider * p = _findSlider(sliderId);
    if( p )
    {
        WgRect canvas = m_pSkin ? m_pSkin->ContentRect( PixelSize(), m_state, m_scale ) : WgRect(PixelSize());
        WgRect sliderGeo = _sliderGeo(*p, canvas);

        pos.x = sliderGeo.x + (int)(sliderGeo.w * p->handlePos.x);
        pos.y = sliderGeo.y + (int)(sliderGeo.h * p->handlePos.y);

        if( m_pSkin )
            pos += m_pSkin->ContentOfs( m_state, m_scale );
    }
    return pos;
}

//____ MarkTest() _____________________________________________________________

bool WgMultiSlider::MarkTest(const WgCoord& ofs)
{
	// First do the normal transparency based test. This will call _onAlphaTest() and 
	// take markOpacity into account.

	if (WgWidget::MarkTest(ofs))
		return true;

	// Check against our sliders (backgrounds)

	if (_markedSlider(ofs))
		return true;

	// Check against our slider handles

	if (_markedSliderHandle(ofs))
		return true;

	return false;
}


//____ SetSkin() ______________________________________________________________

void WgMultiSlider::SetSkin(const WgSkinPtr& pSkin)
{
	if (pSkin != m_pSkin)
	{
		m_pSkin = pSkin;
		_requestResize();
		_requestRender();
	}
}

//____ SetPressMode() _________________________________________________________

void WgMultiSlider::SetPressMode(PressMode mode)
{
	m_pressMode = mode;
}

//____ SetFinetune() __________________________________________________________

void WgMultiSlider::SetFinetune(int stepSize, float stepIncrement)
{
	m_finetuneStepSize		= stepSize;
	m_finetuneStepIncrement = stepIncrement;
}

//____ SetModifierKeys() ______________________________________________________

void WgMultiSlider::SetModifierKeys(WgModifierKeys finetune, WgModifierKeys axisLock, WgModifierKeys override )
{
	m_finetuneModifier = finetune;
	m_axisLockModifier = axisLock;
	m_overrideModifier = override;
}


//____ _markedSlider() ________________________________________________________

WgMultiSlider::Slider * WgMultiSlider::_markedSlider(WgCoord ofs, WgCoord * pOfsOutput)
{
	Slider *	pMarked = nullptr;
	WgCoord		markedOfs = { 0,0 };

	if (ofs.x == 0 && ofs.y == 0)
		return nullptr;

	for (auto& slider : m_sliders)
	{
		WgRect sliderGeo = _sliderSkinGeo(slider,_sliderGeo(slider, PixelSize()));

		WgRect sliderMarkGeo = sliderGeo + (slider.sliderMarkExtension.IsEmpty() ? m_defaultSliderMarkExtension : slider.sliderMarkExtension);

		if (sliderMarkGeo.Contains(ofs) )
		{
			markedOfs = ofs - sliderGeo.Pos();
			pMarked = &slider;
		}
	}

	if (pOfsOutput)
		*pOfsOutput = markedOfs;

	return pMarked;
}


//____ _markedSliderHandle() ________________________________________________________

WgMultiSlider::Slider * WgMultiSlider::_markedSliderHandle(WgCoord ofs, WgCoord * pOfsOutput )
{
	//TODO: Check distance for slightly marked and keep the closest one.

	Slider *	pSlightlyMarked = nullptr;
	WgCoord		slightlyMarkedOfs;

	Slider *	pFullyMarked = nullptr;
	WgCoord		fullyMarkedOfs;

	if (m_bGhostHandle || (ofs.x == 0 && ofs.y == 0) )
		return nullptr;

	for (auto& slider : m_sliders)
	{
		WgSkinPtr pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;
		if (pHandleSkin )
		{
			WgRect sliderGeo = _sliderGeo(slider, PixelSize());
			WgRect handleGeo = _sliderHandleGeo(slider, sliderGeo);

			if (handleGeo.Contains(ofs) && pHandleSkin->MarkTest(ofs - handleGeo.Pos(), handleGeo.Size(), slider.handleState, m_markOpacity, m_scale))
			{
				fullyMarkedOfs = ofs - handleGeo.Pos();
				pFullyMarked = &slider;
			}

			WgBorders markExtension = slider.handleMarkExtension.IsEmpty() ? m_defaultHandleMarkExtension : slider.handleMarkExtension;

			if (!markExtension.IsEmpty())
			{
				WgRect extendedGeo = handleGeo + markExtension.Scale(m_scale);

				if (extendedGeo.Contains(ofs))
				{
					slightlyMarkedOfs = ofs - handleGeo.Pos();
					pSlightlyMarked = &slider;
				}
			}
		}
	}

	if (pFullyMarked)
	{
		if (pOfsOutput)
			*pOfsOutput = fullyMarkedOfs;
		return pFullyMarked;
	}
	else
	{
		if (pOfsOutput)
			*pOfsOutput = slightlyMarkedOfs;
		return pSlightlyMarked;
	}
}

//____ _markSliderHandle() __________________________________________________________

void WgMultiSlider::_markSliderHandle(Slider * pSlider)
{
	if (pSlider)
		m_hoveredSliderHandle = pSlider - &m_sliders.front();
	else
		m_hoveredSliderHandle = -1;

	_updateSliderStates();
}

//____ _selectSliderHandle() ________________________________________________________

void WgMultiSlider::_selectSliderHandle(Slider * pSlider)
{
	if (pSlider && m_selectedSliderHandle == pSlider - &m_sliders.front())
		return;											// Already selected, nothing to do.

	// Select this slider

	if (pSlider)
	{
		// Clear drag related information

		m_dragFraction = { 0.f,0.f };
		m_dragStartFraction = pSlider->handlePos;
		m_finetuneRemainder = { 0,0 };
		m_selectedSliderHandle = pSlider - &m_sliders.front();
	}
	else
		m_selectedSliderHandle = -1;

	_updateSliderStates();
}

//____ _setSliderStates() ___________________________________________________

void WgMultiSlider::_setSliderStates(Slider& slider, WgState newSliderState, WgState newHandleState)
{
	WgSkin * pSliderSkin = slider.pBgSkin ? slider.pBgSkin.GetRealPtr() : m_pDefaultBgSkin.GetRealPtr();
	WgSkin * pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin.GetRealPtr() : m_pDefaultHandleSkin.GetRealPtr();

	if (pSliderSkin && !pSliderSkin->IsStateIdentical(slider.sliderState, newSliderState))
		_requestRenderSlider(&slider);
	else if (pHandleSkin && !pHandleSkin->IsStateIdentical(slider.handleState, newHandleState))
		_requestRenderHandle(&slider);

	slider.sliderState = newSliderState;
	slider.handleState = newHandleState;
}


//____ _requestRenderSlider() _________________________________________________

void WgMultiSlider::_requestRenderSlider(Slider * pSlider)
{
	WgRect sliderGeo = _sliderGeo(*pSlider, PixelSize());
	WgRect handleGeo = _sliderHandleGeo(*pSlider, sliderGeo);

	WgSkinPtr	pBgSkin = pSlider->pBgSkin ? pSlider->pBgSkin : m_pDefaultBgSkin;

	if (pBgSkin)
	{
		WgRect sliderSkinGeo = _sliderSkinGeo(*pSlider, sliderGeo);
		handleGeo.GrowToContain(sliderSkinGeo);
	}

	_requestRender(handleGeo);
}

//____ _requestRenderHandle() _________________________________________________

void WgMultiSlider::_requestRenderHandle(Slider * pSlider)
{
	WgRect sliderGeo = _sliderGeo(*pSlider, PixelSize());
	WgRect handleGeo = _sliderHandleGeo(*pSlider, sliderGeo);

	_requestRender(handleGeo);
}


//____ _ updatePointerStyle() _________________________________________________

void WgMultiSlider::_updatePointerStyle(WgCoord pointerOfs)
{
	Slider * pMarked = nullptr;

	if (m_selectedSliderHandle != -1)
		pMarked = &m_sliders[m_selectedSliderHandle];
	else if (m_selectedSlider != -1)
		pMarked = &m_sliders[m_selectedSlider];
	else
		pMarked = _markedSliderHandle(pointerOfs);

	if (!pMarked && (m_pressMode == PressMode::SetValue || m_pressMode == PressMode::MultiSetValue))
		pMarked = _markedSlider(pointerOfs);

	if( pMarked )
	{
		switch (pMarked->origo)
		{
		case WG_NORTH:
		case WG_SOUTH:
			m_pointerStyle = WG_POINTER_SIZE_N_S;
			break;
		case WG_WEST:
		case WG_EAST:
			m_pointerStyle = WG_POINTER_SIZE_W_E;
			break;
		default:
			m_pointerStyle = WG_POINTER_SIZE_ALL;
			break;
		}
	}
	else
		m_pointerStyle = WG_POINTER_DEFAULT;
}

//____ _updateSliderStates() __________________________________________________

void WgMultiSlider::_updateSliderStates()
{

	if (!m_state.isEnabled())
	{
		for (auto& slider : m_sliders)
		{
			if (slider.sliderState.isEnabled())
				_setSliderStates(slider, WG_STATE_DISABLED, WG_STATE_DISABLED);
		}
		return;
	}

	// Update sliderStates (states for background)

	if (m_selectedSlider >= 0 || m_selectedSliderHandle >= 0)
	{
		// Fullfills the following rules:
		// 1. If some HANDLE is SELECTED -> That sliders background is PRESSED, no other background is HOVERED.
		// 2. If some SLIDER is PRESSED->That sliders background is PRESSED, no other background is HOVERED.

		Slider& selected = m_sliders[m_selectedSliderHandle >= 0 ? m_selectedSliderHandle : m_selectedSlider];
		for ( auto& slider : m_sliders )
		{
			if (&slider == &selected)
			{
				if(!slider.sliderState.isPressed() )
					_setSliderStates(slider, slider.sliderState + WG_STATE_PRESSED, slider.handleState );
			}
			else
			{
				if( slider.sliderState.isHovered())
					_setSliderStates(slider, slider.sliderState - WG_STATE_HOVERED, slider.handleState);
			}
		}

	}
	else if (m_pressMode == PressMode::MultiSetValue && m_state.isPressed())
	{
		// Fullfills the following rules:
		// 3. If DRAW MODE and mouse pressed over background or handle->background is PRESSED.

		if (m_hoveredSlider >= 0 || m_hoveredSliderHandle >= 0)
		{
			Slider& hovered = m_sliders[m_hoveredSliderHandle >= 0 ? m_hoveredSliderHandle : m_hoveredSlider];
			for (auto& slider : m_sliders)
			{
				if (&slider == &hovered)
				{
					if (!slider.sliderState.isPressed())
						_setSliderStates(slider, slider.sliderState + WG_STATE_PRESSED, slider.handleState);
				}
				else
				{
					if (slider.sliderState.isHovered())
						_setSliderStates(slider, slider.sliderState - WG_STATE_HOVERED, slider.handleState);
				}
			}
		}
		else
		{
			for (auto& slider : m_sliders)
			{
				if (slider.sliderState.isHovered())
					_setSliderStates(slider, slider.sliderState - WG_STATE_HOVERED, slider.handleState);
			}
		}
	}
	else
	{
		// Fullfills the following rules:
		// 4. If mouse over selection area OR handle->That sliders background is HOVERED.

		if( m_hoveredSlider >= 0 || m_hoveredSliderHandle >= 0 )
		{
			Slider& hovered = m_sliders[m_hoveredSliderHandle >= 0 ? m_hoveredSliderHandle : m_hoveredSlider];
			for (auto& slider : m_sliders)
			{
				if (&slider == &hovered)
				{
					if( slider.sliderState.isPressed() )
						_setSliderStates(slider, slider.sliderState - WG_STATE_PRESSED, slider.handleState);
					else if (!slider.sliderState.isHovered())
						_setSliderStates(slider, slider.sliderState + WG_STATE_HOVERED, slider.handleState);
				}
				else
				{
					if (slider.sliderState.isHovered())
						_setSliderStates(slider, slider.sliderState - WG_STATE_HOVERED, slider.handleState);
				}
			}
		}
		else
		{
			for (auto& slider : m_sliders)
			{
				if (slider.sliderState.isHovered())
					_setSliderStates(slider, slider.sliderState - WG_STATE_HOVERED, slider.handleState);
			}
		}
	}

	// Update handle states

	if (m_bGhostHandle)
	{
		// Fullfills the following rules:
		// 1. If GHOST mode->All handles follows their backgrounds states.

		for (auto& slider : m_sliders)
		{
			if( slider.handleState != slider.sliderState )
				_setSliderStates(slider, slider.sliderState, slider.sliderState);
		}
	}
	else if (m_selectedSliderHandle >= 0)
	{
		// Fullfills the following rules:
		// 2. If a handle is SELECTED->That handle is PRESSED, no other handle is HOVERED.

		Slider& selected = m_sliders[m_selectedSliderHandle];
		for (auto& slider : m_sliders)
		{
			if (&slider == &selected)
			{
				if (!slider.handleState.isPressed())
					_setSliderStates(slider, slider.sliderState, slider.handleState + WG_STATE_PRESSED );
			}
			else
			{
				if (slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WG_STATE_HOVERED);
			}
		}

	}
	else if (m_pressMode == PressMode::MultiSetValue && m_state.isPressed())
	{
		// Fullfills the following rules:
		// 3. If DRAW MODE and mouse pressed over background or handle-> handle is PRESSED.

		if (m_hoveredSliderHandle >= 0)
		{
			Slider& hovered = m_sliders[m_hoveredSliderHandle];
			for (auto& slider : m_sliders)
			{
				if (&slider == &hovered)
				{
					if (!slider.handleState.isPressed())
						_setSliderStates(slider, slider.sliderState, slider.handleState + WG_STATE_PRESSED );
				}
				else
				{
					if (slider.handleState.isHovered())
						_setSliderStates(slider, slider.sliderState, slider.handleState - WG_STATE_HOVERED);
				}
			}
		}
		else
		{
			for (auto& slider : m_sliders)
			{
				if (slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WG_STATE_HOVERED);
			}
		}
	}

	else if (m_hoveredSliderHandle >= 0)
	{
		// Fullfills the following rules:
		// 4. If a handle is HOVERED->That handle is HOVERED.

		Slider& hovered = m_sliders[m_hoveredSliderHandle];
		for (auto& slider : m_sliders)
		{
			if (&slider == &hovered)
			{
				if (slider.handleState.isPressed())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WG_STATE_PRESSED);
				if (!slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState + WG_STATE_HOVERED);
			}
			else
			{
				if (slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WG_STATE_HOVERED);
			}
		}
	}
	else
	{
		for (auto& slider : m_sliders)
		{
			if (slider.handleState.isHovered())
				_setSliderStates(slider, slider.sliderState, slider.handleState - WG_STATE_HOVERED);
		}
	}
}


//____ _onEvent() _____________________________________________________________

void WgMultiSlider::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	//TODO: Handle swallowing of events.
	//TODO: Send SliderPressed events for all mouse buttons, not just button 1.
	//TODO: Detect release of modifier keys for finetune and axislock and handle it in a better way.

	bool	bSwallow = false;

	switch (pEvent->Type())
	{
		case WG_EVENT_KEY_RELEASE:
		{
			// Check if a modifier key has been released and act accordingly.


			WgModifierKeys modKeys = pEvent->ModKeys();

			if( (modKeys & m_finetuneModifier) != m_finetuneModifier )
				m_finetuneRemainder = { 0,0 };

			if ((modKeys & m_axisLockModifier) != m_axisLockModifier)
				m_axisLockState = AxisLockState::Unlocked;
			break;
		}


		case WG_EVENT_MOUSE_LEAVE:
			m_state.setHovered(false);

			if (m_selectedSliderHandle == -1)
			{
				m_hoveredSlider = -1;
				m_hoveredSliderHandle = -1;
				_updateSliderStates();
			}
			break;

		case WG_EVENT_MOUSE_ENTER:
			m_state.setHovered(true);
												// No break on purpose!
		case WG_EVENT_MOUSE_MOVE:
			if (m_selectedSliderHandle == -1)
			{
				Slider * p = _markedSliderHandle(pEvent->PointerPixelPos() );
				
				if(p)
					_markSliderHandle(p);
				else
				{
					m_hoveredSlider = -1;
					m_hoveredSliderHandle = -1;

					p = _markedSlider(pEvent->PointerPixelPos());
					if (p)
						m_hoveredSlider = p - &m_sliders.front();
					else
						int dbg = 1;

					_updateSliderStates();
				}
			}
			_updatePointerStyle(pEvent->PointerPixelPos());
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			if (!m_state.isEnabled())
				break;

			const WgEvent::MouseButtonPress * pEv = static_cast<const WgEvent::MouseButtonPress*>(pEvent);
			WgCoord	pointerPos = pEvent->PointerPixelPos();

			// Find pressed slider and whether handle was pressed

			WgCoord markOfs;
			Slider * pMarked = _markedSliderHandle(pointerPos, nullptr);
			bool		bHandlePressed = pMarked;		// If press really was on handle, needed for message.

			if(!bHandlePressed)
				pMarked = _markedSlider(pointerPos, &markOfs);

			// Queue the event. Need to do this before we update any handle position.

			if (pMarked)
			{
				// Set pressOfs for the event

				WgOrigo pressOfs = WG_CENTER;

				if (!bHandlePressed)
				{
					WgSize widgetSize = PixelSize();
					WgCoordF relPos = { markOfs.x / (pMarked->geo.w*widgetSize.w), markOfs.y / (pMarked->geo.h*widgetSize.h) };

					if (pMarked->origo == WG_WEST || pMarked->origo == WG_EAST)		// Horizontal slider
					{
						if (relPos.x < pMarked->handlePos.x)
							pressOfs = WG_WEST;
						else
							pressOfs = WG_EAST;
					}
					else if (pMarked->origo == WG_NORTH || pMarked->origo == WG_SOUTH)		// Vertical slider
					{
						if (relPos.y < pMarked->handlePos.y)
							pressOfs = WG_NORTH;
						else
							pressOfs = WG_SOUTH;
					}
					else																// 2D slider
					{
						if (relPos.y < pMarked->handlePos.y)
						{
							if (relPos.x < pMarked->handlePos.x)
								pressOfs = WG_NORTHWEST;
							else
								pressOfs = WG_NORTHEAST;
						}
						else
						{
							if (relPos.x < pMarked->handlePos.x)
								pressOfs = WG_SOUTHWEST;
							else
								pressOfs = WG_SOUTHEAST;
						}
					}
				}

				pHandler->QueueEvent(new WgEvent::SliderPressed(this, pMarked->id, pEv->Button(), pressOfs));
			}

			// Do everyting else

			if (pEv->Button() == 1)
				m_state.setPressed(true);

			if (pEv->Button() == 1 && (m_overrideModifier == WG_MODKEY_NONE || (pEv->ModKeys() != m_overrideModifier)) )
			{
				GrabFocus();

				if (bHandlePressed)
				{
					_selectSliderHandle(pMarked);
				}
				else if (pMarked)
				{
					WgSize widgetSize = PixelSize();

					m_selectedSlider = pMarked - &m_sliders.front();
					m_selectedSliderHandle = -1;

					// Convert the press offset to fraction.

					WgCoordF relPos = { markOfs.x / (pMarked->geo.w*widgetSize.w), markOfs.y / (pMarked->geo.h*widgetSize.h) };

					// In PressMode SetValue and MultiSetValue we set the value directly.

					if (m_pressMode == PressMode::SetValue || m_pressMode == PressMode::MultiSetValue)
					{
						if (m_bPassive)
							_calcSendValue(*pMarked, relPos);
						else
							_setHandlePosition(*pMarked, relPos);

						m_hoveredSliderHandle = pMarked - &m_sliders.front();
						m_hoveredSlider = -1;
					}

					if (m_pressMode == PressMode::SetValue)
                    {
						_selectSliderHandle(pMarked);				// In SetValue mode we actually select the handle

                        // HACK:
                        // In passive mode we need to set dragStartFraction directly since
                        // slider.handlePos is out of date (waiting to be set externally).

                        if( m_bPassive )
                        {
                            relPos = _alignPosToStep(*pMarked, relPos);

                            WG_LIMIT(relPos.x, 0.f, 1.f);
                            WG_LIMIT(relPos.y, 0.f, 1.f);

                            m_dragStartFraction = relPos;
                        }
                    }
					else
					{
						m_selectedSlider = -1;
						_updateSliderStates();
					}
				}
			}

			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			const WgEvent::MouseButtonRelease * p = static_cast<const WgEvent::MouseButtonRelease*>(pEvent);

			if (m_state.isEnabled() && p->Button() == 1)
			{
				m_state.setPressed(false);

				m_selectedSlider = -1;
				m_selectedSliderHandle = -1;

				m_hoveredSliderHandle = -1;
				m_hoveredSlider = -1;

				Slider * p = _markedSliderHandle(pEvent->PointerPixelPos());
				if (p)
					m_hoveredSliderHandle = p - &m_sliders.front();
				else
				{
					p = _markedSlider(pEvent->PointerPixelPos());
					if (p)
						m_hoveredSlider = p - &m_sliders.front();
				}

				_updateSliderStates();
			}
			break;
		}

		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			const WgEvent::MouseButtonDrag * pEv = static_cast<const WgEvent::MouseButtonDrag*>(pEvent);

			// If a modKey that we don't support is pressed, we ignore all modkeys.

			int modKeys = pEv->ModKeys();
			if (modKeys != (modKeys & (m_finetuneModifier | m_axisLockModifier)))
				modKeys = 0;


			if (m_state.isEnabled() && pEv->Button() == 1 && (m_overrideModifier == WG_MODKEY_NONE || (pEv->ModKeys() != m_overrideModifier)) )
			{
				WgCoordF	fraction;

				if (m_selectedSliderHandle >= 0)
				{
					Slider& slider = m_sliders[m_selectedSliderHandle];

                    //

					WgRect sliderGeo = _sliderGeo(slider, PixelSize());

					WgCoord dragNow = pEv->DraggedNowPixels();

					WgCoordF movement;
					

					if (modKeys & m_finetuneModifier)
					{
						int pixelStepSize = m_finetuneStepSize == 0 ? m_scale / WG_SCALE_BASE : m_finetuneStepSize * m_scale / WG_SCALE_BASE;
						WgCoordF stepIncrement;
						
						if (m_finetuneStepIncrement == 0.f)
						{
							stepIncrement.x = 1 / (sliderGeo.w - 0.0001);
							stepIncrement.y = 1 / (sliderGeo.h - 0.0001);
						}
						else
						{
							stepIncrement.x = m_finetuneStepIncrement;
							stepIncrement.y = m_finetuneStepIncrement;
						}

						m_finetuneRemainder += dragNow;

						movement.x = (m_finetuneRemainder.x / pixelStepSize) * stepIncrement.x;
						movement.y = (m_finetuneRemainder.y / pixelStepSize) * stepIncrement.y;

						m_finetuneRemainder.x %= pixelStepSize;
						m_finetuneRemainder.y %= pixelStepSize;
					}
					else
					{
						movement.x = sliderGeo.w == 0 ? 0 : ((float) dragNow.x) / (sliderGeo.w-0.0001);
						movement.y = sliderGeo.h == 0 ? 0 : ((float)dragNow.y) / (sliderGeo.h-0.0001);

						m_finetuneRemainder = { 0,0 };
					}

					if (modKeys & m_axisLockModifier && slider.is2D)
					{
						switch (m_axisLockState)
						{
						case AxisLockState::Unlocked:
						{
							m_axisLockPosition = pEv->PrevPixelPos();
							m_axisLockFraction = m_dragFraction;
							m_axisLockState = AxisLockState::Locking;
							break;
						}

						case AxisLockState::Locking:
						{
							WgCoord drag = pEv->CurrPixelPos() - m_axisLockPosition;
							drag.x = abs(drag.x);
							drag.y = abs(drag.y);

							if (drag.x >= 3 && drag.x > drag.y * 2)
								m_axisLockState = AxisLockState::Horizontal;
							else if (drag.y >= 3 && drag.y > drag.x * 2)
								m_axisLockState = AxisLockState::Vertical;
							break;
						}
						case AxisLockState::Horizontal:
							m_dragFraction.x += movement.x;
							break;
						case AxisLockState::Vertical:
							m_dragFraction.y += movement.y;
							break;
						default:
							assert(false);							// Should never get here...
						}
					}
					else
					{
						m_dragFraction += movement;
					}

					fraction = m_dragStartFraction + m_dragFraction;

					WG_LIMIT(fraction.x, 0.f, 1.f);
					WG_LIMIT(fraction.y, 0.f, 1.f);

					if (m_bDeltaDrag)
						m_dragFraction = fraction - m_dragStartFraction;

					if (m_bPassive)
						_calcSendValue(slider, fraction);
					else
						_setHandlePosition(slider, fraction);
				}
				else
				{
					if (m_pressMode == PressMode::MultiSetValue)
					{

						WgCoord prevPos = pEv->PrevPixelPos();
						WgCoord currPos = pEv->PointerPixelPos();

						for (auto& slider : m_sliders)
						{
							WgRect sliderGeo = _sliderSkinGeo(slider, _sliderGeo(slider, PixelSize()));
							WgRect sliderMarkGeo = sliderGeo + (slider.sliderMarkExtension.IsEmpty() ? m_defaultSliderMarkExtension : slider.sliderMarkExtension);

							if (prevPos.x < sliderMarkGeo.x && currPos.x > sliderMarkGeo.x)
								int a = 0;

							if (sliderMarkGeo.IntersectsWithOrContains(prevPos, currPos) )
							{
								WgCoord clippedPrevPos = prevPos;
								WgCoord clippedCurrPos = currPos;
								sliderMarkGeo.ClipLine(&clippedPrevPos, &clippedCurrPos);

								WgCoord	handlePos = clippedCurrPos - sliderGeo.Pos();

								fraction.x = sliderGeo.w == 0 ? 0 : handlePos.x / (float)sliderGeo.w;
								fraction.y = sliderGeo.h == 0 ? 0 : handlePos.y / (float)sliderGeo.h;

								WG_LIMIT(fraction.x, 0.f, 1.f);
								WG_LIMIT(fraction.y, 0.f, 1.f);

								if (m_bPassive)
									_calcSendValue(slider, fraction);
								else
									_setHandlePosition(slider, fraction);

								if (modKeys & m_finetuneModifier)
								{
									_selectSliderHandle(&slider);
									break;										// Only one can be selected to finetune.
								}
							}
						}
/*
						WgCoord		handlePos;
						pAffectedSlider = _markedSlider(pEv->CurrPixelPos(), &handlePos);
						if (pAffectedSlider)
						{
							WgRect sliderGeo = _sliderGeo(*pAffectedSlider, PixelSize());

							fraction.x = sliderGeo.w == 0 ? 0 : handlePos.x / (float)sliderGeo.w;
							fraction.y = sliderGeo.h == 0 ? 0 : handlePos.y / (float)sliderGeo.h;

							if (pEv->ModKeys() == m_finetuneModifier)
								_selectSliderHandle(pAffectedSlider);
						}
*/
					}
				}


			}

			break;
		}
        default:
            break;
	}
}

//____ _onCloneContent() _______________________________________________________

void WgMultiSlider::_onCloneContent( const WgWidget * _pOrg )
{
	WgMultiSlider * pOrg = (WgMultiSlider*) _pOrg;

}

//____ _onRender() _____________________________________________________________

void WgMultiSlider::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	WgWidget::_onRender(pDevice, _canvas, _window, _clip);

	WgRect contentCanvas = m_pSkin ? m_pSkin->ContentRect(_canvas, m_state, m_scale) : _canvas;


	for (auto& slider : m_sliders)
	{
		WgRect sliderGeo = _sliderGeo(slider, contentCanvas);

		WgSkinPtr pBgSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;
		if (pBgSkin)
		{
			WgRect bgGeo = _sliderSkinGeo(slider, sliderGeo);

			if (bgGeo.IntersectsWith(_clip))
			{
				WgState	emptyPartState = slider.sliderState;
				WgState filledPartState = slider.sliderState;
				filledPartState.setSelected(true);


				if (pBgSkin->IsStateIdentical(emptyPartState, filledPartState))
					pBgSkin->Render(pDevice, emptyPartState, bgGeo, _clip, m_scale);
				else
				{
					WgCoord divider = { sliderGeo.x + (int)(slider.handlePos.x*sliderGeo.w), sliderGeo.y + (int)(slider.handlePos.y*sliderGeo.h) };

					switch (slider.origo)
					{
					case WG_NORTHWEST:
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,divider.y,bgGeo.w, bgGeo.y + bgGeo.h - divider.y }), m_scale);
						break;
					case WG_NORTH:
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,bgGeo.w,divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x, divider.y, bgGeo.w, bgGeo.y + bgGeo.h - divider.y }), m_scale);
						break;
					case WG_NORTHEAST:
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,divider.y,bgGeo.w, bgGeo.y + bgGeo.h - divider.y }), m_scale);
						break;
					case WG_EAST:
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,bgGeo.h }), m_scale);
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.h }), m_scale);
						break;
					case WG_SOUTHEAST:
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,bgGeo.w, divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,divider.y,divider.x - bgGeo.x,bgGeo.y + bgGeo.h - divider.y }), m_scale);
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { divider.x, divider.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.y + bgGeo.h - divider.y }), m_scale);
						break;
					case WG_SOUTH:
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,bgGeo.w,divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { bgGeo.x, divider.y, bgGeo.w, bgGeo.y + bgGeo.h - divider.y }), m_scale);
						break;
					case WG_SOUTHWEST:
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,bgGeo.w, divider.y - bgGeo.y }), m_scale);
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { bgGeo.x,divider.y,divider.x - bgGeo.x,bgGeo.y + bgGeo.h - divider.y }), m_scale);
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { divider.x, divider.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.y + bgGeo.h - divider.y }), m_scale);
						break;
					case WG_WEST:
						pBgSkin->Render(pDevice, filledPartState, bgGeo, WgRect(_clip, { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,bgGeo.h }), m_scale);
						pBgSkin->Render(pDevice, emptyPartState, bgGeo, WgRect(_clip, { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.h }), m_scale);
						break;

					default:
						assert(0);
					}
				}

			}
		}

		WgSkinPtr pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;
		if (pHandleSkin)
		{
			WgRect sliderHandleGeo = _sliderHandleGeo(slider, sliderGeo);
			pHandleSkin->Render(pDevice, slider.handleState, sliderHandleGeo, _clip, m_scale);
		}
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgMultiSlider::_onAlphaTest( const WgCoord& ofs )
{
	if (WgWidget::_onAlphaTest(ofs))
		return true;

	for (auto& slider : m_sliders)
	{
		WgRect sliderGeo = _sliderGeo(slider, PixelSize() );

		WgSkinPtr pBgSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;
		if (pBgSkin)
		{
			WgRect bgGeo = _sliderSkinGeo(slider, sliderGeo);

			if( bgGeo.Contains(ofs) && pBgSkin->MarkTest(ofs-bgGeo.Pos(), bgGeo.Size(), slider.sliderState, m_markOpacity, m_scale) )
				return true;
		}

		if (!m_bGhostHandle)
		{
			WgSkinPtr pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;
			if (pHandleSkin)
			{
				WgRect handleGeo = _sliderHandleGeo(slider, sliderGeo);

				if (handleGeo.Contains(ofs) && pHandleSkin->MarkTest(ofs - handleGeo.Pos(), handleGeo.Size(), slider.handleState, m_markOpacity, m_scale))
					return true;
			}
		}
	}

	return false;
}

//____ _setScale() ____________________________________________________________

void WgMultiSlider::_setScale(int scale)
{
	WgWidget::_setScale(scale);
}

//____ _updateHandlePos() ______________________________________________________

void WgMultiSlider::_updateHandlePos(Slider& slider)
{
	WgCoordF	handlePos;

	if (slider.is2D)
	{
		WgCoordF values;

		values.x = (slider.value[0] - slider.bounds[0].min) / (slider.bounds[0].max - slider.bounds[0].min);
		values.y = (slider.value[1] - slider.bounds[1].min) / (slider.bounds[1].max - slider.bounds[1].min);

		switch (slider.origo)
		{
		case WG_NORTHWEST:
			handlePos = values;
			break;
		case WG_NORTHEAST:
			handlePos.x = 1.f - values.x;
			handlePos.y = values.y;
			break;
		case WG_SOUTHEAST:
			handlePos.x = 1.f - values.x;
			handlePos.y = 1.f - values.y;
			break;
		case WG_SOUTHWEST:
			handlePos.x = values.x;
			handlePos.y = 1.f - values.y;
			break;
        default:
            assert(false);   // Should never get here!
		}
	}
	else
	{
		float value;
			
		value = (slider.value[0] - slider.bounds[0].min) / (slider.bounds[0].max - slider.bounds[0].min);

		switch (slider.origo)
		{
		case WG_WEST:
			handlePos.x = value;
			break;
		case WG_EAST:
			handlePos.x = 1.f -value;
			break;
		case WG_NORTH:
			handlePos.y = value;
			break;
		case WG_SOUTH:
			handlePos.y = 1.f - value;
			break;
        default:
            assert(false);   // Should never get here!
		}
	}

	// Align position to even step sizes.

	handlePos = _alignPosToStep(slider, handlePos);

	// Limit range

	WG_LIMIT(handlePos.x, 0.f, 1.f);
	WG_LIMIT(handlePos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (handlePos == slider.handlePos)
		return;


	// Render what needs to be rendered

	WgRect sliderGeo = _sliderGeo(slider, PixelSize());

	WgRect oldHandleGeo = _sliderHandleGeo(slider, sliderGeo);

	slider.handlePos = handlePos;

	WgRect newHandleGeo = _sliderHandleGeo(slider, sliderGeo);

	if (newHandleGeo != oldHandleGeo)
	{
		_requestRender(oldHandleGeo);
		_requestRender(newHandleGeo);
	}

	WgSkinPtr pBgSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;
	if (pBgSkin && !pBgSkin->IsStateIdentical(slider.handleState, slider.handleState + WG_STATE_SELECTED))
	{	
		WgRect sliderSkinGeo = _sliderSkinGeo(slider, sliderGeo);
		_requestRender(sliderSkinGeo);
	}


	return;
}



//____ _updateGeo() ______________________________________________________

void WgMultiSlider::_updateGeo(Slider& slider)
{
	slider.geoState = 1;		// Update in progress

	SetGeoVisitor visitor(this, &slider);


	WgRectF newGeo = slider.pSetGeoFunc(visitor);

	if (newGeo != slider.geo)
	{
		WgRect	widgetGeo = PixelSize();

		WgRect oldSliderGeo = _sliderGeo(slider, widgetGeo);

		slider.geo = newGeo;

		WgRect newSliderGeo = _sliderGeo(slider, widgetGeo);

		// Check on pixel-geo as well, might be it has not been affected...

		if (newSliderGeo != oldSliderGeo)
		{
			WgRect oldGeo = _sliderHandleGeo(slider, oldSliderGeo);
			WgRect newGeo = _sliderHandleGeo(slider, newSliderGeo);

			// If we have background we just merge that into the geometries

			if (slider.pBgSkin || m_pDefaultBgSkin)
			{
				oldGeo.GrowToContain(_sliderSkinGeo(slider, oldSliderGeo));
				newGeo.GrowToContain(_sliderSkinGeo(slider, newSliderGeo));
			}

			_requestRender(oldGeo);
			_requestRender(newGeo);
		}

	}

	slider.geoState = 2;		// Update completed
}

//____ _sliderGeo() ___________________________________________________________

WgRect  WgMultiSlider::_sliderGeo(Slider& slider, const WgRect& _canvas )
{
    WgRect canvas = m_pSkin ? m_pSkin->ContentRect( _canvas, m_state, m_scale ) : _canvas;

	return { canvas.x + (int)(slider.geo.x * canvas.w + 0.5f), canvas.y + (int)(slider.geo.y * canvas.h + 0.5f), (int)(canvas.w * slider.geo.w), (int)(canvas.h * slider.geo.h) };
}

//____ _sliderSkinGeo() _______________________________________________________

WgRect  WgMultiSlider::_sliderSkinGeo(Slider& slider, const WgRect& sliderGeo)
{
	WgSkinPtr pSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;

    if(pSkin)
    {
        WgRect bgGeo = sliderGeo;
        bgGeo -= pSkin->ContentOfs(m_state, m_scale);
        bgGeo += pSkin->ContentPadding(m_scale);

        WgSize min = pSkin->PreferredSize(m_scale);

        return { bgGeo.Pos(),std::max(bgGeo.w,min.w), std::max(bgGeo.h,min.h) };
    }
    else
        return sliderGeo;

}

//____ _sliderHandleGeo() _____________________________________________________

WgRect  WgMultiSlider::_sliderHandleGeo(Slider& slider, const WgRect& sliderGeo)
{
	WgSkinPtr pSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;

	WgSize sz = pSkin ? pSkin->PreferredSize(m_scale) : WgSize(0,0);
	WgCoordF handleHotspot = slider.handleHotspot.x == -1.f ? m_defaultHandleHotspot : slider.handleHotspot;


	WgCoord pos = { (int)(slider.handlePos.x * sliderGeo.w), (int)(slider.handlePos.y * sliderGeo.h) };
	pos += sliderGeo.Pos();
	pos -= { (int)(handleHotspot.x * sz.w), (int)(handleHotspot.y * sz.h) };

	return { pos,sz };
}


//____ invokeSetValueCallback() _____________________________________________

void WgMultiSlider::_invokeSetValueCallback(Slider& slider, float& value, float& value2)
{
	if (slider.pSetValueFunc)
	{
		SetValueVisitor v(this, &slider, value);
		value = slider.pSetValueFunc(v);
		value2 = NAN;
	}
	else if (slider.pSetValueFunc2D)
	{
		SetValueVisitor2D v(this, &slider, { value, value2 });
		WgCoordF values = slider.pSetValueFunc2D(v);
		value = values.x;
		value2 = values.y;
	}
}



//____ _setValue() ____________________________________________________________

float WgMultiSlider::_setValue(Slider& slider, float value, float value2, bool bSendOnUpdate )
{
	WG_LIMIT(value, slider.bounds[0].min, slider.bounds[0].max);

	bool bUpdate = false;

    if(value != slider.value[0] )
    {
        slider.value[0] = value;
        bUpdate = true;
    }

	if ( !isnan(value2) )
	{
		WG_LIMIT(value2, slider.bounds[1].min, slider.bounds[1].max);

        if(value2 != slider.value[1] )
        {
            slider.value[1] = value2;
            bUpdate = true;
        }
	}

    if( bUpdate )
    {
        // Update handle position

        _updateHandlePos(slider);

        // Update geo in case some sliders geo is affected

        _refreshSliderGeo();

		// Send values

		if( bSendOnUpdate )
			_sendValue(slider, value, value2);
 
    }
	return value;
}

//____ _alignPosToStep() ______________________________________________________

WgCoordF WgMultiSlider::_alignPosToStep(Slider& slider, WgCoordF pos) const
{
	// Get parameters controlled by X and Y axies.

	int x = -1;
	int y = -1;

	switch (slider.origo)
	{
	case WG_WEST:
	case WG_EAST:
		x = 0;
		break;

	case WG_NORTH:
	case WG_SOUTH:
		y = 0;
		break;

	case WG_NORTHWEST:
	case WG_NORTHEAST:
	case WG_SOUTHEAST:
	case WG_SOUTHWEST:
		x = 0;
		y = 1;
		break;
	default:
		assert(false);   // Should never get here!
	}

	if (x >= 0)
	{
		if (slider.bounds[x].steps != 0)
		{
//			float stepPos = 1.f / (slider.bounds[x].steps - 1);
//			float stepSize = 1.f / (slider.bounds[x].steps);
//			int step = pos.x / stepSize;
//			pos.x = stepPos * step;

			float stepSize = 1.f / (slider.bounds[x].steps - 1);
			pos.x += stepSize / 2;
			pos.x -= fmod(pos.x, stepSize);
		}
	}

	if (y >= 0)
	{
		if (slider.bounds[y].steps != 0)
		{
			float stepSize = 1.f / (slider.bounds[y].steps / 1);
			pos.y += stepSize / 2;
			pos.y -= fmod(pos.y, stepSize);
		}
	}

	return pos;
}

//____ _calcSendValue() _______________________________________________________

WgCoordF WgMultiSlider::_calcSendValue(Slider& slider, WgCoordF pos)
{
	// Align position to even step sizes.

	pos = _alignPosToStep(slider, pos);

	// Limit range

	WG_LIMIT(pos.x, 0.f, 1.f);
	WG_LIMIT(pos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (pos == slider.handlePos)
		return pos;

	// Set handle position and handle re-rendering.

//	_requestRenderSlider(&slider);
//	slider.handlePos = pos;
//	_requestRenderSlider(&slider);

	// Calculate values from handle position

	float	value;
	float	value2;


	WgCoordF	handleFactor = _convertFactorPos(pos, slider.origo);

	if (slider.is2D)
	{
		value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		value2 = slider.bounds[1].min + (slider.bounds[1].max - slider.bounds[1].min)*handleFactor.y;
	}
	else
	{
		if (slider.origo == WG_WEST || slider.origo == WG_EAST )
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		else
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.y;
		value2 = NAN;
	}

	// Set values and possibly update any other affected sliders

	_sendValue(slider, value, value2);
	return pos;
}




//____ _sendValue() ____________________________________________________________

void WgMultiSlider::_sendValue(Slider& slider, float value, float value2)
{
	// Callback

	if (m_callback)
		m_callback(slider.id, value, value2);

	// Send event

	WgEventHandler * pHandler = _eventHandler();
	if (pHandler)
		pHandler->QueueEvent(new WgEvent::SliderMoved(this, slider.id, value, value2));

}


//____ _setHandlePosition() ______________________________________________

WgCoordF WgMultiSlider::_setHandlePosition(Slider& slider, WgCoordF pos)
{
	// Align position to even step sizes.

	pos = _alignPosToStep(slider, pos);

	// Limit range

	WG_LIMIT(pos.x, 0.f, 1.f);
	WG_LIMIT(pos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (pos == slider.handlePos)
		return pos;

	// Set handle position and handle re-rendering.

	_requestRenderSlider(&slider);
	slider.handlePos = pos;
	_requestRenderSlider(&slider);

	// Calculate values from handle position

	float	value;
	float	value2;

	WgCoordF	handleFactor = _convertFactorPos(slider.handlePos, slider.origo);

	if (slider.is2D)
	{
		value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		value2 = slider.bounds[1].min + (slider.bounds[1].max - slider.bounds[1].min)*handleFactor.y;
	}
	else
	{
		if (slider.origo == WG_WEST || slider.origo == WG_EAST)
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		else
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.y;
		value2 = NAN;
	}

	// Set values and possibly update any other affected sliders

	_invokeSetValueCallback(slider, value, value2);
	_setValue(slider, value, value2, true);
	return pos;
}


//____ _convertFactorPos() ________________________________________________________

WgCoordF WgMultiSlider::_convertFactorPos(WgCoordF in, WgOrigo origo)
{
	WgCoordF	pos = in;

	switch (origo)
	{
	case WG_WEST:
	case WG_NORTH:
	case WG_NORTHWEST:
		break;

	case WG_NORTHEAST:
	case WG_EAST:
		pos.x = 1.f - pos.x;
		break;

	case WG_SOUTHEAST:
		pos.x = 1.f - pos.x;
		pos.y = 1.f - pos.y;
		break;

	case WG_SOUTH:
	case WG_SOUTHWEST:
		pos.y = 1.f - pos.y;
		break;
    default:
        assert(false);   // Should never get here!
	}

	return pos;
}




//____ _refreshSliders() ___________________________________________________

void WgMultiSlider::_refreshSliders()
{
	for (auto& slider : m_sliders)
		slider.geoState = 0;

	for (auto& slider : m_sliders)
	{
		_updateHandlePos(slider);
		if(slider.geoState == 0 )
			_updateGeo(slider);
	}
}


//____ _refreshSliderGeo() ___________________________________________________

void WgMultiSlider::_refreshSliderGeo()
{
	for (auto& slider : m_sliders)
		slider.geoState = 0;

	for (auto& slider : m_sliders)
	{
		if (slider.geoState == 0)
			_updateGeo(slider);
	}
}

//____ _findSlider() __________________________________________________________

WgMultiSlider::Slider * WgMultiSlider::_findSlider(int sliderId)
{
	for (auto& slider : m_sliders)
		if (slider.id == sliderId)
			return &slider;

	return nullptr;
}

//____ SetValueVisitorBase::Constructor _______________________________________

WgMultiSlider::SetValueVisitorBase::SetValueVisitorBase(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider) : Visitor(pWidget,pSlider)
{
}

//____ SetValueVisitorBase::value() _______________________________________

float WgMultiSlider::SetValueVisitorBase::value(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || p->is2D)
		return NAN;

	return p->value[0];
}

//____ SetValueVisitorBase::setValue() _______________________________________

float WgMultiSlider::SetValueVisitorBase::setValue(int sliderId, float value)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || p->is2D)
		return NAN;

	bool bSendOnUpdate = (p == m_pSlider) ? false : true;
	return m_pWidget->_setValue(*p, value, NAN, bSendOnUpdate);
}

//____ SetValueVisitorBase::value2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitorBase::value2D(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || !p->is2D)
		return WgCoordF(NAN,NAN);

	return { p->value[0], p->value[0] };
}

//____ SetValueVisitorBase::setValue2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitorBase::setValue2D(int sliderId, WgCoordF value)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || !p->is2D)
		return WgCoordF(NAN, NAN);

	bool bSendOnUpdate = (p == m_pSlider) ? false : true;
	m_pWidget->_setValue(*p, value.x, value.y, bSendOnUpdate);

	return { p->value[0], p->value[1] };
}


//____ SetValueVisitor::Constructor ___________________________________________

WgMultiSlider::SetValueVisitor::SetValueVisitor(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider, float value) : SetValueVisitorBase(pWidget,pSlider) 
{
	m_value = value;
}

//____ SetValueVisitor::value() ___________________________________________

float WgMultiSlider::SetValueVisitor::value()
{
	return m_value;
}

//____ SetValueVisitor::valueBounds() ___________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetValueVisitor::valueBounds()
{
	return m_pSlider->bounds[0];
}


//____ SetValueVisitor2D::Constructor ___________________________________________

WgMultiSlider::SetValueVisitor2D::SetValueVisitor2D(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider, WgCoordF value) : SetValueVisitorBase(pWidget, pSlider)
{
	m_value = value;
}

//____ SetValueVisitor2D::value2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitor2D::value2D()
{
	return m_value;
}

//____ SetValueVisitor2D::valueBoundsX() ___________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetValueVisitor2D::valueBoundsX()
{
	return m_pSlider->bounds[0];
}

//____ SetValueVisitor2D::valueBoundsY() ___________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetValueVisitor2D::valueBoundsY()
{
	return m_pSlider->bounds[1];
}

//____ SetGeoVisitor::Constructor ___________________________________________________

WgMultiSlider::SetGeoVisitor::SetGeoVisitor(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider) : Visitor(pWidget, pSlider)
{
}

//____ SetGeoVisitor::handlePos() ___________________________________________________

WgCoordF WgMultiSlider::SetGeoVisitor::handlePos(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);

	if (p)
		return p->handlePos;

	return WgCoordF(NAN, NAN);
}

//____ SetGeoVisitor::geo() ___________________________________________________

WgRectF WgMultiSlider::SetGeoVisitor::geo(int sliderId)
{
	Slider * pSlider = m_pWidget->_findSlider(sliderId);
	assert(pSlider->geoState != 1);

	if (pSlider->geoState == 0)
		m_pWidget->_updateGeo(*pSlider);

	return pSlider->geo;
}