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

const std::unordered_map<std::string, std::string> default_client_config {
    {"端口号", "41047"},
    {"最小配置终端数量", "5"},
    {"最大配置终端数量", "28"},
    {"每个终端最大虚屏数量", "10"}
};

using Options = std::unordered_map<std::string, std::string>;
void print_help();
// Options parse_arguments(int argc, char **argv, bool is_client);
Options parse_arguments(std::ifstream &ifs, bool is_client);

#endif // PARSE_ARGUMENTS_H
