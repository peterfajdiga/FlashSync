#pragma once

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>


#ifdef _WIN32
class uofstream : public boost::filesystem::wofstream {
    
public:
    explicit uofstream();
    explicit uofstream(const boost::filesystem::path& filename, std::ios_base::openmode mode = ios_base::out);
    void open(const boost::filesystem::path& filename, std::ios_base::openmode mode = ios_base::out);
};
#else
typedef boost::filesystem::ofstream uofstream;
#endif
