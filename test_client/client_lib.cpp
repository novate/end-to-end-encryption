#include "client_lib.hpp"

int loop_client_fork(const Options &opt, int begin_dev_id, int n_devid) {
    pid_t pid;
    for (int i = 0; i < n_devid; i++) {
        pid = fork();
        if (pid == -1) {
            graceful("loop_client_fork fork", -20);
        } else if(pid == 0) {
            Client client(begin_dev_id + i);
            // child: only terminate if communication succeed
            int socketfd;
            do {
                socketfd = create_connection(opt);
            }
            while (client.client_communicate(socketfd, opt));
            close(socketfd);
            exit(0);
        } else { // parent: noop
        }
        // limit fork number
        if (i >= kNumProcess) {
            waitpid(-1, NULL, 0); // waiting for all children
        }
    }
    for (int i = 0; i < max(n_devid - kNumProcess, n_devid); i++) {
        waitpid(-1, NULL, 0); // waiting for all children
    }
    return 0;
}

int create_connection(const Options &opt) {
    // create a connection from opt
    // return negative code on error, socketfd on success

    struct sockaddr_in   servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(stoi(opt.at("端口号")));
    if(inet_pton(AF_INET, opt.at("服务器IP地址").c_str(), &servaddr.sin_addr) < 0)
        graceful_return("Invalid ip address", -1);

    // get a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        graceful_return("socket", -2);

    // connect blocking
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        graceful_return("connect", -3);
    }
    return sockfd;
}
