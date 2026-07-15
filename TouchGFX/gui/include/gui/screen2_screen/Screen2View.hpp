#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/BackTriangle.hpp>
#include <touchgfx/events/ClickEvent.hpp>
#include <touchgfx/widgets/Box.hpp>
class Screen2View : public Screen2ViewBase
{
public:
    Screen2View() {}
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void handleClickEvent(const touchgfx::ClickEvent& evt);
    virtual void handleTickEvent();
    virtual void tearDownScreen();
protected:
    BackTriangle backIcon;
};

#endif // SCREEN2VIEW_HPP
