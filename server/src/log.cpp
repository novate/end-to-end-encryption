#include "../include/log.hpp"

std::string logify_data(std::vector<uint8_t> & message) {
    std::stringstream ss, ss_word;
    u_int i;
    for (i = 0; i < message.size(); i++) {
        if (i % 16 == 0) {
            ss << ss_word.str() << std::endl;
            ss_word.clear();    //clear any bits set
            ss_word.str(std::string());
            ss << std::uppercase << ' ' << std::setw(4) << std::setfill('0') << std::hex << i << ": ";
        }
        else if (i % 8 == 0) {
            ss << "- ";
        }
        ss << std::setw(2) << std::setfill('0') << std::hex << +message[i] << ' ';
        // print printable char.
        char ch = (message[i] > 31 && message[i] < 127) ? message[i] : '.';
        ss_word << ch;
    }  
    if (i%16==0){
        ss << std::setw(0) << ss_word.str();
    }
    else {
        auto interval = 3 * (16 - (i % 16)) + (i % 16 > 8 ? 0 : 2);
        ss << std::setw(interval) << std::setfill(' ') << ' ' << std::setw(0) << ss_word.str();
    }
    return ss.str();
}

std::string logify_data(const uint8_t* data, const int len) {
    std::stringstream ss, ss_word;
    // ss_word.str(std::string());
    int i;
    for (i = 0; i < len; i++) {
        if (i % 16 == 0) {
            ss << ss_word.str() << std::endl;
            ss_word.clear();    //clear any bits set
            ss_word.str(std::string());
            ss << ' ' << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << i << ": ";
        }
        else if (i % 8 == 0) {
            ss << "- ";
        }
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << +data[i] << ' ';
        // print printable char.
        char ch = (data[i] > 31 && data[i] < 127) ? data[i] : '.';
        ss_word << ch;
        // ss_word << data[i];
    }  
    if (i%16==0){
        ss << std::setw(0) << ss_word.str();
    }
    else {
        auto interval = 3 * (16 - (i % 16)) + (i % 16 > 8 ? 0 : 2);
        // cout << "i: " << i << ", interval: " << interval << endl;
        ss << std::setw(interval) << std::setfill(' ') << ' ' << std::setw(0) << ss_word.str();
    }
    return ss.str();
}

namespace fly
{

Log& Log::get()
{
    static Log m_instance;
    return m_instance;
}


std::ostream& Log::getStream()
{
    if ((u_int)m_logStream->tellp() > m_mainLogSize) {
        m_logStream->seekp(0, std::ios::beg);
    }
    return (m_onScreen ? *tee_logStream : *m_logStream);
}

std::ostream& Log::getErrStream()
{
    if ((u_int)m_errStream->tellp() > m_subLogSize) {
        m_errStream->seekp(0, std::ios::beg);
    }
    return (m_onScreen ? *tee_errStream : *m_errStream);
}

std::ostream& Log::getDBStream()
{
    if ((u_int)m_dbStream->tellp() > m_subLogSize) {
        m_dbStream->seekp(0, std::ios::beg);
    }
    return (m_onScreen ? *tee_dbStream : *m_dbStream);
}

std::ostream& Log::getStream(Level level)
{
    static std::ofstream void_stream("/dev/null", std::ios::out);
    if ((u_int)m_logStream->tellp() > m_mainLogSize) {
        m_logStream->seekp(0, std::ios::beg);
    }
    switch(level) {
        case Level::Debug:
        {
            if (level != Level::Debug) {
                return void_stream;
                break;
            }
            else {
                return (m_onScreen ? *tee_logStream : *m_logStream);
                break;
            }
        }
        case Level::TP_S:
        {
            if (m_logEnv[0][0]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::TP_R:
        {
            if (m_logEnv[0][1]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::TP_SD:
        {
            if (m_logEnv[0][2]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::TP_RD:
        {
            if (m_logEnv[0][3]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::TS_S:
        {
            if (m_logEnv[1][0]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::TS_R:
        {
            if (m_logEnv[1][1]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::TS_SD:
        {
            if (m_logEnv[1][2]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::TS_RD:
        {
            if (m_logEnv[1][3]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DP_S:
        {
            if (m_logEnv[2][0]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DP_R:
        {
            if (m_logEnv[2][1]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DP_SD:
        {
            if (m_logEnv[2][2]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DP_RD:
        {
            if (m_logEnv[2][3]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DS_S:
        {
            if (m_logEnv[3][0]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DS_R:
        {
            if (m_logEnv[3][1]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DS_SD:
        {
            if (m_logEnv[3][2]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }
        case Level::DS_RD:
        {
            if (m_logEnv[3][3]) {
                return (m_onScreen ? *tee_logStream : *m_logStream);
            }
            else {
                return void_stream;
            }
            break;
        }        
        default:
        {
            return void_stream;
            break;
        } 
    }
}

void Log::setLogConf(std::ostream& log_stream, std::ostream& err_stream, std::ostream& db_stream, const bool (*log_env)[4], const bool on_screen, const u_int main_log_size, const u_int sub_log_size)
{
    m_logStream = &log_stream;
    m_errStream = &err_stream;
    m_dbStream = &db_stream;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m_logEnv[i][j] = log_env[i][j];
        }
    }
    static TeeStream tee_log(std::cout, *m_logStream);
    static TeeStream tee_err(std::cout, *m_errStream); 
    static TeeStream tee_db(std::cout, *m_dbStream);
    if(on_screen) {
        m_onScreen = true;
        tee_logStream = &tee_log;
        tee_errStream = &tee_err;
        tee_dbStream = &tee_db;
    }
    m_mainLogSize = main_log_size;
    m_subLogSize = sub_log_size;
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
