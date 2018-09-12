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

#include <wg_chart.h>
#include <wg_gfxdevice.h>
#include <wg_pen.h>
#include <wg_base.h>
#include <wg_texttool.h>
#include <wg_util.h>

static const char	c_widgetType[] = {"Chart"};

//____ Constructor ____________________________________________________________

WgChart::WgChart()
{
	m_defaultSize = WgSize(200,100);
	m_bOpaque = false;
	m_mode = WG_MODE_NORMAL;

	m_bDynamicValueRange = true;
	m_bDynamicSampleRange = true;

	m_topValue			= 1.f;
	m_bottomValue		= -1.f;

	m_firstSample		= 0.f;
	m_lastSample		= 0.f;

	m_sampleLabelStyle.alignment = WG_SOUTH;
	m_sampleLabelStyle.offset = { 0,0 };

	m_valueLabelStyle.alignment = WG_WEST;
	m_valueLabelStyle.offset = { 0,0 };

	m_waveIdCounter = 1;
}

//____ Destructor _____________________________________________________________

WgChart::~WgChart()
{
}

//____ Type() _________________________________________________________________

const char * WgChart::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgChart::GetClass()
{
	return c_widgetType;
}

//____ SetPreferredChartSize() _______________________________________________________

void WgChart::SetPreferredChartSize( const WgSize& size )
{
	if( size != m_defaultSize )
	{
		m_defaultSize = size;
		_requestResize();
	}
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgChart::PreferredPixelSize() const
{
	WgSize sz = (m_defaultSize * m_scale) / WG_SCALE_BASE;

	sz += m_pixelPadding;
	if (m_pSkin)
		sz = m_pSkin->SizeForContent(sz,m_scale);

	return sz;
}

//____ SetSkin() ______________________________________________________________

void WgChart::SetSkin(const WgSkinPtr& pSkin)
{
	m_pSkin = pSkin;
	_requestResize();		// Just in case
	_requestRender();
}

//____ SetCanvasPadding() _____________________________________________________

void WgChart::SetCanvasPadding(WgBorders padding)
{
	m_pointPadding = padding;
	WgBorders pixelPadding( (padding.left * m_scale) >> WG_SCALE_BINALS, (padding.top * m_scale) >> WG_SCALE_BINALS, 
							(padding.right * m_scale) >> WG_SCALE_BINALS, (padding.bottom * m_scale) >> WG_SCALE_BINALS );

	if (pixelPadding != m_pixelPadding)
	{
		m_pixelPadding = pixelPadding;
		_resampleAllWaves();
		_requestRender();
	}
}

//____ CanvasPadding() ________________________________________________________

WgBorders WgChart::CanvasPadding() const
{
	return m_pointPadding;
}

//____ ClearWaves() ___________________________________________________________

void WgChart::ClearWaves()
{
	m_waves.clear();
	_requestRender();
}

//____ AddWave() ______________________________________________________________

int WgChart::AddWave()
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();
	w.id = m_waveIdCounter++;
	return w.id;
}

//____ IsWaveDisplayed() ______________________________________________________

bool WgChart::IsWaveDisplayed(int waveId) const
{
	const Wave * p = _getWave(waveId);
	if (!p)
		return false;

	return !p->bHidden;
}

//____ HideWave() _____________________________________________________________

bool WgChart::HideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == false)
	{
		p->bHidden = true;
		_requestRender();
	}
	return true;
}

//____ UnhideWave() ___________________________________________________________

bool WgChart::UnhideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == true)
	{
		p->bHidden = false;
		_requestRender();
	}
	return true;
}

//____ HideAllWaves() _________________________________________________________

void WgChart::HideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (!wave.bHidden)
		{
			wave.bHidden = true;
			_requestRender();
		}
	}
}

//____ UnhideAllWaves() _______________________________________________________

void WgChart::UnhideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (wave.bHidden)
		{
			wave.bHidden = false;
			_requestRender();
		}
	}
}



