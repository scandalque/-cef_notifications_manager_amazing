#pragma once

#include <RakHook/rakhook.hpp>
#include <RakHook/detail.hpp>

#include "singleton.h"


namespace offsets {
	namespace chat {
		constexpr std::uintptr_t ref[] = { 0x21A0E4, 0x21A0EC, 0x26E8C8, 0x26E9F8, 0x26EB80, 0x2ACA10 };
		constexpr std::uintptr_t add_message[] = { 0x645A0, 0x64670, 0x679F0, 0x68130, 0x68170, 0x67BE0 };
	}
	namespace input {
		constexpr std::uintptr_t ref[] = { 0x21A0E8, 0x21A0F0, 0x26E8CC, 0x26E9FC, 0x26EB84, 0x2ACA14 };
		constexpr std::uintptr_t add_command[] = { 0x65AD0, 0x65BA0, 0x69000, 0x69730, 0x69770, 0x691B0 };
	}
	namespace remote_player {
		constexpr std::uintptr_t set_marker_state[] = { 0x10FF0, 0x11090, 0x14120, 0x144B0, 0x14500, 0x14390 };
		constexpr std::uintptr_t does_exists[] = { 0x1080 };
		constexpr std::uintptr_t get_color_as_argb[] = { 0x12A00 };
	}
	namespace player_pool {
		constexpr std::uintptr_t get_player[] = { 0x10F0, 0x10F0, 0x10F0, 0x10F0, 0x10F0, 0x10F0 };
		constexpr std::uintptr_t get_name[] = { 0x13CE0 };
	}
	namespace entity {
		constexpr std::uintptr_t get_model_index[] = { 0x9A670 };
	}
	namespace netgame {
		constexpr std::uintptr_t ref[] = { 0x21A0F8, 0x21A100, 0x26E8DC, 0x26EA0C, 0x26EB94, 0x2ACA24 };
		constexpr std::uintptr_t get_player_pool[] = { 0x1160, 0x1170, 0x1160, 0x1170, 0x1170, 0x1170 };
	}
	namespace game {
		constexpr std::uintptr_t ref[] = { 0x21A10C, 0x21A114, 0x26E8F4, 0x26EA24, 0x26EBAC, 0x2ACA3C };
		constexpr std::uintptr_t set_cursor_mode[] = { 0x9BD30, 0x9BDD0, 0x9FFE0, 0xA0720, 0xA06F0, 0xA0530 };
	}
}


#define VERSION rakhook::detail::to_underlying(rakhook::samp_version())


class c_chat : public singleton<c_chat> {

public:

	c_chat*& ref() {
		return *(c_chat**)rakhook::samp_addr(offsets::chat::ref[VERSION]);
	}

	void add_message(unsigned long color, std::string text) {
		((void(__thiscall*)(c_chat*, unsigned long, const char*)) rakhook::samp_addr(offsets::chat::add_message[VERSION]) )(this, color, text.c_str());
	}
};


class c_input : public singleton<c_input> {

	using cmdproc_t = void(__cdecl*)(const char*);

public:

	enum {
		MAX_CLIENT_CMDS = 144,
		MAX_CMD_LENGTH = 32,
	};
	
	char pad00[0xC];
	cmdproc_t command_proc[MAX_CLIENT_CMDS];
	char command_name[MAX_CLIENT_CMDS][MAX_CMD_LENGTH + 1];
	int command_count;


	c_input*& ref() {
		return *(c_input**)rakhook::samp_addr(offsets::input::ref[VERSION]);
	}
	
	void add_command(std::string name, cmdproc_t cmdproc) {
		((void(__thiscall*)(c_input*, const char*, cmdproc_t)) rakhook::samp_addr(offsets::input::add_command[VERSION]) )(this, name.c_str(), cmdproc);
	}
	
	void remove_command(std::string name) {
		
		for (int i = 0; i < command_count; i++) {
			
			if (std::string(command_name[i]) == name) {

				int needs = command_count - i;
				int clear = i;
				
				if (needs > 0) {
					
					memmove(command_name[i], command_name[i + 1], sizeof(command_name[i]) * needs);
					memmove(command_proc + i, command_proc + i + 1, sizeof(command_proc[i]) * needs);

					clear = i + needs;
				}
				
				memset(command_name[clear], 0, sizeof(command_name[clear]));

				command_proc[clear] = nullptr;
				command_count -= 1;
			}
		}
	}
};


class c_entity {

public:

	char pad_4[60];
	::c_entity* game_entity;
	int handle;

	int get_model_index() {
		return ((int(__thiscall*)(c_entity*)) rakhook::samp_addr(offsets::entity::get_model_index[VERSION]))(this);
	}
};


class c_ped : public c_entity {

public:

	enum { MAX_ACCESSORIES = 10 };

	struct accessory {
		int model;
		int bone;
		float offset[3];
		float rotation[3];
		float scale[3];
		unsigned long first_material_color;
		unsigned long second_material_color;
	};

	enum stuff_type {
		STUFF_TYPE_NONE,
		STUFF_TYPE_BEER,
		STUFF_TYPE_DYN_BEER,
		STUFF_TYPE_PINT_GLASS,
		STUFF_TYPE_CIGGI
	};

	BOOL using_cellphone;

	struct {
		BOOL not_empty[MAX_ACCESSORIES];
		accessory info[MAX_ACCESSORIES];
		class CObject* object[MAX_ACCESSORIES];
	} accessories;

	::c_ped* game_ped;
	int pad_2a8[2];
	unsigned char player_number;
	int pad_2b1[2];
	int parachute_object;
	int urinating_particle;

