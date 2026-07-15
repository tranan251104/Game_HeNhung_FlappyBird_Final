#include <gui/screen2_screen/Screen2View.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/Color.hpp>
#include <texts/TextKeysAndLanguages.hpp>

extern "C" {
    extern uint8_t screenNumber;
    extern uint16_t score;
    extern uint16_t topScore;
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();
    screenNumber = 2;

    touchgfx::Unicode::snprintf(endScoreTextBuffer, ENDSCORETEXT_SIZE, "%u", (unsigned)score);
    endScoreText.setTypedText(touchgfx::TypedText(T___SINGLEUSE_IPTN));
    endScoreText.setWildcard(endScoreTextBuffer);
    endScoreText.resizeToCurrentText();
    endScoreText.invalidate();

    touchgfx::Unicode::snprintf(topScoreTextBuffer, TOPSCORETEXT_SIZE, "%u", (unsigned)topScore);
    topScoreText.setTypedText(touchgfx::TypedText(T___SINGLEUSE_YE2B));
    topScoreText.setWildcard(topScoreTextBuffer);
    topScoreText.resizeToCurrentText();
    topScoreText.invalidate();

    // Replace BACK text with a left-pointing triangle icon.
    exitText.setVisible(false);
    exitText.invalidate();

    const int16_t iconW = 22;
    const int16_t iconH = 22;
    const int16_t iconX = exitBox.getX() + (exitBox.getWidth() - iconW) / 2;
    const int16_t iconY = exitBox.getY() + (exitBox.getHeight() - iconH) / 2;

    backIcon.setPosition(iconX, iconY, iconW, iconH);
    backIcon.setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
    add(backIcon);
}

void Screen2View::handleClickEvent(const touchgfx::ClickEvent& evt)
{
    if (evt.getType() != touchgfx::ClickEvent::RELEASED &&
        evt.getType() != touchgfx::ClickEvent::PRESSED) {
        return;
    }

    const int16_t x = evt.getX();
    const int16_t y = evt.getY();
    const int16_t w = 240;
    const int16_t h = 320;
    // Exit button bounds.
    const int16_t bx = exitBox.getX();
    const int16_t by = exitBox.getY();
    const int16_t bw = exitBox.getWidth();
    const int16_t bh = exitBox.getHeight();

    auto inBox = [&](int16_t px, int16_t py) {
        return px >= bx && px < (bx + bw) && py >= by && py < (by + bh);
    };

    if (inBox(x, y) ||
        inBox(y, x) ||
        inBox(w - 1 - x, y) ||
        inBox(x, h - 1 - y) ||
        inBox(w - 1 - y, x) ||
        inBox(y, h - 1 - x)) {
        application().gotoScreen3ScreenNoTransition();
    }
}

void Screen2View::handleTickEvent()
{
    // Override base behavior to keep our score values.
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}
