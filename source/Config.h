#pragma once

#include "global.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>


class Config {
    
public:
    id_t id;
    boost::filesystem::path dir;
    
public:
    Config(boost::filesystem::path dir);
    
private:
    void load();
    void generateId();
    void setup();
};
