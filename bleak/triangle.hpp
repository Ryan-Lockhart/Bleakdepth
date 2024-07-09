#pragma once

#include "bleak/primitive.hpp"

#include <cstdlib>

#include "bleak/color.hpp"
#include "bleak/extent.hpp"
#include "bleak/offset.hpp"
#include "bleak/renderer.hpp"

namespace bleak {
	struct triangle_2d_t {
		using thickness_t = extent_2d_t::scalar_t;

		offset_2d_t vertices[3]{ 0, 0, 0 };

		constexpr triangle_2d_t() {}

		constexpr triangle_2d_t(cref<offset_2d_t> p1, cref<offset_2d_t> p2, cref<offset_2d_t> p3) noexcept : vertices{ p1, p2, p3 } {}

		constexpr offset_2d_t::product_t perimeter() const noexcept {
			return offset_2d_t::distance<offset_2d_t::product_t>(vertices[0], vertices[1]) + offset_2d_t::distance<offset_2d_t::product_t>(vertices[1], vertices[2])
				 + offset_2d_t::distance<offset_2d_t::product_t>(vertices[2], vertices[0]);
		}

		constexpr offset_1d_t::product_t area() const noexcept {
			return std::abs(vertices[0].x * (vertices[1].y - vertices[2].y) + vertices[1].x * (vertices[2].y - vertices[0].y) + vertices[2].x * (vertices[0].y - vertices[1].y)) / 2;
		}

		template<fill_type_t Fill> inline void draw(ref<renderer_t> renderer, cref<color_t>) const noexcept;

		template<fill_type_t Fill> inline void draw(ref<renderer_t> renderer, thickness_t thickness, cref<color_t>) const noexcept;

		template<fill_type_t Fill> inline void draw(ref<renderer_t> renderer, thickness_t thickness, cref<color_t>, cref<color_t>) const noexcept;

		template<> inline void draw<fill_type_t::Outline>(ref<renderer_t> renderer, cref<color_t> outline) const noexcept {
			renderer.draw_line(vertices[0], vertices[1], outline);
			renderer.draw_line(vertices[1], vertices[2], outline);
			renderer.draw_line(vertices[2], vertices[0], outline);
		}

		template<> inline void draw<fill_type_t::Outline>(ref<renderer_t> renderer, thickness_t thickness, cref<color_t> outline) const noexcept {
			renderer.draw_line(vertices[0], vertices[1], outline, thickness);
			renderer.draw_line(vertices[1], vertices[2], outline, thickness);
			renderer.draw_line(vertices[2], vertices[0], outline, thickness);
		}

		template<> inline void draw<fill_type_t::Outline>(ref<renderer_t> renderer, thickness_t thickness, cref<color_t> fill, cref<color_t> outline) const noexcept = delete;

		template<> inline void draw<fill_type_t::Fill>(ref<renderer_t> renderer, cref<color_t> fill) const noexcept {}

		template<> inline void draw<fill_type_t::Fill>(ref<renderer_t> renderer, thickness_t thickness, cref<color_t> fill) const noexcept = delete;

		template<> inline void draw<fill_type_t::Fill>(ref<renderer_t> renderer, thickness_t thickness, cref<color_t> fill, cref<color_t> outline) const noexcept = delete;

		template<fill_type_t Fill> inline void draw(ref<renderer_t> renderer, cref<color_t>, extent_2d_t::scalar_t) const noexcept;

		template<fill_type_t Fill> inline void draw(ref<renderer_t> renderer, cref<color_t>, cref<color_t>) const noexcept;
		template<fill_type_t Fill> inline void draw(ref<renderer_t> renderer, cref<color_t>, cref<color_t>, extent_2d_t::scalar_t) const noexcept;
	};
} // namespace bleak