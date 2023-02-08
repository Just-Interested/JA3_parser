#pragma once

#include <string>
#include <fstream>
#include <vector>

class OutputWriter
{
private:
    std::uint32_t max_n_files;
    std::uintmax_t max_file_size;
    std::string output_dir;
    std::ofstream output;

    std::string GetOutputFilePath();
    bool open();

    // helper functions
    std::string NormalizePath(const std::string& messyPath);
    std::string GetCurrentDateTime();
    
public:
    OutputWriter();
    OutputWriter(std::string _output_dir);
    ~OutputWriter();

    bool write(std::string data);
    bool write(std::vector<std::string> data);
};


