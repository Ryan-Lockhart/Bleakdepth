#include "bleak/primitive.hpp"

#include "bleak/extent.hpp"
#include "bleak/rect.hpp"

#include "bleak/color.hpp"

namespace bleak {
	template<> struct primitive_t<primitive_type_t::Line, fill_type_t::Outline> {
		rect_t rect;

		extent_2d_t::product_t thickness;

		color_t color;
	};

	template<> struct primitive_t<primitive_type_t::Line, fill_type_t::Fill> {
		rect_t rect;

		color_t color;
	};

	template<> struct primitive_t<primitive_type_t::Line, fill_type_t::Composite> {
		rect_t rect;

		extent_2d_t::product_t thickness;

		color_t outline_color;
		color_t fill_color;
	};
} // namespace bleak