
#include "skinchanger.hpp"

c_glovechanger glove_changer;

#define INVALID_EHANDLE_INDEX 0xFFFFFFFF

bool c_glovechanger::apply_knife_model(attributable_item_t* weapon, const char* model) noexcept {
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return false;

	auto viewmodel = reinterpret_cast<base_view_model*>(interfaces::entity_list->get_client_entity_handle(local_player->view_model()));

	if (!viewmodel)
		return false;

	auto h_view_model_weapon = viewmodel->m_hweapon();

	if (!h_view_model_weapon)
		return false;

	auto view_model_weapon = reinterpret_cast<attributable_item_t*>(interfaces::entity_list->get_client_entity_handle(h_view_model_weapon));

	if (view_model_weapon != weapon)
		return false;

	viewmodel->model_index() = interfaces::model_info->get_model_index(model);

	return true;
}

bool c_glovechanger::apply_knife_skin(attributable_item_t* weapon, int item_definition_index, int paint_kit, int model_index, int entity_quality, float fallback_wear) noexcept {
	weapon->item_definition_index() = item_definition_index;
	weapon->fallback_paint_kit() = paint_kit;
	weapon->model_index() = model_index;
	weapon->entity_quality() = entity_quality;
	weapon->fallback_wear() = fallback_wear;

	return true;
}

void c_glovechanger::run() noexcept {

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	auto active_weapon = local_player->active_weapon();

	if (!active_weapon)
		return;


	auto model_karambit = "models/weapons/v_knife_karam.mdl";
	auto index_karambit = interfaces::model_info->get_model_index("models/weapons/v_knife_karam.mdl");


	auto my_weapons = local_player->get_weapons();
	for (int i = 0; my_weapons[i] != INVALID_EHANDLE_INDEX; i++) {
		auto my_weapons = local_player->get_weapons();
		for (int i = 0; my_weapons[i] != INVALID_EHANDLE_INDEX; i++) {
			auto weapon = reinterpret_cast<attributable_item_t*>(interfaces::entity_list->get_client_entity_handle(my_weapons[i]));

			float wear = 0.001f;
			int bruh = 3;

			apply_knife_model(weapon, model_karambit);


			//apply knife skins
	//		if (weapon->client_class()->class_id == class_ids::cknife) {
			apply_knife_skin(weapon, WEAPON_KNIFE_KARAMBIT, 0, index_karambit, 3, wear);

			//	}

			weapon->original_owner_xuid_low() = 0;
			weapon->original_owner_xuid_high() = 0;
			weapon->fallback_seed() = 661;
			weapon->item_id_high() = -1;
		}
	}
}
