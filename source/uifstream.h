#pragma once

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>


#ifdef _WIN32
class uifstream : public boost::filesystem::wifstream {
    
public:
    explicit uifstream();
    explicit uifstream(const boost::filesystem::path& filename, std::ios_base::openmode mode = ios_base::in);
    void open(const boost::filesystem::path& filename, std::ios_base::openmode mode = ios_base::in);
};
#else
typedef boost::filesystem::ifstream uifstream;
#endif
