#include <iostream>
#include <string>
#include <cstdint>
#include "../include/server/server.hpp"

int main(int argc, char* argv[]){
    if(argc < 3){
        std::cerr << "ERROR: Insufficient Arguments. Usage: ./server <IP> <PORT>\n";
    }

    std::string ip = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    Server server(ip, port);
    
    server.Start();

    return 0;
}