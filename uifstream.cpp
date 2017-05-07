#ifdef _WIN32
#include "uifstream.h"

#include <boost/locale.hpp>

namespace fs = boost::filesystem;


uifstream::uifstream() {}

uifstream::uifstream(const fs::path& filename, std::ios_base::openmode mode) {
    open(filename, mode);
}

void uifstream::open(const fs::path& filename, std::ios_base::openmode mode) {
    boost::locale::generator gen;
    imbue(gen("UTF-8"));
    fs::wifstream::open(filename, mode);
}
#endif
