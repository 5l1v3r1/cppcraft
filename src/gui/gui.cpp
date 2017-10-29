#include "gui.hpp"
#include "../renderman.hpp"
#include <nanogui/nanogui.h>

namespace cppcraft
{
  nanogui::Screen* GUI::m_screen = nullptr;

  void GUI::init(Renderer& renderer)
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

    this->m_screen = new nanogui::Screen();
    m_screen->initialize(renderer.window(), true);

    renderer.on_terminate([] { delete m_screen; });

    auto* gui = new nanogui::FormHelper(m_screen);
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

    m_screen->setVisible(true);
    m_screen->performLayout();
    wnd->center();
  }

  void GUI::render()
  {
    m_screen->drawContents();
    m_screen->drawWidgets();
  }

  void GUI::takeover()
  {
    auto* window = m_screen->glfwWindow();
    glfwSetCursorPosCallback(window,
        [] (GLFWwindow*, double x, double y) {
            m_screen->cursorPosCallbackEvent(x, y);
        });
    glfwSetMouseButtonCallback(window,
        [] (GLFWwindow*, int button, int action, int modifiers) {
            m_screen->mouseButtonCallbackEvent(button, action, modifiers);
        });
    glfwSetKeyCallback(window,
        [] (GLFWwindow*, int key, int scancode, int action, int mods) {
            m_screen->keyCallbackEvent(key, scancode, action, mods);
        });
    glfwSetCharCallback(window,
        [] (GLFWwindow*, unsigned int codepoint) {
            m_screen->charCallbackEvent(codepoint);
        });
    glfwSetDropCallback(window,
        [] (GLFWwindow*, int count, const char** filenames) {
            m_screen->dropCallbackEvent(count, filenames);
        });
    glfwSetScrollCallback(window,
        [] (GLFWwindow*, double x, double y) {
            m_screen->scrollCallbackEvent(x, y);
        });
    glfwSetFramebufferSizeCallback(window,
        [] (GLFWwindow*, int width, int height) {
            m_screen->resizeCallbackEvent(width, height);
        });
  }
}
