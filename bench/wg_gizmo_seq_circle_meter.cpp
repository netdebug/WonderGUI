/*
Contains all definitions for the member functions of class WgGizmoSeqCircleMeter.

Author: Filip Thunstr�m
Created: 2018-10-29

NOTES:
*Have to see how this works on Mac with Retina resolution. Retina version of WonderGUI seems to exist.

*m_rescale should be equal to 2. Its purpose was to m_rescale size of the highlight circles, but the value is changed the highlight circles will not be centered.
*/

#include "wg_gizmo_seq_circle_meter.h"
#include <wg_gfxdevice.h>
#include <wg_eventhandler.h>
#include <iterator>
#include <cmath>

static const char	c_widgetType[] = { "SeqCircleMeter" };

double PI = 3.14159265358979323846264338327;

using namespace std;

//__Default constructor _____________________________
WgGizmoSeqCircleMeter::WgGizmoSeqCircleMeter() :
    WgGizmoSeqCircleMeter(-1)
{

}

//__ Constructor ___________________
WgGizmoSeqCircleMeter::WgGizmoSeqCircleMeter(int id) :
    m_meter_id(id),
    m_beats({ 1, 1, 1, 1 }),
    m_delta_phi((2 * PI) / m_beats.size()),
    m_main_ring_width(2),
    m_main_ring_radius(250),
    m_main_ring_color(WgColor::whitesmoke),
    m_main_fill_color(WgColor::transparent),
    m_beat_ring_width(3),
    m_beat_ring_color(WgColor::white),
    m_beat_fill_color(WgColor::mediumvioletred),
    m_beat_ring_radius(m_main_ring_radius / 8), //8 is an arbitrary value which seems to work well.
    m_play_circle_width(3),
    m_play_circle_color(WgColor::white),
    m_play_fill_color(WgColor::white),
    m_play_circle_pos(0),
    m_rescale(2),
    m_highlight_circle_width(5),
    m_highlight_circle_color(WgColor::white),
    m_highlight_fill_color(WgColor::transparent),
    m_highlight_release(1.0f),
    m_highlight_circle_max_radius(m_beat_ring_radius*m_rescale),
    m_changed_size(true),
    m_x_play_center_render(m_main_ring_radius - m_beat_ring_radius + m_beat_ring_radius / 4.0f),
    m_y_play_center_render(m_x_play_center_render),
    m_x_high_center_render(m_main_ring_radius - m_highlight_circle_radius / 2.0f - m_beat_ring_radius / 2.0f),
    m_y_high_center_render(m_x_high_center_render)
{
    _startReceiveTicks();
}


//__ SetBeats ___________________
void WgGizmoSeqCircleMeter::SetBeats(vector<int> & beats)
{
    m_beats = beats;
    m_delta_phi = (2 * PI) / m_beats.size();
}


//_________ SetValue __________________
void WgGizmoSeqCircleMeter::SetValue(float val)
{
    if (val == 1.0f)
    {
        m_play_circle_pos = 0.0f;
    }
    else if (val > 1.0f) //return error?
    {
        m_play_circle_pos = val - 1.0f;
    }
    else
    {
        m_play_circle_pos = val;
    }
}


//__________ HighLight ______________
//Calculates the scaling factor for highlighting the beat circles as the play circle passes over them. 
//Also updates the members m_previous_beat, m_next_beat and m_halfway_to_next_beat.
//
float WgGizmoSeqCircleMeter::HighLightFactor()
{
    float circle_pos_beat;

    if (m_play_circle_pos != 1.0f)
    {
        circle_pos_beat = m_play_circle_pos*(float)m_beats.size();
    }

    m_previous_beat = floor(circle_pos_beat);
    m_halfway_to_next_beat = m_previous_beat + 0.5f;
    m_next_beat = m_previous_beat + 1.0f;

    if (m_next_beat > m_beats.size() - 1) //If completed one lap
    {
        m_next_beat = 0.0f;
    }

    float fade_factor = 1.0f;

    if (m_beats[(int)m_previous_beat] && circle_pos_beat < m_halfway_to_next_beat) //For fade-out.
    {
        m_highlight_beat = m_previous_beat;
        fade_factor = (circle_pos_beat - m_previous_beat) / (m_highlight_release*0.6f); //0.6 = 0.5 + 0.1, where 0.5 is the total distance travelled in terms of beats. Makes the fadeout a bit nicer than if it ends at precisely 0.5.		
    }
    else if (m_next_beat == 0.0f && m_beats[(int)m_next_beat] && m_play_circle_pos*2.0f*PI*m_main_ring_radius >= 2.0f*PI*m_main_ring_radius - m_beat_ring_radius) //Fade-in
    {
        m_highlight_beat = m_next_beat;
        float distance_to_next_beat = 2.0f*PI*m_main_ring_radius - m_play_circle_pos*2.0f*PI*m_main_ring_radius;

        fade_factor = distance_to_next_beat / m_beat_ring_radius;
    }
    else if (m_next_beat != 0.0f && m_beats[(int)m_next_beat] && m_play_circle_pos*2.0f*PI*m_main_ring_radius >= m_next_beat*2.0f*PI*m_main_ring_radius / m_beats.size() - m_beat_ring_radius)//Fade-in
    {
        m_highlight_beat = m_next_beat;
        float distance_to_next_beat = m_next_beat*2.0f*PI*m_main_ring_radius / m_beats.size() - m_play_circle_pos*2.0f*PI*m_main_ring_radius; //In terms of length units		

        fade_factor = distance_to_next_beat / m_beat_ring_radius;
    }

    return 1.0f - fade_factor;
}


