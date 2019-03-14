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


#include <wg_scrollchart.h>
#include <wg_surfacefactory.h>
#include <wg_gfxdevice.h>
#include <wg_pen.h>
#include <wg_base.h>
#include <wg_texttool.h>
#include <wg_util.h>
#include <wg_eventhandler.h>


#include <assert.h>

static const char	c_widgetType[] = { "ScrollChart" };

//____ Constructor ____________________________________________________________

WgScrollChart::WgScrollChart()
{
	m_defaultSize = WgSize(200, 100);
	m_bOpaque = false;
	m_mode = WG_MODE_NORMAL;

	m_bDynamicValueRange = true;

	m_setTopValue = 1.f;
	m_setBottomValue = -1.f;

	m_topValue = 1.f;
	m_bottomValue = -1.f;

	m_pCanvas = nullptr;
	m_pFactory = nullptr;

	m_bScrollFromStart = true;
	m_bStarted = false;
	m_bPaused = false;

	m_sampleTTL = 0;
	m_waveIdCounter = 1;

	m_windowBegin = 0;
	m_windowEnd = 0;

	m_scrollAmount = 0;
	m_scrollFraction = 0.f;

	m_sampleBeginTimestamp = 0;
	m_sampleEndTimestamp = 0;

	m_sampleLabelStyle.alignment = WgOrigo::South;
	m_sampleLabelStyle.offset = { 0,0 };

	m_valueLabelStyle.alignment = WgOrigo::West;
	m_valueLabelStyle.offset = { 0,0 };

	m_chartColor = WgColor::White;
}

//____ Destructor _____________________________________________________________

WgScrollChart::~WgScrollChart()
{
	_stopReceiveTicks();
}

//____ Type() _________________________________________________________________

const char * WgScrollChart::Type(void) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgScrollChart::GetClass()
{
	return c_widgetType;
}

//____ SetPreferredChartSize() _______________________________________________________

