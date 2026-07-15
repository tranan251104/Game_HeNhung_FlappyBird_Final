#ifndef PLAYTRIANGLE_HPP
#define PLAYTRIANGLE_HPP

#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/hal/Types.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/widgets/Widget.hpp>

class PlayTriangle : public touchgfx::Widget
{
public:
    PlayTriangle()
        : color(0)
    {
    }

    void setColor(touchgfx::colortype newColor)
    {
        if (color != newColor)
        {
            color = newColor;
            invalidate();
        }
    }

    virtual void draw(const touchgfx::Rect& invalidatedArea) const
    {
        const int16_t w = getWidth();
        const int16_t h = getHeight();
        if (w <= 0 || h <= 0)
        {
            return;
        }

        const int16_t absX = getX();
        const int16_t absY = getY();
        const int16_t lastY = h - 1;
        const int16_t mid = lastY / 2;

        touchgfx::Rect absInvalidated = invalidatedArea;
        absInvalidated.x += absX;
        absInvalidated.y += absY;

        for (int16_t y = 0; y < h; ++y)
        {
            int16_t xEnd = 0;
            if (y <= mid)
            {
                const int16_t denom = (mid == 0) ? 1 : mid;
                xEnd = (int16_t)(((w - 1) * y) / denom);
            }
            else
            {
                const int16_t denom = (lastY - mid == 0) ? 1 : (lastY - mid);
                xEnd = (int16_t)(((w - 1) * (lastY - y)) / denom);
            }

            if (xEnd < 0)
            {
                continue;
            }

            touchgfx::Rect lineRect(absX, absY + y, xEnd + 1, 1);
            lineRect &= absInvalidated;
            if (!lineRect.isEmpty())
            {
                touchgfx::HAL::lcd().fillRect(lineRect, color);
            }
        }
    }

    virtual touchgfx::Rect getSolidRect() const { return touchgfx::Rect(); }

private:
    touchgfx::colortype color;
};

#endif // PLAYTRIANGLE_HPP
