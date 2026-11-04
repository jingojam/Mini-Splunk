#include "../../include/server/server.hpp"

Server::Server(std::string& ip_address, uint16_t& port){
    struct sockaddr_in addr;
    
    this->ip_address = ip_address;
    this->port = port;

    // UDP socket for RFC 3164
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

    if(fd == -1){
        std::cerr << "[ERROR] Cannot Create Socket.\n";
        return;
    }

    //reuse the address to avoid address problems
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // convert address string into binary representation
    if(inet_pton(AF_INET, ip_address.c_str(), &addr.sin_addr) != 1){
        std::cerr << "[ERROR] Cannot Convert Address " << ip_address << ":" << port << " To Network Bytes.\n";
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
    int epoll_fd = epoll_create1(0);

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

void Server::Manifold(std::string& command){
    struct Command c = IdentifyCommand(command);

    switch(c.type){
         case 0:
            //ingest
            break;
        case 1:
            //purge
            break;
        case 2:
            //search_date
            break;
        case 3:
            //search_host
            break;
        case 4:
            //search_daemon
            break;
        case 5:
            //search_severity
            break;
        case 6:
            //search_keyword
            break;
        case 7:
            //count keyword
            break;
        default:
            break;
    }
}

void Server::EpollEvent(){
    struct epoll_event events[MAX_EVENTS];
    int i, num_fds;

    // event loop
    while(1){
        num_fds = epoll_wait(this->epoll_fd, events, MAX_EVENTS, 0);

        for(i = 0; i < num_fds; i++){
            // event on the server socket (data incoming)
            if(events[i].data.fd == this->server_fd){
                struct sockaddr_in addr;
                socklen_t addrlen = sizeof(addr);
                char buf[MAX_DATA_SIZE];

                ssize_t data = recvfrom(this->server_fd, buf, MAX_DATA_SIZE, 0, (struct sockaddr*)&addr, &addrlen);
                std::string command(buf);

                Manifold(command);
            }
        }
    }
}

void Server::Start(){
    int l = listen(this->server_fd, MAX_CONNECTIONS);
    
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
        EpollEvent();
    }
}