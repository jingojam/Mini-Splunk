#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstring>
#include <thread>
#include <shared_mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "../../src/parser/parser.cpp"

// just an arbitrary constant I made up
static const uint8_t MAX_DATA_SIZE = 65535;

// arbitrary constant as per the linux epoll man page
static const uint8_t MAX_EVENTS = 10;

/**
 * Master Log Vector (Shared)
 */
static const std::vector<Log> MASTER;

/**
 * Master Log Index Mappings (Shared Resource)
 */
static const std::unordered_map<std::string, std::vector<int>> DATE_INDEX;
static const std::unordered_map<std::string, std::vector<int>> HOST_INDEX;
static const std::unordered_map<std::string, std::vector<int>> DAEMON_INDEX;
static const std::unordered_map<std::string, std::vector<int>> SEVERITY_INDEX;
static const std::unordered_map<std::string, std::vector<int>> KEYWORD_INDEX;

struct Client{
    int fd;
    std::string buf;
};

class Server{
    private:
        int server_fd;
        int epoll_fd;
        std::string ip_address;
        uint16_t port;
        Parser parser;

        int RegisterServer();

        void EpollEvent();

        void Manifold(std::string command);

    public:
        Server(std::string& ip_address, uint16_t& port);

        bool Ingest(std::string& argument);

        bool Purge();

        std::vector<std::string> SearchDate(std::string& argument);

        std::vector<std::string> SearchHost(std::string& argument);

        std::vector<std::string> SearchDaemon(std::string& argument);

        std::vector<std::string> SearchSeverity(std::string& argument);

        std::vector<std::string> SearchKeyword(std::string& argument);

        std::vector<std::string> CountKeyword(std::string& argument);

        void Start();
};

#endif