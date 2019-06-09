#include "../include/instances.hpp"
using namespace std;
using namespace fly;

PresentationLayer PreLayerInstance;
TransferLayer TransLayerInstance;

Options opt;

void output(queue<vector <uint8_t> > &test)
{
    while(!test.empty()) {
        vector<uint8_t> test_vec = test.front();
        vector<uint8_t>::iterator it;
        for(it = test_vec.begin(); it != test_vec.end(); it++) {
                cout << (unsigned)*it << "*";
        }
        cout << endl;
        test.pop();
    }
}

int log_init(std::ofstream &log_stream, const std::string log_name, std::ofstream &err_stream, const std::string err_name, std::ofstream &db_stream, const std::string db_name, const Level level, const bool (*log_env)[4], const bool on_screen, const u_int main_log_size, const u_int sub_log_size) {
    // streams must not be opened before getting into this function.
    if (log_stream.is_open() || err_stream.is_open() || db_stream.is_open()) {
        return -1;
    }
    if (main_log_size == 0) {
        log_stream.open("/dev/null", ios::out);
    }
    else {
        log_stream.open(log_name, ios::out|ios::trunc);
    }
    if (sub_log_size == 0) {
        err_stream.open("/dev/null", ios::out);
        db_stream.open("/dev/null", ios::out);
    }
    else {
        err_stream.open(err_name, ios::out|ios::trunc);
        db_stream.open(db_name, ios::out|ios::trunc);
    }
    // if (!log_stream.is_open() || !err_stream.is_open() || !db_stream.is_open()) {
    //     return -2;
    // }
    u_int s_main_size = main_log_size - kSub;
    u_int s_sub_size = sub_log_size - kSub;
    Log::get().setLogConf(log_stream, err_stream, db_stream, log_env, on_screen, s_main_size, s_sub_size);
    Log::get().setLevel(level);
    return 0;
}

int main()
{
    // Read config
    ifstream ifs(kFnConfServer);
    if (!ifs.is_open()) {
        cout << "找不到该配置文件：" << kFnConfServer << endl;
        cout << "请将该配置文件和本可执行文件置于同一个目录下" << endl;
        return -1;
    }

    Options opt = parse_arguments(ifs);

    bool log_env[4][4];
    string s_tp = opt.at("tmp_packet");
    string s_ts = opt.at("tmp_socket");
    string s_dp = opt.at("dev_packet");
    string s_ds = opt.at("dev_socket");
    for (u_int i = 0; i < 4; i++) {
        log_env[0][i] = (s_tp[i] == '1');
    }
    for (u_int i = 0; i < 4; i++) {
        log_env[1][i] = (s_ts[i] == '1');
    }
    for (u_int i = 0; i < 4; i++) {
        log_env[2][i] = (s_dp[i] == '1');
    }
    for (u_int i = 0; i < 4; i++) {
        log_env[3][i] = (s_ds[i] == '1');
    }

    bool print_on_screen = (opt.at("屏幕显示") == "1");
    u_int main_log_size = stoi(opt.at("主日志大小")) * 1024;
    u_int sub_log_size = stoi(opt.at("分日志大小")) * 1024;
    string db_ip = opt.at("服务器IP地址");
    u_int db_port = stoi(opt.at("服务器端口号"));
    string db_name = opt.at("数据库名");
    string db_username = opt.at("用户名");
    string db_pwd = opt.at("用户口令");

    // Log initialization
    ofstream log_stream, err_stream, db_stream;
    if (log_init(log_stream, "yzmond.log", err_stream, "yzmond_err.log", db_stream, "yzmond_db.log", Level::Debug, log_env, print_on_screen, main_log_size, sub_log_size) < 0) {
        std::cout << "Open log error!" << endl;
        return -1;
    }

    LENV << "客户端参数读取结果：" << endl;
    for (const auto &el : opt) {
        LENV << el.first << "=" << el.second << endl;
    }

    time_t before = time(0);
    char timestamp[100] = "";
    strftime(timestamp, 100, "%F %T", localtime(&before));
    std::string cur_time(timestamp);
    LENV << "服务器主进程开始，当前时间" << cur_time << std::endl;

    int listener = TransLayerInstance.get_listener(stoi(opt.at("监听端口号")));
    TransLayerInstance.select_loop(listener);

    return 0;
}