void WgScrollChart::SetPreferredChartSize(const WgSize& size)
{
	if (size != m_defaultSize)
	{
		m_defaultSize = size;
		_requestResize();
	}
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgScrollChart::PreferredPixelSize() const
{
	WgSize sz = (m_defaultSize * m_scale) / WG_SCALE_BASE;

	sz += m_pixelPadding;
	if (m_pSkin)
		sz = m_pSkin->SizeForContent(sz, m_scale);

	return sz;
}

//____ SetSurfaceFactory() ____________________________________________________

void WgScrollChart::SetSurfaceFactory(WgSurfaceFactory * pFactory)
{
	m_pFactory = pFactory;
	_regenCanvas();
	_requestRender();
}


//____ SetSkin() ______________________________________________________________

void WgScrollChart::SetSkin(const WgSkinPtr& pSkin)
{
	m_pSkin = pSkin;

	if (pSkin->IsOpaque())
		m_bOpaque = true;
	else
		m_bOpaque = false;

	_requestResize();		// Just in case
	_requestRender();
}

//____ SetChartColor() ________________________________________________________

void WgScrollChart::SetChartColor(WgColor color)
{
	if (color != m_chartColor)
	{
		m_chartColor = color;
		m_bRefreshCanvas = true;
		_requestRender();
	}
}

//____ SetScrollFromStart() ___________________________________________________

void WgScrollChart::SetScrollFromStart(bool bScrollOnStart)
{
	m_bScrollFromStart = bScrollOnStart;
}


//____ SetCanvasPadding() _____________________________________________________

void WgScrollChart::SetCanvasPadding(WgBorders padding)
{
	m_pointPadding = padding;
	WgBorders pixelPadding( (padding.top * m_scale) >> WG_SCALE_BINALS, (padding.right * m_scale) >> WG_SCALE_BINALS, 
							(padding.bottom * m_scale) >> WG_SCALE_BINALS, (padding.left * m_scale) >> WG_SCALE_BINALS );

	if (pixelPadding != m_pixelPadding)
	{
		m_pixelPadding = pixelPadding;
		_regenCanvas();
	}
}

//____ Start() ________________________________________________________________

bool WgScrollChart::Start(int sampleTTL)
{
	if (sampleTTL < 100)
		return false;

	if (m_bStarted)
		Stop();

	m_sampleTTL = sampleTTL;
	m_bStarted = true;

	m_sampleBeginTimestamp = 0;
	m_sampleEndTimestamp = m_bScrollFromStart ? sampleTTL : 0;


	// Add start samples so that we start to draw the waves.

	for (auto& wave : m_waves)
		wave.samples.push_front({ (int) m_sampleEndTimestamp,wave.startTopSample,wave.startBottomSample });

	if (m_bDynamicValueRange && _updateDynamics() )
	{
		if (m_valueRangeResponder)
			m_valueRangeResponder(this, m_topValue, m_bottomValue);

		m_bRefreshCanvas = true;
		_requestRender();
	}


	_startReceiveTicks();
	return true;
}

//____ Stop() _________________________________________________________________

void WgScrollChart::Stop()
{
	m_bStarted = false;
	m_bPaused = false;
	m_bRefreshCanvas = true;
	m_sampleBeginTimestamp = 0;
	m_sampleEndTimestamp = 0;

	for (auto& wave : m_waves)
		wave.samples.clear();

	m_topValue = m_setTopValue;
	m_bottomValue = m_setBottomValue;

	_stopReceiveTicks();
}

//____ Pause() ________________________________________________________________

void WgScrollChart::Pause()
{
	if (m_bStarted && !m_bPaused)
	{
		m_bPaused = true;
		_stopReceiveTicks();
	}
}

//____ Continue() _____________________________________________________________

void WgScrollChart::Continue()
{
	if (m_bStarted && m_bPaused)
	{
		m_bPaused = false;
		_startReceiveTicks();
	}
}

//____ SetValueRange() ________________________________________________________

bool WgScrollChart::SetValueRange(float topValue, float bottomValue)
{
	if (topValue == bottomValue)
		return false;

	if (topValue != m_setTopValue || bottomValue != m_setBottomValue)			
	{
		m_setTopValue = topValue;
		m_setBottomValue = bottomValue;

		bool	bRangeModified;

		if( m_bDynamicValueRange )
			bRangeModified = _updateDynamics();
		else
		{
			m_topValue = topValue;
			m_bottomValue = bottomValue;
			bRangeModified = true;
		}

		if (bRangeModified)
		{
			if (m_valueRangeResponder)
				m_valueRangeResponder(this, m_topValue, m_bottomValue);

			m_bRefreshCanvas = true;
			_requestRender();
		}
	}

	return true;
}

//____ SetDynamicValueRange() ________________________________________________________

void WgScrollChart::SetDynamicValueRange( bool bDynamic )
{
	if (bDynamic == m_bDynamicValueRange)
		return;

	m_bDynamicValueRange = bDynamic;

	bool	bRangeModified = false;

	if (m_bDynamicValueRange)
	{
		bRangeModified = _updateDynamics();
	}
	else if (m_topValue != m_setTopValue || m_bottomValue != m_setBottomValue)
	{
			m_topValue = m_setTopValue;
			m_bottomValue = m_setBottomValue;
			bRangeModified = true;
	}

	if (bRangeModified)
	{
		if (m_valueRangeResponder)
			m_valueRangeResponder(this, m_topValue, m_bottomValue);

		m_bRefreshCanvas = true;
		_requestRender();
	}
}

//____ SetSampleLabelStyle() __________________________________________________

void WgScrollChart::SetSampleLabelStyle(WgOrigo alignment, WgCoord offset, const WgSkinPtr& pSkin, const WgTextpropPtr& prop)
{
	m_sampleLabelStyle.alignment = alignment;
	m_sampleLabelStyle.offset = offset;
	m_sampleLabelStyle.pSkin = pSkin;
	m_sampleLabelStyle.pTextStyle = prop;
	_requestRender();
}

//____ SetSampleGridLines() ____________________________________________________

void WgScrollChart::SetSampleGridLines(int nLines, GridLine * pLines)
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

void WgScrollChart::SetValueLabelStyle(WgOrigo alignment, WgCoord offset, const WgSkinPtr& pSkin, const WgTextpropPtr& prop)
{
	m_valueLabelStyle.alignment = alignment;
	m_valueLabelStyle.offset = offset;
	m_valueLabelStyle.pSkin = pSkin;
	m_valueLabelStyle.pTextStyle = prop;
	_requestRender();
}

//____ SetValueGridLines() ____________________________________________________

void WgScrollChart::SetValueGridLines(int nLines, GridLine * pLines)
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

void WgScrollChart::SetResizeResponder(std::function<void(WgScrollChart * pWidget, WgSize newSize)> func)
{
	m_resizeResponder = func;
	if (func)
		func(this, PixelSize());
}

//____ SetValueRangeResponder() _____________________________________________________

void WgScrollChart::SetValueRangeResponder(std::function<void(WgScrollChart * pWidget, float topValue, float bottomValue)> func)
{
	m_valueRangeResponder = func;
	if (func)
		func(this, m_topValue, m_bottomValue);
}


//____ StartLineWave() ____________________________________________________________

int	WgScrollChart::StartLineWave(float startSample, float thickness, WgColor color, std::function<float(uint64_t timeCode)> sampleFeeder)
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();

	w.startTimestamp = m_windowEnd;

	w.id = m_waveIdCounter++;
	w.type = WaveType::Line;
	w.startTopSample = startSample;
	w.startBottomSample = 0.f;
	w.topLineThickness = thickness;
	w.topLineColor = color;
	w.simpleSampleFeeder = sampleFeeder;
	w.samples.push_back({ (int) (m_sampleEndTimestamp - m_sampleBeginTimestamp),startSample,0 });

	w.nextTopSample = startSample;
	w.nextBottomSample = 0.f;

	return w.id;
}

