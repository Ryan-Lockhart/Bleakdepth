#pragma once

#include "bleak/typedef.hpp" // IWYU pragma: export

#include <SDL.h>

extern "C" {
	struct c_cardinal_t;
	struct c_quadrant_t;

	struct c_offset_1d_t;
	struct c_offset_2d_t;
	struct c_offset_3d_t;

	struct c_extent_1d_t;
	struct c_extent_2d_t;
	struct c_extent_3d_t;

	struct c_line_2d_t;
	struct c_line_3d_t;

	struct c_rect_t;
	struct c_cuboid_t;

	struct c_color_t;
	struct c_glyph_t;
}

namespace bleak {
	namespace sdl {
		using point = SDL_Point;
		using fpoint = SDL_FPoint;

		using rect = SDL_Rect;
		using frect = SDL_FRect;

		using color = SDL_Color;
	} // namespace sdl

	struct cardinal_t;
	struct quadrant_t;

	struct offset_1d_t;
	struct offset_2d_t;
	struct offset_3d_t;

	struct extent_1d_t;
	struct extent_2d_t;
	struct extent_3d_t;

	struct line_2d_t;
	struct line_3d_t;

	struct rect_t;
	struct cuboid_t;

	struct color_t;
	struct glyph_t;

	template<typename T> constexpr bool is_1d = false;
	template<typename T>
	concept Is1D = is_1d<T>;

	template<> inline constexpr bool is_1d<offset_1d_t> = true;
	template<> inline constexpr bool is_1d<extent_1d_t> = true;

	template<typename T> constexpr bool is_2d = false;
	template<typename T>
	concept Is2D = is_2d<T>;

	template<> inline constexpr bool is_2d<offset_2d_t> = true;
	template<> inline constexpr bool is_2d<extent_2d_t> = true;

	template<typename T> constexpr bool is_3d = false;
	template<typename T>
	concept Is3D = is_3d<T>;

	template<> inline constexpr bool is_3d<offset_3d_t> = true;
	template<> inline constexpr bool is_3d<extent_3d_t> = true;

	enum class primitive_type_t {
		Point,
		Line,
		Triangle,
		Rectangle,
		Circle,
		Ellipse,
		Capsule,
	};

	enum class fill_type_t {
		Outline,
		Fill,
		Composite,
	};

	template<primitive_type_t Primitive, fill_type_t Fill> struct primitive_t;

	template<> struct primitive_t<primitive_type_t::Point, fill_type_t::Outline>;
	template<> struct primitive_t<primitive_type_t::Point, fill_type_t::Fill>;
	template<> struct primitive_t<primitive_type_t::Point, fill_type_t::Composite>;

	template<> struct primitive_t<primitive_type_t::Line, fill_type_t::Outline>;
	template<> struct primitive_t<primitive_type_t::Line, fill_type_t::Fill>;
	template<> struct primitive_t<primitive_type_t::Line, fill_type_t::Composite>;

	template<> struct primitive_t<primitive_type_t::Triangle, fill_type_t::Outline>;
	template<> struct primitive_t<primitive_type_t::Triangle, fill_type_t::Fill>;
	template<> struct primitive_t<primitive_type_t::Triangle, fill_type_t::Composite>;

	template<> struct primitive_t<primitive_type_t::Rectangle, fill_type_t::Outline>;
	template<> struct primitive_t<primitive_type_t::Rectangle, fill_type_t::Fill>;
	template<> struct primitive_t<primitive_type_t::Rectangle, fill_type_t::Composite>;

	template<> struct primitive_t<primitive_type_t::Circle, fill_type_t::Outline>;
	template<> struct primitive_t<primitive_type_t::Circle, fill_type_t::Fill>;
	template<> struct primitive_t<primitive_type_t::Circle, fill_type_t::Composite>;

	template<> struct primitive_t<primitive_type_t::Ellipse, fill_type_t::Outline>;
	template<> struct primitive_t<primitive_type_t::Ellipse, fill_type_t::Fill>;
	template<> struct primitive_t<primitive_type_t::Ellipse, fill_type_t::Composite>;

	template<> struct primitive_t<primitive_type_t::Capsule, fill_type_t::Outline>;
	template<> struct primitive_t<primitive_type_t::Capsule, fill_type_t::Fill>;
	template<> struct primitive_t<primitive_type_t::Capsule, fill_type_t::Composite>;
} // namespace bleak