#include "client_lib.hpp"
#include "client_communicate.hpp"

using namespace std;

const bool is_client = true;

int main(int argc, char *argv[])
{
    string fn_conf = is_client ? kFnConfClient : kFnConfServer;
    ifstream ifs(fn_conf);
    Options opt = parse_arguments(ifs, is_client);

    bool log_env[6];
    string s_env = opt.at("DEBUG设置");
    for (u_int i = 0; i < s_env.length(); i++) {
        log_env[i] = (s_env[i] == '1');
    }
    bool print_on_screen = (opt.at("DEBUG屏幕显示") == "1");
	bool is_trunc = (opt.at("删除日志文件") == "1");

    // Log initialization
    ofstream log_stream;
    if (log_init(log_stream, "ts.log", Level::RDATA, log_env, print_on_screen, is_trunc) < 0) {
        std::cout << "[Client] Open log error!" << endl;
        return -1;
    }

    int begin_dev_id, n_devid;
    if (argc < 3) {
        cerr << "您需要输入2个参数！\n";
        print_help();
        return -1;
    } else {
        begin_dev_id = stoi(argv[1]);
        n_devid = stoi(argv[2]);
    }

    LOG(Level::ENV) << "fork子进程开始" << std::endl;
    time_t before = time(0);
    int ret = loop_client_fork(opt, begin_dev_id, n_devid);
    time_t after = time(0);
    double elapsed_time = difftime(after, before);
    if (elapsed_time < 1) {
        LOG(Level::ENV) << "子进程回收全部完成，总数=" << n_devid << "，耗时小于1秒" << std::endl;
    }
    else {
        LOG(Level::ENV) << "子进程回收全部完成，总数=" << n_devid << "，耗时=" << elapsed_time << "秒" << std::endl;
    }
    // LOG(Level::ENV) << "子进程回收全部完成，总数=" << n_devid << "，耗时=" << elapsed_time << "秒" << std::endl;
    
    return ret;
}
