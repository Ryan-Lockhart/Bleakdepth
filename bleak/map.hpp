#pragma once

#include "bleak/offset/offset_2d.hpp"
#include "bleak/typedef.hpp"

#include <algorithm>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <string>

#include "bleak/array.hpp"
#include "bleak/atlas.hpp"
#include "bleak/cardinal.hpp"
#include "bleak/extent.hpp"
#include "bleak/offset.hpp"
#include "extent/extent_2d.hpp"

namespace bleak {
	// generator for map randomization is fixed due to lack of support for function template partial specialization
	using MapRandomizer = std::minstd_rand;

	enum class cell_trait_t : u8 { Open, Solid, Transperant, Opaque, Seen, Explored, Unseen, Unexplored, Dry, Damp, Cold, Warm, Odorless, Smelly, Safe, Toxic };

	struct cell_state_t {
	  public:
		bool solid : 1 { false };
		bool opaque : 1 { false };
		bool seen : 1 { false };
		bool explored : 1 { false };
		bool damp : 1 { false };
		bool warm : 1 { false };
		bool smelly : 1 { false };
		bool toxic : 1 { false };

		constexpr cell_state_t() noexcept = default;

		constexpr cell_state_t(cref<cell_state_t> other) noexcept :
			solid{ other.solid },
			opaque{ other.opaque },
			seen{ other.seen },
			explored{ other.explored },
			damp{ other.damp },
			warm{ other.warm },
			smelly{ other.smelly },
			toxic{ other.toxic } {}

		constexpr cell_state_t(rval<cell_state_t> other) noexcept :
			solid{ other.solid },
			opaque{ other.opaque },
			seen{ other.seen },
			explored{ other.explored },
			damp{ other.damp },
			warm{ other.warm },
			smelly{ other.smelly },
			toxic{ other.toxic } {}

		constexpr ref<cell_state_t> operator=(cref<cell_state_t> other) noexcept {
			if (this == &other) {
				return *this;
			}

			solid = other.solid;
			opaque = other.opaque;
			seen = other.seen;
			explored = other.explored;
			damp = other.damp;
			warm = other.warm;
			smelly = other.smelly;
			toxic = other.toxic;

			return *this;
		}

		constexpr ref<cell_state_t> operator=(rval<cell_state_t> other) noexcept {
			if (this == &other) {
				return *this;
			}

			solid = other.solid;
			opaque = other.opaque;
			seen = other.seen;
			explored = other.explored;
			damp = other.damp;
			warm = other.warm;
			smelly = other.smelly;
			toxic = other.toxic;

			return *this;
		}

		template<typename... Traits> constexpr cell_state_t(Traits... traits)
		requires (sizeof...(Traits) > 0) && (std::is_same_v<Traits, cell_trait_t> && ...){
			for (cell_trait_t trait : { traits... }) {
				set(trait);
			}
		}

		constexpr inline cell_state_t operator+(cell_trait_t trait) const noexcept {
			cell_state_t state{ *this };

			state.set(trait);

			return state;
		}

		constexpr inline ref<cell_state_t> operator+=(cell_trait_t trait) noexcept {
			set(trait);

			return *this;
		}

		constexpr inline cell_state_t operator-(cell_trait_t trait) const noexcept {
			cell_state_t state{ *this };

			state.unset(trait);

			return state;
		}

		constexpr inline ref<cell_state_t> operator-=(cell_trait_t trait) noexcept {
			unset(trait);

			return *this;
		}

		constexpr void set(cell_trait_t trait) noexcept {
			switch (trait) {
			case cell_trait_t::Open:
				solid = false;
				break;
			case cell_trait_t::Solid:
				solid = true;
				break;
			case cell_trait_t::Transperant:
				opaque = false;
				break;
			case cell_trait_t::Opaque:
				opaque = true;
				break;
			case cell_trait_t::Unseen:
				seen = false;
				break;
			case cell_trait_t::Seen:
				seen = true;
				break;
			case cell_trait_t::Unexplored:
				explored = false;
				break;
			case cell_trait_t::Explored:
				explored = true;
				break;
			case cell_trait_t::Dry:
				damp = false;
				break;
			case cell_trait_t::Damp:
				damp = true;
				break;
			case cell_trait_t::Cold:
				warm = false;
				break;
			case cell_trait_t::Warm:
				warm = true;
				break;
			case cell_trait_t::Smelly:
				smelly = true;
				break;
			case cell_trait_t::Odorless:
				smelly = false;
				break;
			case cell_trait_t::Toxic:
				toxic = true;
				break;
			case cell_trait_t::Safe:
				toxic = false;
				break;
			default:
				break;
			}
		}

