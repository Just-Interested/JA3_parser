#include "JA3_parser.h"

#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "IPLayer.h"
#include "TcpLayer.h"
#include "SSLLayer.h"
#include "SSLHandshake.h"

#define JA3_STATUS_SUCCESS 0

JA3_parser::JA3_parser(){
    workDir = "./captured";
    results.reserve(100000);
}

JA3_parser::~JA3_parser(){
    parsedFiles.clear();
    filesToParseQueue.clear();
    results.clear();
}

bool JA3_parser::parseDir(std::string path){
    if (path != "")
        workDir = path;
    // Получаем список файлов в директории
    if (!std::filesystem::exists(workDir)){
        std::cout << "Directory " << workDir << " doesn't exist!" << std::endl;
        return 1;
    }
    std::vector<std::string> file_names;
    for (const auto & entry : std::filesystem::directory_iterator(workDir))
        if (entry.path().extension() == ".pcap" || entry.path().extension() == ".pcapng")
            file_names.push_back(entry.path());
    
    // Пробуем прочитать каждый файл
    pcpp::IFileReaderDevice* reader;
    for (const auto & file : file_names){
        if (parseFile(file))
            std::cout << "Error while parsing file " << file << "!" << std::endl;
    }
    return 0;
}


bool JA3_parser::parseFile(std::string fileName){
    const auto start_time = std::chrono::steady_clock::now();
    pcpp::IFileReaderDevice* reader = pcpp::IFileReaderDevice::getReader(fileName);
    if (reader == NULL) {
        std::cerr << "Cannot determine reader for file type" << std::endl;
        return 1;
    }

    if (!reader->open()) {
        std::cerr << "Cannot open .pcap for reading" << std::endl;
        return 1;
    }

    pcpp::RawPacket rawPacket;
    while (reader->getNextPacket(rawPacket)) {
        parsePacket(rawPacket);
    }
    reader->close();
    updateResults();
    flowTable.clear();
    const auto end_time = std::chrono::steady_clock::now();
    const auto dur = end_time - start_time;
    std::cout << "File " << fileName
                  << " parsed in: " << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()
                  << " ms" << std::endl;
    return 0;
}


bool JA3_parser::parsePacket(pcpp::RawPacket& rawPacket){
    pcpp::Packet parsedPacket(&rawPacket);
    if (parsedPacket.isPacketOfType(pcpp::SSL)){
        pcpp::SSLHandshakeLayer* sslHandshakeLayer = parsedPacket.getLayerOfType<pcpp::SSLHandshakeLayer>();
        if (sslHandshakeLayer != NULL){
            // Вычисляем хэш для каждого соединения (считается от ипов, портов, и хз))))
            uint32_t hashVal = pcpp::hash5Tuple(&parsedPacket);
            if (flowTable.find(hashVal) == flowTable.end()){
                pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
                if (tcpLayer != NULL){
                    flowTable[hashVal].srcPort = tcpLayer->getSrcPort();
                    flowTable[hashVal].dstPort = tcpLayer->getDstPort();
                }
                pcpp::IPLayer* ipLayer = parsedPacket.getLayerOfType<pcpp::IPLayer>();
                if (ipLayer != NULL){
                    flowTable[hashVal].srcIP = ipLayer->getSrcIPAddress();
                    flowTable[hashVal].dstIP = ipLayer->getDstIPAddress();
                }
            }

            // Получаем сообщение Client Hello и извлекаем JA3 и имя сервера
            pcpp::SSLClientHelloMessage* clientHelloMessage = sslHandshakeLayer->getHandshakeMessageOfType<pcpp::SSLClientHelloMessage>();
            if (clientHelloMessage != NULL){
                pcpp::SSLClientHelloMessage::ClientHelloTLSFingerprint tlsFingerprint = clientHelloMessage->generateTLSFingerprint();
                std::pair<std::string, std::string> tlsFingerprintStringAndMD5 = tlsFingerprint.toStringAndMD5();
                flowTable[hashVal].clientJA3 = tlsFingerprintStringAndMD5.second;
                
                pcpp::SSLServerNameIndicationExtension* sniExt = clientHelloMessage->getExtensionOfType<pcpp::SSLServerNameIndicationExtension>();
                if (sniExt != NULL){
                    flowTable[hashVal].serverName = sniExt->getHostName();
                }
                flowTable[hashVal].packetTime = rawPacket.getPacketTimeStamp();
            }

            // Получаем сообщение ServerHello и извлекаем JA3
            pcpp::SSLServerHelloMessage* servertHelloMessage = sslHandshakeLayer->getHandshakeMessageOfType<pcpp::SSLServerHelloMessage>();
            if (servertHelloMessage != NULL){
                pcpp::SSLServerHelloMessage::ServerHelloTLSFingerprint tlsFingerprint = servertHelloMessage->generateTLSFingerprint();
                std::pair<std::string, std::string> tlsFingerprintStringAndMD5 = tlsFingerprint.toStringAndMD5();
                flowTable[hashVal].serverJA3 = tlsFingerprintStringAndMD5.second;
            }
        }
        
    }
    else {
        return 1;
    }
    return 0;
}


void JA3_parser::printResult() const{
    for (const auto& res : results){
        std::cout << res << std::endl;
    }
}

void JA3_parser::updateResults(){
    std::stringstream buffer;
    SSLFlowData flow_data;
    for (const auto& record : flowTable){
        flow_data = record.second;
        buffer << flow_data.packetTime.tv_sec << "," 
            << flow_data.srcIP.toString() << ","
            << std::dec << flow_data.srcPort << ","
            << flow_data.clientJA3 << ","
            << flow_data.dstIP.toString() << ","
            << std::dec << flow_data.dstPort << ","
            << flow_data.serverJA3 << ","
            << flow_data.serverName << std::endl;
        results.push_back(buffer.str());
        buffer.str(std::string());
    }
}

std::vector<std::string> JA3_parser::getResults(){
    return std::move(results);
}

void JA3_parser::parseFilesInQueue(){
    for (const auto& file : filesToParseQueue){
        if (parseFile(file) == JA3_STATUS_SUCCESS)
            parsedFiles.push_back(file);
    }
    filesToParseQueue.clear();
}

void JA3_parser::addNewFilesToQueue(){
    if (std::filesystem::exists(workDir)){
        std::vector<std::string> file_names;
        for (const auto& entry : std::filesystem::directory_iterator(workDir)){
            if (entry.path().extension() == ".pcap" || entry.path().extension() == ".pcapng")
                file_names.push_back(entry.path());
        }
        for (const auto& file : file_names){
            if (std::find(parsedFiles.begin(), parsedFiles.end(), file) == parsedFiles.end())
                filesToParseQueue.push_back(file);
        }

        // нужно подчищать список обработаных файлов, удаляя файлы, которых уже нет в каталоге
        for (const auto& file : parsedFiles){
            const auto & existing_file = std::find(file_names.begin(), file_names.end(), file);
            if (existing_file == file_names.end())
                parsedFiles.erase(std::remove(parsedFiles.begin(), parsedFiles.end(), file), parsedFiles.end());
        } 
    }
    else {
        std::cout << "Directory " << workDir << " doesn't exist!" << std::endl;
    }
}

void JA3_parser::update(){
    addNewFilesToQueue();
    parseFilesInQueue();
}