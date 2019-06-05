#include <iostream>
#include "shared_library.hpp"
#include "log.hpp"

using namespace std;
using namespace fly;

int main() {
    ofstream log_stream;
    if(log_init(log_stream, "end-to-end-client.log", Info) < 0) {
        cout << "[Client] Open log error!" << endl;
        return -1;
    }
    
    LOG(Info) << "info test" << endl;

}