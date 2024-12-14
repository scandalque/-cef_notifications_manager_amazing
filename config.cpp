#include "config.h"

c_config::c_config(std::string name)
{
	file = std::make_unique<mINI::INIFile>(name);
	
}

void c_config::load_settings() {
	if (!file->read(ini)) {
		save_settings_to_ini();
	}
	config.clear();
	for (auto const& it : ini)
	{
		auto const& section = it.first;
		auto const& collection = it.second;
		for (auto const& it2 : collection)
		{
			auto const& key = it2.first;
			auto const& value = it2.second;
			config[{section, key}] = utils::string_to_bool(value);
		}
	}
}

void c_config::save_settings_to_ini() {
	if (!config.size()) {
		config = {
			{ { "push", "info" }, false },
			{ {"push", "hint"}, false },
			{ {"push", "congratulate"}, false },
			{ {"push", "message"}, false },
			{ {"push", "error"}, false },

			{ {"fullscreen", "hint"}, false },
			{ {"fullscreen", "congratulate"}, false },
			{ {"fullscreen", "error"}, false },

			{ {"centered", "hint"}, false },
		};
	}

	for (const auto& _config : config) {
		ini[_config.first.first][_config.first.second] = utils::bool_to_string(config[{_config.first.first, _config.first.second}]);
	}
	file->generate(ini, true);
	
}

c_config::~c_config()
{
	save_settings_to_ini();
}