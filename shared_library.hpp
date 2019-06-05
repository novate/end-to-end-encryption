#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>
#include <queue>
#include <set>
#include <algorithm>

#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "parse_arguments.hpp"
#include "Log.h"

using namespace fly;
using namespace std;

// gracefully perror and return
#define graceful_return(s, x) {\
    perror((s));\
    return((x)); }

// gracefully perror and exit
inline void graceful(const char *s, int x) { perror(s); exit(x); }

#endif // SHARED_LIBRARY_H