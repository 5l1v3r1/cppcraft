#include "player.hpp"

#include <library/log.hpp>
#include "chat.hpp"
#include "game.hpp"
#include "gameconf.hpp"
#include "gui/menu.hpp"
#include "player_inputs.hpp"
#include "player_logic.hpp"
#include "sun.hpp"
#include "threading.hpp"
#include <SDL.h>
#include <cmath>
#include <set>
#include <string>

using namespace library;

namespace cppcraft
{
	keyconf_t keyconf;

  bool Input::mouse_button(int v)
  {
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(v);
  }
  glm::vec2 Input::mouse_xy()
  {
    int x; int y;
    SDL_GetMouseState(&x, &y);
    return glm::vec2(x, y);
  }
  int Input::mouse_wheel()
  {
    int v = m_wheel_value;
    m_wheel_value = 0;
    return v;
  }
  void Input::mouse_show(bool visible)
  {
    SDL_ShowCursor((visible) ? SDL_ENABLE : SDL_DISABLE);
  }
  void Input::grab(bool grabbed)
  {
    SDL_SetWindowGrab(this->m_window, (grabbed) ? SDL_TRUE : SDL_FALSE);
    SDL_SetRelativeMouseMode((grabbed) ? SDL_TRUE : SDL_FALSE);
  }

  void Input::handle(SDL_Event& event)
  {
    switch (event.type) {
    case SDL_KEYDOWN:
        //printf("Key pressed: %d\n", event.key.keysym.scancode);
        m_keys[event.key.keysym.scancode] = Input::KEY_PRESSED;
        break;
    case SDL_KEYUP:
        m_keys[event.key.keysym.scancode] = Input::KEY_NONE;
        break;
    case SDL_MOUSEMOTION:
        add_rotation(glm::vec2(event.motion.yrel, event.motion.xrel) * m_motion_scale);
        break;
    case SDL_MOUSEWHEEL:
        m_wheel_value += event.wheel.y;
        break;
    default:
        break;
    }
  }
  void Input::init(SDL_Window* wnd, glm::vec2 mscale, glm::vec2 rot)
  {
    assert(wnd != nullptr);
    this->m_window = wnd;
    this->m_rot = rot;
    this->m_motion_scale = mscale;
    mouse_show(false);
    grab(true);
  }

	void PlayerClass::initInputs(SDL_Window& window)
	{
		logger << Log::INFO << "* Initializing input systems" << Log::ENDL;
    game.input().init(&window, glm::vec2(0.001f, 0.002f), player.rot);

		/// Keyboard configuration
		keyconf.k_forward  = config.get("k_forward",  26); // W
		keyconf.k_backward = config.get("k_backward", 22); // S
		keyconf.k_right    = config.get("k_right",     7); // D
		keyconf.k_left     = config.get("k_left",      4); // A

		keyconf.k_jump   = config.get("k_jump", 44);    // Space
		keyconf.k_sprint = config.get("k_sprint", (int) SDL_SCANCODE_LSHIFT);
		keyconf.k_crouch = config.get("k_crouch", (int) SDL_SCANCODE_LCTRL);
		keyconf.k_throw     = config.get("k_throw", 20);    // Q
    keyconf.k_inventory = config.get("k_inventory", 8); // E

		keyconf.k_flying  = config.get("k_flying",  9); // F
		keyconf.k_flyup   = config.get("k_flyup",  23); // T
		keyconf.k_flydown = config.get("k_flydown",21); // R


		/// Mouse configuration

		double mspd  = config.get("mouse.speed", 120) / 1000.0;
		double msens = config.get("mouse.sens",  80)  / 10.0;

		keyconf.alternateMiningButton = config.get("mouse.swap_buttons", false);
		keyconf.mouse_btn_place = (keyconf.alternateMiningButton) ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT;
		keyconf.mouse_btn_mine = (keyconf.alternateMiningButton) ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;

		// initialize joystick support
		keyconf.joy_enabled = config.get("joy.enabled", false);
		if (keyconf.joy_enabled)
		{
			keyconf.joy_index   = config.get("joy.index", 0);
			keyconf.joy_enabled = SDL_IsGameController(keyconf.joy_index);

			if (keyconf.joy_enabled)
			{
				std::string jname(SDL_GameControllerNameForIndex(keyconf.joy_index));
				logger << Log::INFO << "* Joystick: " << jname << Log::ENDL;

				keyconf.joy_deadzone = config.get("joy.deadzone", 0.12);
				keyconf.joy_speed    = config.get("joy.rotspeed", 2.0);

				/// joystick configuration ///
				keyconf.joy_axis_sidestep = config.get("joy.axis_sides", 0);
				keyconf.joy_axis_forward = config.get("joy.axis_forw", 1);

				keyconf.joy_axis_look_xrot = config.get("joy.axis_xrot", 3);
				keyconf.joy_axis_look_yrot = config.get("joy.axis_yrot", 4);

				keyconf.joy_axis_place = config.get("joy.axis_place", 2);

				keyconf.joy_dual_axis_mining = config.get("joy.dual_axis_mining", true);
				if (keyconf.joy_dual_axis_mining)
				{
					// dual axis, so placement has same axis as mining
					keyconf.joy_axis_mine = keyconf.joy_axis_place;
				}
				else
				{
					keyconf.joy_axis_mine = config.get("joy.axis_mine", 2);
				}

				keyconf.joy_btn_jump   = config.get("joy.btn_jump", 0);
				keyconf.joy_btn_sprint = config.get("joy.btn_sprint", 2);
				keyconf.joy_btn_crouch = config.get("joy.btn_crouch", 3);

				keyconf.joy_btn_previtem = config.get("joy.btn_previtem", 4);
				keyconf.joy_btn_nextitem = config.get("joy.btn_nextitem", 5);

				keyconf.joy_btn_flying = config.get("joy.btn_flying", 9);
				keyconf.joy_btn_exit   = config.get("joy.btn_exit", 6);
			}
			else
			{
				logger << Log::INFO << "* Joystick with index " << keyconf.joy_index << " did not exist." << Log::ENDL;
			}
		}
		if (keyconf.joy_enabled == false)
		{
			keyconf.jbuttons = new unsigned char[16]();
			keyconf.jaxis = new float[8]();
		}

	} // PlayerClass::initInputs

