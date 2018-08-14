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

//____ SetParamArray() ________________________________________________________

void WgMultiSlider::SetParamArray(Param * pArray, int size, std::function<void(int paramIdx)> paramModifiedCallback)
{
	m_pParams = pArray;
	m_nParams = size;
	m_paramModifiedCallback = paramModifiedCallback;
}

//____ SetDefaults() __________________________________________________________

void WgMultiSlider::SetDefaults(const WgSkinPtr& pSliderBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders markExtension)
{
	m_pDefaultBgSkin = pSliderBgSkin;
	m_pDefaultHandleSkin = pHandleSkin;
	m_defaultHandleHotspot = handleHotspot;
	m_defaultMarkExtension = markExtension;
}

//____ AddSlider() ____________________________________________________________

int WgMultiSlider::AddSlider(int paramId, WgDirection dir, SetGeoFunc pSetGeoFunc, SetHandlePosFunc pSetHandlePosFunc,  SetValueFunc pSetValueFunc,
							const WgSkinPtr& pBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders markExtension)
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

	s.origo = origo;
	s.iParam = _paramIdToIndex(paramId);
	s.iSecondaryParam = -1;
	s.markExtension = markExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;
	s.pSetHandlePosFunc = pSetHandlePosFunc;
	s.pSetValueFunc = pSetValueFunc;

	s.pSetHandlePosFunc2D = nullptr;
	s.pSetValueFunc2D = nullptr;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.handleState = WG_STATE_NORMAL;

	_updateHandlePos(s);
	_updateGeo(s);

	return m_sliders.size() - 1;
}

//____ AddSlider2D() __________________________________________________________

int WgMultiSlider::AddSlider2D(int XparamId, int YparamId, WgOrigo origo, SetGeoFunc pSetGeoFunc, SetHandlePosFunc2D pSetHandlePosFunc, SetValueFunc2D pSetValueFunc,
							const WgSkinPtr& pBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot, WgBorders markExtension)
{
	if (origo != WG_NORTHWEST && origo != WG_NORTHEAST && origo != WG_SOUTHEAST && origo != WG_SOUTHWEST)
		return -1;

	m_sliders.emplace_back();
	Slider& s = m_sliders.back();

	s.origo = origo;
	s.iParam = _paramIdToIndex(XparamId);
	s.iSecondaryParam = _paramIdToIndex(YparamId);
	s.markExtension = markExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;

	s.pSetHandlePosFunc = nullptr;
	s.pSetValueFunc = nullptr;

	s.pSetHandlePosFunc2D = pSetHandlePosFunc;
	s.pSetValueFunc2D = pSetValueFunc;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.handleState = WG_STATE_NORMAL;

	_updateHandlePos(s);
	_updateGeo(s);

	return m_sliders.size() - 1;
}

//____ ParamsModified() ________________________________________________________

void WgMultiSlider::ParamsModified()
{
	_refreshSliders();
}

//____ ParamModified() ________________________________________________________

