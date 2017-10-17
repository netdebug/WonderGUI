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
#ifndef WG_CHART_DOT_H
#define WG_CHART_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef WG_COLOR_DOT_H
#	include <wg_color.h>
#endif

#ifndef WG_STRING_DOT_H
#	include <wg_string.h>
#endif

#ifndef WG_SKIN_DOT_H
#	include <wg_skin.h>
#endif


#include <functional>
#include <vector>

//____ WgChart ____________________________________________________________

class WgChart : public WgWidget
{
public:
	WgChart();
	virtual ~WgChart();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgChart(); };

	void	SetPreferredChartSize(const WgSize& size);			// Preferred size of the wave area (widget minus all padding) set in POINTS, not pixels!
	WgSize	PreferredSize() const;

	void		SetCanvasPadding(WgBorders);
	WgBorders	CanvasPadding() const;

	void	SetSkin(const WgSkinPtr& pSkin);
	WgSkinPtr	Skin() { return m_pSkin;  }

	struct GridLine
	{
		float	pos;
		float	thickness;
		WgColor	color;
		WgString label;
	};

	void	ClearWaves();
	int		AddWave();

	bool	SetWaveStyle(int waveId, WgColor frontFill, WgColor backFill, float topLineThickness = 0.f, WgColor topLineColor = WgColor::black, float bottomLineThickness = 0.f, WgColor bottomLineColor = WgColor::black);

	bool	SetWaveSamples(int waveId, int firstSample, int nSamples, float * pTopBorderSamples, float * pBottomBorderSamples);
	bool	SetWaveSamples(int waveId, int firstSample, int nSamples, float * pSamples, float floor = 0.f);

	bool	SetFixedValueRange(float topValue, float bottomValue );
	void	SetDynamicValueRange();
	bool	IsValueRangeDynamic() const { return m_bDynamicValueRange;  }

	float	ValueRangeStart() { return m_topValue; }
	float	ValueRangeEnd() { return m_bottomValue; }


	bool	SetFixedSampleRange(float firstSample, float lastSample);
	void	SetDynamicSampleRange();
	bool	IsSampleRangeDynamic() const { return m_bDynamicSampleRange; }

	float	FirstSample() { return m_firstSample;  }
	float	LastSample() { return m_lastSample;  }

//	bool	SetResampleMethod();			// Nearest, Interpolate etc   We have two dimensions here...  Average, Maintain peaks...

	void	SetSampleLabelStyle(WgOrigo alignment, WgCoord offset, const WgSkinPtr& pSkin, const WgTextpropPtr& prop);
	void	SetSampleGridLines(int nLines, GridLine * pLines);

	void	SetValueLabelStyle(WgOrigo alignment, WgCoord offset, const WgSkinPtr& pSkin, const WgTextpropPtr& prop);
	void	SetValueGridLines(int nLines, GridLine * pLines);

	void	SetResizeResponder(std::function<void(WgChart * pWidget, WgSize newSize)> func);	// Called when widgets size has changed.
	void	SetSampleRangeResponder(std::function<void(WgChart * pWidget, float firstSample, float lastSample)> func);	// Called when widgets sample range has changed.
	void	SetValueRangeResponder(std::function<void(WgChart * pWidget, float topValue, float bottomValue)> func);		// Called when widgets sample range has changed.


protected:

	struct Wave
	{
		Wave() {}

		WgColor	frontFill = WgColor::darkgrey;
		WgColor	backFill = WgColor::darkgrey;
		float	topLineThickness = 1.f;
		WgColor topLineColor = WgColor::black;
		float	bottomLineThickness = 0.f;
		WgColor	bottomLineColor = WgColor::black;

		int		firstSample = 0;
		int		nSamples = 0;
		float	defaultSample = 0.f;

		float	minSample = 0.f;
		float	maxSample = 0.f;

		std::vector<float>	orgTopSamples;
		std::vector<float>	orgBottomSamples;

		std::vector<int>	resampledTop;
		std::vector<int>	resampledBottom;
	};

	struct LabelStyle
	{
		WgOrigo				alignment;
		WgCoord				offset;
		WgSkinPtr			pSkin;
		WgTextpropPtr		pTextStyle;
	};


	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onNewSize( const WgSize& size );
	void	_setScale( int scale );

	
	bool	_setWaveSamples(int waveId, int firstSample, int nSamples, float * pTopBorderSamples, float * pBottomBorderSamples, float defaultSample);
	void	_resampleAllWaves();
	void	_resampleWave( Wave * pWave );
	bool	_updateDynamics();
	WgCoord	_placeLabel(WgCoord startPoint, WgOrigo alignment, WgCoord labelOffset, WgSize labelSize ) const;

//	void	_onEnable();
//	void	_onDisable();

private:

	WgSize			m_defaultSize;
	WgMode			m_mode;
	WgSkinPtr		m_pSkin;

	WgBorders		m_pointPadding;			// Padding for the canvas in points. To allow thick lines to fully stay inside widget. Grid is allowed outside.
	WgBorders		m_pixelPadding;			// Same, but in pixels.

	std::vector<Wave>		m_waves;
	std::vector<GridLine>	m_sampleGridLines;
	std::vector<GridLine>	m_valueGridLines;

	bool		m_bDynamicValueRange;
	bool		m_bDynamicSampleRange;

	float		m_topValue;
	float		m_bottomValue;

	float		m_firstSample;
	float		m_lastSample;

	LabelStyle	m_sampleLabelStyle;
	LabelStyle	m_valueLabelStyle;

	std::function<void(WgChart * pWidget, WgSize newSize)>	m_resizeResponder;
	std::function<void(WgChart * pWidget, float firstSample, float lastSample)> m_sampleRangeResponder;
	std::function<void(WgChart * pWidget, float topValue, float bottomValue)> m_valueRangeResponder;
};


#endif //WG_CHART_DOT_H
