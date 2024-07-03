#pragma once

#include "typedef.hpp"

#include <format>
#include <map>

#include <SDL.h>

#include "cardinal.hpp"
#include "input.hpp"
#include "log.hpp"

extern "C" {
	typedef enum {
		SDL_JOYSTICK_BUTTON_CROSS = 0,
		SDL_JOYSTICK_BUTTON_CIRCLE = 1,
		SDL_JOYSTICK_BUTTON_SQUARE = 2,
		SDL_JOYSTICK_BUTTON_TRIANGLE = 3,
		SDL_JOYSTICK_BUTTON_LEFTSTART = 4,
		SDL_JOYSTICK_BUTTON_PLAYSTATION = 5,
		SDL_JOYSTICK_BUTTON_RIGHTSTART = 6,
		SDL_JOYSTICK_BUTTON_LEFTSTICK = 7,
		SDL_JOYSTICK_BUTTON_RIGHTSTICK = 8,
		SDL_JOYSTICK_BUTTON_LEFTSHOULDER = 9,
		SDL_JOYSTICK_BUTTON_RIGHTSHOULDER = 10,
		SDL_JOYSTICK_BUTTON_UP = 11,
		SDL_JOYSTICK_BUTTON_DOWN = 12,
		SDL_JOYSTICK_BUTTON_LEFT = 13,
		SDL_JOYSTICK_BUTTON_RIGHT = 14,
		SDL_JOYSTICK_BUTTON_TOUCHPAD = 15,
		SDL_JOYSTICK_BUTTON_MICROPHONE = 16,
		SDL_NUM_JOYSTICK_BUTTONS = 17
	} SDL_JoystickButton;

	typedef enum {
		SDL_JOYSTICK_AXIS_LEFTX = 0,
		SDL_JOYSTICK_AXIS_LEFTY = 1,
		SDL_JOYSTICK_AXIS_RIGHTX = 2,
		SDL_JOYSTICK_AXIS_RIGHTY = 3,
		SDL_JOYSTICK_AXIS_TRIGGERLEFT = 4,
		SDL_JOYSTICK_AXIS_TRIGGERRIGHT = 5,
		SDL_NUM_JOYSTICK_AXES = 6
	} SDL_JoystickAxis;
}

namespace Bleakdepth {
	using joystick_t = SDL_Joystick;

	struct gamepad_t {
		using button_t = SDL_JoystickButton;
		using axis_t = SDL_JoystickAxis;

		static constexpr i16 JOYSTICK_DEAD_ZONE { 8000 };

		struct stick_t {
		  private:
			static inline constexpr cardinal_t to_cardinal(i16 x, i16 y) noexcept {
				cardinal_t result {};

				if (x < -JOYSTICK_DEAD_ZONE) {
					result += cardinal_t::West;
				} else if (x > JOYSTICK_DEAD_ZONE) {
					result += cardinal_t::East;
				}

				if (y < -JOYSTICK_DEAD_ZONE) {
					result += cardinal_t::North;
				} else if (y > JOYSTICK_DEAD_ZONE) {
					result += cardinal_t::South;
				}

				return result;
			}

			inline cardinal_t get_state() const noexcept {
				auto as_ptr { const_cast<ptr<joystick_t>>(joystick) };
				return to_cardinal(SDL_JoystickGetAxis(as_ptr, x_axis_id), SDL_JoystickGetAxis(as_ptr, y_axis_id));
			}

		  public:
			inline stick_t(cptrc<joystick_t> joystick, axis_t x_axis_id, axis_t y_axis_id) noexcept :
				joystick { joystick },
				x_axis_id { x_axis_id },
				y_axis_id { y_axis_id } {
				if (joystick == nullptr) {
					error_log.add("nullptr passed to stick [{}, {}] constructor\n", (i32)x_axis_id, (i32)y_axis_id);
				}
			}

			cptrc<joystick_t> joystick;
			axis_t x_axis_id, y_axis_id;

			cardinal_t current_state;
			cardinal_t previous_state;

			inline constexpr void update() noexcept {
				previous_state = current_state;
				current_state = get_state();
			}
		};

		struct buttons_t {
			inline buttons_t(cptrc<joystick_t> joystick) noexcept : joystick { joystick } {
				if (joystick == nullptr) {
					error_log.add("nullptr passed to buttons constructor\n");
				}
			}

			cptrc<joystick_t> joystick;

			bool current_state[SDL_NUM_JOYSTICK_BUTTONS];
			bool previous_state[SDL_NUM_JOYSTICK_BUTTONS];

