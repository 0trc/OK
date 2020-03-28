#include "backtrack.h"
#include "../../menu/menu.hpp"
#define TIME_TO_TICKS( dt )		( (int)( 0.5 + (float)(dt) / interfaces::globals->interval_per_tick ) )
vec3_t GetHitboxPositionv69(player_t* obj, int Hitbox)
{

	matrix_t bone_matrix[128];
	if (obj->setup_bones(bone_matrix, 128, 0x00000100, obj->simulation_time())) {
		if (obj->model())
		{
			auto studio_model = interfaces::model_info->get_studio_model(obj->model());
			if (studio_model)
			{
				auto hitbox = studio_model->hitbox_set(0)->hitbox(Hitbox);
				vec3_t Point[] =
				{
					vec3_t(hitbox->mins.x, hitbox->mins.y, hitbox->mins.z),
					vec3_t(hitbox->mins.x, hitbox->maxs.y, hitbox->mins.z),
					vec3_t(hitbox->maxs.x, hitbox->maxs.y, hitbox->mins.z),
					vec3_t(hitbox->maxs.x, hitbox->mins.y, hitbox->mins.z),
					vec3_t(hitbox->maxs.x, hitbox->maxs.y, hitbox->maxs.z),
					vec3_t(hitbox->mins.x, hitbox->maxs.y, hitbox->maxs.z),
					vec3_t(hitbox->mins.x, hitbox->mins.y, hitbox->maxs.z),
					vec3_t(hitbox->maxs.x, hitbox->mins.y, hitbox->maxs.z)
				};
				vec3_t vMin, vMax, vCenter, sCenter;

				int head = 100;
				int body = 100;
				int legs = 100;

				math::transform_vector(hitbox->mins, bone_matrix[hitbox->bone], vMin);
				math::transform_vector(hitbox->maxs, bone_matrix[hitbox->bone], vMax);

				vCenter = ((vMin + vMax) * 0.5f);
				int iCount = 7;

				for (int i = 0; i <= iCount; i++)
				{
					vec3_t vTargetPos;
					switch (i)
					{
					case 0:
					default:
						vTargetPos = vCenter; break;

					case 1:
					{
						vTargetPos = (Point[7] + Point[1]) * (head / 100);
					}
					break;

					case 2:
					{
						vTargetPos = (Point[3] + Point[4]) * (body / 100);
					}
					break;

					case 3:
					{
						vTargetPos = (Point[4] + Point[0]) * (body / 100);
					}
					break;

					case 4:
					{
						vTargetPos = (Point[2] + Point[7]) * (35 / 100);
					}
					break;

					case 5:
					{
						vTargetPos = (Point[4] + Point[0]) * (legs / 100);
					}
					break;
					case 6:
						vTargetPos = (Point[5] + Point[3]) * 0.6;
						break;
					case 7:
						vTargetPos = (Point[1] + Point[2]) * 0.6;
						break;
						//	default: vTargetPos = vCenter;
					}
					return vTargetPos;
				}

			}

		}
	}
	return vec3_t{};
}
void BackTrack::Update(int tick_count)
{
	//if (!menu.config.fakelag_check)
	//	return;

	latest_tick = tick_count;
	for (int i = 0; i < 64; i++)
	{
		UpdateRecord(i);
	}
}

bool BackTrack::IsTickValid(int tick)
{
	int delta = latest_tick - tick;
	float deltaTime = delta * interfaces::globals->interval_per_tick;
	return (fabs(deltaTime) <= 0.2f);
}
void BackTrack::UpdateRecord(int i)
{
	auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));

	if (entity && csgo::local_player->is_alive() && csgo::local_player->dormant())
	{
		float lby = csgo::local_player->lower_body_yaw();
		if (lby != records[i].lby)
		{
			records[i].tick_count = latest_tick;
			records[i].lby = lby;
			records[i].headPosition = GetHitboxPositionv69(csgo::local_player, 0);
		}

		else

			records[i].tick_count = 0;
	}
}

bool BackTrack::RunLBYBackTrack(int i, c_usercmd* cmd, vec3_t& aimdirection)
{
	if (IsTickValid(records[i].tick_count))
	{
	//	aimdirection = records[i].headPosition;
	//	cmd->tick_count = records[i].tick_count;
		return true;
	}
	return false;
}

vec3_t angle_vector(vec3_t meme)
{
	auto sy = sin(meme.y / 180.f * static_cast<float>(M_PI));
	auto cy = cos(meme.y / 180.f * static_cast<float>(M_PI));

	auto sp = sin(meme.x / 180.f * static_cast<float>(M_PI));
	auto cp = cos(meme.x / 180.f * static_cast<float>(M_PI));

	return vec3_t(cp * cy, cp * sy, -sp);
}

float distance_point_to_line(vec3_t Point, vec3_t LineOrigin, vec3_t Dir)
{
	auto PointDir = Point - LineOrigin;

	auto TempOffset = PointDir.dot(Dir) / (Dir.x * Dir.x + Dir.y * Dir.y + Dir.z * Dir.z);
	if (TempOffset < 0.000001f)
		return FLT_MAX;

	auto PerpendicularPoint = LineOrigin + (Dir * TempOffset);

	return (Point - PerpendicularPoint).length();
}

void BackTrack::legitBackTrack(c_usercmd* cmd, player_t* pLocal)
{
	if (cfg.backtrack == 1)
	{
		int bestTargetIndex = -1;
		float bestFov = FLT_MAX;
		player_info_t info;
		if (!pLocal->is_alive())
			return;

		for (int i = 0; i < interfaces::globals->max_clients; i++)
		{
			auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));

			if (!entity || !pLocal)
				continue;

			if (entity == pLocal)
				continue;

			interfaces::engine->get_player_info(i, &info);

			if (entity->dormant())
				continue;

			if (entity->team() == pLocal->team())
				continue;

			if (entity->is_alive())
			{

				float simtime = entity->simulation_time();
				vec3_t hitboxPos = GetHitboxPositionv69(entity, 0);

				float var_x = cfg.backtracktick;
				int var_y = static_cast<int>(var_x);

				headPositions[i][cmd->command_number % var_y] = backtrackData{ simtime, hitboxPos };
				vec3_t ViewDir = angle_vector(cmd->viewangles + (pLocal->aim_punch_angle() * 2.f));
				float FOVDistance = distance_point_to_line(hitboxPos, pLocal->get_eye_pos(), ViewDir);

				if (bestFov > FOVDistance)
				{
					bestFov = FOVDistance;
					bestTargetIndex = i;
				}
			}
		}

		float bestTargetSimTime;
		if (bestTargetIndex != -1)
		{
			float var_x = cfg.backtracktick;
			int var_y = static_cast<int>(var_x);

			float tempFloat = FLT_MAX;
			vec3_t ViewDir = angle_vector(cmd->viewangles + (pLocal->aim_punch_angle() * 2.f));
			for (int t = 0; t < var_y; ++t)
			{
				float tempFOVDistance = distance_point_to_line(headPositions[bestTargetIndex][t].hitboxPos, pLocal->get_eye_pos(), ViewDir);
				if (tempFloat > tempFOVDistance && headPositions[bestTargetIndex][t].simtime > pLocal->simulation_time() - 1)
				{
					tempFloat = tempFOVDistance;
					bestTargetSimTime = headPositions[bestTargetIndex][t].simtime;
				}
			}
			if (cmd->buttons & in_attack)
			{
				cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
			}
		}
	}
}
BackTrack* backtracking = new BackTrack();
backtrackData headPositions[64][12];
