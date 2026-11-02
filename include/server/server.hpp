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
#include <fcntl.h>
#include <sys/epoll.h>

#include "../../src/parser/parser.cpp"

// just an arbitrary constant I made up
static const uint8_t MAX_CONNECTIONS = 32;

// arbitrary constant as per the linux epoll man page
static const uint8_t MAX_EVENTS = 10;

/**
 * Master Log Vector (Shared)
 */
static const std::vector<Log> MASTER;

/**
 * Master Log Index Mappings (Shared)
 */
static const std::unordered_map<std::string, std::vector<int>> DATE_INDEX;
static const std::unordered_map<std::string, std::vector<int>> HOST_INDEX;
static const std::unordered_map<std::string, std::vector<int>> DAEMON_INDEX;
static const std::unordered_map<std::string, std::vector<int>> SEVERITY_INDEX;
static const std::unordered_map<std::string, std::vector<int>> KEYWORD_INDEX;

class Server{
    private:
        int server_fd;
        int epoll_fd;
        std::string ip_address;
        uint16_t port;
        Parser parser;

        int RegisterServer();

        void EpollEvent();

    public:
        Server(std::string& ip_address, uint16_t& port);

        int SendData();

        int ReceiveData();

        void Start();
};

#endif