//____ StartSimpleWave() ______________________________________________________

int WgScrollChart::StartSimpleWave(float startSample, float floor, float topLineThickness, WgColor topLineColor,
	float floorLineThickness, WgColor floorLineColor, WgColor aboveFloorFill, WgColor belowFloorFill, std::function<float(uint64_t timeCode)> sampleFeeder)
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();

	w.startTimestamp = m_windowEnd;

	w.id = m_waveIdCounter++;
	w.type = WaveType::Simple;
	w.startTopSample = startSample;
	w.startBottomSample = 0.f;
	w.floorValue = floor;
	w.topLineThickness = topLineThickness;
	w.topLineColor = topLineColor;
	w.bottomLineThickness = floorLineThickness;
	w.bottomLineColor = floorLineColor;
	w.frontFill = aboveFloorFill;
	w.backFill = belowFloorFill;
	w.simpleSampleFeeder = sampleFeeder;
	w.samples.push_back({ (int)(m_sampleEndTimestamp - m_sampleBeginTimestamp),startSample,0 });

	w.nextTopSample = startSample;
	w.nextBottomSample = 0.f;

	return w.id;
}

//____ StartComplexWave() _____________________________________________________

int WgScrollChart::StartComplexWave(SamplePair startSample, float topLineThickness, WgColor topLineColor,
	float bottomLineThickness, WgColor bottomLineColor, WgColor frontFill, WgColor backFill, std::function<SamplePair(uint64_t timeCode)> sampleFeeder)
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();

	w.startTimestamp = m_windowEnd;

	w.id = m_waveIdCounter++;
	w.type = WaveType::Complex;
	w.startTopSample = startSample.top;
	w.startBottomSample = startSample.bottom;
	w.floorValue = startSample.bottom;
	w.topLineThickness = topLineThickness;
	w.topLineColor = topLineColor;
	w.bottomLineThickness = bottomLineThickness;
	w.bottomLineColor = bottomLineColor;
	w.frontFill = frontFill;
	w.backFill = backFill;
	w.complexSampleFeeder = sampleFeeder;
	w.samples.push_back({(int)(m_sampleEndTimestamp - m_sampleBeginTimestamp),startSample.top,startSample.bottom });

	w.nextTopSample = startSample.top;
	w.nextBottomSample = startSample.bottom;

	return w.id;
}

//____ StopWave() _____________________________________________________________

bool WgScrollChart::StopWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p) 
		return false;

	p->bHidden = true;
	return true;
}

//____ StopAllWaves() _________________________________________________________

void WgScrollChart::StopAllWaves()
{
	for (auto& wave : m_waves)
		wave.bHidden = true;
}

//____ IsWaveDisplayed() ______________________________________________________

bool WgScrollChart::IsWaveDisplayed(int waveId) const
{
	const Wave * p = _getWave(waveId);
	if (!p)
		return false;

	return !p->bHidden;
}

//____ HideWave() _____________________________________________________________

bool WgScrollChart::HideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == false)
	{
		p->bHidden = true;
		m_bRefreshCanvas = true;
	}
	return true;
}

//____ UnhideWave() ___________________________________________________________

bool WgScrollChart::UnhideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == true)
	{
		p->bHidden = false;
		m_bRefreshCanvas = true;
	}
	return true;
}

