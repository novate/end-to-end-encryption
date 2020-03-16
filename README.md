# End to End Encryption Emulator

- [Background](#background)
- [Client](#client)
- [Server](#server)
- [Database Design](docs/database-design.md)
- [Transmission Protocol Design](docs/transmission-protocol-design.md)

## Background

A terminal encryption machine has been applied by each branch of a bank for the transmission encryption of production services. Now it is required to monitor each encryption terminal.

The specific design is:

1. Run a Client program on each encryption terminal.
2. Transmit the configuration, current status, etc. to the main server.
3. There is a Server program running on the main server, which is used to receive the information sent from each encryption terminal, and store the data into MySQL database for Web users to inspect.

![](pics/main-structure.png)

## Client

- [Client Usage](#client-usage)
- [Client Design](#client-design)
- [Client Test](#client-test)
- [Client Difficulties](#client-difficulties)

### Client Usage

1. Make the Client

```bash
# Go into the ./client directory and then make.
make
```

2. Run the Client by CLI with arguments:

- The first argument is the initial `devid` number.
- The second argument is the quantity of the clients.

```bash
# Emulate 50000 Clients with devid from 110101001 to 110151000 to send data to the main server.
./ts 110101001 50000
```

### Client Design

1. The Client is written in C/C++, with **forking method** (one process one socket). Each forked child emulates a `devid` to connect to the main server, and do all the work of data transmission.
2. The main process of the Client records the initial time, then fork childs to emulate data transmission. After the transmission ends, the main process recycles all child processes and records the ending time. The duration of the whole Client work is written into the log.
3. Each child process is designed based on the OSI module. For the convenience and compactness of the child process, all OSI layers are combined together.
4. Children process read configurations from [ts.conf](client/ts.conf) (in Simplified Chinese), read data payload to be transmitted from [config.dat](client/config.dat), [process.dat](client/process.dat) and [usbfiles.dat](client/usbfles.dat). The detailed transmission protocol design can be seen [here](docs/transmission-protocol-design.md).
5. Each child process write into the `ts_count.xls` for a line, including:

- transaction time
- `devid`
- number of devid (1)
- number of ttys
- number of screens

### Client Test

1. A qualified Server daemon will run on the main server, with TCP port number designated and exposed. Server is a black box to the clients.
2. For debugging convenience, the logs of Server will be output to the Client side as `logfile` and `pidfile` with all debugging options open.
3. The database which Server accesses is `yzmon` (see [Database Design](docs/database-design.md)), there are only 3 tables that is related to the Server:

- `devstate_base`: base information of clients.
- `devstate_ttyinfo`: information of the ttys of the clients.
- `devstate_scrinfo`: information of the screens of each tty.

4. Clients should write logs of the transmission, which includes the detailed information of transmission / reception packs, and compare the information sent with the information stroed in the database by the server to verify the correctness of the program.
5. The configuration of the Clients should be read from [ts.conf](client/ts.conf) (in Simplified Chinese), which should not be fixedly written into the program.
6. The quantity of emulated clients is at least 50000, each child process with a different `devid`.
7. The duration time of the whole Client work should be written into the log. The total transmission time should be less than 900s for 50000 `devid`s.
8. After transmission, check the consistency of the Client's `ts_count.xls` and the `SELECT count(*) from devstate_xxx` in `yzmon` database in Server, that whether the sums of dev, ttyinfo and scrinfo are identical.

Client Test: Client(↓)
![Client Test: Client](pics/client-test-client.png)
Client Test: Server(↓)
![Client Test: Server](pics/client-test-server.png)

### Client Difficulties

1. When `devid` is set as a big number (such as 50000), the system resources(memory, total active processes, TCP/IP protocol stack) are not enough. To make all clients run, we set the `kNumProcess` in `client_lib.hpp` to limit the number of the coexisting processes. All children processes should obey FIFO law. The best number is around 100.
2. How to keep the correctness for different processes to write the same file `ts_count.xls`?
- Add a mutex lock for the file accessing.
- Let the main process write the file.

## Server

<!-- ## Makefile Logic

The code base consists of two parts: the ./server and ./client binary executables.

The makefile generates object files for each library source file before linking them together to avoid the recompilation of code that isn't modified.

## Test Driven Development

This project is designed to be developed with a test driven approach to cope with the potential escalation of the complexity of future computer network projects.

### How to Write and Run a Test

First, we should make test run at both server side and client side using following commands:

```bash
# local tests: 9 tests
# server side
make && ./server --ip localhost --port 4000 --block --fork > /dev/null
make && ./server --ip localhost --port 4000 --nonblock --fork > /dev/null
make && ./server --ip localhost --port 4000 --nonblock --nofork > /dev/null
# client side
make && ./client --ip localhost --port 4000 --block --fork --num 50000 > /dev/null
make && ./client --ip localhost --port 4000 --nonblock --fork --num 50000 > /dev/null
make && ./client --ip localhost --port 4000 --nonblock --nofork --num 50000 > /dev/null

# online tests: 9 tests, with server side ip 10.60.102.252(you can modify it)
# server side
make && ./server --ip 0.0.0.0 --port 20350 --block --fork > /dev/null
make && ./server --ip 0.0.0.0 --port 20350 --nonblock --fork > /dev/null
make && ./server --ip 0.0.0.0 --port 20350 --nonblock --nofork > /dev/null
# client side
make && ./client --ip 10.60.102.252 --port 20350 --block --fork --num 50000 > /dev/null
make && ./client --ip 10.60.102.252 --port 20350 --nonblock --fork --num 50000 > /dev/null
make && ./client --ip 10.60.102.252 --port 20350 --nonblock --nofork --num 50000 > /dev/null
```

Then, we should compare txt files in two folders:

```bash
# check file quantity in each folder
ls server_txt/*.txt | wc -l
ls client_txt/*.txt | wc -l

# compare difference recursively in two folders
diff -r server_txt/ client_txt/
```

### Test Results

Here comes the test results:

- Local test(`self-my client` to `self-my server`) results:

|     Server      |     Client      |  num  | Lost | Diff |
| :-------------: | :-------------: | :---: | :--: | :--: |
|   fork/block    |   fork/block    | 50000 |  0   |  0   |
|   fork/block    |  fork/nonblock  | 50000 |  0   |  0   |
|   fork/block    | nofork/nonblock | 50000 |  0   |  0   |
|  fork/nonblock  |   fork/block    | 50000 |  0   |  0   |
|  fork/nonblock  |  fork/nonblock  | 50000 |  0   |  0   |
|  fork/nonblock  | nofork/nonblock | 50000 |  0   |  0   |
| nofork/nonblock |   fork/block    | 50000 |  0   |  0   |
| nofork/nonblock |  fork/nonblock  | 50000 |  0   |  0   |
| nofork/nonblock | nofork/nonblock | 50000 |  0   |  0   |

- Online test(`self-my client` to `self-SJ's server`) results:
  - server: fork/block; client: fork/nonblock: client prompts `loop_client_fork select: Invalid argument`, server no prompt. Then client/server lose 1 each, while no diff occurs.

|     Server      |     Client      |  num  | Lost | Diff |
| :-------------: | :-------------: | :---: | :--: | :--: |
|   fork/block    |   fork/block    | 50000 |  0   |  0   |
|   fork/block    |  fork/nonblock  | 50000 |  1   |  0   |
|   fork/block    | nofork/nonblock | 50000 |  0   |  0   |
|  fork/nonblock  |   fork/block    | 50000 |  0   |  0   |
|  fork/nonblock  |  fork/nonblock  | 50000 |  0   |  0   |
|  fork/nonblock  | nofork/nonblock | 50000 |  0   |  0   |
| nofork/nonblock |   fork/block    | 50000 |  0   |  0   |
| nofork/nonblock |  fork/nonblock  | 50000 |  0   |  0   |
| nofork/nonblock | nofork/nonblock | 50000 |  0   |  0   |

- Online test(`self-my client` to `other's-SJ's server`, vice versa) results:
  - pending.

### More about TDD

[http://www.drdobbs.com/cpp/test-driven-development-in-cc/184401572](http://www.drdobbs.com/cpp/test-driven-development-in-cc/184401572)

[http://alexott.net/en/cpp/CppTestingIntro.html](http://alexott.net/en/cpp/CppTestingIntro.html) -->
