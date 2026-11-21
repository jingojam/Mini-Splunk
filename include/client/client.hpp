#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <cstdlib>
#include <fstream>
#include <vector>

#include "../../include/parser/parser.hpp"

class Client{
    private:
        std::vector<std::string> logs;

    public:
        Client();
        
        void Start();

        int ConnectToServer(std::string& server_ip, uint16_t& port);

        void SendData(int fd, std::string& data);

        void StreamFile(int fd, std::string& filename);

        int ReceiveData();

        void CommandInterface();

        void Ingest(std::string& ip_address, uint16_t port, std::string& filename);

        void SearchDate(std::string& ip_address, uint16_t port, std::string& date);

        void SearchHost(std::string& ip_address, uint16_t port, std::string& hostname);

        void SearchDaemon(std::string& ip_address, uint16_t port, std::string& daemon);

        void SearchSeverity(std::string& ip_address, uint16_t port, std::string& severity);

        void SearchKeyword(std::string& ip_address, uint16_t port, std::string& keyword);

        void Purge(std::string& ip_address, uint16_t port);
};

#endif