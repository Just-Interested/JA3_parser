#include <thread>
#include "stdlib.h"
#include <unistd.h>
#include <getopt.h>
#include <filesystem>
#include <iostream>

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include "JA3_parser.h"
#include "JA3_Config.h"
#include "OutputWriter.h"


// Этот фрагмент добавлен для возможности статически скомпилировать libm.a
#define FEATURE_INDEX_MAX 1

enum {
    COMMON_CPUID_INDEX_1 = 0,
    COMMON_CPUID_INDEX_7,
    COMMON_CPUID_INDEX_80000001,        // for AMD
    // Keep the following line at the end.
    COMMON_CPUID_INDEX_MAX
};

struct cpu_features {
    enum cpu_features_kind  {
        arch_kind_unknown = 0,
        arch_kind_intel,
        arch_kind_amd,
        arch_kind_other
    } kind;
    int max_cpuid;
    struct cpuid_registers {
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
    } cpuid[COMMON_CPUID_INDEX_MAX];
    unsigned int family;
    unsigned int model;
    unsigned int feature[FEATURE_INDEX_MAX];
};


struct cpu_features _dl_x86_cpu_features;
// конец фрагмента ккачательно libm.a


//-----------------------------------------------
// Функция daemonize нужна, чтобы запускать процесс в режиме демона
/*
* This is an answer to the stackoverflow question:
* https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149
* Fork this code: https://github.com/pasce/daemon-skeleton-linux-c
* Read about it: https://nullraum.net/how-to-create-a-daemon-in-c/
*/
// static void daemonize()
// {
//     pid_t pid;
    
//     /* Fork off the parent process */
//     pid = fork();
    
//     /* An error occurred */
//     if (pid < 0)
//         exit(EXIT_FAILURE);
    
//      /* Success: Let the parent terminate */
//     if (pid > 0)
//         exit(EXIT_SUCCESS);
    
//     /* On success: The child process becomes session leader */
//     if (setsid() < 0)
//         exit(EXIT_FAILURE);
    
//     /* Catch, ignore and handle signals */
//     /*TODO: Implement a working signal handler */
//     signal(SIGCHLD, SIG_IGN);
//     signal(SIGHUP, SIG_IGN);
    
//     /* Fork off for the second time*/
//     pid = fork();
    
//     /* An error occurred */
//     if (pid < 0)
//         exit(EXIT_FAILURE);
    
//     /* Success: Let the parent terminate */
//     if (pid > 0)
//         exit(EXIT_SUCCESS);
    
//     /* Set new file permissions */
//     umask(0);
    
//     /* Change the working directory to the root directory */
//     /* or another appropriated directory */
//     chdir("/");
    
//     /* Close all open file descriptors */
//     int x;
//     for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
//     {
//         close (x);
//     }
    
//     /* Open the log file */
//     //openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
// }

//-----------------------------------------------

void update(JA3_parser &parser, Config &config){
    std::ofstream out;
    parser.setDirToParse(config.GetParamByName("input_directory"));
    std::string output_file_name = config.GetParamByName("output_file");
    if (!std::filesystem::exists(output_file_name)){
        out.open(output_file_name, std::ios::out);
        out << "timestamp,src_ip,src_port,client_JA3,dst_ip,dst_port,server_JA3,server_name" << std::endl;
        out.close();
    }
    std::vector<std::string> results;
    while(1){
        parser.update();
        out.open(output_file_name, std::ios::app);
        results = parser.getResults();
        for (const auto& res : results){
            out << res;
        }
        out.close();
        int check_interval = std::atoi(config.GetParamByName("check_interval").c_str());
        std::this_thread::sleep_for(std::chrono::seconds(check_interval));
    }
}


int parse(){
    Config cfg;
    cfg.ReadConfig();
    JA3_parser parser;
    std::thread th1(update, std::ref(parser), std::ref(cfg));
    th1.join();
    return 0;
}

void printUsage(){
    std::cout << "Program checks config file ./ja3_parser.conf or /etc/ja3_parser.conf\n"
            << "and parse specified directory.\n"
            << "Default behavior - run as daemon.\n"
            << "Options: \n"
            << "-h              prints this help\n"
            << "--daemon        run as daemon"
            << std::endl;
}

void printUsage_test(){
    std::cout << "Usage: ja3_parser [filename]\n"
            << "Parse file and write results into ./output.csv\n"
            << "Options: \n"
            << "-h              prints this help\n"
            << std::endl;
}

int running_daemon = 0;

int main(int argc, char** argv) {
    if (argc > 1){
        std::string arg(argv[1]);
        if (arg == "-h" || arg == "--help"){
            printUsage_test();
            return 0;
        }
        // if (arg == "--daemon")
        //     running_daemon = 1;
        // else {
        //     printUsage();
        //     return 0;
        // }
        JA3_parser parser;
        if (!parser.parseFile(arg)){
            std::vector<std::string> results;
            results = parser.getResults();
            OutputWriter writer;
            writer.write(results);
            return 0;
        }

    }
    printUsage_test();
    return 0;
//     if (running_daemon == 1)
//         daemonize();
//     parse();
}


