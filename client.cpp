#include "shared_library.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    // process arguments
    Options opt = parse_arguments(argc, argv, true);

    if (opt.fork)
        loop_client_fork(opt);
    else
        client_nofork(opt);
        
    return 0;
}
