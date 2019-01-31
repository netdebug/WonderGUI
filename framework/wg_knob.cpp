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


#include <wg_knob.h>
#include <wg_gfxdevice.h>
#include <wg_base.h>
//#include <wg_color.h>
//#include "Debug.h"
#include <wg_event.h>
#include <wg_eventhandler.h>
#include <wg_patches.h>

static const char	c_widgetType[] = {"Knob"};

#include <cmath>
#include <algorithm>

#define PI 3.141592653f

//____ Constructor ____________________________________________________________

WgKnob::WgKnob()
{
    m_bOpaque = false;
    m_iNextPixel = 0;
    m_lineColor = WgColor( 38,  169, 224, 255 ); //WgColor::white;
    m_fValue = 0.0f;
    m_preferredSize = WgSize(45,45);

}

WgKnob::WgKnob(WgSurfaceFactory * pFactory)
{
    WgKnob();
    m_bOpaque = false;
    m_iNextPixel = 0;
    m_lineColor = WgColor::White;
    m_fValue = 0.0f;
    m_preferredSize = WgSize(100,100);

    m_pSurf = pFactory->CreateSurface(PixelSize()*m_iOversampleX, WgPixelType::BGRA_8);
    m_pSurf->Fill(WgColor::Transparent);
    m_pSurfaceFactory = pFactory;

    m_size = PixelSize();

    // Initialize arrays
    SetNumSteps(m_iNumSteps);

}


//____ Destructor _____________________________________________________________

WgKnob::~WgKnob()
{
	if(m_pSurf)
		delete m_pSurf;
}

//____ Type() _________________________________________________________________

