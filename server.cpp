#include "shared_library.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    // process arguments
    Options opt = parse_arguments(argc, argv, false);

    int listener = get_listener(opt);

    if (opt.fork)
        loop_server_fork(listener, opt);
    else
        loop_server_nofork(listener, opt);

    // how can the loop possibly return?
    close(listener);
    return 0;
}
