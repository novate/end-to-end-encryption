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
    string s_env = opt.at("DEBUG����");
    for (u_int i = 0; i < s_env.length(); i++) {
        log_env[i] = (s_env[i] == '1');
    }
    bool print_on_screen = (opt.at("DEBUG��Ļ��ʾ") == "1");
	bool is_trunc = (opt.at("ɾ����־�ļ�") == "1");

    // Log initialization
    ofstream log_stream;
    if (log_init(log_stream, "ts.log", Level::RDATA, log_env, print_on_screen, is_trunc) < 0) {
        std::cout << "[Client] Open log error!" << endl;
        return -1;
    }

    int begin_dev_id, n_devid;
    if (argc < 3) {
        cerr << "����Ҫ����2��������\n";
        print_help();
        return -1;
    } else {
        begin_dev_id = stoi(argv[1]);
        n_devid = stoi(argv[2]);
    }

    LOG(Level::ENV) << "fork�ӽ��̿�ʼ" << std::endl;
    time_t before = time(0);
    int ret = loop_client_fork(opt, begin_dev_id, n_devid);
    time_t after = time(0);
    double elapsed_time = difftime(after, before);
    if (elapsed_time < 1) {
        LOG(Level::ENV) << "�ӽ��̻���ȫ����ɣ�����=" << n_devid << "����ʱС��1��" << std::endl;
    }
    else {
        LOG(Level::ENV) << "�ӽ��̻���ȫ����ɣ�����=" << n_devid << "����ʱ=" << elapsed_time << "��" << std::endl;
    }
    // LOG(Level::ENV) << "�ӽ��̻���ȫ����ɣ�����=" << n_devid << "����ʱ=" << elapsed_time << "��" << std::endl;
    
    return ret;
}
