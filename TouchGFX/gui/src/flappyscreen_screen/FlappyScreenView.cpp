#include <gui/flappyscreen_screen/FlappyScreenView.hpp>
#include <touchgfx/Unicode.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "main.h"

extern "C" {
    extern uint8_t screenNumber;
    extern uint16_t score;
    extern uint16_t topScore;
    void SaveTopScoreToFlash(uint16_t newScore);
}

static uint32_t lcgNext(uint32_t state)
{
    return state * 1103515245u + 12345u;
}

static int16_t clamp16(int16_t v, int16_t lo, int16_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static bool shouldUseChallengePipe(uint16_t score, uint32_t state)
{
    return (score >= 6U) && (((state >> 24) % 4U) == 0U);
}

FlappyScreenView::FlappyScreenView()
{
}

void FlappyScreenView::setupScreen()
{
    FlappyScreenViewBase::setupScreen();

    screenNumber = 1;
    gameScore = 0;
    gameRunning = false;
    isPaused = false;
    isDying = false;

    bird.setWidth(24);
    bird.setHeight(24);
    updateScoreText();

    wallUp[0] = &wallUp1;
    wallUp[1] = &wallUp2;
    wallUp[2] = &wallUp3;
    wallUp[3] = &wallUp4;

    wallDown[0] = &wallDown1;
    wallDown[1] = &wallDown2;
    wallDown[2] = &wallDown3;
    wallDown[3] = &wallDown4;

    const int16_t screenH = 320;
    const int16_t gapSize = currentGapSize();
    const int16_t minGapTop = 30;
    const int16_t maxGapTop = screenH - gapSize - 30;
    const int16_t spacing = currentSpacing();
    const int16_t startX = 300;

    rngState = 0xA5A5A5A5u;

    for (int i = 0; i < 4; ++i) {
        pipeX[i] = startX + (i * spacing);
        pipePassed[i] = false;
        pipeChallenge[i] = false;

        if (i == 0) {
            pipeGapTopY[i] = (minGapTop + maxGapTop) / 2;
        } else {
            pipeGapTopY[i] = nextGapTopFrom(pipeGapTopY[i - 1]);
        }

        positionPipe(i, pipeGapTopY[i]);
    }

    birdY_fp = bird.getY() * 256;
    birdVel_fp = 0;
}

void FlappyScreenView::handleKeyEvent(uint8_t key)
{
    if (key == 'p' || key == 'P') {
        togglePause();
    }
}

void FlappyScreenView::handleClickEvent(const touchgfx::ClickEvent& evt)
{
    if (evt.getType() == touchgfx::ClickEvent::PRESSED) {
        flap();
    }
}

void FlappyScreenView::flap()
{
    if (isDying) {
        return;
    }

    if (!gameRunning) {
        gameRunning = true;
    }

    birdVel_fp = -800;
    GameFeedback_Play(35U, GAME_FEEDBACK_LED_GREEN);
}

void FlappyScreenView::handleTickEvent()
{
    if (GameFeedback_ConsumePauseRequest()) {
        togglePause();
    }

    if (isPaused) {
        return;
    }

    if (isDying) {
        const int32_t gravity_dead = 60;
        birdVel_fp += gravity_dead;
        birdY_fp += birdVel_fp;

        int16_t newY = (int16_t)(birdY_fp / 256);
        bird.moveTo(bird.getX(), newY);

        if (newY > 320) {
            endGame();
        }
        return;
    }

    if (!gameRunning) {
        return;
    }

    const int32_t gravity_fp = 45;
    const int16_t speed = currentPipeSpeed();
    const int16_t screenH = 320;
    const int16_t spacing = currentSpacing();

    birdVel_fp += gravity_fp;
    birdY_fp += birdVel_fp;
    int16_t newY = (int16_t)(birdY_fp / 256);

    if (newY < 0 || newY + bird.getHeight() > screenH) {
        startDyingSequence();
        return;
    }
    bird.moveTo(bird.getX(), newY);

    int16_t maxX = pipeX[0];
    int16_t maxIndex = 0;
    for (int i = 1; i < 4; ++i) {
        if (pipeX[i] > maxX) {
            maxX = pipeX[i];
            maxIndex = i;
        }
    }

    for (int i = 0; i < 4; ++i) {
        pipeX[i] -= speed;

        if (pipeX[i] + wallUp[i]->getWidth() < 0) {
            int prevIndex = maxIndex;
            pipeGapTopY[i] = nextGapTopFrom(pipeGapTopY[prevIndex]);
            pipeChallenge[i] = shouldUseChallengePipe(gameScore, rngState);

            pipeX[i] = maxX + spacing;
            pipePassed[i] = false;

            maxX = pipeX[i];
            maxIndex = i;
        }

        positionPipe(i, pipeGapTopY[i]);

        if (!pipePassed[i] && (pipeX[i] + wallUp[i]->getWidth() < bird.getX())) {
            pipePassed[i] = true;
            uint16_t awardedPoints = pipeChallenge[i] ? 2U : 1U;
            gameScore = (uint16_t)(gameScore + awardedPoints);
            GameFeedback_Play(pipeChallenge[i] ? 95U : 55U, GAME_FEEDBACK_LED_GREEN);
            updateScoreText();
        }

        int16_t bx = bird.getX() + 4;
        int16_t by = bird.getY() + 4;
        int16_t bw = bird.getWidth() - 8;
        int16_t bh = bird.getHeight() - 8;

        int16_t ux = wallUp[i]->getX();
        int16_t uy = wallUp[i]->getY();
        int16_t uw = wallUp[i]->getWidth();
        int16_t uh = wallUp[i]->getHeight();

        int16_t dx = wallDown[i]->getX();
        int16_t dy = wallDown[i]->getY();
        int16_t dw = wallDown[i]->getWidth();
        int16_t dh = wallDown[i]->getHeight();

        bool hitUp = (bx < ux + uw) && (bx + bw > ux) && (by < uy + uh) && (by + bh > uy);
        bool hitDown = (bx < dx + dw) && (bx + bw > dx) && (by < dy + dh) && (by + bh > dy);

        if (hitUp || hitDown) {
            startDyingSequence();
            return;
        }
    }
}

uint8_t FlappyScreenView::difficultyLevel() const
{
    uint16_t level = (uint16_t)(gameScore / 6U);
    return (uint8_t)((level > 3U) ? 3U : level);
}

int16_t FlappyScreenView::currentPipeSpeed() const
{
    return (int16_t)(2 + difficultyLevel());
}

int16_t FlappyScreenView::currentGapSize() const
{
    return (int16_t)(92 - (difficultyLevel() * 8));
}

int16_t FlappyScreenView::currentSpacing() const
{
    return (int16_t)(82 + (difficultyLevel() * 4));
}

void FlappyScreenView::togglePause()
{
    if (isDying || !gameRunning) {
        return;
    }

    isPaused = !isPaused;
    GameFeedback_Play(isPaused ? 120U : 55U, GAME_FEEDBACK_LED_GREEN | GAME_FEEDBACK_LED_RED);
}

int16_t FlappyScreenView::nextGapTopFrom(int16_t prevGapTop)
{
    const int16_t screenH = 320;
    const int16_t gapSize = currentGapSize();
    const int16_t minGapTop = 30;
    const int16_t maxGapTop = screenH - gapSize - 30;
    const int16_t deltaMax = (int16_t)(35 + (difficultyLevel() * 8));

    rngState = lcgNext(rngState);
    int16_t delta = (int16_t)((rngState >> 16) % (2 * deltaMax + 1)) - deltaMax;
    return clamp16((int16_t)(prevGapTop + delta), minGapTop, maxGapTop);
}

void FlappyScreenView::positionPipe(int index, int16_t gapTop)
{
    const int16_t screenH = 320;
    int16_t gapSize = currentGapSize();
    if (pipeChallenge[index]) {
        gapSize = clamp16((int16_t)(gapSize - 14), 58, gapSize);
    }
    int16_t gapBottom = gapTop + gapSize;

    int16_t upH = gapTop;
    int16_t downH = screenH - gapBottom;

    if (upH < 1) upH = 1;
    if (downH < 1) downH = 1;

    wallUp[index]->invalidate();
    wallDown[index]->invalidate();

    wallUp[index]->setPosition(pipeX[index], 0, wallUp[index]->getWidth(), upH);
    wallDown[index]->setPosition(pipeX[index], gapBottom, wallDown[index]->getWidth(), downH);

    wallUp[index]->invalidate();
    wallDown[index]->invalidate();
}

void FlappyScreenView::startDyingSequence()
{
    if (isDying) {
        return;
    }

    isPaused = false;
    isDying = true;
    birdVel_fp = -500;
    GameFeedback_Play(180U, GAME_FEEDBACK_LED_RED);
}

void FlappyScreenView::updateScoreText()
{
    touchgfx::Unicode::snprintf(scoreTextBuffer, SCORETEXT_SIZE, "%u", (unsigned)gameScore);
    scoreText.setTypedText(touchgfx::TypedText(T___SINGLEUSE_KDY8));
    scoreText.setWildcard(scoreTextBuffer);
    scoreText.resizeToCurrentText();
    scoreText.invalidate();
}

void FlappyScreenView::endGame()
{
    score = gameScore;
    if (gameScore > topScore) {
        topScore = gameScore;
        SaveTopScoreToFlash(topScore);
        GameFeedback_Play(160U, GAME_FEEDBACK_LED_GREEN | GAME_FEEDBACK_LED_RED);
    }
    gameRunning = false;
    isPaused = false;
    application().gotoScreen2ScreenNoTransition();
}

void FlappyScreenView::tearDownScreen()
{
    FlappyScreenViewBase::tearDownScreen();
}
