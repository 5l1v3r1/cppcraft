#include "render_gui.hpp"

#include "renderman.hpp"
#include "game.hpp"
#include <glm/vec2.hpp>
#include <cmath>
#include <nanogui/nanogui.h>

using namespace glm;

namespace cppcraft
{
	static nanogui::Screen* screen = nullptr;

	void GUIRenderer::initInterfaces(Renderer& renderer)
	{
    bool bvar = true;
    int ivar = 12345678;
    double dvar = 3.1415926;
    float fvar = (float)dvar;
    std::string strval = "A string";
    enum test_enum {
      Item1 = 0,
      Item2,
      Item3
    };
    test_enum enumval = Item2;
    bool enabled = true;
    nanogui::Color colval(0.5f, 0.5f, 0.7f, 1.f);

    screen = new nanogui::Screen();
    screen->initialize(renderer.window(), true);

    renderer.on_terminate([] { delete screen; });

    auto* gui = new nanogui::FormHelper(screen);
    auto wnd = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");

    gui->addGroup("Basic types");
    gui->addVariable("bool", bvar)->setTooltip("Test tooltip.");
    gui->addVariable("string", strval);

    gui->addGroup("Validating fields");
    gui->addVariable("int", ivar)->setSpinnable(true);
    gui->addVariable("float", fvar)->setTooltip("Test.");
    gui->addVariable("double", dvar)->setSpinnable(true);

    gui->addGroup("Complex types");
    gui->addVariable("Enumeration", enumval, enabled)->setItems({ "Item 1", "Item 2", "Item 3" });
    gui->addVariable("Color", colval)
       ->setFinalCallback([](const auto& c) {
             std::cout << "ColorPicker Final Callback: ["
                       << c.r() << ", "
                       << c.g() << ", "
                       << c.b() << ", "
                       << c.w() << "]" << std::endl;
         });

    gui->addGroup("Other widgets");
    gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");

    screen->setVisible(true);
    screen->performLayout();
    wnd->center();
	}

	void GUIRenderer::renderInterfaces(Renderer& renderer)
	{
    (void) renderer;
    screen->drawContents();
    screen->drawWidgets();
	}

}