//____ SetWaveStyle() _________________________________________________________

bool WgChart::SetWaveStyle(int waveId, WgColor frontFill, WgColor backFill, float topLineThickness, WgColor topLineColor, float bottomLineThickness, WgColor bottomLineColor)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;
	  
	p->frontFill = frontFill;
	p->backFill = backFill;

	p->topLineThickness = topLineThickness;
	p->topLineColor = topLineColor;

	p->bottomLineThickness = bottomLineThickness;
	p->bottomLineColor = bottomLineColor;

	_requestRender();
	return true;
}

//____ SetWaveSamples() _______________________________________________________

bool WgChart::SetWaveSamples(int waveId, int firstSample, int nSamples, float * pTopBorderSamples, float * pBottomBorderSamples)
{
	return _setWaveSamples(waveId, firstSample, nSamples, pTopBorderSamples, pBottomBorderSamples, 0.f);
}

bool WgChart::SetWaveSamples(int waveId, int firstSample, int nSamples, float * pSamples, float floor)
{
	return _setWaveSamples(waveId, firstSample, nSamples, pSamples, nullptr, floor);
}


bool WgChart::_setWaveSamples(int waveId, int firstSample, int nSamples, float * pTopBorderSamples, float * pBottomBorderSamples, float defaultSample )
{
	Wave * pWave = _getWave(waveId);
	if (!pWave)
		return false;

	pWave->firstSample = firstSample;
	pWave->nSamples = nSamples;
	pWave->defaultSample = defaultSample;

	float max = -std::numeric_limits<float>::max();
    float min =  std::numeric_limits<float>::max();

	if (pTopBorderSamples)
	{
		pWave->orgTopSamples.resize(nSamples);
		for (int i = 0; i < nSamples; i++)
		{
			float sample = pTopBorderSamples[i];
			pWave->orgTopSamples[i] = sample;

			if (max < sample) max = sample;
			if (min > sample) min = sample;
		}
	}
	else
	{
		pWave->orgTopSamples.clear();
		max = defaultSample;
		min = defaultSample;
	}
	if (pBottomBorderSamples)
	{
		pWave->orgBottomSamples.resize(nSamples);
		for (int i = 0; i < nSamples; i++)
		{
			float sample = pBottomBorderSamples[i];
			pWave->orgBottomSamples[i] = sample;

			if (max < sample) max = sample;
			if (min > sample) min = sample;
		}
	}
	else
	{
		pWave->orgBottomSamples.clear();
		if (max < defaultSample) max = defaultSample;
		if (min > defaultSample) min = defaultSample;
	}

	pWave->maxSample = max;
	pWave->minSample = min;

	bool bResampleAll = _updateDynamics();
	if ( bResampleAll )
		_resampleAllWaves();
	else
		_resampleWave(pWave);

	_requestRender();			//TODO: Optimize, only render rectangle with modifications.
	return true;
}

//____ SetFixedValueRange() ________________________________________________________

bool WgChart::SetFixedValueRange(float topValue, float bottomValue)
{
	if (topValue == bottomValue)
		return false;

	if (m_bDynamicValueRange || topValue != m_topValue || bottomValue != m_bottomValue)
	{
		m_bDynamicValueRange = false;
		m_topValue = topValue;
		m_bottomValue = bottomValue;
		_resampleAllWaves();
		if (m_valueRangeResponder)
			m_valueRangeResponder(this, topValue, bottomValue);
		_requestRender();
	}

	return true;
}

//____ SetDynamicValueRange() ________________________________________________________

void WgChart::SetDynamicValueRange()
{
	if (!m_bDynamicValueRange)
	{
		m_bDynamicValueRange = true;
		_updateDynamics();
		_resampleAllWaves();
		_requestRender();
//		if (m_valueRangeResponder)
//			m_valueRangeResponder(this, m_topValue, m_bottomValue);
	}
}

//____ SetFixedSampleRange() __________________________________________________

