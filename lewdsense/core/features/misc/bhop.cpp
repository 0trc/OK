#include "../features.hpp"
#include "../..//menu/menu.hpp"

void misc::movement::legitbunny_hop(c_usercmd* cmd)
{
	int hitchance = cfg.hitchance;
	int randomnumber = rand() % 100 + 1;
	if (!csgo::local_player->is_alive())
		return;

	static bool bLastJumped = false;
	static bool bShouldFake = false;

	if (randomnumber <= hitchance)
		{
	if (!bLastJumped && bShouldFake)
	{
		bShouldFake = false;
		cmd->buttons |= in_jump;
	}
	else if (cmd->buttons & in_jump)
	{
		if (csgo::local_player->flags() & fl_onground)
			bShouldFake = bLastJumped = true;
		else
		{
			cmd->buttons &= ~in_jump;
			bLastJumped = false;
		}
	}
		}
}
