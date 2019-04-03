/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <string>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

class ExampleApplication : public nanogui::Screen {
public:
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(1024, 768), "NanoGUI Test") {
        using namespace nanogui;

        window_ = new Window(this, "Button demo");
        window_->setPosition(Vector2i(15, 15));
        window_->setLayout(new GroupLayout());

        /* No need to store a pointer, the data structure will be automatically
           freed when the parent window_ is deleted */
        new Label(window_, "Push buttons", "sans-bold");

        Button *b = new Button(window_, "Plain button");
        b->setCallback([] { cout << "pushed!" << endl; });
        b->setTooltip("short tooltip");

        /* Alternative construction notation using variadic template */
        b = window_->add<Button>("Styled", ENTYPO_ICON_ROCKET);
        b->setBackgroundColor(Color(0, 0, 255, 25));
        b->setCallback([] { cout << "pushed!" << endl; });
        b->setTooltip("This button has a fairly long tooltip. It is so long, in "
                "fact, that the shown text will span several lines.");

        new Label(window_, "Toggle buttons", "sans-bold");
        b = new Button(window_, "Toggle me");
        b->setFlags(Button::ToggleButton);
        b->setChangeCallback([](bool state) { cout << "Toggle button state: " << state << endl; });

        new Label(window_, "Radio buttons", "sans-bold");
        b = new Button(window_, "Radio button 1");
        b->setFlags(Button::RadioButton);
        b = new Button(window_, "Radio button 2");
        b->setFlags(Button::RadioButton);

        new Label(window_, "A tool palette", "sans-bold");
        Widget *tools = new Widget(window_);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));

        b = new ToolButton(tools, ENTYPO_ICON_CLOUD);
        b = new ToolButton(tools, ENTYPO_ICON_CONTROLLER_FAST_FORWARD);
        b = new ToolButton(tools, ENTYPO_ICON_COMPASS);
        b = new ToolButton(tools, ENTYPO_ICON_INSTALL);

        new Label(window_, "Popup buttons", "sans-bold");
        PopupButton *popupBtn = new PopupButton(window_, "Popup", ENTYPO_ICON_EXPORT);
        Popup *popup = popupBtn->popup();
        popup->setLayout(new GroupLayout());
        new Label(popup, "Arbitrary widgets can be placed here");
        new CheckBox(popup, "A check box");
        // popup right
        popupBtn = new PopupButton(popup, "Recursive popup", ENTYPO_ICON_FLASH);
        Popup *popupRight = popupBtn->popup();
        popupRight->setLayout(new GroupLayout());
        new CheckBox(popupRight, "Another check box");
        // popup left
        popupBtn = new PopupButton(popup, "Recursive popup", ENTYPO_ICON_FLASH);
        popupBtn->setSide(Popup::Side::Left);
        Popup *popupLeft = popupBtn->popup();
        popupLeft->setLayout(new GroupLayout());
        new CheckBox(popupLeft, "Another check box");

        performLayout();
    }

    // ~ExampleApplication() {}

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        using namespace nanogui;

        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
        if (key == GLFW_KEY_A && action == GLFW_PRESS) {
            if (!containerWidget_) {
                containerWidget_ = window_->add<Widget>();
                containerWidget_->setLayout(new BoxLayout(Orientation::Horizontal,
                                                          Alignment::Middle, 6, 6));
                containerWidget_->add<Label>("label");
                TextBox *tb = containerWidget_->add<TextBox>("textbox");
                tb->setEditable(true);
            }
            performLayout();
        }
        if (key == GLFW_KEY_B && action == GLFW_PRESS) {
            if (containerWidget_) {
                window_->removeChild(containerWidget_);
                containerWidget_ = nullptr;
            }
            performLayout();
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Draw the user interface */
        Screen::draw(ctx);
    }

private:
    nanogui::Window *window_{};
    nanogui::Widget *containerWidget_{};
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        /* scoped variables */ {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