			inline constexpr void update() noexcept {
				auto as_ptr { const_cast<ptr<joystick_t>>(joystick) };

				for (usize i { 0 }; i < SDL_NUM_JOYSTICK_BUTTONS; ++i) {
					previous_state[i] = current_state[i];
					current_state[i] = SDL_JoystickGetButton(as_ptr, i);
				}
			}

			inline input_state_t at(button_t button) const noexcept {
				if (previous_state[button]) {
					return current_state[button] ? input_state_t::Pressed : input_state_t::Up;
				} else {
					return current_state[button] ? input_state_t::Down : input_state_t::Released;
				}
			}

			inline input_state_t at(int button) const noexcept {
				if (previous_state[button]) {
					return current_state[button] ? input_state_t::Pressed : input_state_t::Up;
				} else {
					return current_state[button] ? input_state_t::Down : input_state_t::Released;
				}
			}
		};

		struct dpad_t {
		  private:
			static inline constexpr cardinal_t to_cardinal(cref<buttons_t> buttons) noexcept {
				cardinal_t result {};

				if (buttons.current_state[SDL_JOYSTICK_BUTTON_UP]) {
					result += cardinal_t::North;
				} else if (buttons.current_state[SDL_JOYSTICK_BUTTON_DOWN]) {
					result += cardinal_t::South;
				}

				if (buttons.current_state[SDL_JOYSTICK_BUTTON_LEFT]) {
					result += cardinal_t::West;
				} else if (buttons.current_state[SDL_JOYSTICK_BUTTON_RIGHT]) {
					result += cardinal_t::East;
				}

				return result;
			}

			inline cardinal_t get_state(cref<buttons_t> buttons) const noexcept { return to_cardinal(buttons); }

		  public:
			inline dpad_t(cptrc<joystick_t> joystick) noexcept : joystick { joystick } {
				if (joystick == nullptr) {
					error_log.add("nullptr passed to dpad constructor\n");
				}
			}

			cptrc<joystick_t> joystick;

			cardinal_t current_state;
			cardinal_t previous_state;

			inline constexpr void update(cref<buttons_t> buttons) noexcept {
				previous_state = current_state;
				current_state = get_state(buttons);
			}
		};

		buttons_t buttons;

		stick_t left_stick;
		stick_t right_stick;

		dpad_t dpad;

		ptr<joystick_t> joystick;

		inline gamepad_t(joystick_t* joystick) noexcept :
			buttons { joystick },
			left_stick { joystick, SDL_JOYSTICK_AXIS_LEFTX, SDL_JOYSTICK_AXIS_LEFTY },
			right_stick { joystick, SDL_JOYSTICK_AXIS_RIGHTX, SDL_JOYSTICK_AXIS_RIGHTY },
			dpad { joystick },
			joystick { joystick } {
			if (joystick == nullptr) {
				error_log.add("nullptr passed to gamepad constructor\n");
			}
		}

		inline gamepad_t(cref<gamepad_t> other) noexcept :
			buttons { other.buttons },
			left_stick { other.left_stick },
			right_stick { other.right_stick },
			dpad { other.dpad },
			joystick { other.joystick } {}

		inline gamepad_t(rval<gamepad_t> other) noexcept :
			buttons { std::move(other.buttons) },
			left_stick { std::move(other.left_stick) },
			right_stick { std::move(other.right_stick) },
			dpad { std::move(other.dpad) },
			joystick { std::move(other.joystick) } {
			other.joystick = nullptr;
		}

		inline ~gamepad_t() noexcept {
			if (joystick != nullptr) {
				SDL_JoystickClose(joystick);
				joystick = nullptr;
			}
		}

		inline void update() noexcept {
			buttons.update();
			dpad.update(buttons);

			left_stick.update();
			right_stick.update();
		}

		inline constexpr std::string power_level() const noexcept {
			SDL_JoystickPowerLevel level { SDL_JoystickCurrentPowerLevel(joystick) };

			switch (level) {
			case SDL_JOYSTICK_POWER_UNKNOWN:
				return "unknown";
			case SDL_JOYSTICK_POWER_EMPTY:
				return "empty";
			case SDL_JOYSTICK_POWER_LOW:
				return "low";
			case SDL_JOYSTICK_POWER_MEDIUM:
				return "medium";
			case SDL_JOYSTICK_POWER_FULL:
				return "full";
			case SDL_JOYSTICK_POWER_WIRED:
				return "wired";
			default:
				return "unknown";
			}
		}

		inline bool is_button_pressed(button_t button) const noexcept { return buttons.at(button) == input_state_t::Pressed; }

		inline bool is_button_pressed(int button) const noexcept { return buttons.at(button) == input_state_t::Pressed; }

