#include "log.hpp"

namespace fly
{

Log& Log::get()
{
    static Log m_instance;
    return m_instance;
}


std::ostream& Log::getStream()
{
    return *m_logStream;
}

std::ostream& Log::getStream(Level level)
{
    static std::ofstream void_stream("/dev/null", std::ios::out);
    switch(level) {
        case Level::Debug:
            if (level > m_logLevel) {
                return void_stream;
                break;
            }
            else {
                return *m_logStream;
                break;
            }
        case Level::ENV:
            if (m_logEnv[0]) {
                return *m_logStream;
            }
            else {
                return void_stream;
            }
            break;
        case Level::ERR:
            if (m_logEnv[1]) {
                return *m_logStream;
            }
            else {
                return void_stream;
            }
            break;
        case Level::SPACK:
            if (m_logEnv[2]) {
                return *m_logStream;
            }
            else {
                return void_stream;
            }
            break;
        case Level::RPACK:
            if (m_logEnv[3]) {
                return *m_logStream;
            }
            else {
                return void_stream;
            }
            break;
        case Level::SDATA:
            if (m_logEnv[4]) {
                return *m_logStream;
            }
            else {
                return void_stream;
            }
            break;
        case Level::RDATA:
            if (m_logEnv[5]) {
                return *m_logStream;
            }
            else {
                return void_stream;
            }
            break;
        default:
            return void_stream;
            break;
    }
}

void Log::setLogStream(std::ostream& stream)
{
    m_logStream = &stream;
}

void Log::setEnv(const bool* const log_env)
{
    for (int i = 0; i < 6; i++) {
        m_logEnv[i] = log_env[i];
    }
}

void Log::setOnScreen(const bool on_screen)
{ 
    static TeeStream tee(std::cout, *m_logStream);  
    if(on_screen) {
        m_logStream = &tee;
    }
}

Log& Log::setLevel(Level level)
{
    m_logLevel = level;
    return *this;
}

Level Log::getLevel()
{
    return m_logLevel;
}

std::string Log::getTime()
{
    time_t now = time(0);
    char timestamp[100] = "";
    strftime(timestamp, 100, "%F %T", localtime(&now));
    std::string cur_time(timestamp);
    return cur_time;
}

TeeBuf::TeeBuf(std::streambuf * sb1, std::streambuf * sb2) :
    m_sb1(sb1),
    m_sb2(sb2)
{}
int TeeBuf::overflow(int c)
{
    if (c == EOF)
    {
        return !EOF;
    }
    else
    {
        int const r1 = m_sb1->sputc(c);
        int const r2 = m_sb2->sputc(c);
        return r1 == EOF || r2 == EOF ? EOF : c;
    }
}

int TeeBuf::sync()
{
    int const r1 = m_sb1->pubsync();
    int const r2 = m_sb2->pubsync();
    return r1 == 0 && r2 == 0 ? 0 : -1;
}

TeeStream::TeeStream(std::ostream& o1, std::ostream& o2) :
    std::ostream(&m_tbuf),
    m_tbuf(o1.rdbuf(), o2.rdbuf())
{}

}
