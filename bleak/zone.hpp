#pragma once

#include <bleak/typedef.hpp>

#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <type_traits>

#include <bleak/applicator.hpp>
#include <bleak/array.hpp>
#include <bleak/atlas.hpp>
#include <bleak/cardinal.hpp>
#include <bleak/concepts.hpp>
#include <bleak/extent.hpp>
#include <bleak/log.hpp>
#include <bleak/octant.hpp>
#include <bleak/offset.hpp>
#include <bleak/primitive.hpp>
#include <bleak/random.hpp>
#include <bleak/renderer.hpp>

#include <bleak/constants/numeric.hpp>

namespace bleak {
	template<typename T, extent_t RegionSize, extent_t ZoneSize, extent_t ZoneBorder = extent_t::Zero> struct region_t;

	enum class zone_region_t : u8 { None = 0, Interior = 1 << 0, Border = 1 << 1, All = Interior | Border };

	template<typename T, extent_t Size, extent_t BorderSize = extent_t{ 0, 0 }> class zone_t {
		static_assert(Size > extent_t::Zero, "Map size must be greater than zero.");
		static_assert(Size >= BorderSize, "Map size must be greater than or equal to border size.");

	  private:
		array_t<T, Size> cells;

	  public:
		static constexpr extent_t zone_size{ Size };
		static constexpr extent_t border_size{ BorderSize };

		static constexpr offset_t zone_origin{ 0 };
		static constexpr offset_t zone_extent{ zone_size - 1 };

		static constexpr offset_t interior_origin{ zone_origin + border_size };
		static constexpr offset_t interior_extent{ zone_extent - border_size };

		static constexpr auto zone_area{ zone_size.area() };

		static constexpr extent_t::product_t interior_area{ (zone_size - border_size).area() };
		static constexpr extent_t::product_t border_area{ zone_area - interior_area };

		static constexpr usize byte_size{ zone_area * sizeof(T) };

		constexpr zone_t() : cells{} {}

		constexpr zone_t(cref<std::string> path) : cells{} {
			std::ifstream file{};

			try {
				file.open(path, std::ios::in | std::ios::binary);

				file.read(reinterpret_cast<str>(cells.data_ptr()), cells.byte_size);

				file.close();
			} catch (std::exception e) {
				error_log.add(e.what(), __TIME_FILE_LINE__);
			}
		}

		constexpr zone_t(cref<zone_t<T, Size, BorderSize>> other) : cells{ other.cells } {};

		constexpr zone_t(rval<zone_t<T, Size, BorderSize>> other) : cells{ std::move(other.cells) } {}

		constexpr ref<zone_t<T, Size, BorderSize>> operator=(cref<zone_t<T, Size, BorderSize>> other) noexcept {
			if (this != &other) {
				cells = other.cells;
			}

			return *this;
		}

		constexpr ref<zone_t<T, Size, BorderSize>> operator=(rval<zone_t<T, Size, BorderSize>> other) noexcept {
			if (this != &other) {
				cells = std::move(other.cells);
			}

			return *this;
		}

		constexpr ~zone_t() noexcept {}

		constexpr cref<array_t<T, Size>> data() const noexcept { return cells; }

		constexpr cptr<array_t<T, Size>> data_ptr() const noexcept { return &cells; }

		constexpr array_t<cref<T>, Size> view() const {
			array_t<cref<T>, Size> view{};

			for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
				view[i] = cells[i];
			}

			return view;
		}

		constexpr array_t<ref<T>, Size> proxy() {
			array_t<ref<T>, Size> proxy{};

			for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
				proxy[i] = ref<T>(cells[i]);
			}

