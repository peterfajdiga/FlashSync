#pragma once

#include "global.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>


namespace FileOperations {
    
    hash_t hashFile(const boost::filesystem::path& filename);
    
    // these functions return true if done, false if skipped
    bool copy_file(
        const boost::filesystem::path& from,
        const boost::filesystem::path& to,
        boost::filesystem::copy_option option = boost::filesystem::copy_option::none
    );
    bool remove(const boost::filesystem::path& p);
}
