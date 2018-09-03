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

void WgMultiSlider::SetDefaults(const WgSkinPtr& pSliderBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders markExtension)
{
	m_pDefaultBgSkin = pSliderBgSkin;
	m_pDefaultHandleSkin = pHandleSkin;
	m_defaultHandleHotspot = handleHotspot;
	m_defaultMarkExtension = markExtension;
}

//____ SetCallback() __________________________________________________________

void WgMultiSlider::SetCallback(const std::function<void(int sliderId, float value, float value2 )>& callback)
{
	m_callback = callback;
}


//____ AddSlider() ____________________________________________________________

int WgMultiSlider::AddSlider(	int id, WgDirection dir, SetGeoFunc pSetGeoFunc, float startValue, float minValue, float maxValue, int steps,
								SetHandleFunc pSetHandleFunc, SetValueFunc pSetValueFunc, const WgSkinPtr& pBgSkin,
								const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders markExtension )
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

	s.markExtension = markExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;
	s.pSetHandleFunc = pSetHandleFunc;
	s.pSetValueFunc = pSetValueFunc;

	s.pSetHandleFunc2D = nullptr;
	s.pSetValueFunc2D = nullptr;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.handleState = WG_STATE_NORMAL;

	_updateHandlePos(s);
	_updateGeo(s);

	return m_sliders.size() - 1;
}

//____ AddSlider2D() __________________________________________________________

int WgMultiSlider::AddSlider2D( int id, WgOrigo origo, SetGeoFunc pSetGeoFunc, float startValueX, float startValueY,
								float minValueX, float maxValueX, int stepsX, float minValueY, float maxValueY, int stepsY,
								SetHandleFunc2D pSetHandleFunc, SetValueFunc2D pSetValueFunc,
								const WgSkinPtr& pBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders markExtension )
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

	s.markExtension = markExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;

	s.pSetHandleFunc = nullptr;
	s.pSetValueFunc = nullptr;

	s.pSetHandleFunc2D = pSetHandleFunc;
	s.pSetValueFunc2D = pSetValueFunc;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.handleState = WG_STATE_NORMAL;

	_updateHandlePos(s);
	_updateGeo(s);

	return m_sliders.size() - 1;
}

//____ RemoveAllSliders() _______________________________________________________

void WgMultiSlider::RemoveAllSliders()
{
    m_sliders.clear();
    m_selectedSlider = -1;

    _requestRender();
}


//____ SetSliderValue() _______________________________________________________

