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
#ifndef WG_MULTISLIDERS_DOT_H
#define WG_MULTISLIDERS_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif






//____ WgMultiSlider ____________________________________________________________

class WgMultiSlider : public WgWidget
{
protected:
	struct Slider;

public:
	WgMultiSlider();
	virtual ~WgMultiSlider();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgMultiSlider(); };

	WgSize	PreferredPixelSize() const;

	enum PressMode
	{
		NoMovement,
		SetValue,
//		StepValue,
		MultiSetValue
//		MultiStepValue
	};

	struct Bounds
	{
		float	min;
		float	max;
		int		steps;
	};
	
	class Visitor
	{
	public:
		Visitor(WgMultiSlider * pWidget, Slider * pSlider) : m_pWidget(pWidget), m_pSlider(pSlider) {}
	protected:
		WgMultiSlider * m_pWidget;
		Slider *		m_pSlider;
	};

	class SetValueVisitorBase : public Visitor
	{
	public:
		SetValueVisitorBase(WgMultiSlider * pWidget, Slider * pSlider);

		float		value(int sliderId);
		float		setValue(int sliderId, float value);

		WgCoordF	value2D(int sliderId);
		WgCoordF	setValue2D(int sliderId, WgCoordF value);
	};

	class SetValueVisitor : public SetValueVisitorBase
	{
	public:
		using SetValueVisitorBase::value;
		SetValueVisitor(WgMultiSlider * pWidget, Slider * pSlider, float value);

		float		value();
		Bounds		valueBounds();

		float		m_value;
	};

	class SetValueVisitor2D : public SetValueVisitorBase
	{
	public:
		using SetValueVisitorBase::value2D;
		SetValueVisitor2D(WgMultiSlider * pWidget, Slider * pSlider, WgCoordF value );

		WgCoordF	value2D();
		Bounds		valueBoundsX();
		Bounds		valueBoundsY();

		WgCoordF	m_value;
	};




	class SetGeoVisitor : public Visitor
	{
	public:
		SetGeoVisitor(WgMultiSlider * pWidget, Slider * pSlider);

		WgCoordF	handlePos(int sliderId);
		WgRectF		geo(int sliderId);
	}; 

	typedef std::function<float(SetValueVisitor& visitor)>			SetValueFunc;
	typedef std::function<WgCoordF(SetValueVisitor2D& visitor)>		SetValueFunc2D;

	typedef std::function<WgRectF(SetGeoVisitor& visitor)>			SetGeoFunc;
	

	void	SetDefaults(const WgSkinPtr& pBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot = { 0.5f,0.5f },
						WgBorders handleMarkExtension = WgBorders(0), WgBorders sliderMarkExtension = WgBorders(0) );

	void	SetCallback(const std::function<void(int sliderId, float value, float value2)>& callback);

	void	SetPassive(bool bPassive);							// No slider repositioning on its own, relies callback/event-listener to call SetSliderValue().
	bool	IsPassive() const { return m_bPassive; }

	void	SetDeltaDrag(bool bDelta);							// Use relative movements, instead of the new position, for slider placement.
	bool	IsDeltaDrag() const { return m_bDeltaDrag;  }

	void	SetGhostHandle(bool bGhost);						// Handle is not pressable, only background. Can be useful in some press-modes.
	bool	IsGhostHandle() const { return m_bGhostHandle; }

	void	SetSkin(const WgSkinPtr& pSkin);

	void	SetPressMode(WgMultiSlider::PressMode mode);
	PressMode GetPressMode() const { return m_pressMode; }

	void	SetFinetune( int stepSize, float stepIncrement = 0.f, WgModifierKeys modifier = WG_MODKEY_CTRL);


	int		AddSlider(	int id, WgDirection dir, SetGeoFunc pSetGeoFunc, float startValue = 0.f, float minValue = 0.f, float maxValue = 1.f, int steps = 0,
						SetValueFunc pSetValueFunc = nullptr, const WgSkinPtr& pBgSkin = nullptr, 
						const WgSkinPtr& pHandleSkin = nullptr, WgCoordF handleHotspot = { -1.f,-1.f }, 
						WgBorders handleMarkExtension = WgBorders(0), WgBorders sliderMarkExtension = WgBorders(0) );

	int		AddSlider2D( int id, WgOrigo origo, SetGeoFunc pSetGeoFunc, float startValueX = 0.f, float startValueY = 0.f, 
						float minValueX = 0.f,  float maxValueX = 1.f, int stepsX = 0, float minValueY = 0.f, float maxValueY = 1.f, int stepsY = 0,
						SetValueFunc2D pSetValueFunc = nullptr,
						const WgSkinPtr& pBgSkin = nullptr, const WgSkinPtr& pHandleSkin = nullptr, WgCoordF handleHotspot = { -1.f, -1.f }, 
						WgBorders handleMarkExtension = WgBorders(0), WgBorders sliderMarkExtension = WgBorders(0) );

    void    RemoveAllSliders();

	float	SetSliderValue(int id, float value, float value2 = NAN);

    WgCoord HandlePointPos( int sliderId );
    WgCoord HandlePixelPos( int sliderId );

	bool	MarkTest(const WgCoord& ofs) override;


