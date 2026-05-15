#ifndef SCREENVIEW_HPP
#define SCREENVIEW_HPP

#include <gui_generated/screen_screen/screenViewBase.hpp>
#include <gui/screen_screen/screenPresenter.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/canvas/Circle.hpp>
#include <touchgfx/widgets/canvas/PainterRGB565.hpp>

class screenView : public screenViewBase
{
public:
    screenView();
    virtual ~screenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

    /* Called by the Presenter when a fresh sensor sample (mV) arrives
     * from the Model -- drives the waveform strip and the battery arc. */
    void updateSensor(uint32_t mv);
protected:
    /* Dashboard zone layout on a 480x320 canvas:
     *   top-left  : orbiting "sensor" box (uses inherited box1)
     *   top-right : battery indicator (Circle arc + fill)
     *   bottom    : scrolling waveform bars
     *   far-right : "settings" placeholder button
     */
    /* Bench-found: TouchGFX 4.26.1 has a per-Container child-count
     * threshold somewhere between 40 and 50 above which the render path
     * silently fails (whole screen stays grey).  40 is comfortably
     * under that and still gives a nice-looking waveform.  Bumping the
     * bar width to 8 px so the strip still spans a useful 320 px. */
    static constexpr int WAVE_BARS       = 40;
    static constexpr int WAVE_X0         = 10;
    static constexpr int WAVE_Y0         = 200;
    static constexpr int WAVE_BAR_W      = 8;
    static constexpr int WAVE_BAR_H_MAX  = 90;

    /* Animation state. */
    uint16_t phase;                       /* orbit phase 0..ORBIT_PERIOD */
    int      batteryLevel;                /* 0..100 (now driven by sensor mV) */
    uint8_t  waveHeights[WAVE_BARS];      /* current bar heights */

    /* Widgets. */
    touchgfx::Circle        batteryRing;       /* outer arc (track) */
    touchgfx::Circle        batteryArc;        /* fill arc (level) */
    touchgfx::PainterRGB565 ringPainter;
    touchgfx::PainterRGB565 arcPainter;
    touchgfx::Box    waveBars[WAVE_BARS];
    touchgfx::Box    settingsBtn;
    touchgfx::Box    settingsBtnAccent;
};

#endif // SCREENVIEW_HPP
