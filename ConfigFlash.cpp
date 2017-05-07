#include "ConfigFlash.h"

#include "global.h"

#include <iostream>
#include <string>

namespace fs = boost::filesystem;


extern bool verbose;

ConfigFlash::ConfigFlash(fs::path dir) : Config(dir) {
    loadComputersFile();
}

fs::path ConfigFlash::getFreeFilename() {
    std::string retval;
    while (exists(dir / (retval = std::to_string(filenameCounter)))) {
        filenameCounter++;
    }
    return fs::path(retval);
}

size_t ConfigFlash::getComputerCount() const {
    return computers.size();
}

void ConfigFlash::printComputers() const {
    for (id_t computer_id : computers) {
        std::cout << computer_id << std::endl;
    }
}


void ConfigFlash::registerComputer(const id_t computer_id) {
    if (computers.insert(computer_id).second) {
        // Condition is true if a new element was inserted
        
        if (verbose) {
            std::cerr << "Registering computer " << computer_id << std::endl;
        }
        
        saveComputersFile();
    }
}

void ConfigFlash::unregisterComputer(const id_t computer_id) {
    if (computers.erase(computer_id)) {
        saveComputersFile();
    } else {
        std::cerr << computer_id << " is not a registered computer id.\n";
    }
}


void ConfigFlash::loadComputersFile() {
    fs::ifstream computersFile;
    computersFile.open(dir / "computers");
    if (computersFile.fail()) {
        if (verbose) {
            std::cerr << "No computers registered yet.\n";
        }
        return;
    }
    id_t computerId;
    while (computersFile >> computerId) {
        computers.insert(computerId);
    }
    computersFile.close();
}

void ConfigFlash::saveComputersFile() {
    fs::ofstream computersFile;
    computersFile.open(dir / "computers");
    for (id_t computer_id : computers) {
        computersFile << computer_id << ' ';
    }
    computersFile.close();
    if (computersFile.fail()) {
        std::cerr << "Error saving the computers file in " << dir << std::endl;
        exit(ERR_CONFIG_SAVE);
    }
}
