#include <gui/screen_screen/screenView.hpp>
#include <touchgfx/Color.hpp>
#include <cmath>

screenView::screenView()
    : phase(0)
    , batteryLevel(50)
{
    using namespace touchgfx;

    /* ---------------- Battery indicator (top-right) ---------------- */
    /* CanvasWidgets need (a) a bounding rect from Widget::setPosition()
     * large enough to contain the entire circle, (b) a Painter (not a
     * direct setColor), and (c) a CanvasWidgetRenderer scratch buffer
     * registered globally (done in TouchGFXHAL::initialize). */
    constexpr int CIRCLE_BOX     = 110;
    constexpr int CIRCLE_BOX_X   = 335;
    constexpr int CIRCLE_BOX_Y   = 15;
    constexpr int CIRCLE_CX      = CIRCLE_BOX / 2;
    constexpr int CIRCLE_CY      = CIRCLE_BOX / 2;
    constexpr int CIRCLE_RADIUS  = 50;

    ringPainter.setColor(Color::getColorFromRGB(40, 40, 50));
    batteryRing.setPosition(CIRCLE_BOX_X, CIRCLE_BOX_Y, CIRCLE_BOX, CIRCLE_BOX);
    batteryRing.setCenter(CIRCLE_CX, CIRCLE_CY);
    batteryRing.setRadius(CIRCLE_RADIUS);
    batteryRing.setLineWidth(10);
    batteryRing.setArc(0, 360);
    batteryRing.setPainter(ringPainter);
    add(batteryRing);

    arcPainter.setColor(Color::getColorFromRGB(0, 255, 0));
    batteryArc.setPosition(CIRCLE_BOX_X, CIRCLE_BOX_Y, CIRCLE_BOX, CIRCLE_BOX);
    batteryArc.setCenter(CIRCLE_CX, CIRCLE_CY);
    batteryArc.setRadius(CIRCLE_RADIUS);
    batteryArc.setLineWidth(10);
    batteryArc.setArc(0, (batteryLevel * 360) / 100);
    batteryArc.setPainter(arcPainter);
    add(batteryArc);

    /* ---------------- Waveform bars (bottom strip) ----------------- */
    for (int i = 0; i < WAVE_BARS; ++i) {
        waveHeights[i] = WAVE_BAR_H_MAX / 2;
        waveBars[i].setPosition(
            WAVE_X0 + i * WAVE_BAR_W,
            WAVE_Y0 + (WAVE_BAR_H_MAX - waveHeights[i]),
            WAVE_BAR_W - 1,
            waveHeights[i]);
        const uint8_t g = static_cast<uint8_t>(200 - (i * 150) / WAVE_BARS);
        waveBars[i].setColor(Color::getColorFromRGB(0, g, 255));
        add(waveBars[i]);
    }

    /* ---------------- "Settings" button (far right) ---------------- */
    settingsBtn.setPosition(400, 230, 70, 70);
    settingsBtn.setColor(Color::getColorFromRGB(40, 80, 160));
    add(settingsBtn);

    settingsBtnAccent.setPosition(420, 250, 30, 30);
    settingsBtnAccent.setColor(Color::getColorFromRGB(220, 220, 230));
    add(settingsBtnAccent);
}

void screenView::setupScreen()
{
    screenViewBase::setupScreen();
}

void screenView::tearDownScreen()
{
    screenViewBase::tearDownScreen();
}

void screenView::handleTickEvent()
{
    using namespace touchgfx;

    /* Self-driven orbit animation for box1 (no sensor input needed). */
    constexpr int   CX     = 120;
    constexpr int   CY     = 80;
    constexpr int   RAD    = 50;
    constexpr int   PERIOD = 180;                 /* 3 s @ 60 Hz */
    constexpr float TWO_PI = 6.2831853f;
    const float theta = (static_cast<float>(phase) / PERIOD) * TWO_PI;
    const int   bx    = CX + static_cast<int>(RAD * std::cos(theta));
    const int   by    = CY + static_cast<int>(RAD * std::sin(theta));
    box1.moveTo(bx, by);
    if (++phase >= PERIOD) phase = 0;
}

void screenView::updateSensor(uint32_t mv)
{
    using namespace touchgfx;

    /* Map mV (0..3300) -> bar height (0..WAVE_BAR_H_MAX). */
    const uint32_t clamped = (mv > 3300u) ? 3300u : mv;
    const uint8_t  sample  = static_cast<uint8_t>(
        (clamped * WAVE_BAR_H_MAX) / 3300u);

    /* Scroll history left, append on right. */
    for (int i = 0; i < WAVE_BARS - 1; ++i) {
        waveHeights[i] = waveHeights[i + 1];
    }
    waveHeights[WAVE_BARS - 1] = sample;

    for (int i = 0; i < WAVE_BARS; ++i) {
        const int h = waveHeights[i] + 1;
        waveBars[i].setPosition(
            WAVE_X0 + i * WAVE_BAR_W,
            WAVE_Y0 + (WAVE_BAR_H_MAX - h),
            WAVE_BAR_W - 1,
            h);
    }
    Rect strip(WAVE_X0,
               WAVE_Y0,
               WAVE_BARS * WAVE_BAR_W,
               WAVE_BAR_H_MAX + 1);
    invalidateRect(strip);

    /* Battery level = mV / 33 (0..100 %). */
    const int newLevel = static_cast<int>(clamped / 33u);
    if (newLevel != batteryLevel) {
        batteryLevel = newLevel;
        uint8_t r, g;
        if (batteryLevel < 33)      { r = 255; g = 40;  }
        else if (batteryLevel < 66) { r = 230; g = 200; }
        else                        { r = 30;  g = 220; }
        arcPainter.setColor(Color::getColorFromRGB(r, g, 30));
        batteryArc.updateArc(0, (batteryLevel * 360) / 100);
    }
}