//____ HideAllWaves() _________________________________________________________

void WgScrollChart::HideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (!wave.bHidden)
		{
			wave.bHidden = true;
			m_bRefreshCanvas = true;
		}
	}
}

//____ UnhideAllWaves() _______________________________________________________

void WgScrollChart::UnhideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (wave.bHidden)
		{
			wave.bHidden = false;
			m_bRefreshCanvas = true;
		}
	}
}

//____ FeedSample() ___________________________________________________________

bool WgScrollChart::FeedSample(int waveId, float sample)
{
	Wave * p = _getWave(waveId);
	if (!p || p->type == WaveType::Complex )
		return false;

	p->nextTopSample = sample;
	return true;
}

bool WgScrollChart::FeedSample(int waveId, SamplePair sample)
{
	Wave * p = _getWave(waveId);
	if (!p || p->type != WaveType::Complex)
		return false;

	p->nextTopSample = sample.top;
	p->nextBottomSample = sample.bottom;
	return true;
}

//____ _onEvent() _____________________________________________________________

void WgScrollChart::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	if (pEvent->Type() == WG_EVENT_TICK)
	{
		// Update timestamps

		int ticks = static_cast<const WgEvent::Tick*>(pEvent)->Millisec();
		m_sampleEndTimestamp += ticks;

		// Set length of last sample and add our new sample to end of each running wave

		for (auto& wave : m_waves)
		{
			if (!wave.bStopped)
			{
/*				if (m_bDynamicValueRange)
				{
					if (wave.nextTopSample >= wave.maxSample)
					{
						if (wave.nextTopSample == wave.maxSample)
							wave.maxSampleCount++;
						else
						{
							wave.maxSample = wave.nextTopSample;
							wave.maxSampleCount = 1;
						}

					}
					if (wave.nextTopSample <= wave.minSample)
					{
						wave.minSample = wave.nextTopSample;
						if (wave.nextTopSample == wave.minSample)
							wave.minSampleCount++;
						else
						{
							wave.minSample = wave.nextTopSample;
							wave.minSampleCount = 1;
						}
					}

					if (wave.type == WaveType::Complex)
					{
						if (wave.nextBottomSample >= wave.maxSample)
						{
							if (wave.nextBottomSample == wave.maxSample)
								wave.maxSampleCount++;
							else
							{
								wave.maxSample = wave.nextBottomSample;
								wave.maxSampleCount = 1;
							}

						}
						if (wave.nextBottomSample <= wave.minSample)
						{
							wave.minSample = wave.nextBottomSample;
							if (wave.nextBottomSample == wave.minSample)
								wave.minSampleCount++;
							else
							{
								wave.minSample = wave.nextBottomSample;
								wave.minSampleCount = 1;
							}
						}
					}
				}
*/
				wave.samples.back().length = ticks;
				wave.samples.push_back({ 0, wave.nextTopSample, wave.nextBottomSample });
			}
		}

		int cutAmount  = ((int)(m_sampleEndTimestamp - m_sampleBeginTimestamp)) - m_sampleTTL;

		// Update m_sampleBeginTimestamp and cut samples not within window anymore from beginning of waves

		if (cutAmount > 0)
		{
			m_sampleBeginTimestamp += cutAmount;

			for (auto& wave : m_waves)
			{
				int waveCutAmount = cutAmount;
//				bool bRefreshMinMax = false;

				while (wave.samples.size() > 2 && wave.samples[0].length <= waveCutAmount )
				{
					const Sample& s = wave.samples[0];

					waveCutAmount -= s.length;

					
//				if( s.top == wave.maxSample || s.top == wave.minSample || (wave.type == WaveType::Complex && (s.bottom == wave.maxSample ) 

					wave.samples.pop_front();
				}


				// Update first sample so that it begins at window start.

				float fractionRemoved = waveCutAmount / (float) wave.samples[0].length;

				wave.samples[0].top += (wave.samples[1].top - wave.samples[0].top) * fractionRemoved;
				wave.samples[0].bottom += (wave.samples[1].bottom - wave.samples[0].bottom) * fractionRemoved;

				wave.samples[0].length -= waveCutAmount;
			}
		}



		// Calculate scroll amount and update scrollFraction.

		m_scrollFraction += ticks;
		
		float samplesPerPixel = m_sampleTTL / (float) m_pCanvas->PixelSize().w;

		int scrollAmount =  (int) ((m_scrollFraction-1) / samplesPerPixel);

        m_scrollAmount += scrollAmount;
		m_scrollFraction -= scrollAmount * samplesPerPixel;

		if (scrollAmount > 0)
		{
			_requestRender();

			// Update window end and window start

			m_windowEnd = m_sampleEndTimestamp - m_scrollFraction;

			if (m_windowEnd > m_sampleTTL)
				m_windowBegin = m_windowEnd - m_sampleTTL;
			else
				m_windowBegin = 0;
		}
	}
	else
		pHandler->ForwardEvent(pEvent);

}