		inline bool is_button_released(button_t button) const noexcept { return buttons.at(button) == input_state_t::Released; }

		inline bool is_button_released(int button) const noexcept { return buttons.at(button) == input_state_t::Released; }

		inline bool is_button_down(button_t button) const noexcept { return buttons.at(button) == input_state_t::Down; }

		inline bool is_button_down(int button) const noexcept { return buttons.at(button) == input_state_t::Down; }

		inline bool is_button_up(button_t button) const noexcept { return buttons.at(button) == input_state_t::Up; }

		inline bool is_button_up(int button) const noexcept { return buttons.at(button) == input_state_t::Up; }

		inline bool any_button_pressed() const noexcept {
			for (int i { 0 }; i < SDL_NUM_JOYSTICK_BUTTONS; ++i) {
				if (is_button_pressed(i)) {
					return true;
				}
			}

			return false;
		}

		inline bool any_button_released() const noexcept {
			for (int i { 0 }; i < SDL_NUM_JOYSTICK_BUTTONS; ++i) {
				if (is_button_released(i)) {
					return true;
				}
			}

			return false;
		}

		inline bool any_button_down() const noexcept {
			for (int i { 0 }; i < SDL_NUM_JOYSTICK_BUTTONS; ++i) {
				if (is_button_down(i)) {
					return true;
				}
			}

			return false;
		}

		inline bool any_button_up() const noexcept {
			for (int i { 0 }; i < SDL_NUM_JOYSTICK_BUTTONS; ++i) {
				if (is_button_up(i)) {
					return true;
				}
			}

			return false;
		}

		template<typename... Buttons, typename = button_t> inline bool are_buttons_pressed(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (!is_button_pressed(button)) {
					return false;
				}
			}

			return true;
		}

		template<typename... Buttons, typename = button_t> inline bool are_buttons_released(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (!is_button_released(button)) {
					return false;
				}
			}

