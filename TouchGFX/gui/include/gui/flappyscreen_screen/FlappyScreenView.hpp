#ifndef FLAPPYSCREENVIEW_HPP
#define FLAPPYSCREENVIEW_HPP

#include <gui_generated/flappyscreen_screen/FlappyScreenViewBase.hpp>
#include <gui/flappyscreen_screen/FlappyScreenPresenter.hpp>
#include <touchgfx/events/ClickEvent.hpp>

class FlappyScreenView : public FlappyScreenViewBase
{
public:
    FlappyScreenView();
    virtual ~FlappyScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleKeyEvent(uint8_t key);
    virtual void handleClickEvent(const touchgfx::ClickEvent& evt);
    virtual void handleTickEvent();

protected:
    touchgfx::ScalableImage* wallUp[4];
    touchgfx::ScalableImage* wallDown[4];
    int16_t pipeX[4];
    int16_t pipeGapTopY[4];
    bool pipePassed[4];
    bool pipeChallenge[4];

    int32_t birdY_fp;
    int32_t birdVel_fp;

    bool gameRunning;
    bool isPaused;
    bool isDying;
    uint16_t gameScore;
    uint32_t rngState;

    void updateScoreText();
    void endGame();
    void startDyingSequence();
    void togglePause();
    uint8_t difficultyLevel() const;
    int16_t currentPipeSpeed() const;
    int16_t currentGapSize() const;
    int16_t currentSpacing() const;

    void positionPipe(int index, int16_t gapTop);
    void flap();
    int16_t nextGapTopFrom(int16_t prevGapTop);
};

#endif // FLAPPYSCREENVIEW_HPP
