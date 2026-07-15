#include <gui/screen3_screen/Screen3View.hpp>
#include <touchgfx/Color.hpp>
#include <texts/TextKeysAndLanguages.hpp>

extern "C" {
    extern uint8_t screenNumber;
}

Screen3View::Screen3View()
{

}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    screenNumber = 3;

    // Reposition selection - centered and enlarged (240x320 screen)
    const int16_t screenW = 240;
    const int16_t boxW = 150;
    const int16_t boxH = 100;
    const int16_t boxX = (screenW - boxW) / 2;
    const int16_t boxY = 50;

    selectedProgramBox.setPosition(boxX, boxY, boxW, boxH);

    // Center icon in box (64x55)
    const int16_t iconW = 64;
    const int16_t iconH = 55;
    flappyBirdIcon.setPosition(boxX + (boxW - iconW) / 2, boxY + 5, iconW, iconH);

    // Center "Flappy Bird" text below icon
    textArea1.setTypedText(touchgfx::TypedText(T___SINGLEUSE_5GHL));
    textArea1.resizeToCurrentText();
    textArea1.setXY((screenW - textArea1.getWidth()) / 2, boxY + boxH - 20);

    // Hide option 2 (BACK) - keep only "Chơi game" / Play game option.
    textArea2.setVisible(false);
    textArea2.invalidate();
    cvIcon.setVisible(false);
    cvIcon.invalidate();

}

void Screen3View::handleClickEvent(const touchgfx::ClickEvent& evt)
{
    if (evt.getType() != touchgfx::ClickEvent::RELEASED) {
        return;
    }

    application().gotoFlappyScreenScreenNoTransition();
}

void Screen3View::handleTickEvent()
{
    // Override base behavior to prevent auto-transition to Screen4.
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}
