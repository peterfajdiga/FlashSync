#ifdef _WIN32
#include "uofstream.h"

#include <boost/locale.hpp>

namespace fs = boost::filesystem;


uofstream::uofstream() {}

uofstream::uofstream(const fs::path& filename, std::ios_base::openmode mode) {
    open(filename, mode);
}

void uofstream::open(const fs::path& filename, std::ios_base::openmode mode) {
    boost::locale::generator gen;
    imbue(gen("UTF-8"));
    fs::wofstream::open(filename, mode);
}
#endif
