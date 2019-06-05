#include "communicate.hpp"

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

int client_communicate(Options opt) {
    int socketfd = -1;

    // 三种情况下继续循环
        // 1. 认证请求失败
        // 2. 传输完成
        // 3. socket错误（断开连接）
    for(;;) {
        if (socketfd == -1) {
            // 未连接，发起连接
            socketfd = create_connection(opt);
            // TODO: 错误处理
        } else {
            // 开始收发逻辑...
            // 出现以上三种情况，则continue
            // 收发正确完成，则break
        }

    }

    // \TODO{zzy: send data to parent using blocking pipe before returning a positive value}
    return 1;
}

int server_communicate(int socketfd, Options opt);
    return 0;
}
