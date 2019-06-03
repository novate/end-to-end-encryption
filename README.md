# SJ Group Work 3: TCP Socket Integrated Task

## Makefile Logic

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
make && ./client --ip localhost --port 4000 --block --fork --num 1000 > /dev/null
make && ./client --ip localhost --port 4000 --nonblock --fork --num 1000 > /dev/null
make && ./client --ip localhost --port 4000 --nonblock --nofork --num 1000 > /dev/null

# online tests: 9 tests, with server side ip 10.60.102.252(you can modify it)
# server side
make && ./server --ip 0.0.0.0 --port 20350 --block --fork > /dev/null
make && ./server --ip 0.0.0.0 --port 20350 --nonblock --fork > /dev/null
make && ./server --ip 0.0.0.0 --port 20350 --nonblock --nofork > /dev/null
# client side
make && ./client --ip 10.60.102.252 --port 20350 --block --fork --num 1000 > /dev/null
make && ./client --ip 10.60.102.252 --port 20350 --nonblock --fork --num 1000 > /dev/null
make && ./client --ip 10.60.102.252 --port 20350 --nonblock --nofork --num 1000 > /dev/null
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

* Local test(`self-my client` to `self-my server`) results:

| Server | Client | num | Lost | Diff |
|:------:|:------:|:------:|:------:|:------:|
|fork/block| fork/block | 1000 | 0 | 0 |
|fork/block| fork/nonblock | 1000 | 0 | 0 |
|fork/block| nofork/nonblock | 1000 | 0 | 0 |
|fork/nonblock| fork/block | 1000 | 0 | 0 |
|fork/nonblock| fork/nonblock | 1000 | 0 | 0 |
|fork/nonblock| nofork/nonblock | 1000 | 0 | 0 |
|nofork/nonblock| fork/block | 1000 | 0 | 0 |
|nofork/nonblock| fork/nonblock | 1000 | 0 | 0 |
|nofork/nonblock| nofork/nonblock | 1000 | 0 | 0 |

* Online test(`self-my client` to `self-SJ's server`) results:
  * server: fork/block; client: fork/nonblock: client prompts `loop_client_fork select: Invalid argument`, server no prompt. Then client/server lose 1 each, while no diff occurs.

| Server | Client | num | Lost | Diff |
|:------:|:------:|:------:|:------:|:------:|
|fork/block| fork/block | 1000 | 0 | 0 |
|fork/block| fork/nonblock | 1000 | 1 | 0 |
|fork/block| nofork/nonblock | 1000 | 0 | 0 |
|fork/nonblock| fork/block | 1000 | 0 | 0 |
|fork/nonblock| fork/nonblock | 1000 | 0 | 0 |
|fork/nonblock| nofork/nonblock | 1000 | 0 | 0 |
|nofork/nonblock| fork/block | 1000 | 0 | 0 |
|nofork/nonblock| fork/nonblock | 1000 | 0 | 0 |
|nofork/nonblock| nofork/nonblock | 1000 | 0 | 0 |

* Online test(`self-my client` to `other's-SJ's server`, vice versa) results:
  * pending.

### More about TDD

[http://www.drdobbs.com/cpp/test-driven-development-in-cc/184401572](http://www.drdobbs.com/cpp/test-driven-development-in-cc/184401572)

[http://alexott.net/en/cpp/CppTestingIntro.html](http://alexott.net/en/cpp/CppTestingIntro.html)
