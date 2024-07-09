#pragma once

#include "bleak/offset/offset_1d.hpp"
#include "bleak/typedef.hpp"

#include <initializer_list>

#include "bleak/extent.hpp"
#include "bleak/offset.hpp"
#include "extent/extent_1d.hpp"

namespace bleak {
	template<typename T> struct fwd_iter_t;
	template<typename T> struct rev_iter_t;

	template<extent_1d_t Size> static constexpr extent_1d_t::product_t flatten(cref<offset_1d_t> offset) noexcept;
	template<extent_2d_t Size> static constexpr extent_2d_t::product_t flatten(cref<offset_2d_t> offset) noexcept;
	template<extent_3d_t Size> static constexpr extent_3d_t::product_t flatten(cref<offset_3d_t> offset) noexcept;

	template<extent_1d_t Size> static constexpr offset_1d_t unflatten(cref<extent_1d_t::product_t> index) noexcept;
	template<extent_2d_t Size> static constexpr offset_2d_t unflatten(cref<extent_2d_t::product_t> index) noexcept;
	template<extent_3d_t Size> static constexpr offset_3d_t unflatten(cref<extent_3d_t::product_t> index) noexcept;

	template<typename T, typename OffsetType, typename ExtentType, ExtentType Size> class array_t {
	  private:
		ptr<T> data;

	  public:
		using offset_t = OffsetType;
		using extent_t = ExtentType;

		using index_t = offset_t::product_t;
		using position_t = offset_t::scalar_t;

		static constexpr usize size{ Size.size() };

		static constexpr usize byte_size{ size * sizeof(T) };

		constexpr ptr<T> data_ptr() noexcept { return data; }

		constexpr cptr<T> data_ptr() const noexcept { return data; }

		static_assert(size > 0, "array must have a size greater than zero!");
		static_assert(size * sizeof(T) <= memory::maximum, "array must not exceed the maximum size of an array!");

		static constexpr usize first{ 0 };
		static constexpr usize last{ size - 1 };

		using iterator = fwd_iter_t<T>;
		using const_iterator = fwd_iter_t<const T>;

		using reverse_iterator = rev_iter_t<T>;
		using const_reverse_iterator = rev_iter_t<const T>;

		constexpr array_t();

		constexpr array_t(std::initializer_list<T> elements);

		constexpr array_t(cref<array_t> other);

		constexpr array_t(rval<array_t> other) noexcept;

		constexpr ref<array_t> operator=(cref<array_t> other) noexcept;

		constexpr ref<array_t> operator=(rval<array_t> other) noexcept;

		constexpr ~array_t() noexcept;

		constexpr ref<T> operator[](offset_t offset) noexcept;

		constexpr cref<T> operator[](offset_t offset) const noexcept;

		constexpr bool valid(offset_t offset) const noexcept;

		constexpr ref<T> at(offset_t offset);

		constexpr cref<T> at(offset_t offset) const;

		constexpr ref<T> front() noexcept;

		constexpr cref<T> front() const noexcept;

		constexpr ref<T> back() noexcept;

		constexpr cref<T> back() const noexcept;

		constexpr iterator begin() noexcept;

		constexpr iterator end() noexcept;

		constexpr const_iterator begin() const noexcept;

		constexpr const_iterator end() const noexcept;

		constexpr const_iterator cbegin() const noexcept;

		constexpr const_iterator cend() const noexcept;

		constexpr reverse_iterator rbegin() noexcept;

		constexpr reverse_iterator rend() noexcept;

		constexpr const_reverse_iterator rbegin() const noexcept;

		constexpr const_reverse_iterator rend() const noexcept;

		constexpr const_reverse_iterator crbegin() const noexcept;

		constexpr const_reverse_iterator crend() const noexcept;
	};
} // namespace bleak

#include "bleak/array/array.tpp" // IWYU pragma: export

#include "bleak/array/iter.hpp" // IWYU pragma: export

namespace bleak {
	template<typename T, extent_1d_t Size> using row_t = array_t<T, offset_1d_t, extent_1d_t, Size>;

	template<typename T, extent_2d_t Size> using layer_t = array_t<T, offset_2d_t, extent_2d_t, Size>;

	template<typename T, extent_3d_t Size> using volume_t = array_t<T, offset_3d_t, extent_3d_t, Size>;
} // namespace bleak