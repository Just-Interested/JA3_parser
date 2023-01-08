#pragma once

#include <string>
#include <map>

#include "PcapLiveDeviceList.h"
#include "PcapFileDevice.h"
#include "SystemUtils.h"
#include "PacketUtils.h"

class JA3_parser {
public:
    JA3_parser();
    ~JA3_parser();
    bool parseDir(std::string path = "");
    bool parseFile(std::string fileName);
    bool parsePacket(pcpp::RawPacket& rawPacket);
    void printResult() const;
    std::vector<std::string> getResults();
    void update();
    void setDirToParse(std::string path) { workDir = path; }
private:
    struct SSLFlowData
    {
        pcpp::IPAddress srcIP;
        pcpp::IPAddress dstIP;
        uint16_t srcPort;
        uint16_t dstPort;
        std::string serverName;
        std::string clientJA3;
        std::string serverJA3;
        timespec packetTime;
    };
    void updateResults();
    void parseFilesInQueue();
    void addNewFilesToQueue();
    std::map<uint32_t, SSLFlowData> flowTable;
    std::string workDir;
    std::vector<std::string> results;
    std::vector<std::string> parsedFiles;
    std::vector<std::string> filesToParseQueue;
};