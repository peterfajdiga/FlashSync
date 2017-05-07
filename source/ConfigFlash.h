#pragma once

#include "Config.h"

#include <set>


class ConfigFlash : public Config {
    
private:
    size_t filenameCounter = 0;
    std::set<id_t> computers;
    
public:
    ConfigFlash(boost::filesystem::path dir);
    boost::filesystem::path getFreeFilename();
    size_t getComputerCount() const;
    void printComputers() const;
    void registerComputer(const id_t computer_id);
    void unregisterComputer(const id_t computer_id);
    
private:
    void loadComputersFile();
    void saveComputersFile();
};