		constexpr void unset(cell_trait_t trait) noexcept {
			switch (trait) {
			case cell_trait_t::Open:
				solid = true;
				break;
			case cell_trait_t::Solid:
				solid = false;
				break;
			case cell_trait_t::Transperant:
				opaque = true;
				break;
			case cell_trait_t::Opaque:
				opaque = false;
				break;
			case cell_trait_t::Unseen:
				seen = true;
				break;
			case cell_trait_t::Seen:
				seen = false;
				break;
			case cell_trait_t::Unexplored:
				explored = true;
				break;
			case cell_trait_t::Explored:
				explored = false;
				break;
			case cell_trait_t::Dry:
				damp = true;
				break;
			case cell_trait_t::Damp:
				damp = false;
				break;
			case cell_trait_t::Cold:
				warm = true;
				break;
			case cell_trait_t::Warm:
				warm = false;
				break;
			case cell_trait_t::Smelly:
				smelly = false;
				break;
			case cell_trait_t::Odorless:
				smelly = true;
				break;
			case cell_trait_t::Toxic:
				toxic = false;
				break;
			case cell_trait_t::Safe:
				toxic = true;
				break;
			default:
				break;
			}
		}

		constexpr bool operator==(cref<cell_state_t> other) const noexcept {
			return solid == other.solid && opaque == other.opaque && seen == other.seen && explored == other.explored && damp == other.damp
				&& warm == other.warm && smelly == other.smelly && toxic == other.toxic;
		}

		constexpr bool operator!=(cref<cell_state_t> other) const noexcept {
			return solid != other.solid || opaque != other.opaque || seen != other.seen || explored != other.explored || damp != other.damp
				|| warm != other.warm || smelly != other.smelly || toxic != other.toxic;
		}

		constexpr bool contains(cell_trait_t trait) const noexcept {
			switch (trait) {
			case cell_trait_t::Solid:
				return solid;
			case cell_trait_t::Open:
				return !solid;
			case cell_trait_t::Opaque:
				return opaque;
			case cell_trait_t::Transperant:
				return !opaque;
			case cell_trait_t::Seen:
				return seen;
			case cell_trait_t::Unseen:
				return !seen;
			case cell_trait_t::Explored:
				return explored;
			case cell_trait_t::Unexplored:
				return !explored;
			case cell_trait_t::Damp:
				return damp;
			case cell_trait_t::Dry:
				return !damp;
			case cell_trait_t::Warm:
				return warm;
			case cell_trait_t::Cold:
				return !warm;
			case cell_trait_t::Smelly:
				return smelly;
			case cell_trait_t::Odorless:
				return !smelly;
			case cell_trait_t::Toxic:
				return toxic;
			case cell_trait_t::Safe:
				return !toxic;
			default:
				return false;
			}
		}

		inline constexpr std::string to_tooltip() const {
			return std::format(
				"The cell is physically {} and visibly {}.\nThe cell is {} and is {} by the player.\nIt is {} and {} to the touch.\nThe air within is {} and "
				"{}.",
				solid ? "blocked" : "open",
				seen ? "blocked" : "open",
				solid ? "in view" : "not in view",
				explored ? "explored" : "unexplored",
				damp ? "damp" : "dry",
				warm ? "warm" : "cold",
				smelly ? "pungent" : "odorless",
				toxic ? "toxic" : "harmless"
			);
		}

		inline constexpr operator std::string() const {
			return std::format(
				"[{}, {}, {}, {}, {}, {}, {}, {}]",
				solid ? "Solid" : "Open",
				opaque ? "Opaque" : "Transperant",
				seen ? "Seen" : "Unseen",
				explored ? "Explored" : "Unexplored",
				damp ? "Damp" : "Dry",
				warm ? "Warm" : "Cold",
				smelly ? "Smelly" : "Odorless",
				toxic ? "Toxic" : "Safe"
			);
		}
	};