bool WgChart::SetFixedSampleRange(float firstSample, float lastSample)
{
	if (firstSample == lastSample)
		return false;

	if (m_bDynamicSampleRange || firstSample != m_firstSample || lastSample != m_lastSample)
	{

		m_bDynamicSampleRange = false;
		m_firstSample = firstSample;
		m_lastSample = lastSample;
		_resampleAllWaves();				//TODO: Optimize! Can skip if fractional start offset and width is same.
		if (m_sampleRangeResponder)
			m_sampleRangeResponder(this, firstSample, lastSample);
		_requestRender();
	}
	return true;
}

//____ NativeSampleRange() __________________________________________________

int WgChart::NativeSampleRange() const
{
    int width = PixelSize().w;
    
    if( m_pSkin )
        width -= m_pSkin->ContentPadding(m_scale).w;

    width -= m_pixelPadding.Width();
    
    return width > 0 ? width + 1 : 0;
}


//____ SetDynamicSampleRange() ________________________________________________________

void WgChart::SetDynamicSampleRange()
{
	if (!m_bDynamicSampleRange)
	{
		m_bDynamicSampleRange = true;
		_updateDynamics();
		_resampleAllWaves();
		_requestRender();

//		if (m_sampleRangeResponder)
//			m_sampleRangeResponder(this, m_firstSample, m_lastValue);
	}
}

//____ SetSampleLabelStyle() __________________________________________________

void WgChart::SetSampleLabelStyle(WgOrigo alignment, WgCoord offset, const WgSkinPtr& pSkin, const WgTextpropPtr& prop)
{
	m_sampleLabelStyle.alignment = alignment;
	m_sampleLabelStyle.offset = offset;
	m_sampleLabelStyle.pSkin = pSkin;
	m_sampleLabelStyle.pTextStyle = prop;
	_requestRender();
}

//____ SetSampleGridLines() ____________________________________________________

void WgChart::SetSampleGridLines(int nLines, GridLine * pLines)
{
	if (nLines == 0 || pLines == nullptr)
	{
		m_sampleGridLines.clear();
	}
	else
	{
		m_sampleGridLines.resize(nLines);

		for (int i = 0; i < nLines; i++)
			m_sampleGridLines[i] = pLines[i];
	}
	_requestRender();

}

//____ SetValueLabelStyle() ___________________________________________________

void WgChart::SetValueLabelStyle(WgOrigo alignment, WgCoord offset, const WgSkinPtr& pSkin, const WgTextpropPtr& prop)
{
	m_valueLabelStyle.alignment = alignment;
	m_valueLabelStyle.offset = offset;
	m_valueLabelStyle.pSkin = pSkin;
	m_valueLabelStyle.pTextStyle = prop;
	_requestRender();
}

//____ SetValueGridLines() ____________________________________________________

void WgChart::SetValueGridLines(int nLines, GridLine * pLines)
{
	if (nLines == 0 || pLines == nullptr)
	{
		m_valueGridLines.clear();
	}
	else
	{
		m_valueGridLines.resize(nLines);

		for (int i = 0; i < nLines; i++)
			m_valueGridLines[i] = pLines[i];
	}
	_requestRender();
}

//____ SetResizeResponder() _____________________________________________________

void WgChart::SetResizeResponder(std::function<void(WgChart * pWidget, WgSize newSize)> func)
{
	m_resizeResponder = func;
	if (func)
		func(this, PixelSize());
}

//____ SetSampleRangeResponder() _____________________________________________________

void WgChart::SetSampleRangeResponder(std::function<void(WgChart * pWidget, float firstSample, float lastSample)> func)
{
	m_sampleRangeResponder = func;
	if (func)
		func(this, m_firstSample, m_lastSample);
}

//____ SetValueRangeResponder() _____________________________________________________

void WgChart::SetValueRangeResponder(std::function<void(WgChart * pWidget, float topValue, float bottomValue)> func)
{
	m_valueRangeResponder = func;
	if (func)
		func(this, m_topValue, m_bottomValue);
}

