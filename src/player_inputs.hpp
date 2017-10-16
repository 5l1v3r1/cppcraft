#ifndef PLAYER_INPUTS_HPP
#define PLAYER_INPUTS_HPP

#include <glm/vec2.hpp>
#include <SDL.h>
#include <cmath>
#include <map>

namespace cppcraft
{
  struct Input
  {
    enum key_t {
      KEY_NONE,
      KEY_PRESSED,
      KEY_HELD
    };

    key_t key(int value) {
      return m_keys[value];
    }
    void hold(int value) {
      auto it = m_keys.find(value);
      if (it != m_keys.end()) it->second = KEY_HELD;
    }
    bool mouse_button(int value);
    glm::vec2 mouse_xy();
    int  mouse_wheel();
    void mouse_show(bool visible);
    void grab(bool);

    const std::string& text() {
      return text_buffer;
    }
    void text_clear() {
      text_buffer.clear();
    }
    void text_backspace() {
      text_buffer.pop_back();
    }

    const glm::vec2& rotation() const {
      return m_rot;
    }
    void add_rotation(glm::vec2 rot) {
      m_rot += rot;
    }

    void init(SDL_Window*, glm::vec2 motion_scale);
    void handle(SDL_Event&);

  private:
    std::map<int, key_t> m_keys;
    glm::vec2 m_rot;
    glm::vec2 m_motion_scale;
    SDL_Window* window = nullptr;
    int m_wheel_value = 0;
    std::string text_buffer;
  };
  extern Input input;

	struct keyconf_t
	{
		/// Keyboard related ///
    int k_escape = 41; // ESC

		int k_forward;
		int k_backward;
		int k_right;
		int k_left;

		int k_jump;
		int k_crouch;
		int k_sprint;
		int k_throw;
		int k_inventory;

		int k_flying;
		int k_flyup;
		int k_flydown;

		/// Mouse related ///

		bool alternateMiningButton;
		int  mouse_btn_mine;
		int  mouse_btn_place;

		/// Joystick related ///
		bool joy_enabled;
		bool joy_dual_axis_mining;

		char joy_index;
		float joy_deadzone;
		float joy_speed;

		int joy_button_count;
		unsigned char const* jbuttons;
		int joy_axis_count;
		float const* jaxis;

		unsigned char joy_axis_forward;
		unsigned char joy_axis_sidestep;
		unsigned char joy_axis_look_xrot;
		unsigned char joy_axis_look_yrot;

		unsigned char joy_axis_mine;
		unsigned char joy_axis_place;

		unsigned char joy_btn_jump;
		unsigned char joy_btn_crouch;
		unsigned char joy_btn_sprint;

		unsigned char joy_btn_previtem;
		unsigned char joy_btn_nextitem;

		unsigned char joy_btn_flying;
		unsigned char joy_btn_exit;

	};
	extern keyconf_t keyconf;

	inline float tresholdValue(float x)
	{
		if (std::abs(x) < keyconf.joy_deadzone) return 0.0;
		if (x > 0)
			return (x - keyconf.joy_deadzone) / (1.0 - keyconf.joy_deadzone);
		else
			return (x + keyconf.joy_deadzone) / (1.0 - keyconf.joy_deadzone);
	}
}

#endif
