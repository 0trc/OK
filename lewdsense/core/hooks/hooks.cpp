#pragma once
#include "../../dependencies/common_includes.hpp"
#include "../features/features.hpp"
#include "../features/misc/engine_prediction.hpp"
#include "../menu/menu.hpp"
#include "../features/misc/backtrack.h"
#include "../features/misc/skinchanger.hpp"

/*how to get entity:

	for (int i = 1; i <= interfaces::globals->max_clients; i++) {
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
	}

*/

hooks::create_move::fn create_move_original = nullptr;
hooks::paint_traverse::fn paint_traverse_original = nullptr;
hooks::in_key_event::fn in_key_event_original = nullptr;
hooks::frame_stage_notify::fn frame_stage_notify_original = nullptr;
hooks::scene_end::fn scene_end_original = nullptr;
unsigned int get_virtual(void* class_, unsigned int index) { return (unsigned int)(*(int**)class_)[index]; }

bool hooks::initialize() {
	auto create_move_target = reinterpret_cast<void*>(get_virtual(interfaces::clientmode, 24));
	auto paint_traverse_target = reinterpret_cast<void*>(get_virtual(interfaces::panel, 41));
	auto in_key_event_target = reinterpret_cast<void*>(get_virtual(interfaces::client, 21));
	auto frame_stage_notify_target = reinterpret_cast<void*>(get_virtual(interfaces::client, 37));
	auto scene_end_target = reinterpret_cast<void*>(get_virtual(interfaces::render_view, 9));

	if (MH_Initialize() != MH_OK) {
		throw std::runtime_error("failed to initialize MH_Initialize.");
		return false;
	}

	if (MH_CreateHook(create_move_target, &create_move::hook, reinterpret_cast<void**>(&create_move_original)) != MH_OK) {
		throw std::runtime_error("failed to initialize create_move. (outdated index?)");
		return false;
	}
	if (MH_CreateHook(scene_end_target, &scene_end::hook, reinterpret_cast<void**>(&scene_end_original)) != MH_OK) {
		throw std::runtime_error("scene_end failed");
		return false;
	}
	if (MH_CreateHook(frame_stage_notify_target, &frame_stage_notify::hook, reinterpret_cast<void**>(&frame_stage_notify_original)) != MH_OK) {
		throw std::runtime_error("frame_stage");
		return false;
	}
	if (MH_CreateHook(paint_traverse_target, &paint_traverse::hook, reinterpret_cast<void**>(&paint_traverse_original)) != MH_OK) {
		throw std::runtime_error("failed to initialize paint_traverse. (outdated index?)");
		return false;
	}
	if (MH_CreateHook(in_key_event_target, &in_key_event::hook, reinterpret_cast<void**>(&in_key_event_original)) != MH_OK) {
		throw std::runtime_error("failed to initialize in_key_event. (outdated index?)");
		return false;
	}

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		throw std::runtime_error("failed to enable hooks.");
		return false;
	}

#ifdef debug_build
	console::log("[setup] hooks initialized!\n");
