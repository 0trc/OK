#include "visuals.h"
#include "../../menu/menu.hpp"

void visuals::run() {
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));





	if (!local_player)
		return;

	for (int i = 1; i <= interfaces::globals->max_clients; i++) 
	{
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
		auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

		if (!entity)
			continue;

		if (entity->team() == local_player->team() && !cfg.team_check)
			continue;


		if (entity == csgo::local_player)
			continue;

		if (entity->health() <= 0)
			continue;


		visuals::player_esp(entity);

		if (cfg.radar)
			entity->spotted() = true;

	}
	
}



void visuals::player_esp(player_t* entity)
{

	if (entity->dormant())
		return;

	player_info_t info;
	interfaces::engine->get_player_info(entity->index(), &info);
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
	
	box b_box;
	if (!get_playerbox(entity, b_box))
		return;



	if (cfg.defusing)
	{
		if (entity->is_defusing())
		{
			render::draw_text_string(b_box.x + (b_box.w / 2), b_box.y - 27, render::fonts::esp_font, "DEFUSING", true, color(255, 255, 255, cfg.esp_opacity));
		}
	}


	if (cfg.box_esp == 1)
	{
		render::draw_outline(b_box.x - 1, b_box.y - 1, b_box.w + 2, b_box.h + 2, color(0, 0, 0, cfg.esp_opacity - 30));
		render::draw_rect(b_box.x, b_box.y, b_box.w, b_box.h, color(255, 255, 255, cfg.esp_opacity - 30));
		render::draw_outline(b_box.x + 1, b_box.y + 1, b_box.w - 2, b_box.h - 2, color(0, 0, 0, cfg.esp_opacity - 30));
	}



	if (cfg.name_esp == 1)
	{

		render::draw_text_string(b_box.x + (b_box.w / 2), b_box.y - 15, render::fonts::esp_font, info.name, true, color(255, 255, 255, cfg.esp_opacity));
	}



	if (cfg.health_bar == 1)
	{
		box temp(b_box.x - 5, b_box.y + (b_box.h - b_box.h * (utilities::math::clamp_value<int>(entity->health(), 0, 100.f) / 100.f)), 1, b_box.h * (utilities::math::clamp_value<int>(entity->health(), 0, 100) / 100.f) - (entity->health() >= 100 ? 0 : -1));
		box temp_bg(b_box.x - 5, b_box.y, 1, b_box.h);


		if (entity->health() <= 100)
		{
			auto health_color = color(0, 255, 0, cfg.esp_opacity - 30);
			render::draw_filled_rect(temp_bg.x - 1, temp_bg.y - 1, temp_bg.w + 2 + 1, temp_bg.h + 2, color(0, 0, 0, cfg.esp_opacity - 30));
			render::draw_filled_rect(temp.x, temp.y, temp.w + 1, temp.h, color(health_color));
		}
		if (entity->health() <= 75)
		{
			auto  health_color = color(255, 255, 0, cfg.esp_opacity - 30);
			render::draw_filled_rect(temp_bg.x - 1, temp_bg.y - 1, temp_bg.w + 2 + 1, temp_bg.h + 2, color(0, 0, 0, cfg.esp_opacity - 30));
			render::draw_filled_rect(temp.x, temp.y, temp.w + 1, temp.h, color(health_color));
		}
		if (entity->health() <= 50)
		{
			auto health_color = color(255, 150, 0, cfg.esp_opacity - 30);
			render::draw_filled_rect(temp_bg.x - 1, temp_bg.y - 1, temp_bg.w + 2 + 1, temp_bg.h + 2, color(0, 0, 0, cfg.esp_opacity - 30));
			render::draw_filled_rect(temp.x, temp.y, temp.w + 1, temp.h, color(health_color));
		}
		if (entity->health() <= 25)
		{
			auto health_color = color(255, 0, 0, cfg.esp_opacity - 30);
			render::draw_filled_rect(temp_bg.x - 1, temp_bg.y - 1, temp_bg.w + 2 + 1, temp_bg.h + 2, color(0, 0, 0, cfg.esp_opacity - 30));
			render::draw_filled_rect(temp.x, temp.y, temp.w + 1, temp.h, color(health_color));
		}

	}
	if (cfg.weapon_esp == 1)
	{
		auto weapon = entity->active_weapon();

		if (!weapon)
			return;

		std::string names;
		names = clean_item_name(weapon->get_weapon_data()->m_szWeaponName);

		render::draw_text_string(b_box.x + (b_box.w / 2), b_box.h + b_box.y + 2, render::fonts::esp_font, names, true, color(255, 255, 255, cfg.esp_opacity));
	}
}