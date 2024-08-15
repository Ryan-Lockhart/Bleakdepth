#pragma once

#include <bleak/typedef.hpp>

#include <format>

#include <SDL.h>

#include <bleak/extent.hpp>
#include <bleak/gamepad.hpp>
#include <bleak/keyboard.hpp>
#include <bleak/log.hpp>
#include <bleak/mouse.hpp>
#include <bleak/offset.hpp>
#include <bleak/rect.hpp>
#include <bleak/subsystem.hpp>

namespace bleak {
	namespace sdl {
		using window = SDL_Window;

		using window_flags = SDL_WindowFlags;

		constexpr window_flags WINDOW_FLAGS_NONE{ SDL_WINDOW_SHOWN };
		constexpr i32 WINDOW_POSITION_CENTERED{ SDL_WINDOWPOS_CENTERED };

		static inline ptr<window> create_window(cstr title, cref<extent_t> size) noexcept {
			ptr<window> handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<i32>(size.w), static_cast<i32>(size.h), WINDOW_FLAGS_NONE);

			if (handle == nullptr) {
				error_log.add("failed to create window: {}", get_error());
			}

			return handle;
		}

		static inline ptr<window> create_window(cstr title, cref<extent_t> size, window_flags flags) noexcept {
			ptr<window> handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<i32>(size.w), static_cast<i32>(size.h), flags);

			if (handle == nullptr) {
				error_log.add("failed to create window: {}", get_error());
			}

			return handle;
		}

		static inline ptr<window> create_window(cstr title, cref<extent_t> size, cref<offset_t> position, window_flags flags) noexcept {
			ptr<window> handle = SDL_CreateWindow(title, static_cast<i32>(position.x), static_cast<i32>(position.y), static_cast<i32>(size.w), static_cast<i32>(size.h), flags);

			if (handle == nullptr) {
				error_log.add("failed to create window: {}", get_error());
			}

			return handle;
		}

		static inline void destroy_window(ref<ptr<window>> handle) noexcept {
			if (handle != nullptr) {
				SDL_DestroyWindow(handle);
				handle = nullptr;
			} else {
				error_log.add("cannot destroy window: window handle is null!");
			}
		}
	} // namespace sdl

	class window_t {
	  private:
		ptr<sdl::window> window;

	  public:
		const cstr title;
		const extent_t size;
		const u32 flags;

	  private:
		bool closing = false;

	  public:
		inline window_t() = delete;

		inline window_t(cstr title, cref<extent_t> size, sdl::window_flags flags) noexcept : window{ sdl::create_window(title, size, flags) }, title{ title }, size{ size }, flags{ flags } {}

		inline window_t(cstr title, cref<offset_t> position, cref<extent_t> size, sdl::window_flags flags) noexcept : window{ sdl::create_window(title, size, position, flags) }, title{ title }, size{ size }, flags{ flags } {}

		inline window_t(cref<window_t> other) noexcept = delete;

		inline window_t(rval<window_t> other) noexcept : window(std::move(other.window)), title(std::move(other.title)), size(std::move(other.size)), flags(std::move(other.flags)) { other.window = nullptr; }

		inline ref<window_t> operator=(cref<window_t> other) noexcept = delete;

		inline ref<window_t> operator=(rval<window_t> other) noexcept = delete;

		inline ~window_t() { sdl::destroy_window(window); }

		inline void poll_events() {
			if (!is_valid()) {
				return;
			}

			SDL_Event event;

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					closing = true;
					break;

				case SDL_MOUSEMOTION:
				case SDL_MOUSEWHEEL:
					Mouse::process_event(event);
					break;

				case SDL_JOYDEVICEADDED:
				case SDL_JOYDEVICEREMOVED:
					GamepadManager::process_event(event);
					break;

				default:
					break;
				}
			}

			GamepadManager::update();
			Keyboard::update();
			Mouse::update();
		}

		constexpr bool is_valid() const noexcept { return window != nullptr; }

		constexpr bool is_closing() const noexcept { return closing; }

		constexpr bool is_running() const noexcept { return !closing; }

		constexpr void close() noexcept { closing = true; }

		constexpr ptr<sdl::window> handle() noexcept { return window; }

		constexpr cptr<sdl::window> handle() const noexcept { return window; }

		constexpr offset_t origin() const noexcept { return offset_t::Zero; }

		constexpr offset_t center() const noexcept { return origin() + size / 2; }

		constexpr offset_t extents() const noexcept { return origin() + size - 1; }

		constexpr rect_t bounds() const noexcept { return { origin(), size }; }
	};
} // namespace bleak