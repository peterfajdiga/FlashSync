#include "ConfigComputer.h"

#include "global.h"
#include "uifstream.h"
#include "uofstream.h"

#include <iostream>

namespace fs = boost::filesystem;


extern bool verbose;

void ConfigComputer::loadPathFile(const id_t flash_id) {
    uifstream pathFile;
    pathFile.open(getPathFile(flash_id));
    pathFile >> syncDir;
    pathFile.close();
    if (pathFile.fail()) {
        setupPathFile(flash_id);
    } else if (verbose) {
        std::cerr << "Sync dir for this flash drive is set to " << syncDir << std::endl;
    }
}

void ConfigComputer::setupPathFile(const id_t flash_id) {
    std::cout << "This computer has no sync directory set for this flash drive. Set one now (the directory must already be up to date): ";
    fs::path newSyncDir;
    
    ask_for_dir: std::cin >> newSyncDir;
    if (!is_directory(newSyncDir)) {
        std::cout << newSyncDir << " is not a valid directory. Try again: ";
        goto ask_for_dir;
    }
    
    setSyncDir(flash_id, newSyncDir);
}

void ConfigComputer::setSyncDir(const id_t flash_id, const fs::path& newSyncDir) {
    syncDir = fs::absolute(newSyncDir);
    uofstream pathFile;
    pathFile.open(getPathFile(flash_id));
    pathFile << syncDir.generic_path();
    pathFile.close();
    if (pathFile.fail()) {
        std::cerr << "Error creating the path file in " << dir << std::endl;
        exit(ERR_CONFIG_SAVE);
    }
}

void ConfigComputer::unregister(const id_t flash_id) const {
    fs::remove(getPathFile(flash_id));
}


inline fs::path ConfigComputer::getPathFile(const id_t flash_id) const {
    return dir / std::to_string(flash_id);
}
