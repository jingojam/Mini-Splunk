#include "../../include/server/server.hpp"

Server::Server(std::string& ip_address, uint16_t& port){
    struct sockaddr_in addr;
    
    this->ip_address = ip_address;
    this->port = port;

    // we need a socket for standard TCP/IPv4 transport
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

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

void Server::EpollEvent(){
    struct epoll_event events[MAX_EVENTS];
    int i;

    // event loop
    while(1){
        int num_fds = epoll_wait(this->epoll_fd, events, MAX_EVENTS, 0);

        for(i = 0; i < num_fds; i++){
            // get the file desc of the events (linearly)
            int fd = events[i].data.fd;

            // event on the server socket
            if(fd == this->server_fd){
                //all pending connections
                while(1){
                    struct sockaddr_in addr;
                    socklen_t addrlen = sizeof(addr);
                    struct epoll_event event;

                    // get the client socket file desc from connection
                    int client_fd = accept4(this->server_fd, (struct sockaddr*)&addr, &addrlen, SOCK_NONBLOCK);

                    event.data.fd = client_fd;
                    event.events = EPOLLIN;

                    if(client_fd < 0){
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            break;
                        }
                        break;
                    }

                    // add client file desc to epoll interest list
                    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0){
                        std::cerr << "[ERROR] Failed To Add Client Socket File Descriptor to Interest List.\n";
                    } else{
                        std::cout << "[STATUS] Client Connected\n";
                    }
                }
            // event is from an existing client socket file desc in the interest list 
            } else{
                
                // client socket is ready for input
                if(events[i].events & EPOLLIN){

                } else if(events[i].events & EPOLLOUT){ // for output

                }
            }
        }
    }
}

int Server::SendData(int fd, std::string data){
    uint32_t prefix = htonl(data.length());

    std::string msg = std::to_string(prefix) + " " + data;

    ssize_t bytes_sent = send(fd, msg.c_str(), msg.length(), 0);

    return bytes_sent;
}

// int Server::ReceiveData(){
    
// }

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