#pragma once

#include <string>
#include <map>
#include <vector>

class Config {
public:
    Config();
    ~Config() = default;
    int ReadConfig();
    std::string GetParamByName(std::string name);
    void PrintParametrs() const;
private:
    std::vector<std::string> params_names;
    std::map<std::string, std::string> parameters;
};