#pragma once

#include <string>

class Config {
public:
    Config() : pcap_files_dir("./captured"), output_file("./output.csv"), check_interval(60){};
    ~Config() = default;
    int ReadConfig();
    std::string GetPcapFilesDir() { return pcap_files_dir; }
    std::string GetOutputFileName() { return output_file; }
    int GetCheckInterval() { return check_interval; }
private:
    std::string pcap_files_dir;
    std::string output_file;
    int check_interval;
};