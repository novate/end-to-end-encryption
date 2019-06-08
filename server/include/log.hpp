#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cstring>
#include <vector>
#include <ctime>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

/*
#define LOG(level) \
if (level > fly::Log::get().getLevel()) ; \
else fly::Log::get().getStream() << fly::Log::get().getTime() << " [" << getpid() << "] "
*/
// else fly::Log::get().getStream() << "["#level"]" << "[" << __FILENAME__ << ":" << std::dec << __LINE__ << "] "

#define LOG(level) fly::Log::get().getStream(level) << dec << fly::Log::get().getTime() << " [" << getpid() << "] "
#define LERR fly::Log::get().getErrStream() << dec << fly::Log::get().getTime() << " [" << getpid() << "] "
#define LDB fly::Log::get().getDBStream() << dec << fly::Log::get().getTime() << " [" << getpid() << "] "

namespace fly
{

enum class Level: int
{
    TP_S,
    TP_R,
    TP_SD,
    TP_RD,
    TS_S,
    TS_R,
    TS_SD,
    TS_RD,
    DP_S,
    DP_R,
    DP_SD,
    DP_RD,
    DS_S,
    DS_R,
    DS_SD,
    DS_RD,
    Debug
};

class Log
{
public:
    void setLogConf(std::ostream& log_stream, std::ostream& err_stream, std::ostream& db_stream, const bool (*log_env)[4], const bool on_screen, const u_int main_log_size, const u_int sub_log_size);
    Log& setLevel(Level level);

    Level getLevel();
    std::ostream& getStream();
    std::ostream& getStream(Level level);
    std::ostream& getErrStream();
    std::ostream& getDBStream();
    static Log& get();
    std::string getTime();
private:
    Level m_logLevel;
    std::ostream* m_logStream;
    std::ostream* m_errStream;
    std::ostream* m_dbStream;
    std::ostream* tee_logStream;
    std::ostream* tee_errStream;
    std::ostream* tee_dbStream;
    bool m_logEnv[4][4] {{false, false, false, false}, {false, false, false, false}, {false, false, false, false}, {false, false, false, false}};
    bool m_onScreen = false;
    u_int m_mainLogSize = 10240 * 1024;
    u_int m_subLogSize = 1024 * 1024;
};


// Courtesy of http://wordaligned.org/articles/cpp-streambufs#toctee-streams
class TeeBuf : public std::streambuf
{
    public:
        // Construct a streambuf which tees output to both input
        // streambufs.
        TeeBuf(std::streambuf* sb1, std::streambuf* sb2);
    private:
        // This tee buffer has no buffer. So every character "overflows"
        // and can be put directly into the teed buffers.
        virtual int overflow(int c);
        // Sync both teed buffers.
        virtual int sync();
    private:
        std::streambuf* m_sb1;
        std::streambuf* m_sb2;
};

// Can be used to log into two ostream objects (like cout plus file)
class TeeStream : public std::ostream
{
    public:
        // Construct an ostream which tees output to the supplied
        // ostreams.
        TeeStream(std::ostream& o1, std::ostream& o2);
    private:
        TeeBuf m_tbuf;
};

};
#endif // LOG_H