const char * WgKnob::Type( void ) const
{
    return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgKnob::GetClass()
{
    return c_widgetType;
}

//____ SetValue() _____________________________________________________________

void WgKnob::SetValue( float fValue )
{
    if(m_fValue == fValue)
        return;

	_myRequestRender(_calcUdateRect(m_fAngleStart, m_fAngleEnd, fValue));

    m_fValue = fValue;
}


//____ SetPreferredPixelSize() __________________________________________________________
/*
void WgKnob::SetPreferredPixelSize(WgSize size)
 {
 m_preferredSize = size;
 }
 */


 //____ PreferredPixelSize() __________________________________________________________

WgSize WgKnob::PreferredPixelSize() const
{
    return m_preferredSize;
}

//____ SetNumSteps() __________________________________________________________

void WgKnob::SetNumSteps(int steps)
{
    if(m_iNumSteps == steps)
        return;

    int maxNumSteps = 11;
    int minNumSteps = 2;
    m_iNumSteps = steps;
    if(steps < minNumSteps || steps > maxNumSteps)
        return;

    // Idea:
    // 2 steps: two large segments  ([0.3 0.45], [0.55 0.7])
    // 3 steps: [
    m_AngleStart.resize(steps);
    m_AngleEnd.resize(steps);

    float divs = (float)(m_iNumSteps * 2 - 1);
    //float length = (m_fAngleEnd - m_fAngleStart)*steps/maxNumSteps;

    float weight = float(steps - minNumSteps)/float(maxNumSteps - minNumSteps);
    float length = (m_fAngleEnd - m_fAngleStart)*weight + 0.25f*(1.0f-weight);
    float start = 0.5f - length/2.0f; // start the sections

    for(int i=0;i<steps;i++)
    {
        m_AngleStart[i] = start + 2*i*length/divs;
        m_AngleEnd[i]   = start + (2*i+1)*length/divs;
        m_AngleStart[i] *= 2*PI;
        m_AngleEnd[i] *= 2*PI;
    }
}

//____ SetAngleOffset() _______________________________________________________

void WgKnob::SetAngleOffset(float offset) 
{ 
	if (offset != m_fAngleOffset)
	{
		m_fAngleOffset = offset; 
		_myRequestRender();
	}
}

//____ _calcUpdateRect() ______________________________________________________

WgRect WgKnob::_calcUdateRect(float newAngleStart, float newAngleEnd, float newValue)
{
	WgRect fullRect = m_pSurf->PixelSize();

	if (m_bOptimizeUpdateRect)
	{
		// Subtract 2π from the angles if the start angle is offsetted more than 2π.
		const float old_K = (m_fAngleStart + m_fAngleOffset > 1.0f ? 1.0f : 0.0f);

		float old_start = (m_fAngleStart + m_fAngleOffset - old_K) * 2 * PI;
		float old_end = (m_fAngleEnd + m_fAngleOffset - old_K) * 2 * PI;
		float old_value = (old_end - old_start) * m_fValue + old_start;

		const float new_K = (newAngleStart + m_fAngleOffset > 1.0f ? 1.0f : 0.0f);

		float new_start = (newAngleStart + m_fAngleOffset - new_K) * 2 * PI;
		float new_end = (newAngleEnd + m_fAngleOffset - new_K) * 2 * PI;
		float new_value = (new_end - new_start) * newValue + new_start;

		float changeBeg, changeEnd;

		if (new_start == old_start)
			changeBeg = std::min(old_value, new_value);
		else
			changeBeg = std::min(old_start, new_start);


		if (new_value == old_value)
			changeEnd = std::max(old_start, new_start);
		else
			changeEnd = std::max(old_value, new_value);

		// Apparently we need some margin because of the fade at the end...

		changeBeg -= 0.3;
		changeEnd += 0.3;

		//

		if (changeBeg < PI / 2)
		{
			if (changeEnd < PI / 2)
				return { fullRect.x, fullRect.y + fullRect.h / 2, fullRect.w / 2, fullRect.h / 2 };
			else if (changeEnd < PI)
				return { fullRect.x, fullRect.y, fullRect.w / 2, fullRect.h };
		}
		else if (changeBeg < PI)
		{
			if (changeEnd < PI)
				return { fullRect.x, fullRect.y, fullRect.w / 2, fullRect.h / 2 };
			else if (changeEnd < PI + PI / 2)
				return { fullRect.x, fullRect.y, fullRect.w, fullRect.h / 2 };
		}
		else if (changeBeg < PI + PI / 2)
		{
			if (changeEnd < PI + PI / 2)
				return { fullRect.x + fullRect.w / 2, fullRect.y, fullRect.w / 2, fullRect.h / 2 };
			else if (changeEnd < PI * 2)
				return { fullRect.x + fullRect.w / 2, fullRect.y, fullRect.w / 2, fullRect.h };
		}
		else if (changeBeg < PI * 2)
		{
			if (changeEnd < PI * 2)
				return { fullRect.x + fullRect.w / 2, fullRect.y + fullRect.h / 2, fullRect.w / 2, fullRect.h / 2 };
			else if (changeEnd < PI * 2 + PI / 2)
				return { fullRect.x, fullRect.y + fullRect.h / 2, fullRect.w, fullRect.h / 2 };
		}
	}
	return fullRect;
}


//____ SetAngles() __________________________________________________________
void WgKnob::SetAngles(float angleStart, float angleEnd)
{
    if((m_fAngleStart == angleStart) && (m_fAngleEnd == angleEnd))
        return;

	_myRequestRender( _calcUdateRect( angleStart, angleEnd, m_fValue ));

    m_fAngleStart = angleStart;
    m_fAngleEnd = angleEnd;
}

void WgKnob::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
    switch( pEvent->Type() )
    {
        case    WG_EVENT_MOUSE_ENTER:
            m_bPointerInside = true;
            _myRequestRender();
            break;

        case    WG_EVENT_MOUSE_LEAVE:
            m_bPointerInside = false;
            _myRequestRender();
            break;

        case WG_EVENT_MOUSEBUTTON_PRESS:
        {
            int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
            if( button == 1 )
                m_bPressed = true;

            _myRequestRender();
            break;
        }
        case WG_EVENT_MOUSEBUTTON_RELEASE:
        {
            int button = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->Button();
            if( button == 1 )
                m_bPressed = false;

            _myRequestRender();
            break;
        }
        default:
            break;

    }
    pHandler->ForwardEvent( pEvent );

}

//____ _onCloneContent() _______________________________________________________

