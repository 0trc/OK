#pragma once
#pragma once
#include "../..//dependencies/common_includes.hpp"


class cmenu
{
public:
	void run();
	void addchildren();
	void renderbackground();
	void handlechildren();
	void handleinput();
	void renderchildren();
}; extern cmenu menu;

enum menuitemtype
{
	SWITCH = 0,
	slider,
	slider1,
	slider01,
	section,
	lilsection,
	blanksection
};
struct menuitems_t
{
	menuitems_t(std::string name, float* value = 0, int itemtype = menuitemtype::SWITCH, int minimumvalue = 0, int maximumvalue = 1)
	{
		this->name = name;
		this->value = value;
		this->itemtype = itemtype;
		this->minimumvalue = minimumvalue;
		this->maximumvalue = maximumvalue;
	}

	std::string name;
	float* value;
	int itemtype;
	int minimumvalue;
	int maximumvalue;


};
extern cmenu menu;

namespace gui {
	void initialize();

}



struct config {
	bool menuopened;
	float radar;
	float autoaccept;
	float team_check;
	float name_esp;
	float health_bar;
	float ticks = 0.1;
	float aimbot_fov = 0;
	float aimbot;
	float lewdsenseanim;
	float lewdsensestatic;
	float edgejump;
	float skinchanger;
	float uwuowoanim;
	float weapon_esp;
	float box_esp;
	float bunny_hop;
	float throughsmoke;
	float aaleft;
	float aaright;
	float triggeronkey;
	float backtracktick = 1;
	float noflash;
	float triggerbot;
	float defusing;
	float headonly = 0;
	float nosmoke;
	float wallhackdot;
	float backtrack;
	float legitbunny_hop;
	float clantag;
	float rank_revealer;
	float esp_opacity = 255;
	float hitchance = 100;
	float chamcolorred = 0;
	float chamcolorgreen = 255;
	float chamcolorblue = 0;
	float chamxqzred = 255;
	float chamxqzgreen = 0;
	float chamxqzblue = 0;
	float chams_enemy;
	float chams_enemy_xqz;
	float chams_material;

	bool enabled = false;
	bool friendlyFire = false;
	bool scopedOnly = true;
	bool ignoreFlash = false;
	bool ignoreSmoke = false;
	bool killshot = false;
	bool onKey = false;
	int key = 0;
	int hitgroup = 0;
	int shotDelay = 0;
	int minDamage = 1;
};
extern config cfg;