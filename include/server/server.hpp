#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../src/parser/parser.cpp"

/**
 * Master Log Vector (Shared)
 */
std::vector<Log> MASTER;

/**
 * Master Log Index Mappings (Shared)
 */
std::unordered_map<std::string, std::vector<int>> DATE_INDEX;
std::unordered_map<std::string, std::vector<int>> HOST_INDEX;
std::unordered_map<std::string, std::vector<int>> DAEMON_INDEX;
std::unordered_map<std::string, std::vector<int>> SEVERITY_INDEX;
std::unordered_map<std::string, std::vector<int>> KEYWORD_INDEX;

class Server{
    private:
        int server_fd;
        Parser parser;

    public:
        Server(std::string& ip_address, uint16_t& port);

        
};

#endif