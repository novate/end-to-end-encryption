#include "client_lib.hpp"

using namespace std;

const bool is_client = true;

int main(int argc, char *argv[])
{
    // argv: ./tc 100
    int n_devid;
    if (argc != 2) {
        cerr << "argc != 2\n";
        return 1;
    } else {
        n_devid = stoi(argv[1]);
    }

    string fn_conf = is_client ? kFnConfClient : kFnConfServer;
    ifstream ifs(fn_conf);
    Options opt = parse_arguments(ifs, is_client);

    int ret = loop_client_fork(opt, n_devid);

    // \TODO{zzy: collect data using pipe and write to file}

    return ret;
}