	struct {
		int type;
		int object;
		int drunk_level;
	} stuff;

	int arrow;
	char field_2de;
	BOOL is_dancing;
	int dance_style;
	int last_dance_move;
	char pad_2de[20];
	BOOL is_urinating;
	char pad[55];
};


class c_remote_player {

public:

	enum player_state {
		PLAYER_STATE_NONE,
		PLAYER_STATE_ONFOOT = 17,
		PLAYER_STATE_DRIVER = 19,
		PLAYER_STATE_PASSENGER = 18,
		PLAYER_STATE_WASTED = 32,
		PLAYER_STATE_SPAWNED = 33
	};

	enum update_type {
		UPDATE_TYPE_NONE,
		UPDATE_TYPE_ONFOOT = 16,
		UPDATE_TYPE_DRIVER = 17,
		UPDATE_TYPE_PASSENGER = 18,
	};

	enum player_status { PLAYER_STATUS_TIMEOUT = 2 };
	
	struct s_remote_player_data {
		c_ped* ped;
		void* vehicle;
	};

	s_remote_player_data* remote_player;
	

	void set_marker_state(int state) {
		((void(__thiscall*)(c_remote_player*, BOOL)) rakhook::samp_addr(offsets::remote_player::set_marker_state[VERSION]) )(this, state);
	}

	int does_exist() {
		return ((int(__thiscall*)(c_remote_player*)) rakhook::samp_addr(offsets::remote_player::does_exists[VERSION]) )(this);
	}

	unsigned long get_color_as_argb() {
		return ((unsigned long(__thiscall*)(c_remote_player*)) rakhook::samp_addr(offsets::remote_player::get_color_as_argb[VERSION]) )(this);
	}
};


class c_player_pool {

public:

	enum { MAX_PLAYERS = 1004 };
	
	int largest_id;

	struct SAMPAPI_EXPORT {
		uint16_t id;
	public:
		std::string name;
		void* object;
		int ping;
		int score;
	} local_info;

	c_remote_player* get_player(uint16_t id) {
		return ((c_remote_player*(__thiscall*)(c_player_pool*, uint16_t)) rakhook::samp_addr(offsets::player_pool::get_player[VERSION]) )(this, id);
	}
	
	const char* get_name(uint16_t id) {
		return ((const char* (__thiscall*)(c_player_pool*, uint16_t)) rakhook::samp_addr(offsets::player_pool::get_name[VERSION]) )(this, id);
	}
};


class c_netgame : public singleton<c_netgame> {

public:

	enum game_mode {
		GAME_MODE_WAITCONNECT = 9,
		GAME_MODE_CONNECTING = 13,
		GAME_MODE_CONNECTED,
		GAME_MODE_WAITJOIN,
		GAME_MODE_RESTARTING = 18
	};
	enum {
		NETMODE_STATS_UPDATE_DELAY = 1000,
		NETMODE_INCAR_SENDRATE = 30,  // passenger/trailer/incar/unoccupied
		NETMODE_ONFOOT_SENDRATE = 30, // onfoot/unoccupied
		NETMODE_FIRING_SENDRATE = 30,
		LANMODE_INCAR_SENDRATE = 15,
		LANMODE_ONFOOT_SENDRATE = 15,
		NETMODE_SEND_MULTIPLIER = 2,
		NETMODE_AIM_UPDATE_DELAY = 500,
		NETMODE_HEAD_UPDATE_DELAY = 1000,
		NETMODE_TARGET_UPDATE_DELAY = 100,
		NETMODE_PLAYERS_UPDATE_DELAY = 3000,
	};

	struct pools {
		void* actor;
		void* object;
		void* gang_zone;
		void* label;
		void* text_draw;
		void* menu;
		void* player;
		void* vehicle;
		void* pickup;
	};

	struct settings {
		bool use_cj_walk;
		unsigned int dead_drops_money;
		float world_boundaries[4];
		bool allow_weapons;
		float gravity;
		bool enter_exit;
		BOOL vehicle_friendly_fire;
		bool hold_time;
		bool instagib;
		bool zone_names;
		bool friendly_fire;
		BOOL classes_available;
		float name_tags_draw_dist;
		bool manual_vehicle_engine_and_light;
		unsigned char world_time_hour;
		unsigned char world_time_minute;
		unsigned char weather;
		bool no_nametags_behind_walls;
		int player_markers_mode;
		float chat_radius;
		bool name_tags;
		bool ltd_chat_radius;
	};

	char pad_0[32];
	char host_address[257];
	char hostname[257];
	bool disable_collision; // turns off interacting with any player in a vehicle
	bool update_camera_target;
	bool nametag_status;
	int port;
	BOOL lan_mode;
	int map_icons[100];
	int game_state;
	int last_connect_attempt;

	c_netgame*& ref() {
		return *(c_netgame**)rakhook::samp_addr(offsets::netgame::ref[VERSION]);
	}

	c_player_pool* get_player_pool() {
		return ((c_player_pool * (__thiscall*)(c_netgame*)) rakhook::samp_addr(offsets::netgame::get_player_pool[VERSION]))(this);
	}
};

class c_game : public singleton<c_game> {

public:

	c_game*& ref() {
		return *(c_game**)rakhook::samp_addr(offsets::game::ref[VERSION]);
	}

	void set_cursor_mode(int mode, int hide) {
		((void(__thiscall*)(c_game*, int, int)) rakhook::samp_addr(offsets::game::set_cursor_mode[VERSION]) )(this, mode, hide);
	}
};


#undef VERSION