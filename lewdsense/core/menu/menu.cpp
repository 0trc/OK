#include "menu.hpp"
#include <iostream>
#include <ctime>
config cfg;
std::vector<menuitems_t> menuitems;
cmenu menu;


int x = 50, y = 100, w = 200, h = 15;
float idk = 0;
float idk2 = 0;
float hit = 100;
float cat_0;
float boobie;
float cat_1;
float ccat_1;
float bhop;
float cat_2;
float cat_aim;
float cat_3;
float cat_4;
float cat_6;
float cat_7;
float cat_ecolor;
float cat_tcolor;
float cat_exqzcolor;
float cat_txqzcolor;
int bruh = 0;

int currentselecteditem = 0;
int menu_alpha = 255;
int menubackground_alpha = 200;
int menubox_alpha = 255;
int menu_pos;

char localtag[16];



void cmenu::run()
{
	
	static bool _pressed = true;

	if (!_pressed && GetAsyncKeyState(VK_INSERT))
		_pressed = true;
	else if (_pressed && !GetAsyncKeyState(VK_INSERT))
	{
		_pressed = false;
		cfg.menuopened = !cfg.menuopened;
	}

	if (cfg.menuopened)
	{
		menu_alpha = min(menu_alpha + 7, 255);
		menubackground_alpha = min(menubackground_alpha + 7, 200);
		menubox_alpha = min(menubox_alpha + 7, 210);
		menu_pos = 255;
	}
	else
	{
		menubackground_alpha = max(menubackground_alpha - 7, 0);
		menu_alpha = max(menu_alpha - 7, 0);
		menubox_alpha = max(menubox_alpha - 7, 0);
		menu_pos = 255;
	}

	static vec2_t oldpos;
	static vec2_t mousepos;
	static bool _dragging = false;
	bool _click = false;
	static bool _resizing = false;
	static int _drag_x = 300;
	static int _drag_y = 350;
	int finalsize = 100;
	vec2_t _mouse_pos = interfaces::surface->GetMousePosition();
	if (GetAsyncKeyState(VK_LBUTTON))
		_click = true;

	if (_dragging && !_click)
		_dragging = false;

	if (_resizing && !_click)
		_resizing = false;

	if (_dragging && _click)
	{
		x = _mouse_pos.x - _drag_x;
		y = _mouse_pos.y - _drag_y;
	}

	if (interfaces::surface->MouseInRegion(x + menu_pos, y, finalsize, finalsize) && !(interfaces::surface->MouseInRegion(
		x + 10, y + 20, finalsize - 10, finalsize - 10)))
	{
		_dragging = true;
		_drag_x = _mouse_pos.x - x;
		_drag_y = _mouse_pos.y - y;
	}

	this->addchildren();
	this->handleinput();
	this->renderbackground();
	this->handlechildren();

	menuitems.clear();
	h = 15;


}








