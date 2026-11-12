#include "../../include/server/server.hpp"

/**
 *****************************
 * Server network functions
 *****************************
 */

 /**
  * Parameterized constructor
  *     - creates and binds server socket, and creates
  *       epoll instance
  */
Server::Server(std::string& ip_address, uint16_t& port){
    struct sockaddr_in addr;
    
    this->ip_address = ip_address;
    this->port = port;

    // TCP socket
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

    // create an epoll instance
    int epoll_fd = epoll_create1(0);

    if(epoll_fd < 0){
        std::cerr << "[ERROR] Cannot Create Epoll Instance.\n";
        return;
    }

    this->epoll_fd = epoll_fd;
}

int Server::SetToWriteable(int fd){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLOUT; //add epollout for triggering writes

    // modify fd in the interest list
    int ctl_stat = epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, fd, &event);

    return ctl_stat;
}

int Server::RevertToDefault(int fd){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;

    // modify fd in the interest list
    int ctl_stat = epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, fd, &event);

    return ctl_stat;
}

int Server::Deregister(int fd){
    this->clients.erase(fd);

    //remove fd from interest list
    int ctl_stat = epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL);

    return ctl_stat;
}

/**
 * Register function
 *      - registers a socket file descriptor to epoll list
 */
int Server::Register(int fd){
    // create event entry for server socket file descriptor as read op
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;

    // add entry to interest list
    int ctl_stat = epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &event);

    return ctl_stat;
}   

/**
 * Creates a worker thread
 */
void Server::Dispatch(int fd){
    auto it = this->clients.find(fd);

    if(it == this->clients.end()){
        return;
    }

    std::thread process_thread(&Server::ProcessData, this, it->second.inbound_buffer);

    process_thread.detach(); // run the thread in the background
}

/**
 * Main worker thread function
 */
void Server::ProcessData(std::vector<std::string>& buffer){
    std::string message;
    std::string command;
    std::string data;
    uint8_t index;

    while(!buffer.is_empty()){
        message = buffer.get_back();
        buffer.pop_back();

        bool ingest_command = message.starts_with("INGEST");
        bool purge_command = message.starts_with("PURGE");
        bool search_date_command = message.starts_with("SEARCH_DATE");
        bool search_timestamp_command = message.starts_with("SEARCH_TIMESTAMP");
        bool search_host_command = message.starts_with("SEARCH_HOST");
        bool search_daemon_command = message.starts_with("SEARCH_DAEMON");
        bool search_severity_command = message.starts_with("SEARCH_SEVERITY");
        bool search_keyword_command = message.starts_with("SEARCH_KEYWORD");
        bool log_message = message.starts_with("LOG");

        // if message is a command (any of the search functions/ingest/purge routines)
        if(ingest_command){
            Ingest(buffer);
        } else if(purge_command){
            Purge();
        } else if(search_date_command || search_timestamp_command || 
                  search_host_command || search_daemon_command ||
                  search_severity_command || search_keyword_command){

            size_t space_pos = command.find(' ');

            if(space_pos != std::string::npos){
                command = message.substr(space_pos);
                data = message.substr(space_pos + 1);
            }

            auto it = COMMAND_INDEX_MAP.find(command);
            
            if(it != COMMAND_INDEX_MAP.end()){
                index = it->second;
            }

            if(SetToWriteable(fd) == 0){
                Query(buffer, data);
            } else{
                std::cerr << "[ERROR] Cannot Modify Socket File Descriptor Event\n";
            }
        }
    }
}


/**
 * Ingest (Write operation)
 */
void Server::Ingest(std::vector<std::string>& buffer){
    // acquire write lock
    std::unique_lock<std::shared_mutex> lock(mtx);
    std::string data;
    struct Log log;

    int index = 0;
    
    while(!buffer.is_empty()){
        data = buffer.back();
        log = ParseLog(data);
        
        MASTER.push_back(data);
        INDEX[DATE_INDEX][log.date].push_back(index);
        INDEX[TIMESTAMP_INDEX][log.timestamp].push_back(index);
        INDEX[HOST_INDEX][log.host].push_back(index);
        INDEX[DAEMON_INDEX][log.daemon].push_back(index);
        INDEX[SEVERITY_INDEX][log.severity].push_back(index);

        for(std::string& keyword : Tokenize(client.message)){
            INDEX[KEYWORD_INDEX][keyword].push_back(index);
        }

        buffer.pop_back();

        index += 1;
    }
}

/**
 * Purge (Write operation)
 */