//___________ Type ________________
const char * WgGizmoSeqCircleMeter::Type(void) const
{
    return c_widgetType;
}



//___________ PreferredPixelsize ______________
WgSize WgGizmoSeqCircleMeter::PreferredPixelSize() const
{
    return WgSize(m_main_ring_radius * 2, m_main_ring_radius * 2);
}

//______ _RenderAll ________________________ //Placeholder function until I've spoken to Tord. Maybe there is a better way.
void WgGizmoSeqCircleMeter::_RenderAll()
{
    _requestRender();
}

//___________ _onEvent ___________
void WgGizmoSeqCircleMeter::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
    switch (pEvent->Type())
		case WG_EVENT_TICK:
    {
        SetValue(m_play_circle_pos += 0.002f);

        float m_x_play_center_render = m_main_ring_radius - m_beat_ring_radius + m_beat_ring_radius / 4.0f;
        float m_y_play_center_render = m_x_play_center_render;
        float m_x_high_center_render = m_main_ring_radius - m_highlight_circle_radius / 2.0f - m_beat_ring_radius / 2.0f;
        float m_y_high_center_render = m_x_high_center_render;

        if (m_changed_size)
        {
            if (PixelSize().w < PixelSize().h) //Adjust y
            {
                m_main_ring_radius = PixelSize().w / 2;
                m_beat_ring_radius = m_main_ring_radius / 8;
                m_highlight_circle_max_radius = m_beat_ring_radius*m_rescale;
                m_y_play_center_render += PixelSize().h / 2 - m_main_ring_radius;
                m_y_high_center_render += PixelSize().h / 2 - m_main_ring_radius;
            }
            else //Adjust x
            {
                m_main_ring_radius = PixelSize().h / 2;
                m_beat_ring_radius = m_main_ring_radius / 8;
                m_highlight_circle_max_radius = m_beat_ring_radius*m_rescale;
                m_x_play_center_render += PixelSize().w / 2 - m_main_ring_radius;
                m_x_high_center_render += PixelSize().w / 2 - m_main_ring_radius;
            }
        }

        m_x_play_current = m_x_play_center_render + (m_main_ring_radius - m_highlight_circle_max_radius - m_beat_ring_radius / 8)*std::cos(m_phi_play);
        m_y_play_current = m_y_play_center_render + (m_main_ring_radius - m_highlight_circle_max_radius - m_beat_ring_radius / 8)*std::sin(m_phi_play);

        int play_size = m_beat_ring_radius + (int)m_beat_ring_radius / 2;
        WgRect play_clip_current = { m_x_play_current, m_y_play_current, play_size, play_size };

        m_x_high_current = m_x_high_center_render + (m_main_ring_radius - m_highlight_circle_max_radius - m_beat_ring_radius / 8)*std::cos(m_phi_highlight);
        m_y_high_current = m_y_high_center_render + (m_main_ring_radius - m_highlight_circle_max_radius - m_beat_ring_radius / 8)*std::sin(m_phi_highlight);

        if (m_highlight_scalefactor != 0) //If the highlight circle should be rendered. Problem here
        {
            int highlight_size_current = m_highlight_circle_radius + m_beat_ring_radius;
            WgRect high_clip_current = { m_x_high_current, m_y_high_current, highlight_size_current, highlight_size_current};
            _requestRender(high_clip_current);
        }
        _requestRender(play_clip_current); //And problem here
    }
}








