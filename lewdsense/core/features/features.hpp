#pragma once
#include "../../dependencies/common_includes.hpp"
#include <map>
#include <vector>
#include <string>


namespace misc {
	namespace movement {
		void legitbunny_hop(c_usercmd* cmd);
	}

	void edgejump(c_usercmd* cmd);

	//void aimbotrun(c_usercmd* cmd);

	void removesmoke();
	void addsmoke();

	void CreateMove_Post();
	void lewdsenseanim();
	void lewdsensestatic();
	void uwuowoanim();
	void ClanTagoff();

	void autoaccept(c_usercmd* cmd);
	void noflash(c_usercmd* cmd);

	void triggerbot(c_usercmd* cmd);



}


namespace visuals {

	void run();
	void player_esp(player_t* entity);
	void entity_esp(player_t* entity);

}






