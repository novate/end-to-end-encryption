#include "client_lib.hpp"

using namespace std;

int loop_client_fork_high_concurrency(const Options &opt, int begin_dev_id, int n_devid) {
    // Create semaphores
    string sem_name = "/client_fork_sem";
    errno = 0;
    sem_t* sem = sem_open(sem_name.c_str(), O_CREAT, 0644, 0);
    // int sem_value;
    // sem_getvalue(sem, &sem_value);
    // cout << "Set value: " << kNumConcurrency << ". Semaphore value: " << sem_value << endl;
    if (sem == SEM_FAILED) {
        perror(("semaphore {" + sem_name + "} initilization").c_str());
        exit(1);
    }

    for (int i = 0; i < kNumConcurrency; i++) {
        // hack
        sem_post(sem);
    }

    pid_t pid;
    for (int i = 0; i < n_devid; i++) {
        // fork with a semaphore
        // sem_getvalue(sem, &sem_value);
        // cout << "Loop " << i << ". Semaphore value: " << sem_value << endl;
        cout << "Loop " << i << endl;
        sem_wait(sem);
        pid = fork();
        if (pid == -1) { // Fork error
            graceful("loop_client_fork fork", -20);
        } else if (pid == 0) { // Child process
            Client client(begin_dev_id + i);
            int socketfd;
            sem_wait(sem);
            do {
                socketfd = create_connection(opt);
            }
            while (client.client_communicate(socketfd, opt));
            sem_post(sem);
        } else { // Parent process
            sem_post(sem);
        }
    }
    for (int i = 0; i < n_devid; i++) {
        waitpid(-1, NULL, 0); // waiting for all children
    }
    return 0;
}

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
