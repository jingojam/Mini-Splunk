#include "../../include/server/server.hpp"

Server::Server(std::string& ip_address, uint16_t& port){
    struct sockaddr_in addr;
    
    this->ip_address = ip_address;
    this->port = port;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // convert address string into binary representation
    if(inet_pton(AF_INET, ip_address.c_str(), &addr.sin_addr) != 1){
        std::cerr << "[ERROR] Cannot Convert Address " << ip_address << ":" << port << " To Network Bytes.\n";
        return;
    }

    // we need a socket for standard TCP/IPv4 transport
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if(fd == -1){
        std::cerr << "[ERROR] Cannot Create Socket.\n";
        return;
    }

    // bind the socket pair
    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        std::cerr << "[ERROR] Cannot Bind Server Socket.\n";
        return;
    }

    this->server_fd = fd;
}

/**
 * Registers server socket to epoll list
 */
int Server::RegisterServer(){
    // create an epoll instance
    int epoll_fd = epoll_create(1);

    if(epoll_fd < 0){
        return -2; // -2 signifies that epoll_ctl failed
    }

    this->epoll_fd = epoll_fd;

    // create event entry for server socket file descriptor as read op
    struct epoll_event event;
    event.data.fd = this->server_fd;
    event.events = EPOLLIN;

    // add entry to interest list
    int ctl_stat = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->server_fd, &event);

    return ctl_stat;
}   

int Server::EpollEvent(){
    
}

void Server::Start(){
    int l = listen(this->server_fd, MAX_CONNECTION_QUEUE);
    
    if(l == -1){
        std::cout << "[ERROR] Server Failed To Start.\n";
    } else{
        std::cout << "[STATUS] Server Started: Listening on " << this->ip_address << ":" << this->port << "\n";

        int ep = RegisterServer();

        if(ep < 0){
            std::cerr << "[ERROR] Failed to Register Server Socket to Epoll.\n";
            return;
        }

        // main listening loop
        while(1){
            // struct sockaddr addr;
            // struct socklen_t addrlen;

            // int conn_stat = accept(this->server_fd, &addr, &addrlen);
        }
    }
}