//____ _onCloneContent() ______________________________________________________

void WgScrollChart::_onCloneContent(const WgWidget * _pOrg)
{
	WgScrollChart * pOrg = (WgScrollChart*)_pOrg;

	m_defaultSize = pOrg->m_defaultSize;
}

//____ _renderPatches() _______________________________________________________

void WgScrollChart::_renderPatches(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{

	if (m_pCanvas)
	{
		WgSize sz = m_pCanvas->PixelSize();
		double	timestampInc = m_sampleTTL / (double)sz.w;

		if (m_bRefreshCanvas || m_scrollAmount >= sz.w)
		{
			WgSurface * pOldCanvas = pDevice->Canvas();
			pDevice->SetCanvas(m_pCanvas);

			pDevice->Fill(sz, m_chartColor);
			_renderGridLines(pDevice, sz, sz, sz);

/*
			double winBeg = m_windowBegin;

			for (int i = 0; i < sz.w; i++)
			{
				_renderWaveSegment(pDevice, { i,0,1,sz.h }, winBeg, winBeg+timestampInc, timestampInc);
				winBeg += timestampInc;
			}
*/

			_renderWaveSegment(pDevice, sz, m_windowBegin, m_windowEnd, timestampInc);
			pDevice->SetCanvas(pOldCanvas);

			if (m_windowEnd - m_windowBegin < m_sampleTTL)
			{
				m_canvasOfs = (int) ((m_windowEnd - m_windowBegin) / timestampInc);
			}
			else
				m_canvasOfs = 0; // m_scrollAmount % m_pCanvas->Size().w;

			m_scrollAmount = 0;

			m_bRefreshCanvas = false;
		}
		else if (m_scrollAmount > 0)
		{
			// Adjust our windows to account for thicker lines

			int margin = ((int)(_thickestLine() / 2)) + 1;

			m_canvasOfs = (m_canvasOfs + sz.w - margin) % sz.w;
			m_scrollAmount += margin;

			//

			WgSurface * pOldCanvas = pDevice->Canvas();
			pDevice->SetCanvas(m_pCanvas);

			if (m_canvasOfs + m_scrollAmount <= sz.w )
			{
				double windowBegin = m_windowEnd - m_scrollAmount*timestampInc;
				double windowEnd = m_windowEnd;

				WgRect canvas(m_canvasOfs, 0, m_scrollAmount, sz.h);
				pDevice->Fill(canvas, m_chartColor);
				_renderGridLines(pDevice, canvas, canvas, canvas);
				_renderWaveSegment(pDevice, canvas, windowBegin, windowEnd, timestampInc);
			}
			else
			{
				int width1 = sz.w - m_canvasOfs;
				int width2 = m_scrollAmount - width1;

				double windowBegin = m_windowEnd - m_scrollAmount*timestampInc;
				double windowEnd = m_windowEnd - width2*timestampInc;

				WgRect canvas(m_canvasOfs, 0, width1, sz.h);
				pDevice->Fill(canvas, m_chartColor);
				_renderGridLines(pDevice, canvas, canvas, canvas);
				_renderWaveSegment(pDevice, canvas, windowBegin, windowEnd, timestampInc);

				windowBegin = m_windowEnd - width2*timestampInc;
				windowEnd = m_windowEnd;

				WgRect canvas2(0, 0, width2, sz.h);
				pDevice->Fill(canvas2, m_chartColor);
				_renderGridLines(pDevice, canvas2, canvas2, canvas2);
				_renderWaveSegment(pDevice, canvas2, windowBegin, windowEnd, timestampInc);
			}


			pDevice->SetCanvas(pOldCanvas);

			m_canvasOfs = (m_canvasOfs + m_scrollAmount) % sz.w;
			m_scrollAmount = 0;
		}
	}

	WgWidget::_renderPatches(pDevice, _canvas, _window, _pPatches);
}

//____ _thickestLine() ________________________________________________________

float WgScrollChart::_thickestLine() const
{
	float thickness = 0.f;

	for (auto& w : m_waves)
	{
		if (w.topLineThickness > thickness)
			thickness = w.topLineThickness;

		if (w.bottomLineThickness > thickness)
			thickness = w.bottomLineThickness;
	}
	return thickness;
}


//____ _renderWaveSegment() ___________________________________________________

void WgScrollChart::_renderWaveSegment(WgGfxDevice * pDevice, const WgRect& _canvas, double startTimestamp, double endTimestamp, float timestampInc)
{
	// Get margin and resample start/end timestamps that take thickest line into account

	int margin = ((int)(_thickestLine() / 2)) + 1;
	double  resampleStartTimestamp = startTimestamp - timestampInc*margin;
	double  resampleEndTimestamp = endTimestamp + timestampInc*margin;


	// Setup resample buffer

	int nSamples = (_canvas.w+1) + margin*2;
	int bufferSize = nSamples * sizeof(int) * 2;

	int * pTopBuffer = (int*) WgBase::MemStackAlloc(bufferSize);
	int * pBottomBuffer = pTopBuffer + nSamples;

	// Calculate yOfs, chart floor and valueFactor

	int canvasHeight = m_pCanvas->PixelSize().h;
	float valueFactor = canvasHeight / (m_bottomValue - m_topValue);

	float graphFloor;
	float yOfs;
	if (valueFactor < 0)
	{
		graphFloor = m_bottomValue;
		yOfs = canvasHeight * 256;
	}
	else
	{
		graphFloor = m_topValue;
		yOfs = 0;
	}

	//

	for (auto& wave : m_waves)
	{
		if (!wave.bHidden)
		{
			int waveSamples = nSamples;
			int waveSamplesOffset = 0;

			_resampleWavePortion(waveSamplesOffset, waveSamples, pTopBuffer, pBottomBuffer, wave, resampleStartTimestamp, resampleEndTimestamp, timestampInc);



			int floor = yOfs + (int)((wave.floorValue - graphFloor) * valueFactor * 256);

			WgWaveLine topLine = { waveSamples, wave.topLineThickness, wave.topLineColor, pTopBuffer, 0 };
			WgWaveLine bottomLine = { waveSamples, wave.bottomLineThickness, wave.bottomLineColor, pBottomBuffer, floor };

			switch (wave.type)
			{
			case WaveType::Line:
				//TODO: Implement!
				break;
			case WaveType::Simple:
				bottomLine.length = 0;
				bottomLine.pWave = nullptr;
				pDevice->ClipDrawHorrWave(_canvas, { _canvas.x - margin + waveSamplesOffset,_canvas.y }, waveSamples-1, topLine, bottomLine, wave.frontFill, wave.backFill);
				break;
			case WaveType::Complex:
				pDevice->ClipDrawHorrWave(_canvas, { _canvas.x - margin + waveSamplesOffset,_canvas.y }, waveSamples-1, topLine, bottomLine, wave.frontFill, wave.backFill);
				break;
			}
		}
	}

	WgBase::MemStackRelease(bufferSize);
}

//____ _resampleWavePortion() _________________________________________________

void WgScrollChart::_resampleWavePortion(int& ofs, int& nSamples, int * pOutTop, int * pOutBottom, const WgScrollChart::Wave& wave, double startTimeStamp, double endTimeStamp, float timestampInc)
{
	double	pixelTimestamp = startTimeStamp;

	uint64_t	sampleTimestamp = m_sampleBeginTimestamp;

	if (sampleTimestamp < wave.startTimestamp)
		sampleTimestamp += wave.startTimestamp + sampleTimestamp;



	// Forward pixels until we have correct prev and next iterators.

	pixelTimestamp += ofs * timestampInc;

	while (nSamples > 0 && sampleTimestamp > pixelTimestamp)
	{
		ofs++;
		nSamples--;
		pixelTimestamp += timestampInc;
	}

	auto itPrev = wave.samples.begin();
	auto itNext = itPrev; itNext++;

	//

	int canvasHeight = m_pCanvas->PixelSize().h;
	float valueFactor = canvasHeight / (m_bottomValue - m_topValue);

	float floor;
	float yOfs;
	if (valueFactor < 0)
	{
		floor = m_bottomValue;
		yOfs = canvasHeight * 256;
	}
	else
	{
		floor = m_topValue;
		yOfs = 0;
	}

	//

	int i = 0;
	while( i < nSamples && itNext != wave.samples.end() )
	{
		while (sampleTimestamp + itPrev->length < pixelTimestamp)
		{
			sampleTimestamp += itPrev->length;
			itPrev = itNext++;
			if (itNext == wave.samples.end())
			{
				nSamples = i;
				return;
			}
		}

		float fraction = (pixelTimestamp - sampleTimestamp) / itPrev->length;


		float top = itPrev->top * (1-fraction) + itNext->top * fraction;
		*pOutTop++ = yOfs + (int) ((top-floor) * valueFactor * 256);


		if (wave.type == WaveType::Complex)
		{
			float bottom = itPrev->bottom * (1-fraction) + itNext->bottom * fraction;
			*pOutBottom++ = yOfs + (int) ((bottom - floor) * valueFactor * 256);
		}
		pixelTimestamp += timestampInc;
		i++;
	}

}

//____ _renderGridLines() _____________________________________________________

void WgScrollChart::_renderGridLines(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip)
{
	// Draw value grid lines

	float top, bottom, mul, startOfs;

	if (!m_valueGridLines.empty())
	{
		// Preparations for both lines and labels

        top = std::min(m_topValue, m_bottomValue);
        bottom = std::max(m_topValue, m_bottomValue);

		mul = _canvas.h / (m_bottomValue - m_topValue);
		startOfs = mul > 0 ? _canvas.y : _canvas.y + _canvas.h;

		// Draw lines

		for (auto& line : m_valueGridLines)
		{
			int yOfs = startOfs + (int)((line.pos - top) * mul + 0.5f);
			pDevice->ClipDrawLine(_clip, { _canvas.x, yOfs }, WgDirection::Right, _canvas.w, line.color, line.thickness * m_scale / WG_SCALE_BASE);
		}
	}
}

//____ _onRender() ____________________________________________________________

void WgScrollChart::_onRender(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip)
{
	WgRect canvas;

	if (m_pSkin)
	{
		if (m_chartColor.a == 255)
		{
			WgRect coverage = m_pSkin->ContentRect(_canvas, WgStateEnum::Normal, m_scale);

			if ( !coverage.contains( _clip) )
			{
				WgRect topSection(canvas.x, canvas.y, canvas.w, coverage.y - canvas.y);
				WgRect leftSection(canvas.x, coverage.y, coverage.x - canvas.x, coverage.h);
				WgRect rightSection(coverage.x + coverage.w, coverage.y, canvas.x + canvas.w - (coverage.x + coverage.w), coverage.h);
				WgRect bottomSection(canvas.x, coverage.y + coverage.h, canvas.w, canvas.y + canvas.h - (coverage.y + coverage.h));

				m_pSkin->Render(pDevice, WgStateEnum::Normal, _canvas, WgRect(topSection, _clip), m_scale);
				m_pSkin->Render(pDevice, WgStateEnum::Normal, _canvas, WgRect(leftSection, _clip), m_scale);
				m_pSkin->Render(pDevice, WgStateEnum::Normal, _canvas, WgRect(rightSection, _clip), m_scale);
				m_pSkin->Render(pDevice, WgStateEnum::Normal, _canvas, WgRect(bottomSection, _clip), m_scale);
			}

		}
		else
			m_pSkin->Render(pDevice, WgStateEnum::Normal, _canvas, _clip, m_scale);

		canvas = m_pSkin->ContentRect(_canvas, WgStateEnum::Normal, m_scale);
	}
	else
		canvas = _canvas;

	// Preparations for both grid and wave drawing

	WgRect scrollCanvas = canvas - m_pixelPadding;

	// Draw sample gird lines

	// Draw value grid lines

		// Done on backbuffer instead!

	// Copy wave from backbuffer

	if (m_pCanvas)
	{
		assert(m_pCanvas->PixelSize() == scrollCanvas.size());
	
		if( m_windowBegin == 0 )
			pDevice->ClipBlitFromCanvas(_clip, m_pCanvas, { 0, 0, scrollCanvas.w, scrollCanvas.h }, scrollCanvas.x, scrollCanvas.y);
		else
		{
			int firstPartLen = m_pCanvas->PixelSize().w - m_canvasOfs;

			pDevice->ClipBlitFromCanvas(_clip, m_pCanvas, { m_canvasOfs, 0, firstPartLen, scrollCanvas.h }, scrollCanvas.x, scrollCanvas.y);
			pDevice->ClipBlitFromCanvas(_clip, m_pCanvas, { 0, 0, scrollCanvas.w - firstPartLen, scrollCanvas.h }, scrollCanvas.x + firstPartLen, scrollCanvas.y);
		}
	}

	// Draw value grid labels

	if (!m_valueGridLines.empty())
	{
		float top, bottom, mul, startOfs;

        top = std::min(m_topValue, m_bottomValue);
        bottom = std::max(m_topValue, m_bottomValue);

		mul = scrollCanvas.h / (m_bottomValue - m_topValue);
		startOfs = mul > 0 ? scrollCanvas.y : scrollCanvas.y + scrollCanvas.h;

		for (auto& line : m_valueGridLines)
		{
			int yOfs = startOfs + (int)((line.pos - top) * mul + 0.5f);

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
					textOfs = m_valueLabelStyle.pSkin->ContentRect(labelSize, WgStateEnum::Normal, m_scale).pos();
				}

				WgCoord labelPos = _placeLabel({ canvas.x, yOfs }, m_valueLabelStyle.alignment, m_valueLabelStyle.offset, labelSize);

				if (m_valueLabelStyle.pSkin)
					m_valueLabelStyle.pSkin->Render(pDevice, WgStateEnum::Normal, { labelPos,labelSize }, _clip, m_scale);

				pen.SetPos(labelPos + textOfs);
				pDevice->PrintLine(pen, attr, line.label.Chars());
			}
		}
	}


}