	enum class map_region_t : u8 { None = 0, Interior = 1 << 0, Border = 1 << 1, All = Interior | Border };

	template<extent_2d_t MapSize, extent_2d_t BorderSize = { 0, 0 }> class map_t {
	  private:
		layer_t<cell_state_t, MapSize> state;

	  public:
		static constexpr extent_2d_t map_size{ MapSize };
		static constexpr extent_2d_t border_size{ BorderSize };

		static constexpr offset_2d_t map_origin{ 0 };
		static constexpr offset_2d_t map_extent{ map_size - 1 };

		static constexpr offset_2d_t border_origin{ map_origin + border_size };
		static constexpr offset_2d_t border_extent{ map_extent - border_size };

		static constexpr auto map_area{ map_size.area() };

		static constexpr usize interior_area{ (map_size - border_size).area() };
		static constexpr usize border_area{ map_area - interior_area };

		inline map_t() : state{} {}

		inline map_t(cref<map_t<MapSize, BorderSize>> other) : state{ other.state } {}

		inline map_t(rval<map_t<MapSize, BorderSize>> other) : state{ std::move(other.state) } {}

		inline ref<map_t<MapSize, BorderSize>> operator=(cref<map_t<MapSize, BorderSize>> other) noexcept {
			if (this != &other) {
				state = other.state;
			}

			return *this;
		}

		inline ref<map_t<MapSize, BorderSize>> operator=(rval<map_t<MapSize, BorderSize>> other) noexcept {
			if (this != &other) {
				state = std::move(other.state);
			}

			return *this;
		}

		inline ~map_t() noexcept {}

		inline cref<layer_t<cell_state_t, MapSize>> data() const noexcept { return state; }

		inline cptr<layer_t<cell_state_t, MapSize>> data_ptr() const noexcept { return &state; }

		inline ref<cell_state_t> operator[](usize index) noexcept { return state[index]; }

		inline cref<cell_state_t> operator[](usize index) const noexcept { return state[index]; }

		inline ref<cell_state_t> operator[](uhalf x, uhalf y) noexcept { return state[x, y]; }

		inline cref<cell_state_t> operator[](uhalf x, uhalf y) const noexcept { return state[x, y]; }

		inline ref<cell_state_t> operator[](cref<offset_2d_t> position) noexcept { return state[position]; }

		inline cref<cell_state_t> operator[](cref<offset_2d_t> position) const noexcept { return state[position]; }

		inline bool on_x_edge(cref<offset_2d_t> position) const noexcept { return position.x == map_origin.x || position.x == map_extent.x; }

		inline bool on_y_edge(cref<offset_2d_t> position) const noexcept { return position.y == map_origin.y || position.y == map_extent.y; }

		inline bool on_edge(cref<offset_2d_t> position) const noexcept { return on_x_edge(position) || on_y_edge(position); }

		inline cardinal_t edge_state(cref<offset_2d_t> position) const noexcept {
			cardinal_t state{ cardinal_t::Central };

			if (!on_edge(position)) {
				return state;
			}

			if (position.x == map_origin.x) {
				state += cardinal_t::West;
			} else if (position.x == map_extent.x) {
				state += cardinal_t::East;
			}

			if (position.y == map_origin.y) {
				state += cardinal_t::North;
			} else if (position.y == map_extent.y) {
				state += cardinal_t::South;
			}

			return state;
		}

		template<map_region_t Region> inline ref<map_t<MapSize, BorderSize>> set(cell_state_t cell_state);

		template<map_region_t Region>
		inline ref<map_t<MapSize, BorderSize>> randomize(ref<MapRandomizer> generator, f64 fill_percent, cell_state_t true_state, cell_state_t false_state);

		template<> inline ref<map_t<MapSize, BorderSize>> set<map_region_t::All>(cell_state_t cell_state) {
			for (usize i{ 0 }; i < map_area; ++i) {
				state[i].set(cell_state);
			}

			return *this;
		}

		template<> inline ref<map_t<MapSize, BorderSize>> set<map_region_t::Interior>(cell_state_t cell_state) {
			for (uhalf y{ border_origin.y }; y <= border_extent.y; ++y) {
				for (uhalf x{ border_origin.x }; x <= border_extent.x; ++x) {
					state[x, y] = cell_state;
				}
			}

			return *this;
		}

		template<> inline ref<map_t<MapSize, BorderSize>> set<map_region_t::Border>(cell_state_t cell_state) {
			for (int y{ 0 }; y < map_size.h; ++y) {
				if (y < border_origin.y || y > border_extent.y) {
					for (int x{ 0 }; x < map_size.w; ++x) {
						state[offset_2d_t{x, y}] = cell_state;
					}
				} else {
					for (int i{ 0 }; i < border_size.w; ++i) {
						state[offset_2d_t{i, y}] = cell_state;
						state[offset_2d_t{map_extent.x - i, y}] = cell_state;
					}
				}
			}

			return *this;
		}

		template<> inline ref<map_t<MapSize, BorderSize>> set<map_region_t::None>(cell_state_t cell_state) { return *this; }

		template<>
		inline ref<map_t<MapSize, BorderSize>>
		randomize<map_region_t::All>(ref<MapRandomizer> generator, f64 fill_percent, cell_state_t true_state, cell_state_t false_state) {
			auto dis{ std::bernoulli_distribution{ fill_percent } };

			for (usize i{ 0 }; i < map_area; ++i) {
				state[i] = dis(generator) ? true_state : false_state;
			}

			return *this;
		}

		template<>
		inline ref<map_t<MapSize, BorderSize>>
		randomize<map_region_t::Interior>(ref<MapRandomizer> generator, f64 fill_percent, cell_state_t true_state, cell_state_t false_state) {
			auto dis{ std::bernoulli_distribution{ fill_percent } };

			for (uhalf y{ border_origin.y }; y <= border_extent.y; ++y) {
				for (uhalf x{ border_origin.x }; x <= border_extent.x; ++x) {
					state[offset_2d_t{x, y}] = dis(generator) ? true_state : false_state;
				}
			}

			return *this;
		}

		template<>
		inline ref<map_t<MapSize, BorderSize>>
		randomize<map_region_t::Border>(ref<MapRandomizer> generator, f64 fill_percent, cell_state_t true_state, cell_state_t false_state) {
			auto dis{ std::bernoulli_distribution{ fill_percent } };

			for (int y{ 0 }; y < map_size.h; ++y) {
				if (y < border_origin.y || y > border_extent.y) {
					for (int x{ 0 }; x < map_size.w; ++x) {
						state[x, y] = dis(generator) ? true_state : false_state;
					}
				} else {
					for (int i{ 0 }; i < border_size.w; ++i) {
						state[i, y] = dis(generator) ? true_state : false_state;
						state[map_extent.x - i, y] = dis(generator) ? true_state : false_state;
					}
				}
			}

			return *this;
		}

		template<>
		inline ref<map_t<MapSize, BorderSize>>
		randomize<map_region_t::None>(ref<MapRandomizer> generator, f64 fill_percent, cell_state_t true_state, cell_state_t false_state) {
			return *this;
		}

		inline u8 neighbour_count(cref<offset_2d_t> position, cell_state_t mask) const noexcept {
			u8 count{ 0 };
			cardinal_t edge{ edge_state(position) };

			if (edge != cardinal_t::Northwest && state[position + offset_2d_t::northwest].contains(mask)) {
				++count;
			}

			if (edge != cardinal_t::North && state[position + offset_2d_t::north].contains(mask)) {
				++count;
			}

			if (edge != cardinal_t::Northeast && state[position + offset_2d_t::northeast].contains(mask)) {
				++count;
			}

			if (edge != cardinal_t::West && state[position + offset_2d_t::west].contains(mask)) {
				++count;
			}

			if (edge != cardinal_t::East && state[position + offset_2d_t::east].contains(mask)) {
				++count;
			}

			if (edge != cardinal_t::Southwest && state[position + offset_2d_t::southwest].contains(mask)) {
				++count;
			}

			if (edge != cardinal_t::South && state[position + offset_2d_t::south].contains(mask)) {
				++count;
			}

			if (edge != cardinal_t::Southeast && state[position + offset_2d_t::southeast].contains(mask)) {
				++count;
			}

			return count;
		}

		template<typename Generator> inline std::optional<offset_2d_t> find_random_cell(ref<Generator> generator, cell_trait_t trait) const {
			std::uniform_int_distribution<offset_2d_t::scalar_t> x_dis{ 0, map_extent.x };
			std::uniform_int_distribution<offset_2d_t::scalar_t> y_dis{ 0, map_extent.y };

			for (usize i{ 0 }; i < map_area; ++i) {
				const offset_2d_t pos{ x_dis(generator), y_dis(generator) };

				if (state[pos].contains(trait)) {
					return pos;
				}
			}

			return std::nullopt;
		}

		template<typename Generator> inline std::optional<offset_2d_t> find_random_cell_interior(ref<Generator> generator, cell_trait_t trait) const {
			std::uniform_int_distribution<uhalf> x_dis{ border_origin.x, border_extent.x };
			std::uniform_int_distribution<uhalf> y_dis{ border_origin.y, border_extent.y };

			for (usize i{ 0 }; i < interior_area; ++i) {
				const offset_2d_t pos{ x_dis(generator), y_dis(generator) };

				if (state[pos].contains(trait)) {
					return pos;
				}
			}

			return std::nullopt;
		}

		template<extent_2d_t AtlasSize> inline void draw(cref<atlas_t<AtlasSize>> atlas) const {
			for (uhalf y{ 0 }; y < AtlasSize.h; ++y) {
				for (uhalf x{ 0 }; x < AtlasSize.w; ++x) {
					const usize idx{ state.flatten(x, y) };
					const cell_state_t cell_state{ operator[](idx) };

					if (cell_state.contains(cell_trait_t::Unexplored)) {
						continue;
					}

					const bool is_solid{ cell_state.contains(cell_trait_t::Solid) };
					const bool is_seen{ cell_state.contains(cell_trait_t::Seen) };

					const u8 rgb{ is_solid ? u8{ 0xC0 } : u8{ 0x40 } };
					const u8 alpha{ is_seen ? u8{ 0xFF } : u8{ 0x80 } };
					const u8 glyph{ is_solid ? u8{ 0xB2 } : u8{ 0xB0 } };

					atlas.draw({ glyph, { rgb, rgb, rgb, alpha } }, { static_cast<i32>(x), static_cast<i32>(y) });
				}
			}
		}

		template<extent_2d_t AtlasSize> inline void draw(cref<atlas_t<AtlasSize>> atlas, cref<offset_2d_t> offset) const {
			for (uhalf y{ 0 }; y < AtlasSize.h; ++y) {
				for (uhalf x{ 0 }; x < AtlasSize.w; ++x) {
					const offset_2d_t pos{ x, y };
					const usize idx{ flatten<AtlasSize>(pos) };
					const cell_state_t cell_state{ operator[](idx) };

					if (cell_state.contains(cell_trait_t::Unexplored)) {
						continue;
					}

					const bool is_solid{ cell_state.contains(cell_trait_t::Solid) };
					const bool is_seen{ cell_state.contains(cell_trait_t::Seen) };

					const u8 rgb{ is_solid ? u8{ 0xC0 } : u8{ 0x40 } };
					const u8 alpha{ is_seen ? u8{ 0xFF } : u8{ 0x80 } };
					const u8 glyph{ is_solid ? u8{ 0xB2 } : u8{ 0xB0 } };

					atlas.draw({ glyph, { rgb, rgb, rgb, alpha } }, pos + offset);
				}
			}
		}

		inline bool serialize(cref<std::string> path, cref<std::string> name) const {
			std::ofstream file{};

			try {
				file.open(std::format("{}\\{}.map.bin", path, name), std::ios::out | std::ios::binary);

				file.write(reinterpret_cast<cstr>(state.data_ptr()), state.byte_size);

				file.close();
			} catch (std::exception e) {
				std::cerr << e.what() << std::endl;
				return false;
			}

			return true;
		}
	};
} // namespace bleak