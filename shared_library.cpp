#include "shared_library.hpp"

int log_init(std::ofstream &log_stream, const std::string log_name, const Level level) {
    // log_stream must not be opened before getting into this function.
    if (log_stream.is_open()) {
        return -1;
    }
    log_stream.open(log_name, ios::out|ios::trunc);
    if (!log_stream.is_open()) {
        return -2;
    }
    Log::get().setLogStream(log_stream);
    Log::get().setLevel(level);
    return 0;
}
