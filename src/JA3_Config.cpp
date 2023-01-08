#include "JA3_Config.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <algorithm>

int Config::ReadConfig(){
    std::string config_file_name = "/etc/ja3_parser.conf";
    if (!std::filesystem::exists(config_file_name)){
        config_file_name = "./ja3_parser.conf";
        if (!std::filesystem::exists(config_file_name)){
            std::cout << "Config file not found!" << std::endl;
            return 1;
        }
    }
    std::ifstream config_file(config_file_name);
    std::string line;
    while (std::getline(config_file, line)){
        if (line.at(0) == '#')
            continue;
        std::string input_dir_param_name = "input_directory";
        std::string output_file_param_name = "output_file";
        std::string check_interval_param_name = "check_interval";
        int pos = 0;
        pos = line.find(input_dir_param_name + "=");
        if (pos != std::string::npos){
            pcap_files_dir = line.substr(pos + input_dir_param_name.length() + 1);
            pcap_files_dir.erase( std::remove(pcap_files_dir.begin(), pcap_files_dir.end(), '\r'), pcap_files_dir.end() );
            std::cout << "Config parameter " << input_dir_param_name << " found: " << pcap_files_dir << std::endl;
            continue;
        }
        pos = 0;
        pos = line.find(output_file_param_name + "=");
        if (pos != std::string::npos){
            output_file = line.substr(pos + output_file_param_name.length() + 1);
            output_file.erase( std::remove(output_file.begin(), output_file.end(), '\r'), output_file.end() );
            std::cout << "Config parameter " << output_file_param_name << " found: " << output_file << std::endl;
            continue;
        }
        pos = 0;
        pos = line.find(check_interval_param_name + "=");
        if (pos != std::string::npos){
            std::string check_interval_str;
            check_interval_str = line.substr(pos + check_interval_param_name.length() + 1);
            check_interval_str.erase( std::remove(check_interval_str.begin(), check_interval_str.end(), '\r'), check_interval_str.end() );
            check_interval = std::stoi(check_interval_str);
            std::cout << "Config parameter " << check_interval_param_name << " found: " << check_interval << "s" << std::endl;
            continue;
        }
    }
    return 0;
}