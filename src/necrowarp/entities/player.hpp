#pragma once

#include <necrowarp/entities/entity.hpp>

#include <necrowarp/game_state.hpp>

namespace necrowarp {
	using namespace bleak;

	template<> struct is_entity<player_t> {
		static constexpr bool value = true;
	};

	template<> struct is_entity_type<player_t, entity_type_t::Player> {
		static constexpr bool value = true;
	};

	template<> struct to_entity_type<entity_type_t::Player> {
		using type = player_t;
	};

	template<> struct is_evil_entity<player_t> {
		static constexpr bool value = true;
	};

	template<> struct is_animate<player_t> {
		static constexpr bool value = true;
	};

	template<> struct is_non_player_entity<player_t> {
		static constexpr bool value = false;
	};

	template<> struct is_player<player_t> {
		static constexpr bool value = true;
	};

	template<> inline constexpr glyph_t entity_glyphs<player_t>{ 0x41, colors::White };

	struct player_t {
		entity_command_t command;
		offset_t position;

		static constexpr i8 MinimumEnergy{ 4 };
		static constexpr i8 MaximumEnergy{ 24 };

		static constexpr i8 MinimumArmor{ 2 };
		static constexpr i8 MaximumArmor{ 12 };

		static constexpr i8 MinimumDivinity{ 3 };
		static constexpr i8 MaximumDivinity{ 9 };

		static constexpr i8 StartingEnergy{ 3 };
		static constexpr i8 StartingArmor{ 0 };
		static constexpr i8 StartingDivinity{ 0 };

		static constexpr i8 MaximumDamage{ 1 };
		static constexpr i8 MinimumDamage{ 1 };

		static constexpr i8 RandomWarpCost{ 1 };
		static constexpr i8 TargetWarpCost{ 2 };

		static constexpr i8 CalciticInvocationCost{ 4 };
		static constexpr i8 SanguinaryInvocationCost{ 4 };
		static constexpr i8 SpectralInvocationCost{ 4 };

		static constexpr i8 NecromanticAscendanceCost{ MaximumEnergy };

		static constexpr i8 SkullBoon{ 1 };
		static constexpr i8 FailedWarpBoon{ 1 };
		static constexpr i8 UnsafeWarpBoon{ 1 };

	  private:
		i8 energy;
		i8 armor;
		i8 divinity;

		inline void set_energy(i8 value) noexcept { energy = clamp<i8>(value, 0, max_energy()); }

		inline void set_armor(i8 value) noexcept { armor = clamp<i8>(value, 0, max_armor()); }

		inline void set_divinity(i8 value) noexcept { divinity = clamp<i8>(value, 0, max_divinity()); }

	  public:
		inline player_t() noexcept : command{}, position{}, energy{ StartingEnergy }, armor{ StartingArmor }, divinity{ StartingDivinity } {}

		inline player_t(cref<offset_t> position) noexcept : command{}, position{ position }, energy{ StartingEnergy }, armor{ StartingArmor }, divinity{ StartingDivinity } {}

		inline i8 get_energy() const noexcept { return energy; }

		inline i8 get_armor() const noexcept { return armor; }

		inline bool has_energy() const noexcept { return energy > 0; }

		inline bool has_armor() const noexcept { return armor > 0; }

		inline bool has_ascended() const noexcept { return divinity > 0; }

		inline i8 max_energy() const noexcept { return clamp(static_cast<i8>(game_stats.minion_kills / globals::KillsPerEnergySlot), MinimumEnergy, MaximumEnergy); }

		inline i8 max_armor() const noexcept { return clamp(static_cast<i8>(game_stats.player_kills / globals::KillsPerArmorSlot), MinimumArmor, MaximumArmor); }

		inline i8 max_divinity() const noexcept { return clamp(static_cast<i8>((game_stats.total_kills() - globals::KillsPerEnergySlot * player_t::MaximumEnergy) / globals::KillsPerDivinityTurn), MinimumDivinity, MaximumDivinity); }

		inline bool can_survive(i8 damage_amount) const noexcept { return armor >= damage_amount; }

		inline void receive_damage(i8 damage_amount) noexcept { set_armor(armor - damage_amount); }

		inline bool can_random_warp() const noexcept { return energy >= RandomWarpCost; }

		inline bool can_random_warp(i8 discount) const noexcept { return energy >= RandomWarpCost - discount; }

		inline bool can_target_warp() const noexcept { return energy >= TargetWarpCost; }