//____ _onAlphaTest() _________________________________________________________

bool WgScrollChart::_onAlphaTest(const WgCoord& ofs)
{
	//TODO: Implement?

	return true;
}

//____ _onNewSize() ___________________________________________________________

void WgScrollChart::_onNewSize(const WgSize& size)
{
	if (m_resizeResponder)
		m_resizeResponder(this, size);

	_regenCanvas();
	WgWidget::_onNewSize(size);
}

//____ _setScale() ____________________________________________________________

void WgScrollChart::_setScale(int scale)
{
	WgWidget::_setScale(scale);

	SetCanvasPadding(m_pointPadding);		// Update m_pixelPadding and resample if needed.
	_requestResize();
	_requestRender();
}

//____ _regenCanvas() _________________________________________________________

void WgScrollChart::_regenCanvas()
{
	if (m_pCanvas)
	{
		delete m_pCanvas;
		m_pCanvas = nullptr;
	}

	if (m_pFactory)
	{
		WgSize sz = _getScrollWindow().size();

		if (sz.w <= 0 && sz.h <= 0)
			return;

		m_pCanvas = m_pFactory->CreateSurface(sz, WgPixelType::BGR_8);
//		m_pCanvas->Fill(m_chartColor);
		m_canvasOfs = 0;
		m_bRefreshCanvas = true;
	}
}