//____ _onCloneContent() _______________________________________________________

void WgChart::_onCloneContent( const WgWidget * _pOrg )
{
	WgChart * pOrg = (WgChart*) _pOrg;

	m_defaultSize = pOrg->m_defaultSize;
}

//____ _onRender() _____________________________________________________________

void WgChart::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	WgWidget::_onRender(pDevice, _canvas, _window, _clip);

	WgRect canvas = m_pSkin ? m_pSkin->ContentRect(_canvas, WG_STATE_NORMAL, m_scale) : _canvas;

	// Preparations for both grid and wave drawing

	WgRect waveCanvas = canvas - m_pixelPadding;
	float sampleScale = waveCanvas.w / (m_lastSample - m_firstSample);

	// Draw sample grid lines

	if (!m_sampleGridLines.empty())
	{
		for (auto& line : m_sampleGridLines)
		{
			int xOfs = waveCanvas.x + (int) ((line.pos - m_firstSample) * sampleScale);
			pDevice->ClipDrawLine(_clip, { xOfs, canvas.y }, WG_DOWN, canvas.h, line.color, line.thickness * m_scale / WG_SCALE_BASE);

			if (!line.label.IsEmpty())
			{
				WgPen	pen(pDevice, _canvas, _clip);
				WgTextAttr attr;

				WgTextTool::AddPropAttributes(attr, WgBase::GetDefaultTextprop());
				WgTextTool::AddPropAttributes(attr, m_sampleLabelStyle.pTextStyle);
				attr.size = attr.size * m_scale >> WG_SCALE_BINALS;
				pen.SetAttributes(attr);


				WgSize labelSize;
				labelSize.w = WgTextTool::lineWidth(nullptr, attr, WG_MODE_NORMAL, line.label.Chars());
				labelSize.h = pen.GetLineHeight();

				WgCoord textOfs;
				if (m_sampleLabelStyle.pSkin)
				{
					labelSize = m_sampleLabelStyle.pSkin->SizeForContent(labelSize, m_scale);
					textOfs = m_sampleLabelStyle.pSkin->ContentRect(labelSize, WG_STATE_NORMAL, m_scale).pos();
				}

				WgCoord labelPos = _placeLabel({ xOfs,canvas.y+canvas.h }, m_sampleLabelStyle.alignment, m_sampleLabelStyle.offset, labelSize);

				if (m_sampleLabelStyle.pSkin)
					m_sampleLabelStyle.pSkin->Render(pDevice, WG_STATE_NORMAL, { labelPos,labelSize }, _clip, m_scale);

				pen.SetPos(labelPos + textOfs);
				pDevice->PrintLine(pen, attr, line.label.Chars());
			}
		}
	}

	// Draw value grid lines

	if (!m_valueGridLines.empty())
	{
		float top = WgMin(m_topValue, m_bottomValue);
		float bottom = WgMax(m_topValue, m_bottomValue);

		float mul = waveCanvas.h / (m_bottomValue - m_topValue);
		int	  startOfs = mul > 0 ? waveCanvas.y : waveCanvas.y + waveCanvas.h;

		for (auto& line : m_valueGridLines)
		{
			int yOfs = startOfs + (int)((line.pos - top) * mul + 0.5f);
			pDevice->ClipDrawLine(_clip, { canvas.x, yOfs }, WG_RIGHT, canvas.w, line.color, line.thickness * m_scale / WG_SCALE_BASE );

			if (!line.label.IsEmpty())
			{
				WgPen	pen(pDevice, _canvas, _clip);
				WgTextAttr attr;

				WgTextTool::AddPropAttributes(attr, WgBase::GetDefaultTextprop());
				WgTextTool::AddPropAttributes(attr, m_valueLabelStyle.pTextStyle);
				attr.size = attr.size * m_scale >> WG_SCALE_BINALS;
				pen.SetAttributes(attr);


				WgSize labelSize;
				labelSize.w = WgTextTool::lineWidth(nullptr, attr, WG_MODE_NORMAL, line.label.Chars());
				labelSize.h = pen.GetLineHeight();

				WgCoord textOfs;
				if (m_valueLabelStyle.pSkin)
				{
					labelSize = m_valueLabelStyle.pSkin->SizeForContent(labelSize, m_scale);
					textOfs = m_valueLabelStyle.pSkin->ContentRect(labelSize, WG_STATE_NORMAL, m_scale).pos();
				}

				WgCoord labelPos = _placeLabel({ canvas.x, yOfs }, m_valueLabelStyle.alignment, m_valueLabelStyle.offset, labelSize);

				if (m_valueLabelStyle.pSkin)
					m_valueLabelStyle.pSkin->Render(pDevice, WG_STATE_NORMAL, { labelPos,labelSize }, _clip, m_scale );

				pen.SetPos(labelPos + textOfs);
				pDevice->PrintLine(pen, attr, line.label.Chars());
			}
		}
	}


	// Render waves

	WgRect waveClip(_clip, { waveCanvas.x, canvas.y, waveCanvas.w, canvas.h });		// Samples stay within padding, values may stretch outside.

	for (auto& wave : m_waves)
	{
		if (wave.bHidden)
			continue;

		int xOfs = (int) ((wave.firstSample - m_firstSample) * sampleScale);

		WgWaveLine top, bottom;

		top.color = wave.topLineColor;
		top.thickness = wave.topLineThickness*m_scale/WG_SCALE_BASE;
		top.length = (int) wave.resampledTop.size();
		top.pWave = wave.resampledTop.data();
		top.hold = wave.resampledDefault;

		bottom.color = wave.bottomLineColor;
		bottom.thickness = wave.bottomLineThickness*m_scale/WG_SCALE_BASE;
		bottom.length = (int)wave.resampledBottom.size();
		bottom.pWave = wave.resampledBottom.data();
		bottom.hold = wave.resampledDefault;

		int length = WgMax(top.length, bottom.length)-1;

		pDevice->ClipDrawHorrWave(waveClip, WgCoord(waveCanvas.x + xOfs, waveCanvas.y), length, top, bottom, wave.frontFill, wave.backFill);
	}

}

