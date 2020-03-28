
#include "../features.hpp"
#include "../../menu/menu.hpp"



vec3_t angle_vectorr(vec3_t meme)
{
	auto sy = sin(meme.y / 180.f * static_cast<float>(M_PI));
	auto cy = cos(meme.y / 180.f * static_cast<float>(M_PI));

	auto sp = sin(meme.x / 180.f * static_cast<float>(M_PI));
	auto cp = cos(meme.x / 180.f * static_cast<float>(M_PI));

	return vec3_t(cp * cy, cp * sy, -sp);
}



bool enemycheck(player_t* entity, c_usercmd* cmd) {

	if (cfg.triggeronkey)
	{
		if (!GetAsyncKeyState(0x56))
			return false;
	}


	auto pLocal = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (entity->dormant())
		return false;


	if (entity == pLocal)
			return false;



	int bruh = 7;
	if (cfg.headonly)
	{
		bruh = 1;
	}
	else
	{
		bruh = 7;
	}

	auto entity_bone_pos = entity->get_hitbox_position(entity, hitbox_head);
	ray_t ray;
	trace_t tr;
	vec3_t vAngles;
	auto eyepos = pLocal->get_eye_pos();
	auto enemeyepos = entity->get_eye_pos();
	vec3_t ViewDir = angle_vectorr(cmd->viewangles + (pLocal->aim_punch_angle() * 2.f));
	auto end = eyepos + ViewDir * 8192.f;
	trace_filter_skip_one_entity filter(pLocal);
	ray.initialize(eyepos, end);

	interfaces::trace_ray->trace_ray(ray, MASK_SHOT, &filter, &tr);

	if (tr.hitGroup <= bruh && tr.hitGroup > 0)
	{
		return true;
	}
	return false;

}



void misc::triggerbot(c_usercmd* cmd)  {

	for (int i = 0; i <= interfaces::globals->max_clients; i++) {

		auto pLocal = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
		auto pEnemny = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));

		float bruh = 7;

		if (enemycheck(pEnemny, cmd))
		{
			cmd->buttons |= in_attack;
			return;
		}
		return;
	}

};