//____ _updateDynamics() ______________________________________________________

bool WgScrollChart::_updateDynamics()
{
	bool bNeedRefresh = false;

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

		if (max < m_setTopValue)
			max = m_setTopValue;

		if (min > m_setBottomValue)
			min = m_setBottomValue;

		if (max != m_topValue || min != m_bottomValue)
		{
			m_topValue = max;
			m_bottomValue = min;
			bNeedRefresh = true;
		}
	}

	return bNeedRefresh;
}

//____ _placeLabel() __________________________________________________________

WgCoord	WgScrollChart::_placeLabel(WgCoord startPoint, WgOrigo alignment, WgCoord labelOffset, WgSize labelSize) const
{
	return startPoint + (labelOffset*m_scale/WG_SCALE_BASE) - WgCoord(labelSize.w, labelSize.h) + WgUtil::OrigoToOfs(alignment, labelSize);
}

//____ _getWave() _____________________________________________________________

WgScrollChart::Wave * WgScrollChart::_getWave(int waveId)
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}

const WgScrollChart::Wave * WgScrollChart::_getWave(int waveId) const
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}

//____ _getScrollWindow() _____________________________________________________

WgRect WgScrollChart::_getScrollWindow() const
{
	WgRect r = PixelSize();

	if (m_pSkin)
		r = m_pSkin->ContentRect(r, WgStateEnum::Normal, m_scale).size();

	r -= m_pixelPadding;

	if (r.w < 0) r.w = 0;
	if (r.h < 0) r.h = 0;

	return r;
}