//____ _placeLabel() __________________________________________________________

WgCoord	WgChart::_placeLabel(WgCoord startPoint, WgOrigo alignment, WgCoord labelOffset, WgSize labelSize) const
{
	return startPoint + (labelOffset*m_scale/WG_SCALE_BASE) -WgCoord(labelSize.w,labelSize.h) + WgUtil::OrigoToOfs(alignment, labelSize);
}

//____ _getWave() _____________________________________________________________

WgChart::Wave * WgChart::_getWave(int waveId)
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}

const WgChart::Wave * WgChart::_getWave(int waveId) const
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}



//____ _onAlphaTest() ___________________________________________________________

bool WgChart::_onAlphaTest( const WgCoord& ofs )
{
	//TODO: Implement?

	return true;
}


//____ _onNewSize() ___________________________________________________________

void WgChart::_onNewSize(const WgSize& size)
{
	if (m_resizeResponder)
		m_resizeResponder(this, size);

	_resampleAllWaves();
	WgWidget::_onNewSize(size);
}

//____ _setScale() ____________________________________________________________

void WgChart::_setScale(int scale)
{
	WgWidget::_setScale(scale);

	SetCanvasPadding(m_pointPadding);		// Update m_pixelPadding and resample if needed.
	_requestResize();
	_requestRender();
}


//____ _updateDynamics() ______________________________________________________

