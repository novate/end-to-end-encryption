#ifndef PARSE_ARGUMENTS_H
#define PARSE_ARGUMENTS_H

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

const std::string kFnConfClient = "ts.conf";
const std::string kFnConfServer = "yzmond.conf";

//struct Options {
    //int num_options = 4;

    //std::string ip; // dot seperated ip address
    //std::string port;
    //unsigned int num = 100; // number of connections. defaults to 100
    //bool block = false; // block/nonblock. defaults to nonblock
    //bool fork = false;  // fork/nofork. defaults to nonblock
//};
using Options = std::unordered_map<std::string, std::string>;

Options parse_arguments(int argc, char **argv, bool is_client);

#endif // PARSE_ARGUMENTS_H