			return proxy;
		}

		constexpr ref<T> operator[](extent_t::product_t index) noexcept { return cells[index]; }

		constexpr cref<T> operator[](extent_t::product_t index) const noexcept { return cells[index]; }

		constexpr ref<T> operator[](extent_t::scalar_t x, extent_t::scalar_t y) noexcept { return cells[x, y]; }

		constexpr cref<T> operator[](extent_t::scalar_t x, extent_t::scalar_t y) const noexcept { return cells[x, y]; }

		constexpr ref<T> operator[](cref<offset_t> position) noexcept { return cells[position]; }

		constexpr cref<T> operator[](cref<offset_t> position) const noexcept { return cells[position]; }

		constexpr bool on_x_edge(cref<offset_t> position) const noexcept { return position.x == zone_origin.x || position.x == zone_extent.x; }

		constexpr bool on_y_edge(cref<offset_t> position) const noexcept { return position.y == zone_origin.y || position.y == zone_extent.y; }

		constexpr bool on_edge(cref<offset_t> position) const noexcept { return on_x_edge(position) || on_y_edge(position); }

		constexpr cardinal_t edge_state(cref<offset_t> position) const noexcept {
			cardinal_t state{ cardinal_t::Central };

			if (!on_edge(position)) {
				return state;
			}

			if (position.x == zone_origin.x) {
				state += cardinal_t::West;
			} else if (position.x == zone_extent.x) {
				state += cardinal_t::East;
			}

			if (position.y == zone_origin.y) {
				state += cardinal_t::North;
			} else if (position.y == zone_extent.y) {
				state += cardinal_t::South;
			}

			return state;
		}

		template<zone_region_t Region> constexpr bool within(cref<offset_t> position) const noexcept {
			if constexpr (Region == zone_region_t::All) {
				return cells.valid(position);
			} else if constexpr (Region == zone_region_t::Interior) {
				return position.x >= interior_origin.x && position.x <= interior_extent.x && position.y >= interior_origin.y && position.y <= interior_extent.y;
			} else if constexpr (Region == zone_region_t::Border) {
				return position.x < interior_origin.x || position.x > interior_extent.x || position.y < interior_origin.y || position.y > interior_extent.y;
			}

			return false;
		}

		template<zone_region_t Region> constexpr ref<zone_t<T, Size, BorderSize>> set(cref<T> value) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] = value;
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] = value;
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] = value;
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] = value;
							cells[zone_extent.x - i, y] = value;
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region, typename U>
			requires std::is_assignable<T, U>::value
		constexpr ref<zone_t<T, Size, BorderSize>> set(cref<U> value) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] = value;
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] = value;
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] = value;
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] = value;
							cells[zone_extent.x - i, y] = value;
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region>
			requires is_operable_unary<T, operator_t::Addition>::value
		constexpr ref<zone_t<T, Size, BorderSize>> apply(cref<T> value) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] += value;
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] += value;
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] += value;
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] += value;
							cells[zone_extent.x - i, y] += value;
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region, typename U>
			requires is_operable<T, U, operator_t::Addition>::value
		constexpr ref<zone_t<T, Size, BorderSize>> apply(cref<U> value) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] += value;
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] += value;
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] += value;
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] += value;
							cells[zone_extent.x - i, y] += value;
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region, typename... Params>
			requires (is_operable<T, Params, operator_t::Addition>::value, ...) && is_plurary<Params...>::value
		constexpr ref<zone_t<T, Size, BorderSize>> apply(cref<Params>... values) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					for (auto value : { values... }) {
						cells[i] += value;
					}
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						for (auto value : { values... }) {
							cells[x, y] += value;
						}
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							for (auto value : { values... }) {
								cells[x, y] += value;
							}
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							for (auto value : { values... }) {
								cells[i, y] += value;
								cells[zone_extent.x - i, y] += value;
							}
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region>
			requires is_operable_unary<T, operator_t::Subtraction>::value
		constexpr ref<zone_t<T, Size, BorderSize>> repeal(cref<T> value) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] -= value;
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] -= value;
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] -= value;
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] -= value;
							cells[zone_extent.x - i, y] -= value;
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region, typename U>
			requires is_operable<T, U, operator_t::Subtraction>::value
		constexpr ref<zone_t<T, Size, BorderSize>> repeal(cref<U> value) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] -= value;
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] -= value;
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] -= value;
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] -= value;
							cells[zone_extent.x - i, y] -= value;
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region, typename... Params>
			requires(is_operable<T, Params, operator_t::Subtraction>::value, ...) && is_plurary<Params...>::value
		constexpr ref<zone_t<T, Size, BorderSize>> repeal(cref<Params>... values) noexcept {
			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					for (auto value : { values... }) {
						cells[i] -= value;
					}
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						for (auto value : { values... }) {
							cells[x, y] -= value;
						}
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							for (auto value : { values... }) {
								cells[x, y] -= value;
							}
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							for (auto value : { values... }) {
								cells[i, y] -= value;
								cells[zone_extent.x - i, y] -= value;
							}
						}
					}
				}
			}

			return *this;
		}

		constexpr void swap(ref<array_t<T, Size>> buffer) noexcept { std::swap(cells, buffer); }

		constexpr void sync(cref<array_t<T, Size>> buffer) noexcept {
			for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
				cells[i] = buffer[i];
			}
		}

		template<zone_region_t Region, typename Randomizer>
		constexpr ref<zone_t<T, Size, BorderSize>> randomize(ref<Randomizer> generator, f64 fill_percent, cref<T> true_value, cref<T> false_value) noexcept
			requires is_random_engine<Randomizer>::value
		{
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			auto dis{ std::bernoulli_distribution{ fill_percent } };

			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] = dis(generator) ? true_value : false_value;
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] = dis(generator) ? true_value : false_value;
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] = dis(generator) ? true_value : false_value;
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] = dis(generator) ? true_value : false_value;
							cells[zone_extent.x - i, y] = dis(generator) ? true_value : false_value;
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region, typename Randomizer>
		constexpr ref<zone_t<T, Size, BorderSize>> randomize(ref<Randomizer> generator, f64 fill_percent, cref<binary_applicator_t<T>> applicator) noexcept
			requires is_random_engine<Randomizer>::value
		{
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			auto dis{ std::bernoulli_distribution{ fill_percent } };

			if constexpr (Region == zone_region_t::All) {
				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					cells[i] = applicator(generator, dis);
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						cells[x, y] = applicator(generator, dis);
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							cells[x, y] = applicator(generator, dis);
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							cells[i, y] = applicator(generator, dis);
							cells[zone_extent.x - i, y] = applicator(generator, dis);
						}
					}
				}
			}

			return *this;
		}

		template<bool Safe = true> constexpr u8 neighbour_count(cref<offset_t> position, cref<T> value) const noexcept {
			u8 count{ 0 };

			if constexpr (Safe) {
				cardinal_t edge{ edge_state(position) };

				if (edge == cardinal_t::Northwest || cells[position + offset_t::Northwest] == value) {
					++count;
				}

				if (edge == cardinal_t::North || cells[position + offset_t::North] == value) {
					++count;
				}

				if (edge == cardinal_t::Northeast || cells[position + offset_t::Northeast] == value) {
					++count;
				}

				if (edge == cardinal_t::West || cells[position + offset_t::West] == value) {
					++count;
				}

				if (edge == cardinal_t::East || cells[position + offset_t::East] == value) {
					++count;
				}

				if (edge == cardinal_t::Southwest || cells[position + offset_t::Southwest] == value) {
					++count;
				}

				if (edge == cardinal_t::South || cells[position + offset_t::South] == value) {
					++count;
				}

				if (edge == cardinal_t::Southeast || cells[position + offset_t::Southeast] == value) {
					++count;
				}
			} else {
				if (cells[position + offset_t::Northwest] == value) {
					++count;
				}
				if (cells[position + offset_t::North] == value) {
					++count;
				}
				if (cells[position + offset_t::Northeast] == value) {
					++count;
				}
				if (cells[position + offset_t::West] == value) {
					++count;
				}
				if (cells[position + offset_t::East] == value) {
					++count;
				}
				if (cells[position + offset_t::Southwest] == value) {
					++count;
				}
				if (cells[position + offset_t::South] == value) {
					++count;
				}
				if (cells[position + offset_t::Southeast] == value) {
					++count;
				}
			}

			return count;
		}

		template<bool Safe = true> constexpr void modulate(ref<array_t<T, Size>> buffer, cref<offset_t> position, u8 threshold, cref<T> true_state, cref<T> false_state) const noexcept {
			u8 neighbours{ neighbour_count<Safe>(position, true_state) };

			if (neighbours > threshold) {
				buffer[position] = true_state;
			} else if (neighbours < threshold) {
				buffer[position] = false_state;
			}
		}

		template<bool Safe = true> constexpr void modulate(ref<array_t<T, Size>> buffer, cref<offset_t> position, u8 threshold, cref<binary_applicator_t<T>> applicator) const noexcept {
			u8 neighbours{ neighbour_count<Safe>(position, applicator.true_value) };

			if (neighbours > threshold) {
				buffer[position] = applicator.true_value;
			} else if (neighbours < threshold) {
				buffer[position] = applicator.false_value;
			}
		}

		template<zone_region_t Region> constexpr ref<zone_t<T, Size, BorderSize>> automatize(ref<array_t<T, Size>> buffer, u8 threshold, cref<T> true_value, cref<T> false_state) const noexcept {
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			if constexpr (Region == zone_region_t::All) {
				for (extent_t::scalar_t y{ zone_origin.y }; y <= zone_extent.y; ++y) {
					for (extent_t::scalar_t x{ zone_origin.x }; x <= zone_extent.x; ++x) {
						modulate(buffer, offset_t{ x, y }, threshold, true_value, false_state);
					}
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				constexpr bool is_not_safe{ border_size.w > 0 && border_size.h > 0 };

				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						modulate<is_not_safe>(buffer, offset_t{ x, y }, threshold, true_value, false_state);
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							modulate(buffer, offset_t{ x, y }, threshold, true_value, false_state);
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							modulate(buffer, offset_t{ i, y }, threshold, true_value, false_state);
							modulate(buffer, offset_t{ zone_extent.x - i, y }, threshold, true_value, false_state);
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region> constexpr ref<zone_t<T, Size, BorderSize>> automatize(ref<array_t<T, Size>> buffer, u8 threshold, cref<binary_applicator_t<T>> applicator) const noexcept {
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			if constexpr (Region == zone_region_t::All) {
				for (extent_t::scalar_t y{ zone_origin.y }; y <= zone_extent.y; ++y) {
					for (extent_t::scalar_t x{ zone_origin.x }; x <= zone_extent.x; ++x) {
						modulate(buffer, { x, y }, threshold, applicator);
					}
				}
			} else if constexpr (Region == zone_region_t::Interior) {
				constexpr bool is_not_safe{ border_size.w > 0 && border_size.h > 0 };

				for (extent_t::scalar_t y{ interior_origin.y }; y <= interior_extent.y; ++y) {
					for (extent_t::scalar_t x{ interior_origin.x }; x <= interior_extent.x; ++x) {
						modulate<is_not_safe>(buffer, { x, y }, threshold, applicator);
					}
				}
			} else if constexpr (Region == zone_region_t::Border) {
				for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
					if (y < interior_origin.y || y > interior_extent.y) {
						for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
							modulate(buffer, { x, y }, threshold, applicator);
						}
					} else {
						for (extent_t::scalar_t i{ 0 }; i < border_size.w; ++i) {
							modulate(buffer, { i, y }, threshold, applicator);
							modulate(buffer, { zone_extent.x - i, y }, threshold, applicator);
						}
					}
				}
			}

			return *this;
		}

		template<zone_region_t Region> constexpr ref<zone_t<T, Size, BorderSize>> automatize(ref<array_t<T, Size>> buffer, u32 iterations, u8 threshold, cref<T> true_value, cref<T> false_state) noexcept {
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			for (u32 i{ 0 }; i < iterations; ++i) {
				automatize<Region>(buffer, threshold, true_value, false_state);
				swap(buffer);
			}

			return *this;
		}

		template<zone_region_t Region> constexpr ref<zone_t<T, Size, BorderSize>> automatize(ref<array_t<T, Size>> buffer, u32 iterations, u8 threshold, cref<binary_applicator_t<T>> applicator) noexcept {
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			for (u32 i{ 0 }; i < iterations; ++i) {
				automatize<Region>(buffer, threshold, applicator);
				swap(buffer);
			}

			return *this;
		}

		template<zone_region_t Region, typename Randomizer>
		constexpr ref<zone_t<T, Size, BorderSize>> generate(ref<Randomizer> generator, f64 fill_percent, u32 iterations, u8 threshold, cref<T> true_value, cref<T> false_state) noexcept
			requires is_random_engine<Randomizer>::value
		{
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			randomize<Region>(generator, fill_percent, true_value, false_state);

			array_t<T, Size> buffer{ cells };

			automatize<Region>(iterations, threshold, true_value, false_state);
			swap(buffer);

			return *this;
		}

		template<zone_region_t Region, typename Randomizer>
		constexpr ref<zone_t<T, Size, BorderSize>> generate(ref<Randomizer> generator, f64 fill_percent, u32 iterations, u8 threshold, cref<binary_applicator_t<T>> applicator) noexcept
			requires is_random_engine<Randomizer>::value
		{
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			randomize<Region>(generator, fill_percent, applicator);

			array_t<T, Size> buffer{ cells };

			automatize<Region>(iterations, threshold, applicator);
			swap(buffer);

			return *this;
		}

		template<zone_region_t Region, typename Randomizer>
		constexpr ref<zone_t<T, Size, BorderSize>> generate(ref<array_t<T, Size>> buffer, ref<Randomizer> generator, f64 fill_percent, u32 iterations, u8 threshold, cref<T> true_value, cref<T> false_state) noexcept
			requires is_random_engine<Randomizer>::value
		{
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			randomize<Region>(generator, fill_percent, true_value, false_state);

			buffer = cells;

			automatize<Region>(iterations, threshold, true_value, false_state);
			swap(buffer);

			return *this;
		}

		template<zone_region_t Region, typename Randomizer>
		constexpr ref<zone_t<T, Size, BorderSize>> generate(ref<array_t<T, Size>> buffer, ref<Randomizer> generator, f64 fill_percent, u32 iterations, u8 threshold, cref<binary_applicator_t<T>> applicator) noexcept
			requires is_random_engine<Randomizer>::value
		{
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			randomize<Region>(generator, fill_percent, applicator);

			buffer = cells;

			automatize<Region>(iterations, threshold, applicator);
			swap(buffer);

			return *this;
		}

		template<zone_region_t Region, typename Randomizer>
		constexpr std::optional<offset_t> find_random(ref<Randomizer> generator, cref<T> value) const noexcept
			requires is_random_engine<Randomizer>::value
		{
			if constexpr (Region == zone_region_t::None) {
				return *this;
			}

			if constexpr (Region == zone_region_t::All) {
				std::uniform_int_distribution<offset_t::scalar_t> x_dis{ 0, zone_extent.x };
				std::uniform_int_distribution<offset_t::scalar_t> y_dis{ 0, zone_extent.y };

				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					const offset_t pos{ x_dis(generator), y_dis(generator) };

					if (cells[pos] == value) {
						return pos;
					}
				}

				return std::nullopt;
			} else if constexpr (Region == zone_region_t::Interior) {
				std::uniform_int_distribution<offset_t::scalar_t> x_dis{ interior_origin.x, interior_extent.x };
				std::uniform_int_distribution<offset_t::scalar_t> y_dis{ interior_origin.y, interior_extent.y };

				for (extent_t::product_t i{ 0 }; i < interior_area; ++i) {
					const offset_t pos{ x_dis(generator), y_dis(generator) };

					if (cells[pos] == value) {
						return pos;
					}
				}

				return std::nullopt;
			} else if constexpr (Region == zone_region_t::Border) {
				std::uniform_int_distribution<offset_t::scalar_t> x_dis{ 0, border_size.w * 2 - 1 };
				std::uniform_int_distribution<offset_t::scalar_t> y_dis{ 0, border_size.h * 2 - 1 };

				for (extent_t::product_t i{ 0 }; i < border_area; ++i) {
					auto x{ x_dis(generator) };
					auto y{ y_dis(generator) };

					x = x < border_size.w ? x : zone_extent.x - x;
					y = y < border_size.h ? y : zone_extent.y - y;

					const offset_t pos{ x, y };

					if (cells[pos] == value) {
						return pos;
					}
				}

				return std::nullopt;
			}

			return std::nullopt;
		}

		template<zone_region_t Region, typename Randomizer, typename U>
		constexpr std::optional<offset_t> find_random(ref<Randomizer> generator, cref<U> value) const noexcept
			requires is_random_engine<Randomizer>::value && is_equatable<T, U>::value
		{
			if constexpr (Region == zone_region_t::All) {
				std::uniform_int_distribution<offset_t::scalar_t> x_dis{ 0, zone_extent.x };
				std::uniform_int_distribution<offset_t::scalar_t> y_dis{ 0, zone_extent.y };

				for (extent_t::product_t i{ 0 }; i < zone_area; ++i) {
					const offset_t pos{ x_dis(generator), y_dis(generator) };

					if (cells[pos] == value) {
						return pos;
					}
				}

				return std::nullopt;
			} else if constexpr (Region == zone_region_t::Interior) {
				std::uniform_int_distribution<offset_t::scalar_t> x_dis{ interior_origin.x, interior_extent.x };
				std::uniform_int_distribution<offset_t::scalar_t> y_dis{ interior_origin.y, interior_extent.y };

				for (extent_t::product_t i{ 0 }; i < interior_area; ++i) {
					const offset_t pos{ x_dis(generator), y_dis(generator) };

					if (cells[pos] == value) {
						return pos;
					}
				}

				return std::nullopt;
			} else if constexpr (Region == zone_region_t::Border) {
				std::uniform_int_distribution<offset_t::scalar_t> x_dis{ 0, border_size.w * 2 - 1 };
				std::uniform_int_distribution<offset_t::scalar_t> y_dis{ 0, border_size.h * 2 - 1 };

				for (extent_t::product_t i{ 0 }; i < border_area; ++i) {
					auto x{ x_dis(generator) };
					auto y{ y_dis(generator) };

					x = x < border_size.w ? x : zone_extent.x - x;
					y = y < border_size.h ? y : zone_extent.y - y;

					const offset_t pos{ x, y };

					if (cells[pos] == value) {
						return pos;
					}
				}

				return std::nullopt;
			}

			return std::nullopt;
		}

		template<extent_t AtlasSize>
		constexpr void draw(cref<atlas_t<AtlasSize>> atlas) const noexcept
			requires is_drawable<T>::value
		{
			for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
				for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
					const offset_t pos{ x, y };
					(*this)[pos].draw(atlas, pos);
				}
			}
		}

		template<extent_t AtlasSize>
		constexpr void draw(cref<atlas_t<AtlasSize>> atlas, cref<offset_t> offset) const noexcept
			requires is_drawable<T>::value
		{
			for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
				for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
					const offset_t pos{ x, y };
					(*this)[pos].draw(atlas, pos + offset);
				}
			}
		}

		template<extent_t AtlasSize>
		constexpr void draw(cref<atlas_t<AtlasSize>> atlas, cref<offset_t> offset, cref<extent_t> scale) const noexcept
			requires is_drawable<T>::value
		{
			for (extent_t::scalar_t y{ 0 }; y < zone_size.h; ++y) {
				for (extent_t::scalar_t x{ 0 }; x < zone_size.w; ++x) {
					const offset_t pos{ x, y };
					(*this)[pos].draw(atlas, pos + offset, scale);
				}
			}
		}

		constexpr cstr serialize() const noexcept { return reinterpret_cast<cstr>(cells.data_ptr()); }

		constexpr bool serialize(cref<std::string> path) const noexcept {
			std::ofstream file{};

			try {
				file.open(path, std::ios::out | std::ios::binary);

				file.write(reinterpret_cast<cstr>(cells.data_ptr()), cells.byte_size);

				file.close();
			} catch (cref<std::exception> e) {
				error_log.add(e.what(), __TIME_FILE_LINE__);
				return false;
			}

			return true;
		}

		constexpr void deserialize(cstr binary_data) noexcept { std::memcpy(reinterpret_cast<str>(cells.data_ptr()), binary_data, cells.byte_size); }
	};
} // namespace bleak
