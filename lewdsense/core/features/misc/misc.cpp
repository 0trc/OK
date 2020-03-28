#include "../features.hpp"
#include "../..//menu/menu.hpp"



void LoadNamedSky(const char* new_skybox)
{

	

}


void misc::CreateMove_Post()
{


}



void misc::removesmoke()
{
	if (!csgo::local_player)
		return;

	static auto smokeCount = *reinterpret_cast<uint32_t**>(utilities::pattern_scan(GetModuleHandleA("client_panorama.dll"), "A3 ? ? ? ? 57 8B CB")+1);
	static std::vector<const char*> smokeMaterials =
	{
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_fire",

	};

		for (auto materialName : smokeMaterials)
		{
			interfaces::material_system->find_material(materialName, TEXTURE_GROUP_OTHER)->set_material_var_flag(material_var_no_draw, true);
		}

		*(int*)smokeCount = 0;		
}



void misc::addsmoke()
{
	if (!csgo::local_player)
		return;

	static auto smokeCount = *reinterpret_cast<uint32_t**>(utilities::pattern_scan(GetModuleHandleA("client_panorama.dll"), "A3 ? ? ? ? 57 8B CB") + 1);
	static std::vector<const char*> smokeMaterials =
	{
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_fire",

	};

	for (auto materialName : smokeMaterials)
	{
		interfaces::material_system->find_material(materialName, TEXTURE_GROUP_OTHER)->set_material_var_flag(material_var_no_draw, false);
	}

	*(int*)smokeCount = 0;
}


void SetMyClanTag(const char* tag, const char* name)
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(utilities::pattern_scan(GetModuleHandleA("engine.dll"), "53 56 57 8B DA 8B F9 FF 15"));

	pSetClanTag(tag, name);
}

void misc::lewdsenseanim()
{

	switch (int(interfaces::globals->cur_time * 4) % 30)
	{
	case 0: SetMyClanTag(" ", "lewdsense.uwu"); break;
	case 1: SetMyClanTag("l", "lewdsense.uwu"); break;
	case 2: SetMyClanTag("le", "lewdsense.uwu"); break;
	case 3: SetMyClanTag("lew", "lewdsense.uwu"); break;
	case 4: SetMyClanTag("lewd", "lewdsense.uwu"); break;
	case 5: SetMyClanTag("lewds", "lewdsense.uwu"); break;
	case 6: SetMyClanTag("lewdse", "lewdsense.uwu"); break;
	case 7: SetMyClanTag("lewdsen", "lewdsense.uwu"); break;
	case 8: SetMyClanTag("lewdsens", "lewdsense.uwu"); break;
	case 9: SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 10:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 11:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 12:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 13:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 14:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 15:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 16:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 17:SetMyClanTag("lewdsense", "lewdsense.uwu"); break;
	case 18:SetMyClanTag("ewdsense", "lewdsense.uwu"); break;
	case 19:SetMyClanTag("wdsense", "lewdsense.uwu"); break;
	case 20:SetMyClanTag("dsense", "lewdsense.uwu"); break;
	case 22:SetMyClanTag("sense", "lewdsense.uwu"); break;
	case 23:SetMyClanTag("ense", "lewdsense.uwu"); break;
	case 24:SetMyClanTag("nse", "lewdsense.uwu"); break;
	case 25:SetMyClanTag("se", "lewdsense.uwu"); break;
	case 26:SetMyClanTag("e", "lewdsense.uwu"); break;
	case 27: SetMyClanTag(" ", "lewdsense.uwu"); break;
	case 28: SetMyClanTag(" ", "lewdsense.uwu"); break;
	case 29: SetMyClanTag(" ", "lewdsense.uwu"); break;
	}
};

void misc::uwuowoanim()
{

	switch (int(interfaces::globals->cur_time * 4) % 2)
	{
	case 0: SetMyClanTag("uwu", "uwu"); break;
	case 1: SetMyClanTag("owo", "uwu"); break;
	}
};

void misc::lewdsensestatic()
{
		SetMyClanTag("lewdsense", "lewdsense.uwu");
};


void misc::ClanTagoff()
{
	SetMyClanTag("", "");
};

void misc::autoaccept(c_usercmd* cmd)
{
	static auto set_local_player_ready = reinterpret_cast<void(__fastcall*)(const char*)>(utilities::pattern_scan(GetModuleHandleA("engine.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));
	set_local_player_ready("accept");
	return;
}


void misc::noflash(c_usercmd* cmd)
{

	if (csgo::local_player->is_flashed())
		csgo::local_player->flash_duration() = 0.f;
	else
		return;

}




vec3_t angle_vectorrr(vec3_t meme)
{
	auto sy = sin(meme.y / 180.f * static_cast<float>(M_PI));
	auto cy = cos(meme.y / 180.f * static_cast<float>(M_PI));

	auto sp = sin(meme.x / 180.f * static_cast<float>(M_PI));
	auto cp = cos(meme.x / 180.f * static_cast<float>(M_PI));

	return vec3_t(cp * cy, cp * sy, -sp);
}


void misc::edgejump(c_usercmd* cmd)
{


}


int GetBone(int i) {
	switch (i) {
	case 0:
		return 8;
		break;
	case 1:
		return 7;
		break;
	case 2:
		return 0;
		break;
	case 3:
		return 5;
		break;
	}
}


