#ifndef PARSE_ARGUMENTS_H
#define PARSE_ARGUMENTS_H

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

const std::string kFnConfServer = "yzmond.conf";

const std::unordered_map<std::string, std::string> default_client_config {
    {"监听端口号", "41713"},
    {"设备连接间隔", "15"},
    {"设备采样间隔", "900"},
    {"服务器IP地址", "localhost"},
    {"服务器端口号", "3306"},
    {"数据库名", "yzmon_1551713"},
    {"用户名", "dbuser_1551713"},
    {"用户口令", "yzmond.1551713"},
    {"未应答超时", "30"},
    {"传输超时", "60"},
    {"主日志大小", "10240"},
    {"分日志大小", "1024"},
    {"屏幕显示", "1"},
    {"tmp_packet", "0000"},
    {"tmp_socket", "0000"},
    {"dev_packet", "0000"},
    {"dev_socket", "0000"}
};

using Options = std::unordered_map<std::string, std::string>;

Options parse_arguments(std::ifstream &ifs);

#endif // PARSE_ARGUMENTS_H