void cmenu::renderbackground()
{



	render::draw_filled_rect(x + menu_pos, y, w, h, color(75, 75, 75, menubackground_alpha));

	if (cfg.menuopened)
	{
		switch (int(interfaces::globals->cur_time * 24) % 24)
		{
		case 0: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(200, 200, 200, menu_alpha));; break;
		case 1: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(190, 190, 190, menu_alpha));; break;
		case 2: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(180, 180, 180, menu_alpha));; break;
		case 3: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(170, 170, 170, menu_alpha));; break;
		case 4: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(160, 160, 160, menu_alpha));; break;
		case 5: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(150, 150, 150, menu_alpha));; break;
		case 6:		render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(140, 140, 140, menu_alpha));; break;
		case 7: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(130, 130, 130, menu_alpha));; break;
		case 8: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(120, 120, 120, menu_alpha));; break;
		case 9: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(110, 110, 110, menu_alpha));; break;
		case 10: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(100, 100, 100, menu_alpha));; break;
		case 11:	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(90, 90, 90, menu_alpha));; break;
		case 12: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(80, 80, 80, menu_alpha));; break;
		case 13: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(90, 90, 90, menu_alpha));; break;
		case 14: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(100, 100, 100, menu_alpha));; break;
		case 15: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(110, 110, 110, menu_alpha));; break;
		case 16: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(120, 120, 120, menu_alpha));; break;
		case 17: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(130, 130, 130, menu_alpha));; break;
		case 18: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(140, 140, 140, menu_alpha));; break;
		case 19: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(150, 150, 150, menu_alpha));; break;
		case 20: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(160, 160, 160, menu_alpha));; break;
		case 21: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(170, 170, 170, menu_alpha));; break;
		case 22: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(180, 180, 180, menu_alpha));; break;
		case 23: 	render::draw_outline(x - 1 + menu_pos, y - 1, w + 2, h + 2, color(190, 190, 190, menu_alpha));; break;
		}
		switch (int(interfaces::globals->cur_time * 24) % 24)
		{
		case 0: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(200, 200, 200, menu_alpha));; break;
		case 1: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(190, 190, 190, menu_alpha));; break;
		case 2: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(180, 180, 180, menu_alpha));; break;
		case 3: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(170, 170, 170, menu_alpha));; break;
		case 4: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(160, 160, 160, menu_alpha));; break;
		case 5: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(150, 150, 150, menu_alpha));; break;
		case 6:		render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(140, 140, 140, menu_alpha));; break;
		case 7: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(130, 130, 130, menu_alpha));; break;
		case 8: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(120, 120, 120, menu_alpha));; break;
		case 9: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(110, 110, 110, menu_alpha));; break;
		case 10: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(100, 100, 100, menu_alpha));; break;
		case 11:	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(90, 90, 90, menu_alpha));; break;
		case 12: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(80, 80, 80, menu_alpha));; break;
		case 13:	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(90, 90, 90, menu_alpha));; break;
		case 14: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(100, 100, 100, menu_alpha));; break;
		case 15: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(110, 110, 110, menu_alpha));; break;
		case 16: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(120, 120, 120, menu_alpha));; break;
		case 17: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(130, 130, 130, menu_alpha));; break;
		case 18: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(140, 140, 140, menu_alpha));; break;
		case 19: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(150, 150, 150, menu_alpha));; break;
		case 20: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(160, 160, 160, menu_alpha));; break;
		case 21: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(170, 170, 170, menu_alpha));; break;
		case 22: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(180, 180, 180, menu_alpha));; break;
		case 23: 	render::draw_outline(x - 2 + menu_pos, y - 2, w + 4, h + 4, color(190, 190, 190, menu_alpha));; break;
		}
	}



	render::draw_outline(x + menu_pos, y, w, h, color(55, 55, 55, menu_alpha / 2));
	render::draw_filled_rect(x + menu_pos, y, w, 15, color(100, 100, 100, menu_alpha));
	render::draw_outline(x + menu_pos, y, w, 15, color(255, 255, 255, menu_alpha));
	render::draw_text_string(x + menu_pos + ((w / 2 )- 55), y + 1, render::fonts::esp_font, "lewdsense.exe", true, color(250, 250, 250, menu_alpha));

	render::draw_filled_rect(x + 166 + menu_pos, y + 4, 8, 8, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 176 + menu_pos, y + 4, 8, 8, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 186 + menu_pos, y + 4, 8, 8, color(0, 0, 0, menu_alpha));

	render::draw_filled_rect(x + 165 + menu_pos, y + 3.75, 8, 8, color(230, 230, 230, menu_alpha));
	render::draw_filled_rect(x + 175 + menu_pos, y + 3.75, 8, 8, color(230, 230, 230, menu_alpha));
	render::draw_filled_rect(x + 185 + menu_pos, y + 3.75, 8, 8, color(230, 230, 230, menu_alpha));

	render::draw_filled_rect(x + 167 + menu_pos, y + 8.5, 4, 1.5, color(0, 0, 0, menu_alpha));

	render::draw_filled_rect(x + 187 + menu_pos, y + 5, 1, 1, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 188 + menu_pos, y + 6, 1, 1, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 189 + menu_pos, y + 7, 1, 1, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 190 + menu_pos, y + 8, 1, 1, color(0, 0, 0, menu_alpha));

	render::draw_filled_rect(x + 190 + menu_pos, y + 5, 1, 1, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 189 + menu_pos, y + 6, 1, 1, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 188 + menu_pos, y + 7, 1, 1, color(0, 0, 0, menu_alpha));
	render::draw_filled_rect(x + 187 + menu_pos, y + 8, 1, 1, color(0, 0, 0, menu_alpha));

	render::draw_outline(x + 177 + menu_pos, y + 5, 5, 5, color(0, 0, 0, menu_alpha));

	render::draw_filled_rect(x + 175 + menu_pos, y + 15, 25, h - 15, color(150, 150, 150, menubackground_alpha));



	if (cfg.menuopened)
	{
		switch (int(interfaces::globals->cur_time * 14) % 14)
		{
		case 0: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 250));; break;
		case 1: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 225));; break;
		case 2: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 200));; break;
		case 3: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 175));; break;
		case 4: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 150));; break;
		case 5: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 125));; break;
		case 6: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 100));; break;
		case 7: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 75));; break;
		case 8: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 100));; break;
		case 9: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 125));; break;
		case 10: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 150));; break;
		case 11: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 175));; break;
		case 12: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 200));; break;
		case 13: render::draw_filled_rect(x + menu_pos + 1, y + 15 + (15 * currentselecteditem), w - 1 - 1, 15 - 1, color(150, 0, 255, menu_alpha - 225));; break;

		}
	}


	if (boobie == 1)
	{
	}
	else
	{
		return;
	}
}
void cmenu::addchildren()
{



	menuitems.push_back(menuitems_t("LEGIT", &cat_aim, menuitemtype::section));
	if (cat_aim)
	{
	//	menuitems.push_back(menuitems_t("aimbot", &cfg.aimbot));
		menuitems.push_back(menuitems_t("triggerbot", &cfg.triggerbot));
		if (cfg.triggerbot)
		{
			menuitems.push_back(menuitems_t("headonly", &cfg.headonly));
			menuitems.push_back(menuitems_t("on key (v)", &cfg.triggeronkey));

		}
		menuitems.push_back(menuitems_t("backtrack", &cfg.backtrack));
		if (cfg.backtrack)
		{
			menuitems.push_back(menuitems_t("ticks", &cfg.backtracktick, menuitemtype::slider1, 1, 12));
		}
	}

	menuitems.push_back(menuitems_t("VISUAL", &cat_1, menuitemtype::section));

	if (cat_1)
	{
		menuitems.push_back(menuitems_t("chams/walls", &cat_2, menuitemtype::lilsection));
		if (cat_2)
		{
			menuitems.push_back(menuitems_t("enemy chams", &cfg.chams_enemy));
			menuitems.push_back(menuitems_t("enemy xqz", &cfg.chams_enemy_xqz));
			menuitems.push_back(menuitems_t("through smoke", &cfg.throughsmoke));
		}
		menuitems.push_back(menuitems_t("esp", &ccat_1, menuitemtype::lilsection));
		if (ccat_1)
		{
			menuitems.push_back(menuitems_t("name", &cfg.name_esp));
			menuitems.push_back(menuitems_t("weapon", &cfg.weapon_esp));
			menuitems.push_back(menuitems_t("health bar", &cfg.health_bar));
			menuitems.push_back(menuitems_t("box", &cfg.box_esp));
			menuitems.push_back(menuitems_t("defuse warning", &cfg.defusing));
			menuitems.push_back(menuitems_t("show teammates", &cfg.team_check));
			menuitems.push_back(menuitems_t("opacity", &cfg.esp_opacity, menuitemtype::slider, 0, 255));
		}
		menuitems.push_back(menuitems_t("other", &cat_6, menuitemtype::lilsection));
		if (cat_6)
		{
			menuitems.push_back(menuitems_t("noflash", &cfg.noflash));
			menuitems.push_back(menuitems_t("nosmoke", &cfg.nosmoke));
		}
	}



	menuitems.push_back(menuitems_t("MOVEMENT", &cat_7, menuitemtype::section));
	if (cat_7)
	{
		menuitems.push_back(menuitems_t("bhop", &cfg.legitbunny_hop));

		if (cfg.legitbunny_hop)
		{
			menuitems.push_back(menuitems_t("hit chance", &cfg.hitchance, menuitemtype::slider, 0, 100));

		}
	}




	menuitems.push_back(menuitems_t("MISC", &cat_3, menuitemtype::section));

	if (cat_3)
	{
		menuitems.push_back(menuitems_t("clantag", &cfg.clantag, menuitemtype::lilsection));
		if (cfg.clantag)
		{
			menuitems.push_back(menuitems_t("lewdsense.uwu animated", &cfg.lewdsenseanim));
			menuitems.push_back(menuitems_t("lewdsense.uwu static", &cfg.lewdsensestatic));
			menuitems.push_back(menuitems_t("uwu/owo animated", &cfg.uwuowoanim));
		}
		menuitems.push_back(menuitems_t("engine radar", &cfg.radar));
		menuitems.push_back(menuitems_t("autoaccept", &cfg.autoaccept));
		menuitems.push_back(menuitems_t("rank revealer", &cfg.rank_revealer));
	}

	menuitems.push_back(menuitems_t("COLORS", &cat_4, menuitemtype::section));
	if (cat_4)
	{
		menuitems.push_back(menuitems_t("enemy chams", &cat_ecolor, menuitemtype::lilsection));
		if (cat_ecolor)
		{
			menuitems.push_back(menuitems_t("red", &cfg.chamcolorred, menuitemtype::slider, 0, 255));
			menuitems.push_back(menuitems_t("green", &cfg.chamcolorgreen, menuitemtype::slider, 0, 255));
			menuitems.push_back(menuitems_t("blue", &cfg.chamcolorblue, menuitemtype::slider, 0, 255));
		}
		menuitems.push_back(menuitems_t("enemy walls ", &cat_exqzcolor, menuitemtype::lilsection));

		if (cat_exqzcolor)
		{
			menuitems.push_back(menuitems_t("red", &cfg.chamxqzred, menuitemtype::slider, 0, 255));
			menuitems.push_back(menuitems_t("green", &cfg.chamxqzgreen, menuitemtype::slider, 0, 255));
			menuitems.push_back(menuitems_t("blue", &cfg.chamxqzblue, menuitemtype::slider, 0, 255));
		}
	}
	h += 15 * menuitems.size();
}

