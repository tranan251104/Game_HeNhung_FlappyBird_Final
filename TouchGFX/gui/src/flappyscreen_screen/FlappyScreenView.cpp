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

static const int16_t SCREEN_HEIGHT = 320;
static const int16_t PIPE_COUNT = 4;
static const int16_t BIRD_SIZE = 24;
static const int16_t FIXED_POINT_SCALE = 256;
static const int16_t HITBOX_INSET = 4;
static const int16_t GAP_MARGIN = 30;
static const int16_t START_PIPE_X = 300;

static const int32_t GRAVITY_FP = 45;
static const int32_t DEATH_GRAVITY_FP = 60;
static const int32_t FLAP_VELOCITY_FP = -800;
static const int32_t DEATH_BOUNCE_VELOCITY_FP = -500;

static const uint16_t SCORE_PER_DIFFICULTY_LEVEL = 6U;
static const uint8_t MAX_DIFFICULTY_LEVEL = 3U;
static const uint32_t INITIAL_RNG_STATE = 0xA5A5A5A5u;

static const int16_t BASE_PIPE_SPEED = 2;
static const int16_t BASE_GAP_SIZE = 92;
static const int16_t GAP_SIZE_STEP = 8;
static const int16_t BASE_PIPE_SPACING = 82;
static const int16_t PIPE_SPACING_STEP = 4;
static const int16_t BASE_GAP_DELTA = 35;
static const int16_t GAP_DELTA_STEP = 8;
static const int16_t CHALLENGE_GAP_REDUCTION = 14;
static const int16_t MIN_CHALLENGE_GAP = 58;

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
    return (score >= SCORE_PER_DIFFICULTY_LEVEL) && (((state >> 24) % PIPE_COUNT) == 0U);
}

static bool rectanglesOverlap(int16_t ax, int16_t ay, int16_t aw, int16_t ah,
                              int16_t bx, int16_t by, int16_t bw, int16_t bh)
{
    return (ax < bx + bw) && (ax + aw > bx) && (ay < by + bh) && (ay + ah > by);
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

    bird.setWidth(BIRD_SIZE);
    bird.setHeight(BIRD_SIZE);
    updateScoreText();

    wallUp[0] = &wallUp1;
    wallUp[1] = &wallUp2;
    wallUp[2] = &wallUp3;
    wallUp[3] = &wallUp4;

    wallDown[0] = &wallDown1;
    wallDown[1] = &wallDown2;
    wallDown[2] = &wallDown3;
    wallDown[3] = &wallDown4;

    const int16_t gapSize = currentGapSize();
    const int16_t minGapTop = GAP_MARGIN;
    const int16_t maxGapTop = SCREEN_HEIGHT - gapSize - GAP_MARGIN;
    const int16_t spacing = currentSpacing();

    rngState = INITIAL_RNG_STATE;

    for (int i = 0; i < PIPE_COUNT; ++i) {
        pipeX[i] = START_PIPE_X + (i * spacing);
        pipePassed[i] = false;
        pipeChallenge[i] = false;

        if (i == 0) {
            pipeGapTopY[i] = (minGapTop + maxGapTop) / 2;
        } else {
            pipeGapTopY[i] = nextGapTopFrom(pipeGapTopY[i - 1]);
        }

        positionPipe(i, pipeGapTopY[i]);
    }

    birdY_fp = bird.getY() * FIXED_POINT_SCALE;
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

    birdVel_fp = FLAP_VELOCITY_FP;
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
        birdVel_fp += DEATH_GRAVITY_FP;
        birdY_fp += birdVel_fp;

        int16_t newY = (int16_t)(birdY_fp / FIXED_POINT_SCALE);
        bird.moveTo(bird.getX(), newY);

        if (newY > SCREEN_HEIGHT) {
            endGame();
        }
        return;
    }

    if (!gameRunning) {
        return;
    }

    const int16_t speed = currentPipeSpeed();
    const int16_t spacing = currentSpacing();

    birdVel_fp += GRAVITY_FP;
    birdY_fp += birdVel_fp;
    int16_t newY = (int16_t)(birdY_fp / FIXED_POINT_SCALE);

    if (newY < 0 || newY + bird.getHeight() > SCREEN_HEIGHT) {
        startDyingSequence();
        return;
    }
    bird.moveTo(bird.getX(), newY);

    int16_t maxX = pipeX[0];
    int16_t maxIndex = 0;
    for (int i = 1; i < PIPE_COUNT; ++i) {
        if (pipeX[i] > maxX) {
            maxX = pipeX[i];
            maxIndex = i;
        }
    }

    for (int i = 0; i < PIPE_COUNT; ++i) {
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

        int16_t bx = bird.getX() + HITBOX_INSET;
        int16_t by = bird.getY() + HITBOX_INSET;
        int16_t bw = bird.getWidth() - (HITBOX_INSET * 2);
        int16_t bh = bird.getHeight() - (HITBOX_INSET * 2);

        int16_t ux = wallUp[i]->getX();
        int16_t uy = wallUp[i]->getY();
        int16_t uw = wallUp[i]->getWidth();
        int16_t uh = wallUp[i]->getHeight();

        int16_t dx = wallDown[i]->getX();
        int16_t dy = wallDown[i]->getY();
        int16_t dw = wallDown[i]->getWidth();
        int16_t dh = wallDown[i]->getHeight();

        bool hitUp = rectanglesOverlap(bx, by, bw, bh, ux, uy, uw, uh);
        bool hitDown = rectanglesOverlap(bx, by, bw, bh, dx, dy, dw, dh);

        if (hitUp || hitDown) {
            startDyingSequence();
            return;
        }
    }
}

