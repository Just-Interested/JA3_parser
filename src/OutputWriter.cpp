#include "OutputWriter.h"

#include <filesystem>
#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>
#include <regex>

std::string OutputWriter::GetOutputFilePath()
{
    std::string file_format = ".csv";

    if (!std::filesystem::exists(output_dir)){
        if (!std::filesystem::create_directory(output_dir)){
            std::cout << "Couldn't create output directory!" << std::endl;
            return std::string();
        }
    }

    std::vector<std::string> file_paths;
    for (const auto& dir_entry : std::filesystem::directory_iterator(output_dir)){
        std::regex re("output_[0-9]{8}_[0-9]{6}");
        std::smatch path_match;
        std::string c_path = dir_entry.path().generic_string();
        if(std::regex_search(c_path, path_match, re)){
            if(path_match.suffix().str() != file_format){
                continue;
            }
            file_paths.push_back(c_path);
        }
    }

    if (!file_paths.empty()){
        // after sort, the oldest file will be the first, and last used file will be the last
        std::sort(file_paths.begin(), file_paths.end());

        std::string last_used_file = file_paths.back();
        std::error_code ec;
        std::uintmax_t file_size = std::filesystem::file_size(last_used_file, ec);
        if (ec){
            std::cout << last_used_file << " : " << ec.message() << std::endl;
        }
        else {
            if (file_size < max_file_size || max_file_size == 0){
                return last_used_file;
            }
        }

        // remove the oldest file, if limit reached
        if (file_paths.size() >= max_n_files) {
            if (std::remove(file_paths.front().c_str())){
                std::cout << "Couldn't remove file: " << file_paths.front() << std::endl;
                return std::string();
            }
        }
    }

    std::string new_file_name = "output_" + GetCurrentDateTime() + ".csv";

    std::string new_file_path = output_dir;
    new_file_path += std::filesystem::path::preferred_separator;
    new_file_path += new_file_name;

    return new_file_path;
}

bool OutputWriter::open()
{
    if (output.is_open())
        return true;
    
    std::string header_string = "timestamp,src_ip,src_port,client_JA3,dst_ip,dst_port,server_JA3,server_name";

    std::string current_filename = GetOutputFilePath();
    // BUG: если файл создается в ту же секунду, то данные запишутся в старый
    if (current_filename != std::string()){
        std::cout << "Output filename: " << current_filename << std::endl;
        output.open(current_filename, std::ios::app);
        if (!output)
            return false;
        if (output.tellp() == 0){
            output << header_string << std::endl;
        }
        return true;
    }
    return false;
}

std::string OutputWriter::NormalizePath(const std::string &messyPath)
{
    std::filesystem::path path(messyPath);
    std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
    std::string npath = canonicalPath.make_preferred().string();
    return npath;
}

std::string OutputWriter::GetCurrentDateTime()
{
    std::time_t t = std::time(nullptr);
    std::tm* now_utc = std::gmtime(&t);
    std::stringstream ss;
    ss << std::put_time(now_utc, "%Y%m%d_%H%M%S");
    return ss.str();
}

OutputWriter::OutputWriter() : max_n_files(0), max_file_size(0)
{
    output_dir = NormalizePath("./");
}

OutputWriter::OutputWriter(std::string _output_dir) : max_n_files(0), max_file_size(0)
{
    output_dir = NormalizePath(_output_dir);
}

OutputWriter::~OutputWriter()
{
}

bool OutputWriter::write(std::string data)
{
    if (open()){
        output << data;
        if (max_file_size == 0)
            return true;
        std::uint64_t pos = std::abs(output.tellp());
        if (pos >= max_file_size){
            output.close();
        }
        return true;
    }
    return false;
}

bool OutputWriter::write(std::vector<std::string> data)
{
    if (open()){
        for (const auto& line : data){
            output << line;
        }
        if (max_file_size == 0)
            return true;
        std::uint64_t pos = std::abs(output.tellp());
        if (pos >= max_file_size){
            output.close();
        }
        return true;
    }
    return false;
}