void cmenu::renderchildren()
{
	if (!(menuitems.size() > 0))
		return;

	for (int i = 0; i < menuitems.size(); i++)
	{
		if (menuitems[i].itemtype == menuitemtype::section)
		{
			render::draw_text_string(x + menu_pos + 5, y + 16 + (15 * i), render::fonts::esp_font, menuitems[i].name, false, *menuitems[i].value ? color(255, 255, 255, menu_alpha) : color(255, 255, 255, menu_alpha));
			render::draw_text_string_alternative(x + menu_pos + 8 + 164, y + 16 + (15 * i), render::fonts::watermark_font, *menuitems[i].value ? "x" : "-", true, *menuitems[i].value ? color(255, 255, 255, menu_alpha) : color(255, 255, 255, menu_alpha));
		}
		else if (menuitems[i].itemtype == menuitemtype::lilsection)
		{
			render::draw_text_string(x + menu_pos + 10, y + 16 + (15 * i), render::fonts::esp_font, menuitems[i].name, false, *menuitems[i].value ? color(255, 255, 255, menu_alpha) : color(255, 255, 255, menu_alpha));
			render::draw_text_string_alternative(x + menu_pos + 8 + 164, y + 16 + (15 * i), render::fonts::watermark_font, *menuitems[i].value ? "." : ".", true, *menuitems[i].value ? color(255, 255, 255, menu_alpha) : color(255, 255, 255, menu_alpha));
		}
		else if (menuitems[i].itemtype == menuitemtype::blanksection)
		{
			render::draw_text_string(x + menu_pos + 10, y + 16 + (15 * i), render::fonts::esp_font, menuitems[i].name, false, *menuitems[i].value ? color(255, 255, 255, menu_alpha) : color(255, 255, 255, menu_alpha));
			render::draw_text_string_alternative(x + menu_pos + 8 + 164, y + 16 + (15 * i), render::fonts::watermark_font, *menuitems[i].value ? " " : " ", true, *menuitems[i].value ? color(255, 255, 255, menu_alpha) : color(255, 255, 255, menu_alpha));
		}
		else if (menuitems[i].itemtype == menuitemtype::SWITCH)
		{
			render::draw_text_string(x + menu_pos + 15, y + 16 + (15 * i), render::fonts::esp_font, menuitems[i].name, false, color(255, 255, 255, menu_alpha));
			render::draw_text_string_alternative(x + menu_pos + 8 + 186.5, y + 16 + (15 * i), render::fonts::watermark_font, *menuitems[i].value ? "on" : "off", true, *menuitems[i].value ? color(10, 235, 10, menu_alpha) : color(235, 10, 10, menu_alpha));


		}
		else
		{

			render::draw_text_string(x + menu_pos + 15, y + 16 + (15 * i), render::fonts::esp_font, menuitems[i].name, false, color(255, 255, 255, menu_alpha));
			render::draw_text_string_alternative(x + menu_pos + 8 + 189, y + 16 + (15 * i), render::fonts::watermark_font, std::to_string((int)*menuitems[i].value), true, color(255, 255, 255, menu_alpha));
			//render::text(x + menu_pos + 8, y + 16 + (15 * i), fontlist::mainfont, display, color(255, 255, 255, 255), false, true, color(0, 0, 0, 255));
		}

	}
}

