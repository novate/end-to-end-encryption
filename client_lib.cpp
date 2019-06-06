#include "client_lib.hpp"
#include "communicate.hpp"

int loop_client_fork_high_concurrency(const Options &opt, int n_devid) {
    int num_concurrent = 0;

    // For now, fork all
    // TODO: limit fork number by using another set of semaphore

    // Create semaphores
    int n_semaphores = ceil(n_devid / kNumConcurrency);
    sem_t* semaphores[n_semaphores];

    pid_t pid;
    for (int i = 0; i < n_devid; i++) {
        if (i < n_semaphores) { // Initialize semaphores
            string sem_name = "client_fork_sem_" + itoa(i);
            errno = 0;
            semaphores[i] = sem_open(sem_name, O_CREAT, 0644, kNumConcurrency);
            if (semaphores[i] == SEM_FAILED) {
                perror("semaphore {" +  sem_name "} initilization");
                exit(1);
            }
        }
        // fork with a semaphore
        pid = fork();
        if (pid == -1) { // Fork error
            graceful("loop_client_fork fork", -20);
        } else if (pid == 0) { // Child process
            // TODO: correct parameters
            client_communicate(???, opt, semaphores[i % n_semaphores]);
        } else { // Parent process

        }
    }
}

int loop_client_fork(const Options &opt, int n_devid) {
    pid_t pid;
    for (int i = 0; i < n_devid; i++) {
        pid = fork();
        if (pid == -1) {
            graceful("loop_client_fork fork", -20);
        } else if(pid == 0) {
            Client client;
            // child: only terminate if communication succeed
            do {
                int sockfd = create_connection(opt);
            }
            while (client.client_communicate(socketfd, opt)) 
                //TODO: fail & reconnect


            // stage done
            // exit(0);
        } else { // parent: noop
        }
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