void Server::Purge(){
    // acquire write lock
    std::unique_lock<std::shared_mutex> lock(mtx);
    // critical section
    std::unordered_map<std::string, std::vector<size_t>> empty_date_index;
    std::unordered_map<std::string, std::vector<size_t>> empty_timestamp_index;
    std::unordered_map<std::string, std::vector<size_t>> empty_host_index;
    std::unordered_map<std::string, std::vector<size_t>> empty_daemon_index;
    std::unordered_map<std::string, std::vector<size_t>> empty_severity_index;
    std::unordered_map<std::string, std::vector<size_t>> empty_keyword_index;

    // clear the master log list and deallocate memory
    MASTER.clear();
    MASTER.shrink_to_fit();

    //swap the existing indexes with an empty one to free memory
    INDEX[DATE_INDEX].swap(empty_date_index);   
    INDEX[TIMESTAMP_INDEX].swap(empty_timestamp_index);
    INDEX[HOST_INDEX].swap(empty_host_index);
    INDEX[DAEMON_INDEX].swap(empty_daemon_index);
    INDEX[SEVERITY_INDEX].swap(empty_severity_index);
    INDEX[KEYWORD_INDEX].swap(empty_keyword_index);
}

/**
 * Query (Read operation)
 */
void Server::Query(uint8_t index, std::string& msg){
    // acquire read lock
    std::shared_lock<std::shared_mutex> lock(mtx);
    // critical section
    auto it = INDEX[index].find(msg);

    if(it != INDEX[index].end()){
        int i;

        for(i = 0; i < it->second.size(); i++){

        }
    }
}

void Server::EpollEvent(){
    struct epoll_event events[MAX_EVENTS];
    int i, num_fds;
    int num_workers = 0;
    int fd;

    // event loop
    while(true){
        num_fds = epoll_wait(this->epoll_fd, events, MAX_EVENTS, 0);

        for(i = 0; i < num_fds; i++){
            fd = events[i].data.fd;

            // event on the server socket
            if(fd == this->server_fd){
                struct sockaddr_in addr;
                socklen_t addrlen = sizeof(addr);

                int clfd = accept4(this->server_fd, (struct sockaddr*)&addr, &addrlen, SOCK_NONBLOCK);

                //add the fd to interest list (client soccket fd)
                int fdstat = Register(clfd);

                if(fdstat < 0){
                    std::cerr << "[ERROR] Failed to Register Client Socket File Descriptor to Epoll Interest List.\n";
                    continue;
                }
                
                struct Client client;
                client.fd = clfd;
                client.stream = false;
                
                this->clients.emplace(clfd, client);

            } else{
                // event is form another fd (a client)

                //client has data available to be read
                if(fd & EPOLLIN){ 
                    Dispatch(fd);

                    ReceiveData(fd);
                } 

                 // client fd ready to be written to
                if(fd & EPOLLOUT){
                    SendData(fd);
                }
            }
        }
    }
}

void Server::Start(){
    int l = listen(this->server_fd, 0);
    
    if(l == -1){
        std::cout << "[ERROR] Server Failed To Start.\n";
    } else{
        std::cout << "[STATUS] Server Started: Listening on " << this->ip_address << ":" << this->port << "\n";

        int svstat = Register(this->server_fd);

        if(svstat < 0){
            std::cerr << "[ERROR] Failed to Register Server Socket to Epoll.\n";
            return;
        }

        // main listening loop
        EpollEvent();
    }
}

void Server::SendData(int fd){
    auto it = this->clients.find(fd);
    std::string data;
    size_t len;
    size_t bytes_to_send;
    size_t total_bytes_to_send;
    ssize_t bytes_sent;

    while(!it->second.outbound_buffer.is_empty()){
        data = it->second.outbound_buffer.get_back();
        len = Prefix(data);
        bytes_to_send = len;
        total_bytes_sent = 0;
        const char* msg = data.c_str();

        //send bytes to the fd until message is fully sent
        while(total_bytes_sent < len){
            bytes_sent = send(client.fd, msg + total_bytes_sent, bytes_to_send, 0);

            //some error happened
            if(bytes_sent < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    continue;
                }
                break;
            }

            bytes_to_send -= bytes_sent;
            total_bytes_sent += bytes_sent;
        }

        it->second.outbound_buffer.pop_back();
    }
}

void Server::ReceiveData(int fd){
    auto it = this->clients.find(fd);
    std::string temp_buffer;
    ssize_t bytes_received;
    ssize_t current_buffer_size;

    while(true){
        current_buffer_size = temp_buffer.size(); // serves as the data pointer

        // allocate more memory to the buffer
        temp_buffer.resize(current_buffer_size + MAX_DATA_SIZE);

        bytes_received = recv(fd, temp_buffer.data() + current_buffer_size, MAX_DATA_SIZE, MSG_DONTWAIT);

        if(bytes_received < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            return;
        }

        if(bytes_received == 0){
            return;
        }

        while(ValidMessage(temp_buffer)){
            // add every complete message to the client's inbound buffer
            if(!it->second.inbound_buffer.is_full()){
                it->second.inbound_buffer.push_back(temp_buffer);
            }
        }
    }
}