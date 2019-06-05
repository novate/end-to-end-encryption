#include "server_lib.hpp"

using namespace std;

const bool is_client = false;

int main(int argc, char *argv[])
{
    // process arguments
    string fn_conf = is_client ? kFnConfClient : kFnConfServer;
    ifstream ifs(fn_conf);
    Options opt = parse_arguments(ifs, is_client);

    int listener = get_listener(opt);
    int ret = loop_server(listener, opt);
    close(listener);
    return ret;
}