float WgMultiSlider::SetSliderValue(int id, float value, float value2)
{
	Slider * p = _findSlider(id);
	if (!p || (p->is2D == isnan(value2)) )
		return NAN;

	return _setValue(*p, value, value2);
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
        WgRect canvas = m_pSkin ? m_pSkin->ContentRect( PixelSize(), WG_STATE_NORMAL, m_scale ) : WgRect(PixelSize());
        WgRect sliderGeo = _sliderGeo(*p, canvas);

        pos.x = sliderGeo.x + sliderGeo.w * p->handlePos.x;
        pos.y = sliderGeo.y + sliderGeo.h * p->handlePos.y;

        if( m_pSkin )
            pos += m_pSkin->ContentOfs( WG_STATE_NORMAL, m_scale );
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

	// Do a secondary check against our markExtensions

	if (_markedSlider(ofs))
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

//____ _markedSlider() ________________________________________________________

WgMultiSlider::Slider * WgMultiSlider::_markedSlider(WgCoord ofs, WgCoord * pOfsOutput )
{
	//TODO: Check distance for slightly marked and keep the closest one.

	Slider *	pSlightlyMarked = nullptr;
	WgRect		slightlyMarkedOfs;

	Slider *	pFullyMarked = nullptr;
	WgRect		fullyMarkedOfs;

	if (ofs.x == 0 && ofs.y == 0)
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

			WgBorders markExtension = slider.markExtension.IsEmpty() ? m_defaultMarkExtension : slider.markExtension;

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

//____ _markSlider() __________________________________________________________

void WgMultiSlider::_markSlider(Slider * pSlider)
{
	if (pSlider && pSlider->handleState.isHovered())
		return;											// Already marked, nothing to do.

	// Unmark any previously marked slider

	for (auto& slider : m_sliders)
	{
		if (slider.handleState.isHovered() )
		{
			//TODO: Only re-render if state change results in graphic change
			//		WgState oldState = pSlider->handleState;
			slider.handleState.setHovered(false);

			_requestRenderHandle(&slider);
		}
	}

	// Mark this slider

	if (pSlider)
	{
		//TODO: Only re-render if state change results in graphic change
		//		WgState oldState = pSlider->handleState;
		pSlider->handleState.setHovered(true);

		switch(pSlider->origo)
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
		_requestRenderHandle(pSlider);
	}
	else
		m_pointerStyle = WG_POINTER_DEFAULT;
}

//____ _selectSlider() ________________________________________________________

void WgMultiSlider::_selectSlider(Slider * pSlider)
{
	if (pSlider && pSlider->handleState.isPressed())
		return;											// Already selected, nothing to do.

	// Unselect previously selected slider

	if (m_selectedSlider >= 0)
	{
		auto p = &m_sliders[m_selectedSlider];
		p->handleState.setPressed(false);
		_requestRenderHandle(p);
	}

	// Select this slider

	if (pSlider)
	{
		//TODO: Only re-render if state change results in graphic change
		//		WgState oldState = pSlider->handleState;
		pSlider->handleState.setPressed(true);
		_requestRenderHandle(pSlider);

		m_selectedSlider = pSlider - &m_sliders.front();
	}
	else
		m_selectedSlider = -1;
}

//____ _requestRenderHandle() _________________________________________________

void WgMultiSlider::_requestRenderHandle(Slider * pSlider)
{
	WgRect sliderGeo = _sliderGeo(*pSlider, PixelSize());
	WgRect handleGeo = _sliderHandleGeo(*pSlider, sliderGeo);

	_requestRender(handleGeo);
}




//____ _onEvent() _____________________________________________________________

void WgMultiSlider::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	switch (pEvent->Type())
	{
		case WG_EVENT_MOUSE_LEAVE:
			if(m_selectedSlider == -1)
				_markSlider(nullptr);
			break;

		case WG_EVENT_MOUSE_ENTER:
		case WG_EVENT_MOUSE_MOVE:
			if (m_selectedSlider == -1)
			{
				Slider * p = _markedSlider(pEvent->PointerPixelPos() );
				_markSlider(p);
			}
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			const WgEvent::MouseButtonPress * p = static_cast<const WgEvent::MouseButtonPress*>(pEvent);
		
			if (p->Button() == 1)
			{
				Slider * p = _markedSlider(pEvent->PointerPixelPos(), &m_selectPressOfs );
				if (p)
					_selectSlider(p);
			}
			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			const WgEvent::MouseButtonRelease * p = static_cast<const WgEvent::MouseButtonRelease*>(pEvent);

			if (p->Button() == 1)
			{
				_selectSlider(nullptr);
				Slider * p = _markedSlider(pEvent->PointerPixelPos());
				_markSlider(p);
			}
			break;
		}

		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			const WgEvent::MouseButtonDrag * p = static_cast<const WgEvent::MouseButtonDrag*>(pEvent);

			if (p->Button() == 1 && m_selectedSlider >=0 )
			{
				Slider& slider = m_sliders[m_selectedSlider];

				WgRect sliderGeo = _sliderGeo(slider, PixelSize());
				WgRect handleGeo = _sliderHandleGeo(slider, sliderGeo);
				WgCoordF handleHotspot = slider.handleHotspot.x == -1.f ? m_defaultHandleHotspot : slider.handleHotspot;


				WgCoord pos = p->PointerPixelPos() - m_selectPressOfs + WgCoord( (int)(handleGeo.w * handleHotspot.x), (int)(handleGeo.h * handleHotspot.y) );		// New hotspot pos for handle.

				pos = sliderGeo.Limit(pos);

				float relX = sliderGeo.w == 0 ? 0 : (pos.x - sliderGeo.x) / (float) sliderGeo.w;
				float relY = sliderGeo.h == 0 ? 0 : (pos.y - sliderGeo.y) / (float)sliderGeo.h;

				_setHandlePosition(slider, { relX,relY } );
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

	WgRect contentCanvas = m_pSkin ? m_pSkin->ContentRect(_canvas, WG_STATE_NORMAL, m_scale) : _canvas;

	for (auto& slider : m_sliders)
	{
		WgRect sliderGeo = _sliderGeo(slider, contentCanvas);

		WgSkinPtr pBgSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;
		if (pBgSkin)
		{
			WgRect bgGeo = _sliderSkinGeo(slider, sliderGeo);
			pBgSkin->Render(pDevice, WG_STATE_NORMAL, bgGeo, _clip, m_scale);
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

			if( pBgSkin->MarkTest(ofs-bgGeo.Pos(), bgGeo.Size(), WG_STATE_NORMAL, m_markOpacity, m_scale) )
				return true;
		}

		WgSkinPtr pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;
		if (pHandleSkin)
		{
			WgRect handleGeo = _sliderHandleGeo(slider, sliderGeo);

			if (pHandleSkin->MarkTest(ofs - handleGeo.Pos(), handleGeo.Size(), slider.handleState, m_markOpacity, m_scale))
				return true;
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

	int		x = -1;
	int		y = -1;

	if (slider.is2D)
	{
		x = 0;
		y = 1;

		WgCoordF values;

		if (slider.pSetHandleFunc2D)
        {
            auto v = SetHandleVisitor2D(this, &slider);
            values = slider.pSetHandleFunc2D(v);
        }
		else
		{
			values.x = (slider.value[0] - slider.bounds[0].min) / (slider.bounds[0].max - slider.bounds[0].min);
			values.y = (slider.value[1] - slider.bounds[1].min) / (slider.bounds[1].max - slider.bounds[1].min);
		}

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
			
		if (slider.pSetHandleFunc)
        {
            auto v = SetHandleVisitor(this, &slider);
			value = slider.pSetHandleFunc(v);
        }
		else
			value = (slider.value[0] - slider.bounds[0].min) / (slider.bounds[0].max - slider.bounds[0].min);

		switch (slider.origo)
		{
		case WG_WEST:
			handlePos.x = value;
			x = 0;
			break;
		case WG_EAST:
			handlePos.x = 1.f -value;
			x = 0;
			break;
		case WG_NORTH:
			handlePos.y = value;
			y = 0;
			break;
		case WG_SOUTH:
			handlePos.y = 1.f - value;
			y = 0;
			break;
        default:
            assert(false);   // Should never get here!
		}
	}

	// Align position to even step sizes.

	if (x >= 0)
	{
		if (slider.bounds[x].steps != 0)
		{
			float stepSize = 1.f / (slider.bounds[x].steps - 1);
			handlePos.x += stepSize / 2;
			handlePos.x -= fmod(handlePos.x, stepSize);
		}
	}

	if (y >= 0)
	{
		if (slider.bounds[y].steps != 0)
		{
			float stepSize = 1.f / (slider.bounds[y].steps / 1);
			handlePos.y += stepSize / 2;
			handlePos.y -= fmod(handlePos.y, stepSize);
		}
	}

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
    WgRect canvas = m_pSkin ? m_pSkin->ContentRect( _canvas, WG_STATE_NORMAL, m_scale ) : _canvas;

	return { canvas.x + (int)(slider.geo.x * canvas.w), canvas.y + (int)(slider.geo.y * canvas.h), (int)(canvas.w * slider.geo.w), (int)(canvas.h * slider.geo.h) };
}

//____ _sliderSkinGeo() _______________________________________________________

WgRect  WgMultiSlider::_sliderSkinGeo(Slider& slider, const WgRect& sliderGeo)
{
	WgSkinPtr pSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;

    if(pSkin)
    {
        WgRect bgGeo = sliderGeo;
        bgGeo -= pSkin->ContentOfs(WG_STATE_NORMAL, m_scale);
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

	WgSize sz = pSkin->PreferredSize(m_scale);
	WgCoordF handleHotspot = slider.handleHotspot.x == -1.f ? m_defaultHandleHotspot : slider.handleHotspot;


	WgCoord pos = { (int)(slider.handlePos.x * sliderGeo.w), (int)(slider.handlePos.y * sliderGeo.h) };
	pos += sliderGeo.Pos();
	pos -= { (int)(handleHotspot.x * sz.w), (int)(handleHotspot.y * sz.h) };

	return { pos,sz };
}

//____ _setValue() ____________________________________________________________

float WgMultiSlider::_setValue(Slider& slider, float value, float value2 )
{
    bool bUpdate = false;

    WG_LIMIT(value, slider.bounds[0].min, slider.bounds[0].max);

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

        // Callback

        if (m_callback)
            m_callback(slider.id, value, value2);

        // Send event

        WgEventHandler * pHandler = _eventHandler();
        if (pHandler)
            pHandler->QueueEvent(new WgEvent::SliderMoved(this,slider.id, value, value2));
        
    }
	return value;
}


//____ _setHandlePosition() ______________________________________________

WgCoordF WgMultiSlider::_setHandlePosition(Slider& slider, WgCoordF pos)
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

	// Align position to even step sizes.

	if (x >= 0)
	{
		if (slider.bounds[x].steps != 0)
		{
			float stepSize = 1.f / (slider.bounds[x].steps -1);
			pos.x += stepSize / 2;
			pos.x -= fmod(pos.x, stepSize);
		}
	}

	if (y >= 0)
	{
		if (slider.bounds[y].steps != 0)
		{
			float stepSize = 1.f / (slider.bounds[y].steps /1);
			pos.y += stepSize / 2;
			pos.y -= fmod(pos.y, stepSize);
		}
	}

	// Limit range

	WG_LIMIT(pos.x, 0.f, 1.f);
	WG_LIMIT(pos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (pos == slider.handlePos)
		return pos;

	// Set handle position and handle re-rendering.

	_requestRenderHandle(&slider);
	slider.handlePos = pos;
	_requestRenderHandle(&slider);

	// Calculate values from handle position

	float	value;
	float	value2;

	if (slider.pSetValueFunc)
	{
		SetValueVisitor v(this, &slider);
		value = slider.pSetValueFunc(v);
		value2 = NAN;
	}
	else if (slider.pSetValueFunc2D)
	{
		SetValueVisitor2D v(this, &slider);
		WgCoordF values = slider.pSetValueFunc2D(v);
		value = values.x;
		value2 = values.y;
	}
	else
	{
		WgCoordF	handleFactor = _handleFactor(slider);

		if (slider.is2D)
		{
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
			value2 = slider.bounds[1].min + (slider.bounds[1].max - slider.bounds[1].min)*handleFactor.x;
		}
		else
		{
			if (x >= 0)
				value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
			else
				value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.y;
			value2 = NAN;
		}
	}

	// Set values and possibly update any other affected sliders

	_setValue(slider, value, value2);
	return pos;
}


//____ _setHandleFactor() _____________________________________________________

WgCoordF WgMultiSlider::_setHandleFactor(Slider& slider, WgCoordF factor)
{
	return _setHandlePosition(slider, _convertFactorPos(factor, slider.origo));
}


//____ _handleFactor() ________________________________________________________

WgCoordF WgMultiSlider::_handleFactor(Slider& slider)
{
	return _convertFactorPos(slider.handlePos, slider.origo);
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

//____ SetValueVisitorBase::handleFactor() _______________________________________

float WgMultiSlider::SetValueVisitorBase::handleFactor(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || p->is2D)
		return NAN;

	WgCoordF pos = m_pWidget->_handleFactor(*p);

	return (p->origo == WG_WEST || p->origo == WG_EAST) ? pos.x : pos.y;
}

//____ SetValueVisitorBase::setHandleFactor() _______________________________________

float WgMultiSlider::SetValueVisitorBase::setHandleFactor(int sliderId, float value)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || p->is2D)
		return NAN;

	WgCoordF pos;
	if (p->origo == WG_WEST || p->origo == WG_EAST)
		pos.x = value;
	else
		pos.y = value;

	pos = m_pWidget->_setHandleFactor(*p, pos);
	return (p->origo == WG_WEST || p->origo == WG_EAST) ? pos.x : pos.y;
}

//____ SetValueVisitorBase::handleFactor2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitorBase::handleFactor2D(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || !p->is2D)
		return WgCoordF(NAN,NAN);

	return m_pWidget->_handleFactor(*p); 
}

//____ SetValueVisitorBase::setHandleFactor2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitorBase::setHandleFactor2D(int sliderId, WgCoordF value)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || !p->is2D)
		return WgCoordF(NAN,NAN);

	return  m_pWidget->_setHandleFactor(*p, value);
}


