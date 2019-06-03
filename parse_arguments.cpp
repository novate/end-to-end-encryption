#include <vector>
#include <iostream>
#include "parse_arguments.hpp"

void print_help(bool is_client) {
    std::cout << "Usage: " << (is_client ? "client" : "server") << " --ip x.x.x.x"
       << " --port xx" << " --block/--nonblock" << " --fork/--nonfork" 
       << (is_client ? " [--num 1~1000]" : "") << std::endl;
}

Options parse_arguments(int argc, char **argv, bool is_client=false) {
    std::vector<std::string> arguments;
    Options opts;
	
	if(is_client)
		opts.num_options = 5;

    //set default server ip 
    if(!is_client)
        opts.ip = "0.0.0.0";
		
    for (int i = 1; i < argc; ++i)
        arguments.push_back(argv[i]);

    int num_options = 0;
    for (auto i = arguments.begin(); i != arguments.end(); i++) {
        auto &&arg = *i;
        if (arg == "--ip") {
            opts.ip = *++i;
            num_options++;
            continue;
        } else if (arg == "--port") {
            opts.port = *++i;
            num_options++;
            continue;
        } else if (arg == "--num") {
            opts.num = stoi(*++i);
            num_options++;
            continue;
        } else if (arg == "--block") {
            opts.block = true;
            num_options++;
            continue;
        } else if (arg == "--nonblock") {
            opts.block = false;
            num_options++;
            continue;
        } else if (arg == "--fork") {
            opts.fork = true;
            num_options++;
            continue;
        } else if (arg == "--nofork") {
            opts.fork = false;
            num_options++;
            continue;
        } else if (arg == "-h" || arg == "--help") {
            break;
        } else {
            std::cerr << "Unknown argument " << arg << std::endl;
            print_help(is_client);
            exit(1);
            break;
        }
    }
    // --nofork & --block, block is invalid
    if(!opts.fork && opts.block)
        opts.block = false;

    if (num_options > opts.num_options || num_options < (is_client?2:1)) {
        std::cerr << "Wrong number of options. Seen " << num_options << " options\n";
        print_help(is_client);
        exit(1);
    }

    return opts;
}