		inline bool can_target_warp(i8 discount) const noexcept { return energy >= TargetWarpCost - discount; }

		inline bool can_perform_calcitic_invocation() const noexcept { return energy >= CalciticInvocationCost; }

		inline bool can_perform_calcitic_invocation(i8 discount) const noexcept { return energy >= CalciticInvocationCost - discount; }

		inline bool can_perform_spectral_invocation() const noexcept { return energy >= SpectralInvocationCost; }

		inline bool can_perform_spectral_invocation(i8 discount) const noexcept { return energy >= SpectralInvocationCost - discount; }

		inline bool can_perform_sanguinary_invocation() const noexcept { return energy >= SanguinaryInvocationCost; }

		inline bool can_perform_sanguinary_invocation(i8 discount) const noexcept { return energy >= SanguinaryInvocationCost - discount; }

		inline bool can_perform_necromantic_ascendance() const noexcept { return energy >= NecromanticAscendanceCost; }

		inline bool can_perform_necromantic_ascendance(i8 discount) const noexcept { return energy >= NecromanticAscendanceCost - discount; }

		inline void pay_random_warp_cost() noexcept { set_energy(energy - RandomWarpCost); }

		inline void pay_random_warp_cost(i8 discount) noexcept { set_energy(energy - RandomWarpCost + discount); }

		inline void pay_target_warp_cost() noexcept { set_energy(energy - TargetWarpCost); }

		inline void pay_target_warp_cost(i8 discount) noexcept { set_energy(energy - TargetWarpCost + discount); }

		inline void pay_calcitic_invocation_cost() noexcept { set_energy(energy - CalciticInvocationCost); }

		inline void pay_calcitic_invocation_cost(i8 discount) noexcept { set_energy(energy - CalciticInvocationCost + discount); }

		inline void pay_spectral_invocation_cost() noexcept { set_energy(energy - SpectralInvocationCost); }

		inline void pay_spectral_invocation_cost(i8 discount) noexcept { set_energy(energy - SpectralInvocationCost + discount); }

		inline void pay_sanguinary_invocation_cost() noexcept { set_energy(energy - SanguinaryInvocationCost); }

		inline void pay_sanguinary_invocation_cost(i8 discount) noexcept { set_energy(energy - SanguinaryInvocationCost + discount); }

		inline void pay_necromantic_ascendance_cost() noexcept { set_energy(energy - NecromanticAscendanceCost); }

		inline void pay_necromantic_ascendance_cost(i8 discount) noexcept { set_energy(energy - NecromanticAscendanceCost + discount); }

		inline void receive_skull_boon() noexcept { set_energy(energy + SkullBoon); }

		inline void receive_failed_warp_boon() noexcept { set_energy(energy + FailedWarpBoon); }

		inline void receive_unsafe_warp_boon() noexcept { set_energy(energy + UnsafeWarpBoon); }

		inline void max_out_energy() noexcept { energy = max_energy(); }

		inline void max_out_armor() noexcept { armor = max_armor(); }

		inline void zero_out_energy() noexcept { energy = 0; }

		inline void zero_out_armor() noexcept { armor = 0; }

		template<entity_type_t EntityType> inline bool will_perish() const noexcept;

		template<entity_type_t EntityType> inline void receive_damage() noexcept;

		template<entity_type_t EntityType> inline void receive_death_boon() noexcept;

		inline command_type_t clash_or_consume(cref<offset_t> position) const noexcept;

		inline void bolster_armor(i8 value) noexcept { set_armor(armor + value); }

		inline void draw() const noexcept { game_atlas.draw(has_armor() ? PlayerArmoredGlyph : entity_glyphs<player_t>, position); }

		inline void draw(cref<offset_t> offset) const noexcept { game_atlas.draw(has_armor() ? PlayerArmoredGlyph : entity_glyphs<player_t>, position + offset); }

		inline void draw(cref<camera_t> camera) const noexcept { game_atlas.draw(has_armor() ? PlayerArmoredGlyph : entity_glyphs<player_t>, position + camera.get_offset()); }

		inline void draw(cref<camera_t> camera, cref<offset_t> offset) const noexcept { game_atlas.draw(has_armor() ? PlayerArmoredGlyph : entity_glyphs<player_t>, position + camera.get_offset() + offset); }

		constexpr operator entity_type_t() const noexcept { return entity_type_t::Player; }
	};
} // namespace necrowarp
