#include "JA3_Config.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <algorithm>

Config::Config() 
{
    params_names.push_back("input_directory");
    params_names.push_back("output_file");
    params_names.push_back("output_dir");
    params_names.push_back("check_interval");
    params_names.push_back("max_number_of_output_files");
    params_names.push_back("max_output_file_size");

    parameters["input_directory"] = "./captured";
    parameters["output_file"] = "./output.csv";
    parameters["output_dir"] = "./output";
    parameters["check_interval"] = "15";
    parameters["max_number_of_output_files"] = "9";
    parameters["max_output_file_size"] = "500";
}

int Config::ReadConfig()
{
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
        const auto first_char_pos = line.find_first_not_of(" \t\n");
        if (line.at(first_char_pos) == '#')
            continue;
        for (const auto param_name : params_names){
            int pos = 0;
            std::string param_value;
            pos = line.find(param_name + "=");
            if (pos != std::string::npos){
                param_value = line.substr(pos + param_name.length() + 1);
                param_value.erase( std::remove(param_value.begin(), param_value.end(), '\r'), param_value.end() );
                parameters[param_name] = param_value;
                continue;
            }
        }
    }
    PrintParametrs();
    return 0;
}

std::string Config::GetParamByName(std::string name)
{
    if (parameters.find(name) != parameters.end())
        return parameters[name];
    return std::string();
}

void Config::PrintParametrs() const
{
    std::cout << "Found config:" << std::endl;
    for (const auto& [key, value] : parameters){
        std::cout << '[' << key << "] = " << value << "; " << std::endl;
    }
}
