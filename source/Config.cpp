#include "Config.h"

#include "global.h"

#include <iostream>

#include <boost/nondet_random.hpp>

namespace fs = boost::filesystem;


Config::Config(fs::path dir) {
    this->dir = dir;
    load();
}

void Config::load() {
    fs::ifstream configFile;
    configFile.open(dir / "id", std::ios::binary);
    configFile >> id;
    configFile.close();
    if (configFile.fail()) {
        std::cout << "No valid config found in " << dir << "\nCreating a new one.\n";
        setup();
    }
}

void Config::generateId() {
    boost::random_device rd;
    id = rd();
}

void Config::setup() {
    generateId();
    try {
        create_directory(dir);
    } catch (std::runtime_error e) {
        std::cerr << "Error creating directory " << dir << std::endl;
        exit(ERR_CONFIG_SAVE);
    }
    fs::ofstream configFile;
    configFile.open(dir / "id");
    configFile << id;
    configFile.close();
    if (configFile.fail()) {
        std::cerr << "Error creating a config in " << dir << std::endl;
        exit(ERR_CONFIG_SAVE);
    }
}
