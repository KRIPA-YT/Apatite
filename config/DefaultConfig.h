#pragma once
#define YAML_CPP_STATIC_DEFINE
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <yaml-cpp/yaml.h>

namespace config {
	class DefaultConfig {
	public:
		std::string fileName;
		YAML::Node config;

		virtual ~DefaultConfig() {
			this->save();
		}

		void save() {
			std::ofstream fout(this->fileName);
			fout << this->config;
			fout.close();
		}

		void load(std::string fileName) {
			this->fileName = fileName;
			std::ifstream file(this->fileName.c_str());
			if (file.good()) {
				this->config = YAML::LoadFile(this->fileName);
			} else {
				this->config = YAML::Load("");
			}
			auto* defaults = this->getDefaults();

			for (std::map<std::string, YAML::Node>::const_iterator it = defaults->begin(); it != defaults->end(); it++) {
				if (config[it->first]) { // If key exists
					continue;
				}
				// Else create key with default value
				config[it->first] = it->second;
			}
			delete defaults;
		}

	protected:
		virtual std::map<std::string, YAML::Node>* getDefaults() {
			return new std::map<std::string, YAML::Node>{};
		}
	};
}