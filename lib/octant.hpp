#pragma once

#include "typedef.hpp"

#include <utility>

namespace Bleakdepth {
	template<typename T = i32> struct octant_t {
		T x, dx, y, dy;

		constexpr octant_t() = delete;

		constexpr octant_t(T x, T dx, T y, T dy) : x(x), dx(dx), y(y), dy(dy) {}

		constexpr octant_t(cref<octant_t> other) : x(other.x), dx(other.dx), y(other.y), dy(other.dy) {}

		constexpr octant_t(rval<octant_t> other) :
			x(std::move(other.x)),
			dx(std::move(other.dx)),
			y(std::move(other.y)),
			dy(std::move(other.dy)) {}

		constexpr ~octant_t() = default;

		constexpr octant_t& operator=(cref<octant_t> other) {
			x = other.x;
			dx = other.dx;

			y = other.y;
			dy = other.dy;

			return *this;
		}

		constexpr octant_t& operator=(rval<octant_t> other) {
			x = std::move(other.x);
			dx = std::move(other.dx);

			y = std::move(other.y);
			dy = std::move(other.dy);

			return *this;
		}
	};
} // namespace Bleakdepth