#endif
	return true;
}
void __stdcall hooks::frame_stage_notify::hook(client_frame_stage_t frame_stage)
{
	if (frame_stage == FRAME_NET_UPDATE_END && interfaces::engine->is_in_game())
	{
		backtracking->Update(interfaces::globals->tick_count);

	}
	frame_stage_notify_original(interfaces::client, frame_stage);
}
void hooks::release() {
	MH_Uninitialize();

	MH_DisableHook(MH_ALL_HOOKS);
}
bool is_behind_smoke(vec3_t start_pos, vec3_t end_pos) {
	typedef bool(__cdecl* line_goes_through_smoke) (vec3_t, vec3_t);
	static line_goes_through_smoke line_goes_through_smoke_fn = 0;

	if (!line_goes_through_smoke_fn)
		line_goes_through_smoke_fn = reinterpret_cast<line_goes_through_smoke>(utilities::pattern_scan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0"));

	if (line_goes_through_smoke_fn)
		return line_goes_through_smoke_fn(start_pos, end_pos);
	return false;
}
void __stdcall hooks::scene_end::hook()
{

	for (int i = 1; i <= interfaces::globals->max_clients; i++) {
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
		auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

		if (!entity || !entity->is_alive() || entity->dormant() || !local_player)
			continue;





		bool teammate = entity->team() == local_player->team();
		bool enemy = entity->team() != local_player->team();






		static i_material* mat = nullptr;
		auto textured = interfaces::material_system->find_material("debug/debugambientcube", TEXTURE_GROUP_MODEL, true, nullptr);
		textured->increment_reference_count();  //we need increment_reference_count cuz without it our materialsystem.dll will crash after  map change 
		mat = textured;


		float chams_enemy[4]{ cfg.chamcolorred / 255.f, cfg.chamcolorgreen / 255.f, cfg.chamcolorblue / 255.f, 255 / 255.f };
		float chams_enemy_xqz[4]{ cfg.chamxqzred / 255.f, cfg.chamxqzgreen / 255.f, cfg.chamxqzblue / 255.f, 255 / 255.f };




		if (enemy) {
			if (cfg.chams_enemy == 1) {

				if (cfg.chams_enemy_xqz == 1) {
					if (is_behind_smoke(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_head)) && cfg.throughsmoke)
					{
						interfaces::render_view->modulate_color(chams_enemy_xqz);
						interfaces::render_view->set_blend(0.2f);
						mat->set_material_var_flag(material_var_ignorez, true);

						interfaces::model_render->override_material(mat);
						entity->draw_model(1, 255);
					}
					if (!(is_behind_smoke(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_head))))
					{
						interfaces::render_view->modulate_color(chams_enemy_xqz);
						interfaces::render_view->set_blend(1.0f);
						mat->set_material_var_flag(material_var_ignorez, true);

						interfaces::model_render->override_material(mat);
						entity->draw_model(1, 255);
					}

				}


				if (is_behind_smoke(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_head)) && cfg.throughsmoke)
				{
					interfaces::render_view->modulate_color(chams_enemy);
					interfaces::render_view->set_blend(0.2f);
					mat->set_material_var_flag(material_var_ignorez, false);

					interfaces::model_render->override_material(mat);
					entity->draw_model(1, 255);
				}
				if (!(is_behind_smoke(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_head))))
				{
					interfaces::render_view->modulate_color(chams_enemy);
					interfaces::render_view->set_blend(1.f);
					mat->set_material_var_flag(material_var_ignorez, false);


					interfaces::model_render->override_material(mat);
					entity->draw_model(1, 255);
				}
			}
		}

		interfaces::model_render->override_material(nullptr);
	}
	return scene_end_original(interfaces::render_view);
}
bool fasz2 = true;
bool __fastcall hooks::create_move::hook(void* ecx, void* edx, int input_sample_frametime, c_usercmd* cmd) {
	create_move_original(input_sample_frametime, cmd);

	if (!cmd || !cmd->command_number)
		return create_move_original(input_sample_frametime, cmd);

	csgo::local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	uintptr_t* frame_pointer;
	__asm mov frame_pointer, ebp;
	bool& send_packet = *reinterpret_cast<bool*>(*frame_pointer - 0x1C);

	auto old_viewangles = cmd->viewangles;
	auto old_forwardmove = cmd->forwardmove;
	auto old_sidemove = cmd->sidemove;


	if (cfg.legitbunny_hop)
		misc::movement::legitbunny_hop(cmd);

	if (cfg.skinchanger)
		misc::CreateMove_Post();

	if (cfg.nosmoke)
		misc::removesmoke();
	else
		misc::addsmoke();

	if (cfg.triggerbot)
		misc::triggerbot(cmd);

//	if (cfg.aimbot)
	//{
	//	misc::aimbotrun(cmd);
	//}

	if (cfg.edgejump)
		misc::edgejump(cmd);

	if (cfg.autoaccept)
		misc::autoaccept(cmd);

	if (cfg.noflash)
		misc::noflash(cmd); 

	if (cfg.clantag)
	{
		if (cfg.lewdsenseanim)
		{
			misc::lewdsenseanim();
		}

		if (cfg.lewdsensestatic)
		{
			misc::lewdsensestatic();
		}

		if (cfg.uwuowoanim)
		{
			misc::uwuowoanim();
		}
	}
	else
		misc::ClanTagoff();



	if (cfg.rank_revealer)
	{
		if (GetKeyState(VK_TAB))
			utilities::ServerRankRevealAll();
	}

	if (fasz2)
	{
		interfaces::engine->execute_cmd("clear");
		interfaces::engine->execute_cmd("echo uwu, hi its lewdsense xx");
		fasz2 = false;
	}

	prediction::start(cmd); { 
		backtracking->legitBackTrack(cmd, csgo::local_player);
		//aimbot::run(cmd);

	} prediction::end();

	math::correct_movement(old_viewangles, cmd, old_forwardmove, old_sidemove);
	math::Clamp(cmd->viewangles);

	return false;
}

void __stdcall hooks::paint_traverse::hook(unsigned int panel, bool force_repaint, bool allow_force) {
	auto panel_to_draw = fnv::hash(interfaces::panel->get_panel_name(panel));

	switch (panel_to_draw) {
	case fnv::hash("MatSystemTopPanel"):

		if (!interfaces::engine->is_taking_screenshot())
		{
			menu.run();
			visuals::run();

		}


		break;
	}

	paint_traverse_original(interfaces::panel, panel, force_repaint, allow_force);
}



int __fastcall hooks::in_key_event::hook(void* ecx, int edx, int event_code, int key_num, const char* current_binding) {



	return in_key_event_original(event_code, key_num, current_binding);
}           