			return true;
		}

		template<typename... Buttons, typename = button_t> inline bool are_buttons_down(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (!is_button_down(button)) {
					return false;
				}
			}

			return true;
		}

		template<typename... Buttons, typename = button_t> inline bool are_buttons_up(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (!is_button_up(button)) {
					return false;
				}
			}

			return true;
		}

		template<typename... Buttons, typename = button_t> inline bool any_buttons_pressed(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (is_button_pressed(button)) {
					return true;
				}
			}

			return false;
		}

		template<typename... Buttons, typename = button_t> inline bool any_buttons_released(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (is_button_released(button)) {
					return true;
				}
			}

			return false;
		}

		template<typename... Buttons, typename = button_t> inline bool any_buttons_down(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (is_button_down(button)) {
					return true;
				}
			}

			return false;
		}

		template<typename... Buttons, typename = button_t> inline bool any_buttons_up(Buttons... buttons) const noexcept {
			for (button_t button : { buttons... }) {
				if (is_button_up(button)) {
					return true;
				}
			}

			return false;
		}
	};

	struct gamepad_slot_t {
		ptr<gamepad_t> gamepad;

		fn_ptr<void> disconnect_callback { nullptr };
		fn_ptr<void, cptr<gamepad_t>> reconnect_callback { nullptr };

		inline bool has_callbacks() const noexcept { return disconnect_callback != nullptr || reconnect_callback != nullptr; }

		inline void invoke_disconnect() const noexcept {
			if (disconnect_callback != nullptr) {
				disconnect_callback();
			}
		}

		inline void invoke_reconnect() const noexcept {
			if (reconnect_callback != nullptr) {
				reconnect_callback(gamepad);
			}
		}

		inline gamepad_slot_t(ptr<joystick_t> joystick) : gamepad { new gamepad_t { joystick } } {}

		inline gamepad_slot_t(ptr<joystick_t> joystick, fn_ptr<void> disconnect_callback, fn_ptr<void, cptr<gamepad_t>> reconnect_callback) :
			gamepad { new gamepad_t { joystick } },
			disconnect_callback { disconnect_callback },
			reconnect_callback { reconnect_callback } {}

		inline gamepad_slot_t(cref<gamepad_slot_t> other) noexcept :
			gamepad { other.gamepad },
			disconnect_callback { other.disconnect_callback },
			reconnect_callback { other.reconnect_callback } {}

		inline gamepad_slot_t(rval<gamepad_slot_t> other) noexcept :
			gamepad { std::move(other.gamepad) },
			disconnect_callback { std::move(other.disconnect_callback) },
			reconnect_callback { std::move(other.reconnect_callback) } {}

		inline ~gamepad_slot_t() noexcept {
			if (gamepad != nullptr) {
				delete gamepad;
				gamepad = nullptr;

				invoke_disconnect();
			}
		}
	};

	struct GamepadManager {
	  private:
		static inline std::map<i32, gamepad_slot_t> slots;

		static inline bool initialized;

	  public:
		static inline bool is_initialized() noexcept { return initialized; }

		static inline void initialize() noexcept {
			if (initialized) {
				return;
			}

			if (SDL_WasInit(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == 0 && SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
				error_log.add("failed to initialize game controller subsystem: {}\n", SDL_GetError());
				return;
			}

			int num_controllers { SDL_NumJoysticks() };

			for (i32 i { 0 }; i < num_controllers; ++i) {
				if (!SDL_IsGameController(i)) {
					error_log.add("joystick {} is not a game controller\n", i);
					continue;
				}

				ptr<joystick_t> gamepad { SDL_JoystickOpen(i) };

				if (gamepad == nullptr) {
					error_log.add("failed to open joystick {}: {}\n", i, SDL_GetError());
					continue;
				}

				slots.emplace(i, gamepad);
			}

			initialized = true;
		}

		static inline void terminate() noexcept {
			if (!initialized) {
				return;
			}

			slots.clear();

			SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

			initialized = false;
		}

		static inline cptr<gamepad_t> lease(i32 id, fn_ptr<void> disconnected_callback, fn_ptr<void, cptr<gamepad_t>> reconnected_callback) noexcept {
			if (disconnected_callback == nullptr || reconnected_callback == nullptr) {
				error_log.add("a callback was not provided for gamepad lease!\n");
				return nullptr;
			}

			auto iter { slots.find(id) };

			if (iter == slots.end()) {
				error_log.add("attempting to lease gamepad that does not exist!\n");
				return nullptr;
			}

			ref<gamepad_slot_t> target { iter->second };

			if (target.has_callbacks()) {
				error_log.add("attempting to lease gamepad that is already leased!\n");
				return nullptr;
			}

			target.disconnect_callback = disconnected_callback;
			target.reconnect_callback = reconnected_callback;

			return target.gamepad;
		}

		static inline void release(i32 id) noexcept {
			auto iter { slots.find(id) };

			if (iter == slots.end()) {
				error_log.add("attempting to release gamepad that does not exist!\n");
				return;
			}

			if (!iter->second.has_callbacks()) {
				error_log.add("attempting to release gamepad that is not leased!\n");
				return;
			}

			iter->second.disconnect_callback = nullptr;
			iter->second.reconnect_callback = nullptr;
		}

		static inline bool add_joystick(int id) noexcept;
		static inline bool remove_joystick(int id) noexcept;

		static inline void update() noexcept {
			for (auto& [id, slot] : slots) {
				if (slot.gamepad != nullptr) {
					slot.gamepad->update();
				}
			}
		}

		static inline void process_event(ref<SDL_Event> event) noexcept {
			switch (event.type) {
			case SDL_JOYDEVICEADDED: {
				if (!add_joystick(event.jdevice.which)) {
					error_log.add("failed to register joystick {}\n", event.jdevice.which);
				}
			} break;

			case SDL_JOYDEVICEREMOVED: {
				if (!remove_joystick(event.jdevice.which)) {
					error_log.add("failed to unregister joystick {}\n", event.jdevice.which);
				}
			} break;

			default:
				break;
			}
		}
	};

	inline bool GamepadManager::add_joystick(int id) noexcept {
		auto it { slots.find(id) };

		if (it != slots.end() && it->second.gamepad != nullptr) {
			error_log.add("joystick {} already connected\n", id);
			return false;
		}

		if (!SDL_IsGameController(id)) {
			error_log.add("joystick {} is not supported\n", id);
			return false;
		}

		ptr<joystick_t> gamepad { SDL_JoystickOpen(id) };

		if (gamepad == nullptr) {
			error_log.add("failed to open joystick {}: {}\n", id, SDL_GetError());
			return false;
		}

		if (it != slots.end()) {
			it->second.gamepad = new gamepad_t { gamepad };
			it->second.invoke_reconnect();
			return true;
		} else {
			slots.emplace(id, gamepad);
		}

		return true;
	}

	inline bool GamepadManager::remove_joystick(int id) noexcept {
		auto it { slots.find(id) };

		if (it == slots.end()) {
			error_log.add("joystick {} not found\n", id);
			return false;
		}

		if (it->second.gamepad == nullptr) {
			error_log.add("joystick {} already disconnected\n", id);
			return false;
		}

		delete it->second.gamepad;
		it->second.gamepad = nullptr;
		it->second.invoke_disconnect();

		return true;
	}
} // namespace Bleakdepth