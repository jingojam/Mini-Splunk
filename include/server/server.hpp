#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "../../include/parser/parser.hpp"
#include "../../include/buffer/ring_buffer.hpp"

// just an arbitrary constants I made up
static const uint16_t MAX_DATA_SIZE = 1024;

// arbitrary constant as per the linux epoll man page
static const uint8_t MAX_EVENTS = 10;

/**
 * Main data log storage
 */
static std::vector<std::string> MASTER;
static std::array<std::unordered_map<std::string, std::vector<uint8_t>>, 6> INDEX{};

static constexpr uint8_t DATE_INDEX = 0;
static constexpr uint8_t TIMESTAMP_INDEX = 1;
static constexpr uint8_t HOST_INDEX = 2;
static constexpr uint8_t DAEMON_INDEX = 3;
static constexpr uint8_t SEVERITY_INDEX = 4;
static constexpr uint8_t KEYWORD_INDEX = 5;

static const std::unordered_map<std::string, uint8_t> COMMAND_INDEX_MAP = {
    {"SEARCH_DATE", DATE_INDEX},
    {"SEARCH_TIMESTAMP", TIMESTAMP_INDEX},
    {"SEARCH_HOST", HOST_INDEX},
    {"SEARCH_DAEMON", DAEMON_INDEX},
    {"SEARCH_SEVERITY", SEVERITY_INDEX},
    {"SEARCH_KEYWORD", KEYWORD_INDEX},
};

/**
 * Shared mutual exclusion (read/write lock)
 */
static std::shared_mutex mtx;

/**
 * Client sturcture 
 *  fd - client socket file descriptor
 *  inbound_buffer - buffer for data received FROM a client
 *  outbound_buffer - buffer for data to be sent TO a client
 */
struct Client{
    // file descriptor
    int fd;

    // data buffers
    std::string temp_buffer;
    ring_buffer inbound_buffer;
    ring_buffer outbound_buffer;
};

class Server{
    private:
        int server_fd;
        int epoll_fd;
        std::string ip_address;
        uint16_t port;
        std::unordered_map<int, struct Client> clients;

        /**
         * Epoll functions
         */
        int Register(int fd);

        int Deregister(int fd);

        int SetToWriteable(int fd);

        int RevertToDefault(int fd);

        void EpollEvent();

        /**
         * Network functions
         */
        void SendData(int fd);

        void ReceiveData(int fd);

        /**
         * Backend functions
         */
        void Dispatch(int fd);

        void ProcessData(int fd, ring_buffer& inbound_buffer, ring_buffer& outbound_buffer);

        void Ingest(ring_buffer& buffer);

        void Purge();

        void Query(ring_buffer& buffer, uint8_t index, std::string query);

    public:
        Server(std::string& ip_address, uint16_t& port);

        void Start();
};

#endif