void cmenu::handlechildren()
{

	if (!(menuitems.size() > 0))
		return;

	for (int i = 0; i < menuitems.size(); i++)
	{
		if (i == currentselecteditem)
		{
			switch (menuitems[i].itemtype)
			{
			case menuitemtype::SWITCH:
			{
				if (GetAsyncKeyState(VK_RETURN) & 1 && cfg.menuopened & 1)
					*menuitems[i].value = !*menuitems[i].value;

			} break;
			case menuitemtype::slider:
			{
				if ((GetAsyncKeyState(VK_RIGHT) & 1) && *menuitems[i].value < menuitems[i].maximumvalue && cfg.menuopened & 1)
					*menuitems[i].value += 5;

				if ((GetAsyncKeyState(VK_LEFT) & 1) && *menuitems[i].value > menuitems[i].minimumvalue && cfg.menuopened & 1)
					*menuitems[i].value -= 5;
			} break;
			case menuitemtype::slider1:
			{
				if ((GetAsyncKeyState(VK_RIGHT) & 1) && *menuitems[i].value < menuitems[i].maximumvalue && cfg.menuopened & 1)
					*menuitems[i].value += 1;

				if ((GetAsyncKeyState(VK_LEFT) & 1) && *menuitems[i].value > menuitems[i].minimumvalue && cfg.menuopened & 1)
					*menuitems[i].value -= 1;
			} break;
			case menuitemtype::slider01:
			{
				if ((GetAsyncKeyState(VK_RIGHT) & 1) && *menuitems[i].value < menuitems[i].maximumvalue && cfg.menuopened & 1)
					*menuitems[i].value += 0.1;

				if ((GetAsyncKeyState(VK_LEFT) & 1) && *menuitems[i].value > menuitems[i].minimumvalue && cfg.menuopened & 1)
					*menuitems[i].value -= 0.1;
			} break;
			case menuitemtype::section:
			{
				if (GetAsyncKeyState(VK_RETURN) & 1 && cfg.menuopened & 1)
					*menuitems[i].value = !*menuitems[i].value;

			} break;
			case menuitemtype::lilsection:
			{
				if (GetAsyncKeyState(VK_RETURN) & 1 && cfg.menuopened & 1)
					*menuitems[i].value = !*menuitems[i].value;

			} break;
			case menuitemtype::blanksection:
			{
				if (GetAsyncKeyState(VK_RETURN) & 1 && cfg.menuopened & 1)
					*menuitems[i].value = !*menuitems[i].value;

			} break;
			}
		}
	}

	this->renderchildren();
}

void cmenu::handleinput()
{
	if (!cfg.menuopened)
		return;

	if ((GetAsyncKeyState(VK_DOWN) & 1) && cfg.menuopened & 1)
	{
		if (currentselecteditem > (menuitems.size() - 2))
			currentselecteditem = 0;
		else
			currentselecteditem++;
	}


	if ((GetAsyncKeyState(VK_UP) & 1) && cfg.menuopened & 1)
	{
		if (currentselecteditem < 1)
			currentselecteditem = menuitems.size() - 1;
		else
			currentselecteditem--;
	}

}
#pragma endregion