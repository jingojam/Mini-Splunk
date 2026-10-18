#include "../../include/server/server.cpp"

Server::Server(std::string& ip_address, uint16_t& port){
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // convert address string into binary representation
    if(inet_pton(AF_INET, ip_address.c_str(), &addr.sin_addr) != 1){
        std::cerr << "ERROR: Cannot Convert Address " << ip_address << ":" << port << " To Network Bytes.\n";
        return;
    }

    // we need a socket for standard TCP/IPv4 transport
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if(fd == -1){
        std::cerr << "ERROR: Cannot Create Socket.\n";
        return;
    }

    if(bind(fd, (struct sockaddr*)addr, sizeof(addr)) == -1){
        std::cerr << "ERROR: Cannot Bind Server Socket.\n";
        return;
    }

    this->server_fd = fd;
}