	void PlayerClass::handleJoystick()
	{
		/// BUTTONS

		// 0 = A button
		// 1 = B button
		// 2 = X button
		// 3 = Y button

		// 4 = left bumper
		// 5 = right bumper

		// 6 = select/back
		// 7 = start

		// 8 = left rotator button
		// 9 = right rotator button

		// 10 = dpad up
		// 11 = dpad right
		// 12 = dpad down
		// 13 = dpad left

		//keyconf.jbuttons = glfwGetJoystickButtons(keyconf.joy_index, &keyconf.joy_button_count);

		/*for (int i = 0; i < keyconf.joy_button_count; i++)
		{
			if (keyconf.jbuttons[i])
				logger << Log::INFO << "Button pressed: " << i << Log::ENDL;
		}*/

		/// AXES

		// 0 = left rotator (left/right)
		// 1 = left rotator (up/down)

		// 2 = trigger left/right

		// 3 = right rotator (left/right)
		// 4 = right rotator (up/down)

		//keyconf.jaxis = glfwGetJoystickAxes(keyconf.joy_index, &keyconf.joy_axis_count);

		/*for (int i = 0; i < keyconf.joy_axis_count; i++)
		{
			if (std::abs(keyconf.jaxis[i]) > 0.1)
				logger << Log::INFO << "Axis " << i << " value: " << keyconf.jaxis[i] << Log::ENDL;
		}*/
	}

	void PlayerClass::handleInputs()
	{
		// handle joystick if existing
		if (keyconf.joy_enabled) handleJoystick();

		// testing/cheats
		if (busyControls() == false)
		{
			if (game.input().key(SDLK_F1) == Input::KEY_PRESSED)
			{
				game.input().hold(SDLK_F1);

				thesun.setRadianAngle(3.14159 * 1/8);
			}
			if (game.input().key(SDLK_F2) == Input::KEY_PRESSED)
			{
				game.input().hold(SDLK_F2);

				thesun.setRadianAngle(3.14159 * 2/8);
			}
			if (game.input().key(SDLK_F3) == Input::KEY_PRESSED)
			{
				game.input().hold(SDLK_F3);

				thesun.setRadianAngle(3.14159 * 3/8);
			}
			if (game.input().key(SDLK_F4) == Input::KEY_PRESSED)
			{
				game.input().hold(SDLK_F4);

				thesun.setRadianAngle(-1);
			}

			if (game.input().key(keyconf.k_flying) || keyconf.jbuttons[keyconf.joy_btn_flying])
			{
				if (plogic.flylock == false)
				{
					// lock key
					plogic.flylock = true;
					// toggle flying
					player.Flying = ! player.Flying;
				}
			}
			else plogic.flylock = false;

			static bool lock_quickbar_scroll = false;

			int wheel = game.input().mouse_wheel();
			if (wheel > 0 || keyconf.jbuttons[keyconf.joy_btn_nextitem])
			{
				if (lock_quickbar_scroll == false)
				{
					// next quickbar item
					//gui::menu.quickbarX = (gui::menu.quickbarX + 1) % inventory.getWidth();
					lock_quickbar_scroll = true;
				}
			}
			else if (wheel < 0 || keyconf.jbuttons[keyconf.joy_btn_previtem])
			{
				if (lock_quickbar_scroll == false)
				{
					// previous quickbar item
					/*
					if (gui::menu.quickbarX)
						gui::menu.quickbarX -= 1;
					else
						gui::menu.quickbarX = inventory.getWidth()-1;
					*/
					lock_quickbar_scroll = true;
				}
			}
			else
			{
				lock_quickbar_scroll = false;
			}
			// number keys (1-9) to directly select on quickbar
			for (int i = 1; i < 10; i++)
			if (game.input().key(SDLK_0 + i))
			{
				//gui::menu.quickbarX = (i - 1) % inventory.getWidth();
			}

		} // busyControls

		if (game.input().key(keyconf.k_escape) == Input::KEY_PRESSED || keyconf.jbuttons[keyconf.joy_btn_exit])
		{
			game.input().hold(keyconf.k_escape);

			if (chatbox.isOpen())
			{
				chatbox.openChat(false);
			}
			else
			{
        // its over
				game.terminate();
			}
		}

		if (game.input().key(SDLK_RETURN) == Input::KEY_PRESSED)
		{
			game.input().hold(SDLK_RETURN);
			chatbox.openChat(!chatbox.isOpen());

			if (chatbox.isOpen() == false)
			{
				// say something, as long as its something :)
				//if (!game.input().text().empty())
				//	network.sendChat(game.input().text());
			}
			game.input().text_clear();
		}

		if (game.input().key(SDLK_BACKSPACE) == Input::KEY_PRESSED)
		{
			game.input().hold(SDLK_BACKSPACE);
			if (chatbox.isOpen()) {
				game.input().text_backspace();
      }
		}

	} // handleInputs()
}