void WgMultiSlider::ParamModified(int paramId)
{
	_refreshSliders();
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

	Slider * pSlightlyMarked = nullptr;

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
				if (pOfsOutput)
					* pOfsOutput = ofs - handleGeo.Pos();
				return &slider;
			}

			WgBorders markExtension = slider.markExtension.IsEmpty() ? m_defaultMarkExtension : slider.markExtension;

			if (!markExtension.IsEmpty())
			{
				WgRect extendedGeo = handleGeo + markExtension.Scale(m_scale);

				if (extendedGeo.Contains(ofs))
				{
					if (pOfsOutput)
						* pOfsOutput = ofs - handleGeo.Pos();
					pSlightlyMarked = &slider;
				}
			}
		}
	}
	return pSlightlyMarked;
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

		_requestRenderHandle(pSlider);
	}
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

	for (auto& slider : m_sliders)
	{
		WgRect sliderGeo = _sliderGeo(slider, _canvas);

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

	Param * pParamX = nullptr;
	Param * pParamY = nullptr;

	if (slider.iSecondaryParam < 0)
	{
		Param& param = m_pParams[slider.iParam];

		float value;
			
		if (slider.pSetHandlePosFunc)
			value = slider.pSetHandlePosFunc(param);
		else
			value = (param.value - param.min) / (param.max - param.min);

		switch (slider.origo)
		{
		case WG_WEST:
			handlePos.x = value;
			pParamX = &param;
			break;
		case WG_EAST:
			handlePos.x = 1.f -value;
			pParamX = &param;
			break;
		case WG_NORTH:
			handlePos.y = value;
			pParamY = &param;
			break;
		case WG_SOUTH:
			handlePos.y = 1.f - value;
			pParamY = &param;
			break;
		}
	}
	else
	{
		pParamX = &m_pParams[slider.iParam];
		pParamY = &m_pParams[slider.iSecondaryParam];

		WgCoordF values;
		
		if(slider.pSetHandlePosFunc2D)
			values = slider.pSetHandlePosFunc2D(*pParamX, *pParamY);
		else
		{
			values.x = (pParamX->value - pParamX->min) / (pParamX->max - pParamX->min);
			values.y = (pParamY->value - pParamY->min) / (pParamY->max - pParamY->min);
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
		}
	}

	// Align position to even step sizes.

	if (pParamX)
	{
		if (pParamX->steps != 0)
		{
			float stepSize = 1.f / (pParamX->steps - 1);
			handlePos.x += stepSize / 2;
			handlePos.x -= fmod(handlePos.x, stepSize);
		}
	}

	if (pParamY)
	{
		if (pParamY->steps != 0)
		{
			float stepSize = 1.f / (pParamY->steps / 1);
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

			if (slider.pHandleSkin || m_pDefaultBgSkin)
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
	return { _canvas.x + (int)(slider.geo.x * _canvas.w), _canvas.y + (int)(slider.geo.y * _canvas.h), (int)(_canvas.w * slider.geo.w), (int)(_canvas.h * slider.geo.h) };
}

//____ _sliderSkinGeo() _______________________________________________________

WgRect  WgMultiSlider::_sliderSkinGeo(Slider& slider, const WgRect& sliderGeo)
{
	WgSkinPtr pSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;

	WgRect bgGeo = sliderGeo;
	bgGeo -= pSkin->ContentOfs(WG_STATE_NORMAL, m_scale);
	bgGeo += pSkin->ContentPadding(m_scale);

	WgSize min = pSkin->MinSize(m_scale);

	return { bgGeo.Pos(),std::max(bgGeo.w,min.w), std::max(bgGeo.h,min.h) };
}

//____ _sliderHandleGeo() _____________________________________________________

WgRect  WgMultiSlider::_sliderHandleGeo(Slider& slider, const WgRect& sliderGeo)
{
	WgSkinPtr pSkin = slider.pHandleSkin ? slider.pBgSkin : m_pDefaultHandleSkin;

	WgSize sz = pSkin->MinSize(m_scale);
	WgCoordF handleHotspot = slider.handleHotspot.x == -1.f ? m_defaultHandleHotspot : slider.handleHotspot;


	WgCoord pos = { (int)(slider.handlePos.x * sliderGeo.w), (int)(slider.handlePos.y * sliderGeo.h) };
	pos += sliderGeo.Pos();
	pos -= { (int)(handleHotspot.x * sz.w), (int)(handleHotspot.y * sz.h) };

	return { pos,sz };
}

//____ _setValue() ____________________________________________________________

float WgMultiSlider::_setValue(Param& param, float value, Slider * pBySlider )
{
	WG_LIMIT(value, param.min, param.max);

	param.value = value;

	// Callback

	if (m_paramModifiedCallback)
		m_paramModifiedCallback(param.id);

	// Send event

	WgEventHandler * pHandler = _eventHandler();
	if (pHandler)
		pHandler->QueueEvent(new WgEvent::ParamChanged(this,&param - m_pParams, param.id));

	// Update slider positions in case some (other) slider is affected

	for (auto& slider : m_sliders)
	{
		if( &slider != pBySlider )
			_updateHandlePos(slider);
	}

	// Update geo in case some sliders geo is affected

	_refreshSliderGeo();

	return value;
}




//____ _setHandlePosition() ______________________________________________

void WgMultiSlider::_setHandlePosition(Slider& slider, WgCoordF pos)
{
	// Get parameters controlled by X and Y axies.

	Param * pParamX = nullptr;
	Param * pParamY = nullptr;

	switch (slider.origo)
	{
	case WG_WEST:
	case WG_EAST:
		pParamX = &m_pParams[slider.iParam];
		break;

	case WG_NORTH:
	case WG_SOUTH:
		pParamY = &m_pParams[slider.iParam];
		break;

	case WG_NORTHWEST:
	case WG_NORTHEAST:
	case WG_SOUTHEAST:
	case WG_SOUTHWEST:
		pParamX = &m_pParams[slider.iParam];
		pParamY = &m_pParams[slider.iSecondaryParam];
		break;
	}

	// Align position to even step sizes.

	if (pParamX)
	{
		if (pParamX->steps != 0)
		{
			float stepSize = 1.f / (pParamX->steps-1);
			pos.x += stepSize / 2;
			pos.x -= fmod(pos.x, stepSize);
		}
	}

	if (pParamY)
	{
		if (pParamY->steps != 0)
		{
			float stepSize = 1.f / (pParamY->steps/1);
			pos.y += stepSize / 2;
			pos.y -= fmod(pos.y, stepSize);
		}
	}

	// Limit range

	WG_LIMIT(pos.x, 0.f, 1.f);
	WG_LIMIT(pos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (pos == slider.handlePos)
		return;

	// Set handle position and handle re-rendering.

	_requestRenderHandle(&slider);
	slider.handlePos = pos;
	_requestRenderHandle(&slider);

	// Calculate values from handle position

	WgCoordF	paramValues;


	if (slider.pSetValueFunc)
	{
		SetValueVisitor v(this, &slider);
		
		if(pParamX)
			paramValues.x = slider.pSetValueFunc(v);
		else
			paramValues.y = slider.pSetValueFunc(v);
	}
	else if (slider.pSetValueFunc2D)
	{
		SetValueVisitor2D v(this, &slider);
		paramValues = slider.pSetValueFunc2D(v);
	}
	else
	{
		WgCoordF	handleValues = _handleValues(slider);

		if (pParamX)
			paramValues.x = pParamX->min + (pParamX->max - pParamX->min)*handleValues.x;

		if (pParamY)
			paramValues.y = pParamY->min + (pParamY->max - pParamY->min)*handleValues.y;
	}

	// Set values and possibly update any other affected sliders

	if(pParamX)
		_setValue(*pParamX, paramValues.x, &slider);
	
	if(pParamY)
		_setValue(*pParamY, paramValues.y, &slider);
}

//____ _handleValues() ___________________________________________________

WgCoordF WgMultiSlider::_handleValues(Slider& slider)
{
	WgCoordF	pos = slider.handlePos;

	switch (slider.origo)
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

//____ _paramIdToIndex() ______________________________________________________

int WgMultiSlider::_paramIdToIndex(int paramId)
{
	for (int i = 0; i < m_nParams; i++)
		if (m_pParams[i].id == paramId)
			return i;

	return -1;
}



//____ Visitor::paramBegin() ___________________________________________________

const WgMultiSlider::Param * WgMultiSlider::Visitor::paramBegin()
{
	return m_pWidget->m_pParams;
}

//____ Visitor::paramEnd() ___________________________________________________

const WgMultiSlider::Param * WgMultiSlider::Visitor::paramEnd()
{
	return m_pWidget->m_pParams + m_pWidget->m_nParams;
}

//____ Visitor::param() ___________________________________________________

const WgMultiSlider::Param * WgMultiSlider::Visitor::param(int id)
{
	int idx = m_pWidget->_paramIdToIndex(id);

	if (idx < 0 )
		return nullptr;

	return &m_pWidget->m_pParams[idx];
}

//____ SetValueVisitor::Constructor ___________________________________________________

WgMultiSlider::SetValueVisitor::SetValueVisitor(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider)
{
	m_pWidget = pWidget;
	m_pSlider = pSlider;
}

//____ SetValueVisitor::param() _______________________________________________

const WgMultiSlider::Param * WgMultiSlider::SetValueVisitor::param()
{
	return &m_pWidget->m_pParams[m_pSlider->iParam];
}

//____ SetValueVisitor::handleValue() _________________________________________

float WgMultiSlider::SetValueVisitor::handleValue()
{
	WgCoordF values = m_pWidget->_handleValues(*m_pSlider);

	if( m_pSlider->origo == WG_NORTH || m_pSlider->origo == WG_SOUTH )
		return values.y;

	return values.x;
}

//____ SetValueVisitor::setParamValue() __________________________________________

float WgMultiSlider::SetValueVisitor::setParamValue(const Param * pParam, float value)
{
	Param& param = const_cast<Param&>(*pParam);
	return m_pWidget->_setValue(param, value,m_pSlider);
}

//____ SetValueVisitor2D::Constructor ___________________________________________________

WgMultiSlider::SetValueVisitor2D::SetValueVisitor2D(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider)
{
	m_pWidget = pWidget;
	m_pSlider = pSlider;
}

//____ SetValueVisitor2D::paramX() ___________________________________________________

const WgMultiSlider::Param * WgMultiSlider::SetValueVisitor2D::paramX()
{
	return &m_pWidget->m_pParams[m_pSlider->iParam];
}


//____ SetValueVisitor2D::paramY() __________________________________________________

const WgMultiSlider::Param * WgMultiSlider::SetValueVisitor2D::paramY()
{
	return &m_pWidget->m_pParams[m_pSlider->iSecondaryParam];
}


//____ SetValueVisitor2D::handleValue() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitor2D::handleValue()
{
	return m_pWidget->_handleValues(*m_pSlider);
}

//____ SetValueVisitor2D::setParamValue() __________________________________________

float WgMultiSlider::SetValueVisitor2D::setParamValue(const Param * pParam, float value)
{
	Param& param = const_cast<Param&>(*pParam);
	return m_pWidget->_setValue(param, value,m_pSlider);
}

//____ SetGeoVisitor::Constructor ___________________________________________________

WgMultiSlider::SetGeoVisitor::SetGeoVisitor(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider) 
{
	m_pWidget = pWidget;
	m_pSlider = pSlider;
}

//____ SetGeoVisitor::slidersBegin() ___________________________________________________

int WgMultiSlider::SetGeoVisitor::slidersBegin()
{
	return 0;
}

//____ SetGeoVisitor::slidersEnd() ___________________________________________________

int WgMultiSlider::SetGeoVisitor::slidersEnd()
{ 
	return m_pWidget->m_sliders.size();
}

//____ SetGeoVisitor::slider() ___________________________________________________

int WgMultiSlider::SetGeoVisitor::slider()
{
	return m_pSlider - &m_pWidget->m_sliders.front();
}


//____ SetGeoVisitor::handleGeoPos() ___________________________________________________

WgCoordF WgMultiSlider::SetGeoVisitor::handlePos(int slider)
{
	return m_pWidget->m_sliders[slider].handlePos;
}

//____ SetGeoVisitor::geo() ___________________________________________________

WgRectF WgMultiSlider::SetGeoVisitor::geo(int iSlider)
{
	Slider& slider = m_pWidget->m_sliders[iSlider];
	assert(slider.geoState != 1);

	if (slider.geoState == 0)
		m_pWidget->_updateGeo(slider);

	return slider.geo;
}