//______ _onRender ________________________
//Polar coordinates are used. 
void WgGizmoSeqCircleMeter::_onRender(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip)
{
    //SetValue(0.5f);
    //-------------------------
    //Draws the main circle.
    //-------------------------
    double phi_0 = -PI / 2.0; //Needed to set the first circle at 12 o'clock.

    if (m_changed_size)
    {
        if (_canvas.h > _canvas.w)  //If the _canvas is resized, we need to adjust for height/width ratios != 1.
        {
            CircleLimits.y += _canvas.h / 2 - m_main_ring_radius;
            CircleLimits.h = _canvas.w - m_highlight_circle_max_radius * 2;
        }
        else // h < w
        {
            CircleLimits.x += _canvas.w / 2 - m_main_ring_radius;
            CircleLimits.w = _canvas.h - m_highlight_circle_max_radius * 2;
        }

        CircleLimits.x = _canvas.x + m_highlight_circle_max_radius;
        CircleLimits.y = _canvas.y + m_highlight_circle_max_radius;
        CircleLimits.w = _canvas.w - m_highlight_circle_max_radius * 2;
        CircleLimits.h = _canvas.h - m_highlight_circle_max_radius * 2;

        MainCircleCenter.first = CircleLimits.x + CircleLimits.w / 2 - m_beat_ring_radius / 2;
        MainCircleCenter.second = CircleLimits.y + CircleLimits.h / 2 - m_beat_ring_radius / 2;
    }
    //pDevice->ClipDrawElipse(_clip, CircleLimits, m_main_ring_width * 200, m_main_fill_color, m_main_ring_width, m_main_ring_color);
    pDevice->ClipDrawElipse(_clip, CircleLimits, m_main_ring_width * 200, m_main_fill_color, m_main_ring_width, m_main_ring_color);

    //---------------------------
    //Drawing the beat circles. 
    //---------------------------
    if (!m_beats.empty())
    {
        for (auto it = m_beats.begin(); it != m_beats.end(); it++)
        {
            if (*it) //If a beat circle should be drawn at this position.
            {
                double phi = m_delta_phi*distance(m_beats.begin(), it) + phi_0;

                int x_pos = MainCircleCenter.first + (m_main_ring_radius - m_highlight_circle_max_radius - m_main_ring_width / 2)*std::cos(phi);
                int y_pos = MainCircleCenter.second + (m_main_ring_radius - m_highlight_circle_max_radius - m_main_ring_width / 2)*std::sin(phi);

                WgRect new_pos = { x_pos, y_pos, m_beat_ring_radius, m_beat_ring_radius };
               // pDevice->ClipDrawElipse(_clip, new_pos, m_beat_ring_width * 200, m_beat_fill_color, m_beat_ring_width, m_beat_ring_color);

                pDevice->ClipDrawElipse(_clip, new_pos, m_beat_ring_width*200, m_beat_fill_color, m_beat_ring_width, m_beat_ring_color);
            }
        }
    }

    //----------------------------------------------------------------------------------------
    //Draw a highlight circle outside a beat circle which is close enough to the play circle.
    //---------------------------------------------------------------------------------------

    m_highlight_scalefactor = m_rescale* HighLightFactor(); //Between 0 and 1. Multiply this value in order to change the size of the highlight circle! Issue: blinkar n�r fadear ut, med antalet som man multiplicerar HighLightFactor med...

    if (m_highlight_scalefactor != 0) //If the scale_factor should be drawn.
    {
        m_highlight_circle_radius_old = m_highlight_circle_radius;
        m_highlight_circle_radius = m_highlight_circle_max_radius * m_highlight_scalefactor;
        m_highlight_circle_color.a = 255 * m_highlight_scalefactor / m_rescale; //The opacity of the highlight circle.

                                                                                //The angle of the beat circle that should be highlighted.
        m_phi_highlight = m_delta_phi*m_highlight_beat + phi_0;

        m_x_highlight_draw = MainCircleCenter.first + (m_main_ring_radius - m_highlight_circle_max_radius - m_main_ring_width / 2)*std::cos(m_phi_highlight) - (m_highlight_circle_radius) / 4 + (1 - m_highlight_scalefactor)*m_highlight_circle_max_radius / 4;
        m_y_highlight_draw = MainCircleCenter.second + (m_main_ring_radius - m_highlight_circle_max_radius - m_main_ring_width / 2)*std::sin(m_phi_highlight) - (m_highlight_circle_radius) / 4 + (1 - m_highlight_scalefactor)*m_highlight_circle_max_radius / 4;

        WgRect highlight_pos = { m_x_highlight_draw, m_y_highlight_draw, m_highlight_circle_radius, m_highlight_circle_radius };
        pDevice->ClipDrawElipse(_clip, highlight_pos, m_highlight_circle_width * 100, m_highlight_fill_color, m_highlight_circle_width, m_highlight_circle_color);
    }
    //------------------------------
    //Drawing the play circle. 
    //------------------------------
    m_phi_play = m_play_circle_pos * 2 * PI + phi_0; //The current angle phi at which the play circle will be located.

    m_x_play_draw = MainCircleCenter.first + (m_main_ring_radius - m_highlight_circle_max_radius - m_main_ring_width / 2)*std::cos(m_phi_play);
    m_y_play_draw = MainCircleCenter.second + (m_main_ring_radius - m_highlight_circle_max_radius - m_main_ring_width / 2)*std::sin(m_phi_play);

    WgRect play_pos = { m_x_play_draw, m_y_play_draw, m_beat_ring_radius, m_beat_ring_radius };
    pDevice->ClipDrawElipse(_clip, play_pos, m_play_circle_width * 200.0f, m_play_fill_color, m_play_circle_width, m_play_circle_color);
}



//_____ _onCloneContent ____________
void WgGizmoSeqCircleMeter::_onCloneContent(const WgWidget * _pOrg)
{

}




//_______ _onAlphaTest ______________
bool WgGizmoSeqCircleMeter::_onAlphaTest(const WgCoord& ofs)
{
    return true;
}



//_________ _onNewSize ______________
void WgGizmoSeqCircleMeter::_onNewSize(const WgSize& size)
{
    if (m_previous_size == size)
    {
        m_changed_size = false;
    }
    else
    {
        m_changed_size = true;
    }
}
