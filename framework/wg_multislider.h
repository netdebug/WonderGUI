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

	struct Param
	{
		int		id;
		float	value;
		float	min;
		float	max;
		int		steps;
	};


	class Visitor
	{
	public:
		const Param * paramBegin();
		const Param * paramEnd();
		const Param * param(int idx);

	protected:
		WgMultiSlider * m_pWidget;
		Slider *		m_pSlider;
	};

	class SetValueVisitor : public Visitor
	{
	public:
		SetValueVisitor(WgMultiSlider * pWidget, Slider * pSlider);

		using Visitor::param;

		const Param * param();
		float		handleValue();

		float		setParamValue(const Param * pParam, float value);
	};

	class SetValueVisitor2D : public Visitor
	{
	public:
		SetValueVisitor2D(WgMultiSlider * pWidget, Slider * pSlider);

		const Param * paramX();
		const Param * paramY();

		WgCoordF	handleValue();

		float		setParamValue(const Param * pParam, float value);
	};


	class SetGeoVisitor : public Visitor
	{
	public:
		SetGeoVisitor(WgMultiSlider * pWidget, Slider * pSlider);

		int		slidersBegin();
		int		slidersEnd();

		WgCoordF	handlePos(int slider);
		WgRectF		geo(int slider);
	};

	typedef std::function<float(const Param& param)>							SetHandlePosFunc;
	typedef std::function<WgCoordF(const Param& paramX, const Param& paramY)>	SetHandlePosFunc2D;

	typedef std::function<float(SetValueVisitor& visitor)>			SetValueFunc;
	typedef std::function<WgCoordF(SetValueVisitor2D& visitor)>		SetValueFunc2D;

	typedef std::function<WgRectF(SetGeoVisitor& visitor)>			SetGeoFunc;
	

	void	SetParamArray(Param * pArray, int size, std::function<void(int paramIdx)> paramModifiedCallback);

	void	SetDefaults(const WgSkinPtr& pBgSkin, const WgSkinPtr& pHandleSkin, WgBorders markExtension);

	int		AddSlider(	int paramIdx, WgDirection dir, SetGeoFunc pSetGeoFunc, SetHandlePosFunc pSetHandlePosFunc = nullptr, SetValueFunc pSetValueFunc = nullptr,
		const WgSkinPtr& pBgSkin = nullptr, const WgSkinPtr& pHandleSkin = nullptr, WgCoordF handleHotspot = { 0.5f,0.5f }, WgBorders markExtension = WgBorders(0));

	int		AddSlider2D(int XparamIdx, int YparamIdx, WgOrigo origo, SetGeoFunc pSetGeoFunc, SetHandlePosFunc2D pSetHandlePosFunc = nullptr, SetValueFunc2D pSetValueFunc = nullptr,
						const WgSkinPtr& pBgSkin = nullptr, const WgSkinPtr& pHandleSkin = nullptr, WgCoordF handleHotspot = { 0.5f,0.5f }, WgBorders markExtension = WgBorders(0) );

	void	ParamModified(int index);

	bool	MarkTest(const WgCoord& ofs) override;

	void	SetSkin(const WgSkinPtr& pSkin);



protected:

	struct Slider
	{
		WgOrigo			origo;
		WgRectF			geo;				// 0.f -> 1.f, within widgets content rect
		WgCoordF		handlePos;			// 0.f -> 1.f, within geo
		WgCoordF		handleHotspot;		// 0.f -> 1.f, within its own size
		WgState			handleState;
		int				iParam;					
		int				iSecondaryParam;	// -1 if not in use.
		int				geoState;			// 0 = needs refresh, 1 = refresh in progress, 2 = refreshed.

		WgSkinPtr		pBgSkin;
		WgSkinPtr		pHandleSkin;
		WgBorders		markExtension;		// Frame surrounding slider that also marks the slider. Measured in points, not pixels.
		
		SetHandlePosFunc	pSetHandlePosFunc;
		SetHandlePosFunc2D	pSetHandlePosFunc2D;

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

	Slider * _markedSlider(WgCoord ofs, WgCoord * pOfsOutput = nullptr );
	void	_markSlider(Slider * pSlider);
	void	_selectSlider(Slider * pSlider);
	void	_requestRenderHandle(Slider * pSlider);


	void	_setHandlePosition(Slider& slider, WgCoordF pos);					// Set handlePos and update affected parameters.
	float	_setValue(Param& param, float value, Slider * pBySlider);			// Set parameter value and update any affected handles.

	WgCoordF	_handleValues(Slider& slider);									// Get the values (0.f-1.f) for current handle position & origo.
	void		_refreshSliders();
	void		_refreshSliderGeo();



private:

	WgSkinPtr			m_pDefaultBgSkin;
	WgSkinPtr			m_pDefaultHandleSkin;
	WgBorders			m_defaultMarkExtension;

	std::vector<Slider>	m_sliders;

	int					m_selectedSlider = -1;
	WgCoord				m_selectPressOfs;

	Param *				m_pParams = nullptr;
	int					m_nParams = 0;
	std::function<void(int paramIdx)>	m_paramModifiedCallback = nullptr;

};

#endif //WG_MULTISLIDERS_DOT_H
