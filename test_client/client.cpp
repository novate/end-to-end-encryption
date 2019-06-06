#include "client_lib.hpp"
#include "client_communicate.hpp"

using namespace std;

const bool is_client = true;

int main(int argc, char *argv[])
{
    // argv: ./tc 100

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
    if (log_init(log_stream, "ts.log", Level::Debug, log_env, print_on_screen, is_trunc) < 0) {
        std::cout << "[Client] Open log error!" << endl;
        return -1;
    }

    int begin_dev_id, n_devid;
    if (argc < 3) {
        cerr << "argc != 3\n";
        return 1;
    } else {
        begin_dev_id = stoi(argv[1]);
        n_devid = stoi(argv[2]);
    }

    // TODO: begin_dev_id
    int ret = loop_client_fork(opt, begin_dev_id, n_devid);

    // \TODO{zzy: collect data using pipe and write to file}

    return ret;
}