bool WgChart::_updateDynamics()
{
	bool bNeedRefresh = false;

	if (m_bDynamicSampleRange)
	{
		int first = INT_MAX, last = INT_MIN;

		for (auto& it : m_waves)
		{
			if (first > it.firstSample)
				first = it.firstSample;

			if (last < it.firstSample + it.nSamples)
				last = it.firstSample + it.nSamples;
		}

		if ((float)first != m_firstSample || (float)last != m_lastSample)
		{
			m_firstSample = (float) first;
			m_lastSample = (float) last;
			bNeedRefresh = true;
		}
	}

	if (m_bDynamicValueRange)
	{
        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::min();
        
		for (auto& it : m_waves)
		{
			if (min > it.minSample)
				min = it.minSample;

			if (max < it.maxSample)
				max = it.maxSample;
		}

		if ( max != m_topValue || min != m_bottomValue)
		{
			m_topValue = max;
			m_bottomValue = min;
			bNeedRefresh = true;
		}
	}

	return bNeedRefresh;
}

//____ _resampleAllWaves() ____________________________________________________

void WgChart::_resampleAllWaves()
{
	for (auto& wave : m_waves)
		_resampleWave(&wave);
}

//____ _resampleWave() ________________________________________________________

void WgChart::_resampleWave(Wave * pWave)
{
	WgSize	canvas = PixelSize();

	if (m_pSkin)
		canvas = m_pSkin->ContentRect(canvas, WG_STATE_NORMAL, m_scale);


	canvas -= m_pixelPadding;

	float sampleScale = (m_lastSample - m_firstSample) / (canvas.w + 1);

	int nResampled = (int) (pWave->nSamples / sampleScale);
	if (nResampled < 0)
		nResampled = 0;

	float valueFactor = canvas.h / (m_bottomValue - m_topValue);

	float floor;
	float yOfs;
	if (valueFactor < 0)
	{
		floor = m_bottomValue;
		yOfs = canvas.h*256;
	}
	else
	{
		floor = m_topValue;
		yOfs = 0;
	}

	pWave->resampledDefault = yOfs + (int)((pWave->defaultSample - floor) * valueFactor * 256);

	if (pWave->orgTopSamples.empty())
	{
		pWave->resampledTop.clear();
	}
	else
	{
		pWave->resampledTop.resize(nResampled);

		if (nResampled == pWave->nSamples)
		{
			for (int i = 0; i < nResampled; i++)
				pWave->resampledTop[i] = yOfs + (int)((pWave->orgTopSamples[i] - floor) * valueFactor * 256);
		}
		else
		{
			float stepFactor = (pWave->orgTopSamples.size() - 1) / (float) nResampled;

			for (int i = 0; i < nResampled; i++)
			{
				float sample = stepFactor*i;
				int ofs = (int)sample;
				int frac2 = ((int)(sample * 256)) & 0xFF;
				int frac1 = 256 - frac2;

				int val1 = (int)((pWave->orgTopSamples[ofs] - floor) * valueFactor * 256);
				int val2 = (int)((pWave->orgTopSamples[ofs+1] - floor) * valueFactor * 256);

				pWave->resampledTop[i] = yOfs + ((val1*frac1 + val2*frac2) >> 8) ;
			}
		}
	}

	if (pWave->orgBottomSamples.empty())
	{
		pWave->resampledBottom.clear();
	}
	else
	{
		pWave->resampledBottom.resize(nResampled);

		if (nResampled == pWave->nSamples)
		{
			for (int i = 0; i < nResampled; i++)
				pWave->resampledBottom[i] = yOfs + (int)((pWave->orgBottomSamples[i] - floor) * valueFactor * 256);
		}
		else
		{
			float stepFactor = (pWave->orgTopSamples.size() - 1) / (float)nResampled;

			for (int i = 0; i < nResampled; i++)
			{
				float sample = stepFactor*i;
				int ofs = (int)sample;
				int frac2 = ((int)(sample * 256)) & 0xFF;
				int frac1 = 256 - frac2;

				int val1 = (int)((pWave->orgBottomSamples[ofs] - floor) * valueFactor * 256);
				int val2 = (int)((pWave->orgBottomSamples[ofs + 1] - floor) * valueFactor * 256);

				pWave->resampledBottom[i] = yOfs + ((val1*frac1 + val2*frac2) >> 8);
			}
		}
	}
}
