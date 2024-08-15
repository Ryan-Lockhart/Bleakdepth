#pragma once

#include <bleak/typedef.hpp>

#include <bleak/concepts.hpp>
#include <bleak/random.hpp>
#include <type_traits>

namespace bleak {
	template<typename T> struct binary_applicator_t {
		const T true_value;
		const T false_value;

		constexpr binary_applicator_t() noexcept = delete;

		constexpr binary_applicator_t(cref<T> true_value, cref<T> false_value) noexcept : true_value{ true_value }, false_value{ false_value } {}

		constexpr binary_applicator_t(rval<T> true_value, rval<T> false_value) noexcept : true_value{ std::move(true_value) }, false_value{ std::move(false_value) } {}

		constexpr binary_applicator_t(cref<binary_applicator_t<T>> other) noexcept = default;

		constexpr binary_applicator_t(rval<binary_applicator_t<T>> other) noexcept = delete;

		constexpr ref<binary_applicator_t<T>> operator=(cref<binary_applicator_t<T>> other) noexcept = delete;

		constexpr ref<binary_applicator_t<T>> operator=(rval<binary_applicator_t<T>> other) noexcept = delete;

		constexpr ~binary_applicator_t() noexcept = default;

		constexpr T operator()(bool condition) const noexcept { return condition ? true_value : false_value; }

		template<typename Randomizer>
			requires is_random_engine<Randomizer>::value
		constexpr T operator()(ref<Randomizer> generator, f64 probability) const noexcept {
			return std::bernoulli_distribution{ probability }(generator) ? true_value : false_value;
		}

		template<typename Randomizer, typename Distribution>
			requires is_random_engine<Randomizer>::value && is_random_distribution<Distribution>::value && is_numeric<typename Distribution::result_type>::value
		constexpr T operator()(ref<Randomizer> generator, ref<Distribution> distribution) const noexcept {
			return distribution(generator) ? true_value : false_value;
		}
	};

	template<typename T>
		requires is_comparable<T, T, comparator_t::Ordered>::value
	struct ternary_applicator_t {
		const T greater_value;
		const T equal_value;
		const T less_value;

		constexpr ternary_applicator_t() noexcept = delete;

		constexpr ternary_applicator_t(cref<T> greater_value, cref<T> equal_value, cref<T> less_value) noexcept : greater_value{ greater_value }, equal_value{ equal_value }, less_value{ less_value } {}

		constexpr ternary_applicator_t(rval<T> greater_value, rval<T> equal_value, rval<T> less_value) noexcept : greater_value{ std::move(greater_value) }, equal_value{ std::move(equal_value) }, less_value{ std::move(less_value) } {}

		constexpr ternary_applicator_t(cref<ternary_applicator_t<T>> other) noexcept = default;

		constexpr ternary_applicator_t(rval<ternary_applicator_t<T>> other) noexcept = delete;

		constexpr ref<ternary_applicator_t<T>> operator=(cref<ternary_applicator_t<T>> other) noexcept = delete;

		constexpr ref<ternary_applicator_t<T>> operator=(rval<ternary_applicator_t<T>> other) noexcept = delete;

		constexpr ~ternary_applicator_t() noexcept = default;

		constexpr T operator()(i32 target) const noexcept {
			if (target > 0) {
				return greater_value;
			} else if (target < 0) {
				return less_value;
			} else {
				return equal_value;
			}
		}

		template<typename Randomizer>
			requires is_random_engine<Randomizer>::value
		constexpr T operator()(ref<Randomizer> generator) const noexcept {
			const i32 value{ std::uniform_int_distribution<i32>{ -1, 1 }(generator) };

			if (value > 0) {
				return greater_value;
			} else if (value < 0) {
				return less_value;
			} else {
				return equal_value;
			}
		}

		template<typename Randomizer, typename Distribution>
			requires is_random_engine<Randomizer>::value && is_random_distribution<Distribution>::value && is_numeric<typename Distribution::result_type>::value
		constexpr T operator()(ref<Randomizer> generator, ref<Distribution> distribution, typename Distribution::result_type target) const noexcept {
			const auto value{ distribution(generator) };

			if (value > target) {
				return greater_value;
			} else if (value < target) {
				return less_value;
			} else {
				return equal_value;
			}
		}
	};

	template<typename T, Numeric N>
		requires std::is_convertible<T, N>::value
	struct numeric_applicator_t {
		const T minimum_value;
		const T maximum_value;

		constexpr numeric_applicator_t() noexcept = delete;

		constexpr numeric_applicator_t(cref<T> minimum_value, cref<T> maximum_value) noexcept : minimum_value{ minimum_value }, maximum_value{ maximum_value } {}

		constexpr numeric_applicator_t(rval<T> minimum_value, rval<T> maximum_value) noexcept : minimum_value{ std::move(minimum_value) }, maximum_value{ std::move(maximum_value) } {}

		constexpr numeric_applicator_t(cref<numeric_applicator_t<T, N>> other) noexcept = default;

		constexpr numeric_applicator_t(rval<numeric_applicator_t<T, N>> other) noexcept = delete;

		constexpr ref<numeric_applicator_t<T, N>> operator=(cref<numeric_applicator_t<T, N>> other) noexcept = delete;

		constexpr ref<numeric_applicator_t<T, N>> operator=(rval<numeric_applicator_t<T, N>> other) noexcept = delete;

		constexpr ~numeric_applicator_t() noexcept = default;

		template<typename Randomizer>
			requires is_random_engine<Randomizer>::value
		constexpr T operator()(ref<Randomizer> generator) const noexcept {
			const N value{ std::uniform_int_distribution<N>{ static_cast<N>(minimum_value), static_cast<N>(maximum_value) }(generator) };

			return static_cast<T>(value);
		}
	};
} // namespace bleak