protected:

	struct Slider
	{
		int				id;

		bool			is2D;

		float			value[2];
		Bounds			bounds[2];

		WgOrigo			origo;
		WgRectF			geo;				// 0.f -> 1.f, within widgets content rect
		WgCoordF		handlePos;			// 0.f -> 1.f, within geo
		WgCoordF		handleHotspot;		// 0.f -> 1.f, within its own size
		WgState			handleState;
		int				geoState;			// 0 = needs refresh, 1 = refresh in progress, 2 = refreshed.

		WgSkinPtr		pBgSkin;
		WgSkinPtr		pHandleSkin;
		WgBorders		handleMarkExtension;		// Frame surrounding handle that also marks the handle. Measured in points, not pixels.
		WgBorders		sliderMarkExtension;		// Frame surrounding slider that also marks the slider. Measured in points, not pixels.

		SetValueFunc	pSetValueFunc;
		SetValueFunc2D	pSetValueFunc2D;

		SetGeoFunc		pSetGeoFunc;
	};

	void	_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;
	void	_onCloneContent( const WgWidget * _pOrg ) override;
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip ) override;
	bool	_onAlphaTest( const WgCoord& ofs ) override;
	void	_setScale(int scale) override;

	void	_updateHandlePos(Slider& slider);									// Updates handlePos from its parameter values.
	void	_updateGeo(Slider& slider);

	WgRect	_sliderGeo(Slider& slider, const WgRect& canvas);
	WgRect	_sliderSkinGeo(Slider& slider, const WgRect& sliderGeo );
	WgRect	_sliderHandleGeo(Slider& slider, const WgRect& sliderGeo );

	Slider * _markedSliderHandle(WgCoord ofs, WgCoord * pOfsOutput = nullptr );
	void	_markSliderHandle(Slider * pSlider);
	void	_selectSliderHandle(Slider * pSlider);
	void	_requestRenderHandle(Slider * pSlider);

	Slider * _markedSlider(WgCoord ofs, WgCoord * pOfsOutput = nullptr);


	WgCoordF	_setHandlePosition(Slider& slider, WgCoordF pos);					// Set handlePos and update value(s).
	WgCoordF	_calcSendValue(Slider& slider, WgCoordF pos);						// Like _setHandlePosition(), but only calculates and sends the value (no internal update).

	void		_invokeSetValueCallback(Slider& slider, float& value, float& value2);

	float		_setValue(Slider& slider, float valueX, float valueY, bool bSendOnUpdate );			// Set value(s) and update handlePos.
	void		_sendValue(Slider& slider, float value, float value2);

	WgCoordF	_convertFactorPos(WgCoordF in, WgOrigo origo);						// Convert between sliderFactor and sliderPosition.


	void		_refreshSliders();
	void		_refreshSliderGeo();
	void		_updatePointerStyle(WgCoord pointerPos);

	Slider *	_findSlider(int sliderId);

private:


	WgSkinPtr			m_pDefaultBgSkin;
	WgSkinPtr			m_pDefaultHandleSkin;
	WgCoordF			m_defaultHandleHotspot = { 0.5f, 0.5f };
	WgBorders			m_defaultHandleMarkExtension;
	WgBorders			m_defaultSliderMarkExtension;

	std::vector<Slider>	m_sliders;

	int					m_selectedSliderHandle = -1;
	WgCoord				m_selectPressOfs;

	int					m_selectedSlider = -1;								// For press on slider in certain modes.


	bool				m_bPassive = false;
	bool				m_bGhostHandle = false;

	bool				m_bDeltaDrag = false;
	WgCoord				m_totalDrag;
	WgCoord				m_finetuneFraction;

	WgModifierKeys		m_finetuneModifier = WG_MODKEY_CTRL;
	int					m_finetuneStepSize = 5;							// 0 = increment on every pixel, otherwise points to move pointer before value incremented.
	float				m_finetuneStepIncrement = 0.f;					// 0 = increment with value of one pixels movement, otherwise by specified value.

	PressMode			m_pressMode = PressMode::NoMovement;

	std::function<void(int sliderId, float value, float value2 )>	m_callback = nullptr;

};

#endif //WG_MULTISLIDERS_DOT_H
