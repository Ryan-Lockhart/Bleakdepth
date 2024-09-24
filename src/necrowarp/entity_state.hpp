#pragma once

#include <necrowarp/entities/entity.hpp>

#include <queue>

#include <necrowarp/game_state.hpp>

namespace necrowarp {
	using namespace bleak;

	struct entity_registry_t {
		inline entity_type_t at(cref<offset_t> position) const noexcept;

		template<typename T>
			requires is_entity<T>::value
		inline cptr<T> at(cref<offset_t> position) const noexcept;

		template<typename T>
			requires is_entity<T>::value
		inline ptr<T> at(cref<offset_t> position) noexcept;

		inline usize count() const noexcept;

		template<typename... EntityTypes>
			requires((is_entity<EntityTypes>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::Player>::value && ...))
		inline usize count() const noexcept;

		template<entity_type_t EntityType> inline usize count() const noexcept;

		inline bool empty() const noexcept { return false; }

		template<typename... EntityTypes>
			requires((is_entity<EntityTypes>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::None>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::Player>::value && ...))
		inline bool empty() const noexcept;

		template<entity_type_t EntityType> inline bool empty() const noexcept;

		inline bool contains(cref<offset_t> position) const noexcept;

		template<typename... EntityTypes>
			requires((is_entity<EntityTypes>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::Player>::value && ...))
		inline bool contains(cref<offset_t> position) const noexcept;

		template<entity_type_t EntityType> inline bool contains(cref<offset_t> position) const noexcept;

		template<bool Force = false, typename T>
			requires is_entity<T>::value && (!is_entity_type<T, entity_type_t::Player>::value)
		inline bool add(rval<T> entity) noexcept;

		template<entity_type_t EntityType> inline bool remove(cref<offset_t> position) noexcept;

		template<typename T>
			requires is_entity<T>::value && (!is_entity_type<T, entity_type_t::Player>::value)
		inline bool spawn(usize count) noexcept;

		inline bool update(cref<offset_t> current, cref<offset_t> target) noexcept;

		template<entity_type_t EntityType> inline bool update(cref<offset_t> current, cref<offset_t> target) noexcept;

		inline bool is_command_valid(cref<entity_command_t> command) const noexcept;

		inline bool random_warp(cref<offset_t> source) noexcept;

		template<entity_type_t Victim>
			requires (Victim != entity_type_t::None)
		inline bool process_clash(cref<offset_t> target_position, i8 damage_amount) noexcept;

		template<command_type_t CommandType> inline void process_command(cref<entity_command_t> command) noexcept;

		inline void process_commands(ref<std::queue<entity_command_t>> commands) noexcept;

		inline void process_command(cref<entity_command_t> command) noexcept;

		inline void update() noexcept;

		template<typename... EntityTypes>
			requires((is_entity<EntityTypes>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::Player>::value && ...))
		inline void update(ref<std::queue<entity_command_t>> commands) noexcept;

		inline void recalculate_goal_maps() noexcept;

		template<typename... EntityTypes>
			requires((is_entity<EntityTypes>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::None>::value && ...))
		inline void recalculate_goal_maps() noexcept;

		inline void recalculate_good_goal_map() noexcept;

		inline void recalculate_evil_goal_map() noexcept;

		inline void recalculate_alignment_goal_maps() noexcept;

		template<typename... EntityTypes>
			requires((is_entity<EntityTypes>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::Player>::value && ...))
		inline void draw() const noexcept;

		template<typename... EntityTypes>
			requires((is_entity<EntityTypes>::value && ...) && !(is_entity_type<EntityTypes, entity_type_t::Player>::value && ...))
		inline void draw(cref<camera_t> camera) const noexcept;

		inline void draw() const noexcept;

		inline void draw(cref<camera_t> camera) const noexcept;
	} static inline entity_registry{};
} // namespace necrowarp
