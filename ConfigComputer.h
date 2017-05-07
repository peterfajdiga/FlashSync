#pragma once

#include "Config.h"


class ConfigComputer : public Config {
    
public:
    boost::filesystem::path syncDir;
    
private:
    inline boost::filesystem::path getPathFile(const id_t flash_id) const;
    
public:
    using Config::Config;
    void loadPathFile(const id_t flash_id);
    void setupPathFile(const id_t flash_id);
    void setSyncDir(const id_t flash_id, const boost::filesystem::path& newSyncDir);
    void unregister(const id_t flash_id) const;
};