uint8_t FlappyScreenView::difficultyLevel() const
{
    uint16_t level = (uint16_t)(gameScore / SCORE_PER_DIFFICULTY_LEVEL);
    return (uint8_t)((level > MAX_DIFFICULTY_LEVEL) ? MAX_DIFFICULTY_LEVEL : level);
}

int16_t FlappyScreenView::currentPipeSpeed() const
{
    return (int16_t)(BASE_PIPE_SPEED + difficultyLevel());
}

int16_t FlappyScreenView::currentGapSize() const
{
    return (int16_t)(BASE_GAP_SIZE - (difficultyLevel() * GAP_SIZE_STEP));
}

int16_t FlappyScreenView::currentSpacing() const
{
    return (int16_t)(BASE_PIPE_SPACING + (difficultyLevel() * PIPE_SPACING_STEP));
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
    const int16_t gapSize = currentGapSize();
    const int16_t minGapTop = GAP_MARGIN;
    const int16_t maxGapTop = SCREEN_HEIGHT - gapSize - GAP_MARGIN;
    const int16_t deltaMax = (int16_t)(BASE_GAP_DELTA + (difficultyLevel() * GAP_DELTA_STEP));

    rngState = lcgNext(rngState);
    int16_t delta = (int16_t)((rngState >> 16) % (2 * deltaMax + 1)) - deltaMax;
    return clamp16((int16_t)(prevGapTop + delta), minGapTop, maxGapTop);
}

void FlappyScreenView::positionPipe(int index, int16_t gapTop)
{
    int16_t gapSize = currentGapSize();
    if (pipeChallenge[index]) {
        gapSize = clamp16((int16_t)(gapSize - CHALLENGE_GAP_REDUCTION), MIN_CHALLENGE_GAP, gapSize);
    }
    int16_t gapBottom = gapTop + gapSize;

    int16_t upH = gapTop;
    int16_t downH = SCREEN_HEIGHT - gapBottom;

    if (upH < 1) upH = 1;
    if (downH < 1) downH = 1;

    // Cache pointers and values to avoid repeated array indexing
    auto* currentWallUp = wallUp[index];
    auto* currentWallDown = wallDown[index];
    int16_t currentPipeX = pipeX[index];

    // Invalidate the old screen area before moving to clear the previous drawing
    currentWallUp->invalidate();
    currentWallDown->invalidate();

    // Update position and dimensions for the upper and lower walls
    currentWallUp->setPosition(currentPipeX, 0, currentWallUp->getWidth(), upH);
    currentWallDown->setPosition(currentPipeX, gapBottom, currentWallDown->getWidth(), downH);

    // Invalidate the new screen area after moving to render the new drawing
    currentWallUp->invalidate();
    currentWallDown->invalidate();
}

void FlappyScreenView::startDyingSequence()
{
    if (isDying) {
        return;
    }

    isPaused = false;
    isDying = true;
    birdVel_fp = DEATH_BOUNCE_VELOCITY_FP;
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
