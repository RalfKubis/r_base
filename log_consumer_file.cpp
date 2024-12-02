#include "r_base/log_consumer_file.h"

#include <fstream>


namespace nsBase
{
using namespace ::std::string_literals;

static ::fs::path
    log_file_path;

void
log_consumer_file_path_assign(
    ::fs::path const & p
)
{
    log_file_path = p;
}


::fs::path
log_consumer_file_path()
{
    return log_file_path;
}


void
log_consumer_file(
    Log & log
)
{
    static ::std::ofstream
        stream;

    if (!stream.is_open())
    {
        if (log_file_path.empty())
            log_file_path = "./unnamed.log"_path;

        auto mode = ::std::ios::app | ::std::ios::binary | ::std::ios::out;

        stream.open(log_file_path, mode);
    }

    if (!stream.is_open())
        return;

    auto
        line = log.serialize() + "\n";

    stream.write(line.c_str(), line.size()).flush();
}

}
