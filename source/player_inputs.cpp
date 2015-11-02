#include "player.hpp"

#include <library/log.hpp>
#include <library/opengl/input.hpp>
#include "chat.hpp"
#include "gameconf.hpp"
#include "menu.hpp"
#include "player_inputs.hpp"
#include "player_logic.hpp"
#include "sectors.hpp"
#include "sun.hpp"
#include "threading.hpp"
#include <GL/glfw3.h>
#include <cmath>
#include <string>

using namespace library;

namespace cppcraft
{
	keyconf_t keyconf;
	Input     input;
	
	void PlayerClass::initInputs(WindowClass& gameScreen)
	{
		logger << Log::INFO << "* Initializing input systems" << Log::ENDL;
		
		/// Keyboard configuration
		
		keyconf.k_forward  = config.get("k_forward",  87); // W
		keyconf.k_backward = config.get("k_backward", 83); // S
		keyconf.k_right    = config.get("k_right",    68); // D
		keyconf.k_left     = config.get("k_left",     65); // A
		
		keyconf.k_jump   = config.get("k_jump", 32);    // Space
		keyconf.k_sprint = config.get("k_sprint", GLFW_KEY_LEFT_SHIFT);
		keyconf.k_crouch = config.get("k_crouch", GLFW_KEY_LEFT_CONTROL);
		keyconf.k_throw  = config.get("k_throw", 81);   // Q
		
		keyconf.k_flying  = config.get("k_flying", 70);  // F
		keyconf.k_flyup   = config.get("k_flyup",   84); // T
		keyconf.k_flydown = config.get("k_flydown", 82); // R
		
		keyconf.k_inventory = config.get("k_inventory", 73); // I
		
		/// Mouse configuration
		
		double mspd  = config.get("mouse.speed", 120) / 1000.0;
		double msens = config.get("mouse.sens",  80)  / 10.0;
		
		keyconf.alternateMiningButton = config.get("mouse.swap_buttons", false);
		keyconf.mouse_btn_place = (keyconf.alternateMiningButton) ? GLFW_MOUSE_BUTTON_2 : GLFW_MOUSE_BUTTON_1;
		keyconf.mouse_btn_mine = (keyconf.alternateMiningButton) ? GLFW_MOUSE_BUTTON_1 : GLFW_MOUSE_BUTTON_2;
		
		// initialize input systems
		input.init(gameScreen, true, true);
		input.setRotation(vec2(player.xrotrad, player.yrotrad));
		input.grabMouse(true);  // enable fps-like mouse
		input.mouseOptions(mspd, msens); // mouse speed & sensitivity
		input.showMouse(false); // hide mouse
		
		// initialize joystick support
		keyconf.joy_enabled = config.get("joy.enabled", false);
		if (keyconf.joy_enabled)
		{
			keyconf.joy_index   = config.get("joy.index", 0);
			keyconf.joy_enabled = glfwJoystickPresent(keyconf.joy_index) != 0;
			
			if (keyconf.joy_enabled)
			{
				std::string jname = glfwGetJoystickName(keyconf.joy_index);
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
		
		keyconf.jbuttons = glfwGetJoystickButtons(keyconf.joy_index, &keyconf.joy_button_count);
		
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
		
		keyconf.jaxis = glfwGetJoystickAxes(keyconf.joy_index, &keyconf.joy_axis_count);
		
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
			if (input.getKey(GLFW_KEY_F1) == Input::KEY_PRESSED)
			{
				input.hold(GLFW_KEY_F1);
				
				thesun.setRadianAngle(3.14159 * 1/8);
				//Sectors.updateAll();
				//worldbuilder.reset();
			}
			if (input.getKey(GLFW_KEY_F2) == Input::KEY_PRESSED)
			{
				input.hold(GLFW_KEY_F2);
				
				thesun.setRadianAngle(3.14159 * 2/8);
				//Sectors.updateAll();
				//worldbuilder.reset();
			}
			if (input.getKey(GLFW_KEY_F3) == Input::KEY_PRESSED)
			{
				input.hold(GLFW_KEY_F3);
				
				thesun.setRadianAngle(3.14159 * 3/8);
				//Sectors.updateAll();
				//worldbuilder.reset();
			}
			if (input.getKey(GLFW_KEY_F4) == Input::KEY_PRESSED)
			{
				input.hold(GLFW_KEY_F4);
				
				thesun.setRadianAngle(-1);
				//Sectors.updateAll();
				//worldbuilder.reset();
			}
			
			if (input.getKey(GLFW_KEY_C) == Input::KEY_PRESSED)
			{
				input.hold(GLFW_KEY_C);
				
				Sector* sector = sectors.sectorAt(player.X, player.Z);
				if (sector)
				{
					logger << Log::INFO << "Sector (" << sector->getX() << ", " << sector->getZ() << "): " << (int) sector->generated() << Log::ENDL;
				}
			}
			else if (input.getKey(GLFW_KEY_V) == Input::KEY_PRESSED)
			{
				input.hold(GLFW_KEY_V);
				
				/*
				PackCoord plc(player.X, player.Y, player.Z);
				if (plc.valid)
				{
					logger << Log::INFO << "World (" << plc.wc.x << ", " << plc.wc.y << ", " << plc.wc.z << ")" << Log::ENDL;
					logger << Log::INFO << "Block (" << plc.bc.x << ", " << plc.bc.y << ", " << plc.bc.z << ")" << Log::ENDL;
				}*/
			}
			
			if (input.getKey(keyconf.k_flying) || keyconf.jbuttons[keyconf.joy_btn_flying])
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
			
			int wheel = input.getWheel();
			if (wheel > 0 || keyconf.jbuttons[keyconf.joy_btn_nextitem])
			{
				if (lock_quickbar_scroll == false)
				{
					// next quickbar item
					menu.quickbarX = (menu.quickbarX + 1) % inventory.getWidth();
					lock_quickbar_scroll = true;
				}
			}
			else if (wheel < 0 || keyconf.jbuttons[keyconf.joy_btn_previtem])
			{
				if (lock_quickbar_scroll == false)
				{
					// previous quickbar item
					if (menu.quickbarX)
						menu.quickbarX -= 1;
					else
						menu.quickbarX = inventory.getWidth()-1;
					lock_quickbar_scroll = true;
				}
			}
			else
			{
				lock_quickbar_scroll = false;
			}
			// number keys (1-9) to directly select on quickbar
			for (int i = 1; i < 10; i++)
			if (input.getKey(GLFW_KEY_0 + i))
			{
				menu.quickbarX = (i - 1) % inventory.getWidth();
			}
			
		} // busyControls
		
		if (input.getKey(GLFW_KEY_ESCAPE) == Input::KEY_PRESSED || keyconf.jbuttons[keyconf.joy_btn_exit])
		{
			input.hold(GLFW_KEY_ESCAPE);
			
			if (chatbox.isOpen())
			{
				chatbox.openChat(false);
			}
			else
			{
				mtx.terminate = true;
			}
		}
		
		if (input.getKey(GLFW_KEY_ENTER) == Input::KEY_PRESSED)
		{
			input.hold(GLFW_KEY_ENTER);
			chatbox.openChat(!chatbox.isOpen());
			
			if (chatbox.isOpen() == false)
			{
				// say something, as long as its something :)
				//if (input.getText().size())
				//	network.sendChat(input.getText());
			}
			input.clearText();
		}
		
		if (input.getKey(GLFW_KEY_BACKSPACE) == Input::KEY_PRESSED)
		{
			input.hold(GLFW_KEY_BACKSPACE);
			if (chatbox.isOpen())
				input.textBackspace();
		}
		
	}
}
