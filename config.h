#ifndef plugin_config
#define plugin_config

#include <iostream>
#include <map>
#include "mini/src/ini.h"
#include "utils.h"

class c_config
{
public:
	c_config(std::string name);
	~c_config();

	using config_t = std::map<std::pair<std::string, std::string>, bool>;
	
	void load_settings();
	void save_settings_to_ini();

	config_t config = {
			{ { "push", "info" }, false },
			{ {"push", "hint"}, false },
			{ {"push", "congratulate"}, false },
			{ {"push", "message"}, false },
			{ {"push", "error"}, false },

			{ {"fullscreen", "hint"}, false },
			{ {"fullscreen", "congratulate"}, false },
			{ {"fullscreen", "error"}, false },

			{ {"centered", "hint"}, false },
			{ {"centered", "error"}, false },
			{ {"centered", "congratulate"}, false },
	};

private:
	std::unique_ptr<mINI::INIFile> file;
	mINI::INIStructure ini;
};

#endif