void WgKnob::_onCloneContent( const WgWidget * _pOrg )
{
    const WgKnob * pOrg = static_cast<const WgKnob*>(_pOrg);
}

//____ _onNewSize() ___________________________________________________________

void WgKnob::_onNewSize(const WgSize& size)
{
	const int w = std::min(size.w, size.h);
	const int h = w;

	WgSize newSize(w, h);

	if (m_size != newSize )
	{
		delete m_pSurf;
		m_pSurf = m_pSurfaceFactory->CreateSurface(newSize*m_iOversampleX, WgPixelType::BGRA_8);
		m_size = newSize;
	}
	_myRequestRender();
}

//____ _renderPatches() _______________________________________________________

void WgKnob::_renderPatches(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{
	if (!m_pSurf)
		return;

	if( !m_backBufferDirtyRect.isEmpty() )
	{
		_redrawBackBuffer(WgRect({ 0, 0, m_size.w, m_size.h}, m_backBufferDirtyRect));
		m_backBufferDirtyRect.clear();
	}
	for (const WgRect * pRect = _pPatches->Begin(); pRect != _pPatches->End(); pRect++)
	{
		WgRect clip(_window, *pRect);
		if (clip.w > 0 && clip.h > 0)
			_onRender(pDevice, _canvas, _window, clip);
	}
}

//____ _onRender() _____________________________________________________________
void WgKnob::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
    pDevice->ClipBlit(_clip, m_pSurf,{0,0,m_size},_canvas.x,_canvas.y);
}

//____ _redrawBackBuffer() ____________________________________________________

