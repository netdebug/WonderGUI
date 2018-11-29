/*
Header file for class WgGizmoSeqCircleMeter.

Author: Filip Thunstr�m
Created: 2018-10-29
*/

#ifndef WG_WgGizmoSeqCircleMeter_DOT_H
#define WG_WgGizmoSeqCircleMeter_DOT_H

#ifndef WG_WIDGET_DOT_H 
#include <wg_widget.h>
#endif

#ifndef WG_COLOR_DOT_H
#include <wg_color.h>
#endif

#ifndef WG_GEO_DOT_H
#include <wg_geo.h>
#endif

#include <vector>
#include <math.h>

using namespace std;

//____WgGizmoSeqCircleMeter__________
class WgGizmoSeqCircleMeter : public WgWidget
{
public:
    WgGizmoSeqCircleMeter();
    WgGizmoSeqCircleMeter(int id);
    virtual ~WgGizmoSeqCircleMeter() = default;

    virtual const char *Type(void) const;
    virtual WgWidget * NewOfMyType() const { return new WgGizmoSeqCircleMeter(); }
    virtual WgSize PreferredPixelSize() const;

    //____Methods specific for this class______________________________________
    int GetMeterId() const { return m_meter_id; }
    void SetMeterId(int id) { m_meter_id = id; }

    void SetBeats(vector<int> & beats);
    void SetValue(float val);
    float HighLightFactor(); //Checks if a highlight circle should be displayed.

    float GetMainWidth() const { return m_main_ring_width; }
    void SetMainWidth(float val) { m_main_ring_width = val; }
    int GetMainRadius() const { return m_main_ring_radius; }
    void SetMainRadius(int val) { m_main_ring_radius = val; }
    WgColor GetMainRingColor() const { return m_main_ring_color; }
    void SetMainRingColor(const WgColor& color) { m_main_ring_color = color; }
    WgColor GetMainFillColor() const { return m_main_fill_color; }
    void SetMainFillColor(const WgColor& color) { m_main_fill_color = color; }

    float GetBeatWidth() const { return m_beat_ring_width; }
    void SetBeatWidth(float val) { m_beat_ring_width = val; }
    WgColor GetBeatRingColor() const { return m_beat_ring_color; }
    void SetBeatRingColor(const WgColor& color) { m_beat_ring_color = color; }
    WgColor GetBeatFillColor() const { return m_beat_fill_color; }
    void SetBeatFillColor(const WgColor& color) { m_beat_fill_color = color; }

    float GetPlayWidth() const { return m_play_circle_width; }
    void SetPlayWidth(float val) { m_play_circle_width = val; }
    WgColor GetPlayCircleColor() const { return m_play_circle_color; }
    void SetPlayCircleColor(const WgColor& color) { m_play_circle_color = color; }
    WgColor GetPlayFillColor() const { return m_play_fill_color; }
    void SetPlayFillColor(const WgColor& color) { m_play_fill_color = color; }

    float GetHighWidth() const { return m_highlight_circle_width; }
    void SetHighWidth(float val) { m_highlight_circle_width = val; }
    WgColor GetHighCircleColor() const { return m_highlight_circle_color; }
    void SetHighCircleColor(const WgColor& color) { m_highlight_circle_color = color; }
    WgColor GetHighFillColor() const { return m_highlight_fill_color; }
    void SetHighFillColor(const WgColor& color) { m_highlight_fill_color = color; }
    float GetRelease() const { return m_highlight_release; }
    void SetRelease(float val) { m_highlight_release = val; }

    void _RenderAll(); //Placeholder function until I've spoken to Tord.

protected:

    void _onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler);
    void _onRender(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip);
    void _onCloneContent(const WgWidget * _pOrg);
    bool _onAlphaTest(const WgCoord& ofs);
    void _onNewSize(const WgSize& size) override;

    //---------------------------
    //Specific members for this class.
    //---------------------------
    int m_meter_id;

    vector<int> m_beats; //Contains information about where circles should be drawn. 
    double m_delta_phi; //The angle difference between every pair of beat circles.

    float m_main_ring_width;
    int m_main_ring_radius;
    WgColor m_main_ring_color;
    WgColor m_main_fill_color;

    float m_beat_ring_width;
    WgColor m_beat_ring_color;
    WgColor m_beat_fill_color;
    int m_beat_ring_radius;

    float m_play_circle_width;
    WgColor m_play_circle_color;
    WgColor m_play_fill_color;
    float m_play_circle_pos; //Number between 0 and 1, the position of the play circle. Provided by DAW.

                             //Used for drawing and rendering the play- and the highlight circle. 
    float m_previous_beat;
    float m_next_beat;
    float m_halfway_to_next_beat;
    double m_phi_play; //The angle phi at which the play circle will be located.
    int m_x_play_draw;
    int m_y_play_draw;

    int m_rescale; //scales the higlight circle with regards to the beat circle radius
    float m_highlight_circle_width;
    int m_highlight_circle_radius;
    int m_highlight_circle_max_radius;
    WgColor m_highlight_circle_color;
    WgColor m_highlight_fill_color;
    float m_highlight_circle_angle;
    int m_x_highlight_draw;
    int m_y_highlight_draw;
    float m_phi_highlight;
    float m_highlight_beat; //Which beat is supposed to be highlighted
    float m_highlight_release; //Between 0 and 1.0. Lower value --> faster release.
    float m_highlight_scalefactor; //Scales the size and transparency of the highlight circle 

                                   //Should be saved, since these only change when the widget if rescaled....
    float m_x_play_center_render;
    float m_y_play_center_render;
    float m_x_high_center_render;
    float m_y_high_center_render;

    //For rendering. The "old" coordinates maybe not necessary, and if so none of these would be necessary.
    int m_x_play_old;
    int m_y_play_old;
    int m_x_play_current;
    int m_y_play_current;

    int m_x_high_old;
    int m_y_high_old;
    int m_x_high_current;
    int m_y_high_current;
    int m_highlight_circle_radius_old;

    //Tells us if the widget has been rescaled. Used in onNewSize().
    WgSize m_previous_size;
    bool m_changed_size; //True if the height/width ratio has changed.

    WgRect CircleLimits; //A WgRect which is smaller than the canvas, so that nothing will be drawn outside the _canvas.
    pair<int, int> MainCircleCenter; //The center of the main circle. Used as the origin for drawing the beat circles and the play circle.
};

#endif
