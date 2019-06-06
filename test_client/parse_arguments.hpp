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

using Options = std::unordered_map<std::string, std::string>;

// Options parse_arguments(int argc, char **argv, bool is_client);
Options parse_arguments(std::ifstream &ifs, bool is_client);

#endif // PARSE_ARGUMENTS_H
