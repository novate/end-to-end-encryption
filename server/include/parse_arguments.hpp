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
    {"�����˿ں�", "41713"},
    {"�豸���Ӽ��", "15"},
    {"�豸�������", "900"},
    {"������IP��ַ", "localhost"},
    {"�������˿ں�", "3306"},
    {"���ݿ���", "yzmon_1551713"},
    {"�û���", "dbuser_1551713"},
    {"�û�����", "yzmond.1551713"},
    {"δӦ��ʱ", "30"},
    {"���䳬ʱ", "60"},
    {"����־��С", "10240"},
    {"����־��С", "1024"},
    {"��Ļ��ʾ", "1"},
    {"tmp_packet", "0000"},
    {"tmp_socket", "0000"},
    {"dev_packet", "0000"},
    {"dev_socket", "0000"}
};

using Options = std::unordered_map<std::string, std::string>;

Options parse_arguments(std::ifstream &ifs);

#endif // PARSE_ARGUMENTS_H