void WgKnob::_redrawBackBuffer(WgRect region)
{
	int w = m_size.w;
	int h = m_size.h;

	// TODO:
	// Better calculation of angles, without atan and div for every pixel (polygon?)

	const float PI_HALF = PI / 2.0f;

	const int oversampling = m_iOversampleX;

	const float xd = 2.0f / (float)(w * oversampling);
	const float yd = 2.0f / (float)(h * oversampling);
	const float rd = 3.0f * 2.0f / (float)(w * oversampling); // should be sqrt(w+h);
	const float rd_inv = 1.0f / rd;

	const float r_outer = 1.0f - rd;
	const float r_inner = 1.0f - rd - m_iWidth;

	// Subtract 2π from the angles if the start angle is offsetted more than 2π.
	const float K = (m_fAngleStart + m_fAngleOffset > 1.0f ? 1.0f : 0.0f);

	const float a_start_const = (m_fAngleStart + m_fAngleOffset - K) * 2 * PI;
	const float a_end_const = (m_fAngleEnd + m_fAngleOffset - K) * 2 * PI;
	const float a_value_const = (a_end_const - a_start_const) * m_fValue + a_start_const;

	const int value_index = GetValueIndex();

	// Set up colors
	WgColor kForeground = m_kForeground;
	WgColor kBackground = m_kBackground;

	/*    if(m_bPressed)
	kForeground = Blend(m_kForeground, WgColor::white, 0.1f);
	else if(m_bPointerInside)
	kForeground = Blend(m_kForeground, WgColor::white, 0.5f);
	*/
	WgColor kBackTransp = m_kBackground;
	kBackTransp.a = 0;

	int background = (m_kBackground.b) | ((m_kBackground.g) << 8) | ((m_kBackground.r) << 16) | ((m_kBackground.a) << 24);
	int foreground = (kForeground.b) | ((kForeground.g) << 8) | ((kForeground.r) << 16) | ((kForeground.a) << 24);
	int backtransp = (kBackTransp.b) | ((kBackTransp.g) << 8) | ((kBackTransp.r) << 16) | ((kBackTransp.a) << 24);

	WgColor col;

	unsigned char* dest = (unsigned char*)m_pSurf->LockRegion(WgAccessMode::WriteOnly, region );

	float x = 0.0f, y = 0.0f;
	float y_inv = 0.0f;
	int color = 0;
	float r, in, out, weight, R, a;
	for (int yc = region.y; yc< region.y + region.h*oversampling; yc++)
	{
		// [-1, 1] coordinates
		y = (float)yc*yd - 1.0f;
		y_inv = 1.0f / std::max(std::abs(y), 0.0001f);

		unsigned int *ddest = (unsigned int *)(dest + m_pSurf->Pitch() * (yc-region.y));    // Don't forget about pitch, it will turn her into a bitch...
		for (int xc = region.x; xc<region.x + region.w*oversampling; xc++)
		{
			// [-1, 1] coordinates
			x = (float)xc*xd - 1.0f;

			// Radius
			r = x * x + y * y;

			// Reset color
			color = 0;

			// Calc boundaries for AA
			in = r - r_inner; // in > 0
			out = r_outer - r; // out > 0
			weight = 0.0f;

			R = 0.0f;
			a = 0.0f;

			const bool USE_ANGLE_AA = true;
            const float ANGLE_AA_WIDTH = 3.0f / (float)(w * oversampling); //0.05f;
            
			if ((in > -rd) && (out > -rd))
			{
				// Calculate anti-aliasing on the inner and outer circle diameters ("CIRCLE AA")
				if (in < 0 && in > -rd)
					weight = 1.0f + in * rd_inv;
				else if (out < 0 && out > -rd)
					weight = 1.0f + out * rd_inv;
				else
					weight = 1.0f;

				// Calculate angle.
				// We compare atan(-x/y) > a_start. Find equivalent -x/y > tan(a_start)?
				R = -x * y_inv;
				a = std::atan(R);

				if (y < 0.0f)
					a = PI - a;

				if (a < 0.0f)
					a += 2 * PI;

				if (a > 2 * PI)
					a -= 2 * PI;

				if (m_iNumSteps < 2 || m_iNumSteps > 11)
				{
					// Continuous version.
					// ========================================================================

					float a_start = -1.0f;
					float a_end = -1.0f;
					float a_value = -1.0f;

					// 1. Determine current state(s).
					DrawState CurrentDrawState;

					if (a_end_const > 2 * PI)
					{
						// Special case: end angle larger than 2π.
						a_start = a_start_const;
						a_end = a_end_const - 2 * PI;
						a_value = a_value_const;

						if (a_value > 2 * PI)
						{
							a_value -= 2 * PI;

							if (a < a_value)
							{
								CurrentDrawState = DrawState::eDrawForeground;

								if (USE_ANGLE_AA && (a >= a_value - ANGLE_AA_WIDTH))
								{
									CurrentDrawState = DrawState::eDrawForegroundAAEnd;
								}
								else
								{
									// Compensate for gradient calculation.
									a += 2 * PI;
									a_value += 2 * PI;
								}
							}
							else if ((a >= a_value) && (a < a_end))
							{
								CurrentDrawState = DrawState::eDrawBackground;

								if (USE_ANGLE_AA && (a >(a_end - ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
								}
							}
							else if ((a >= a_end) && (a < a_start))
							{
								CurrentDrawState = CurrentDrawState = DrawState::eDrawNothing;
							}
							else
							{
								CurrentDrawState = DrawState::eDrawForeground;

								if (USE_ANGLE_AA && (a < (a_start + ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawForegroundAABegin;
								}

								// Compensate for gradient calculation.
								a_value += 2 * PI;
							}
						}
						else
						{
							if (a < a_end)
							{
								CurrentDrawState = DrawState::eDrawBackground;

								if (USE_ANGLE_AA && (a >(a_end - ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
								}
							}
							else if (a < a_start)
							{
								CurrentDrawState = CurrentDrawState = DrawState::eDrawNothing;
							}
							else if (USE_ANGLE_AA && (a >= a_start) && (a < a_start + ANGLE_AA_WIDTH) && (a_value <= a_start + ANGLE_AA_WIDTH))
							{
								CurrentDrawState = DrawState::eDrawBackgroundAABegin;
							}
							else if ((a >= a_start) && (a < a_value - (USE_ANGLE_AA ? ANGLE_AA_WIDTH : 0.0f)))
							{
								CurrentDrawState = DrawState::eDrawForeground;

								if (USE_ANGLE_AA && (a < (a_start + ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawForegroundAABegin;
								}
							}
							else if (USE_ANGLE_AA && (a >= a_value - ANGLE_AA_WIDTH) && (a < a_value))
							{
								CurrentDrawState = DrawState::eDrawForegroundAAEnd;
							}
							else
							{
								CurrentDrawState = DrawState::eDrawBackground;
							}
						}
					}
					else
					{
						// Regular case.
						a_start = a_start_const;
						a_end = a_end_const;
						a_value = a_value_const;

						if (((a > a_start) && (a < a_value - (USE_ANGLE_AA ? ANGLE_AA_WIDTH : 0.0f))))
						{
							CurrentDrawState = DrawState::eDrawForeground;

							if (USE_ANGLE_AA && (a < (a_start + ANGLE_AA_WIDTH)))
							{
								CurrentDrawState = DrawState::eDrawForegroundAABegin;
							}
						}
						else if (USE_ANGLE_AA && (a >= a_start) && (a < a_start + ANGLE_AA_WIDTH) && (a_value <= a_start + ANGLE_AA_WIDTH))
						{
							CurrentDrawState = DrawState::eDrawBackgroundAABegin;
						}
						else if (USE_ANGLE_AA && (a >= a_value - ANGLE_AA_WIDTH) && (a < a_value) && (a_value > a_start + ANGLE_AA_WIDTH))
						{
							CurrentDrawState = DrawState::eDrawForegroundAAEnd;
						}
						else if ((a >= a_value) && (a < a_end))
						{
							CurrentDrawState = DrawState::eDrawBackground;

							if (USE_ANGLE_AA && (a >(a_end - ANGLE_AA_WIDTH)))
							{
								CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
							}
						}
						else
						{
							CurrentDrawState = DrawState::eDrawNothing;
						}
					}

					// 2. Draw current state.
					switch (CurrentDrawState)
					{
					case DrawState::eDrawNothing:
					{
						// Draw nothing.
						// -------------------------------------------------
						col = kBackTransp;
						break;
					}
					case DrawState::eDrawForegroundAABegin:
					{
						// Draw anti-alias of beginning of foreground.
						// -------------------------------------------------
						if (m_bPressed | m_bPointerInside)
							col = Blend(WgColor(255, 255, 255, 255), kForeground, (a - a_start) / (a_value - a_start));
						else
							col = kForeground;

						const float ww = (a - a_start) * (1.0f / ANGLE_AA_WIDTH);
						col = Blend(col, kBackTransp, ww);

						col = Blend(col, kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawForeground:
					{
						// Draw foreground.
						// -------------------------------------------------
						if (m_bPressed | m_bPointerInside)
							col = Blend(WgColor(255, 255, 255, 255), kForeground, (a - a_start) / (a_value - a_start));
						else
							col = kForeground;

						col = Blend(col, kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawForegroundAAEnd:
					{
						// Draw anti-alias of end of foreground.
						// -------------------------------------------------

						// This is an example of a quick and dirty anti-aliasing
						// that works pretty good.

						// 20 is 1/0.05, which is the angle that's being AAd.
						const float ww = (a - (a_value - ANGLE_AA_WIDTH)) * (1.0f / ANGLE_AA_WIDTH);

						if (m_bPressed | m_bPointerInside)
							col = WgColor(255, 255, 255, 255);
						else
							col = kForeground;

						col = Blend(m_kBackground, col, ww);
						col = Blend(col, kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawBackgroundAABegin:
					{
						// Draw anti-alias of beginning of background.
						// -------------------------------------------------
						const float ww = (a - a_start) * (1.0f / ANGLE_AA_WIDTH);
						col = Blend(m_kBackground, kBackTransp, ww);

						col = Blend(col, kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawBackground:
					{
						// Draw background.
						// -------------------------------------------------
						col = Blend(m_kBackground, kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawBackgroundAAEnd:
					{
						// Draw anti-alias of end of background.
						// -------------------------------------------------
						col = Blend(m_kBackground, kBackTransp, weight); // CIRCLE AA

						const float ww = (a_end - a) * (1.0f / ANGLE_AA_WIDTH);
						col = Blend(col, kBackTransp, ww);
						break;
					}
					}

				}
				else
				{
					// Discrete version.
					// ========================================================================

					bool colorize = false;
					WgColor set_col = m_kBackground;
					for (int i = 0; i<m_iNumSteps; i++)
					{
						if ((a > m_AngleStart[i] && (a < m_AngleEnd[i])))
						{
							colorize = true;
							if (value_index == i)
								set_col = kForeground;
						}
					}
					if (colorize)
						col = Blend(set_col, kBackTransp, weight);
					else
						col = kBackTransp;
				}

				color = (col.b) | ((col.g) << 8) | ((col.r) << 16) | ((col.a) << 24);
			}

			*ddest++ = color;
		}


	}

	m_pSurf->Unlock();

	// Downsample. Oversampling is not used.
	//    _downsample(m_pSurf, m_iOversampleX);

}



WgColor WgKnob::Blend( const WgColor& start, const WgColor& dest, float grade )
{
    WgColor col;

    const unsigned int g = (unsigned int)(grade * 255.0f);
    const unsigned int ig = 255 - g;
    
    // Yes, it's backwards. Sue me.
    /*
    col.r = (Uint8)( (float)start.r * grade + (1.0f-grade) * (float)dest.r );
    col.g = (Uint8)( (float)start.g * grade + (1.0f-grade) * (float)dest.g );
    col.b = (Uint8)( (float)start.b * grade + (1.0f-grade) * (float)dest.b );
    col.a = (Uint8)( (float)start.a * grade + (1.0f-grade) * (float)dest.a );
    */

    col.r = (start.r * g + ig * dest.r) >> 8;
    col.g = (start.g * g + ig * dest.g) >> 8;
    col.b = (start.b * g + ig * dest.b) >> 8;
    col.a = (start.a * g + ig * dest.a) >> 8;
    
    return col;
}

void WgKnob::_downsample(WgSurface* pSurf, const int oversample)
{
    if(oversample == 1)
        return;

    int w = pSurf->PixelSize().w; //Width();
    int h = pSurf->PixelSize().h; //Height();
    unsigned int col = (255) | (2<<8) | (1<<16) | (192<<24);

    unsigned int* data = (unsigned int*)pSurf->Lock(WgAccessMode::ReadWrite);
    int i=0, j=0;

    // Loop over small size
    for(int y=0; y<h/oversample; y++)
    {
        for(int x=0; x<w/oversample; x++)
        {
            i = y*w+x; // every pixel
            j = y*oversample*w + x*oversample; // every other pixel

            data[i] = 0; // clear pixel. Wrong for (0,0).
            uint8_t* qd = (uint8_t*)(&(data[i]));

            for(int ys=0; ys<oversample;ys++)
            {
                for(int xs=0; xs<oversample;xs++)
                {
                    uint8_t* qs = (uint8_t*)(&(data[j + xs + ys*w]));
                    qd[0] += qs[0]>>oversample;
                    qd[1] += qs[1]>>oversample;
                    qd[2] += qs[2]>>oversample;
                    qd[3] += qs[3]>>oversample;
                }
            }
        }
    }

    pSurf->Unlock();
}

//____ _onAlphaTest() ___________________________________________________________

bool WgKnob::_onAlphaTest( const WgCoord& ofs )
{
    return true;
}


//____ _onEnable() _____________________________________________________________

void WgKnob::_onEnable()
{
    _myRequestRender();
}

//____ _onDisable() ____________________________________________________________

void WgKnob::_onDisable()
{
    _myRequestRender();
}

void WgKnob::drawCircle(const int centerX, const int centerY, const float radX, const float radY)
{
    const float rx2 = radX*radX;
    const float ry2 = radY*radY;
    int quarter;

    quarter = (int) std::round(rx2 / sqrtf(rx2 + ry2));
    for (int x=0; x<=quarter; x++) {
        float y = radY * sqrtf(1.f-x*x/rx2);
        float e = y - floor(y);
        plot4(centerX, centerY, x, (int) -floor(y), e);
        plot4(centerX, centerY, x, (int) -floor(y)+1, 1.f-e);
    }

    quarter = (int) std::round(ry2 / sqrtf(rx2 + ry2));
    for (int y=0; y<=quarter; y++) {
        float x = radX * sqrtf(1.f-y*y/ry2);
        float e = x - floor(x);
        plot4(centerX, centerY, (int) -floor(x),   y, e);
        plot4(centerX, centerY, (int) -floor(x)+1, y, 1.f-e);
    }
}

// Xiaolin Wu's line algorithm
void WgKnob::drawLine(float x0, float y0, float x1, float y1)
{
    float dx;
    float dy;
    float intery, gradient;
    int xpxl1, xpxl2;
    int ypxl1, ypxl2;
    float xgap;
    float xend;
    float yend;

    bool steep = fabsf(y1 - y0) > fabs(x1 - x0);

    if(steep) {
        std::swap(x0,y0);
        std::swap(x1,y1);
    }

    if (x0>x1) {
        std::swap(x0,x1);
        std::swap(y0,y1);
    }

    dx = (float)x1 - (float)x0;
    dy = (float)y1 - (float)y0;
    gradient = dy/dx;

    xend = std::round(x0);
    yend = y0 + gradient * (xend - x0);
    xgap = rfpart(x0 + 0.5f);
    xpxl1 = (int) xend;
    ypxl1 = ipart(yend);

    if (steep) {
        plot(ypxl1    , xpxl1, rfpart(yend) * xgap);
        plot(ypxl1 + 1, xpxl1,  fpart(yend) * xgap);
    } else {
        plot(xpxl1, ypxl1,     rfpart(yend) * xgap);
        plot(xpxl1, ypxl1 + 1,  fpart(yend) * xgap);
    }
    intery = yend + gradient;

    // Second end point
    xend = std::round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    xpxl2 = (int) xend;
    ypxl2 = ipart(yend);

    if (steep) {
        plot(ypxl2    , xpxl2, rfpart(yend) * xgap);
        plot(ypxl2 + 1, xpxl2,  fpart(yend) * xgap);
    } else {
        plot(xpxl2, ypxl2,     rfpart(yend) * xgap);
        plot(xpxl2, ypxl2 + 1,  fpart(yend) * xgap);
    }

    for (int x=xpxl1+1; x<xpxl2; x++) {
        if (steep) {
            plot(ipart(intery)    , x, rfpart(intery));
            plot(ipart(intery) + 1, x,  fpart(intery));
        } else {
            plot(x, ipart (intery),  rfpart(intery));
            plot(x, ipart (intery)+1, fpart(intery));
        }
        intery = intery + gradient;
    }

    return;
}


void WgKnob::plot(const int x, const int y, const float alpha)
{
    m_pAAPix[m_iNextPixel] = WgCoord(x, y);
    m_pAACol[m_iNextPixel] = m_lineColor;
    m_pAACol[m_iNextPixel].a = (Uint8)std::round((float)m_lineColor.a * alpha);

    ++m_iNextPixel;

//    DBG_ASSERT(m_iNextPixel < WG_KNOB_PIXEL_BUFFER_SIZE);
}

void WgKnob::plot4(const int centerX, const int centerY, const int deltaX, const int deltaY, const float alpha)
{
    plot(centerX+deltaX, centerY+deltaY, alpha);
    plot(centerX-deltaX, centerY+deltaY, alpha);
    plot(centerX+deltaX, centerY-deltaY, alpha);
    plot(centerX-deltaX, centerY-deltaY, alpha);
}

//____ _myRequestRender() _____________________________________________________

void  WgKnob::_myRequestRender()
{
	m_backBufferDirtyRect = { 0,0, 1000000, 1000000 };
	_requestRender();
}

void  WgKnob::_myRequestRender(const WgRect& rect)
{
	if (m_backBufferDirtyRect.isEmpty())
		m_backBufferDirtyRect = rect;
	else
		m_backBufferDirtyRect.growToContain( rect );
//	_requestRender();
	_requestRender(rect);
}