//____ SetValueVisitor::Constructor ___________________________________________

WgMultiSlider::SetValueVisitor::SetValueVisitor(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider) : SetValueVisitorBase(pWidget,pSlider) 
{
}

//____ SetValueVisitor::handleFactor() ___________________________________________

float WgMultiSlider::SetValueVisitor::handleFactor()
{
	WgCoordF pos = m_pWidget->_handleFactor(*m_pSlider);

	return (m_pSlider->origo == WG_WEST || m_pSlider->origo == WG_EAST) ? pos.x : pos.y;
}

//____ SetValueVisitor::valueBounds() ___________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetValueVisitor::valueBounds()
{
	return m_pSlider->bounds[0];
}


//____ SetValueVisitor2D::Constructor ___________________________________________

WgMultiSlider::SetValueVisitor2D::SetValueVisitor2D(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider) : SetValueVisitorBase(pWidget, pSlider)
{
}

//____ SetValueVisitor2D::handleFactor2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitor2D::handleFactor2D()
{
	return m_pWidget->_handleFactor(*m_pSlider);
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

//____ SetHandleVisitor::Constructor _______________________________________

WgMultiSlider::SetHandleVisitor::SetHandleVisitor(WgMultiSlider * pWidget, Slider * pSlider) : Visitor(pWidget,pSlider)
{
}

//____ SetHandleVisitor::value() ___________________________________________

float WgMultiSlider::SetHandleVisitor::value()
{
	return m_pSlider->value[0];
}

//____ SetHandleVisitor::valueBounds() __________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetHandleVisitor::valueBounds()
{
	return m_pSlider->bounds[0];
}


//____ SetHandleVisitor2D::Constructor _______________________________________

WgMultiSlider::SetHandleVisitor2D::SetHandleVisitor2D(WgMultiSlider * pWidget, Slider * pSlider) : Visitor(pWidget, pSlider)
{
}

//____ SetHandleVisitor2D::valueX() ___________________________________________

float WgMultiSlider::SetHandleVisitor2D::valueX()
{
	return m_pSlider->value[0];
}

//____ SetHandleVisitor2D::valueY() ___________________________________________

float WgMultiSlider::SetHandleVisitor2D::valueY()
{
	return m_pSlider->value[1];
}

//____ SetHandleVisitor2D::valueBoundsX() __________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetHandleVisitor2D::valueBoundsX()
{
	return m_pSlider->bounds[0];
}

//____ SetHandleVisitor2D::valueBoundsY() __________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetHandleVisitor2D::valueBoundsY()
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
