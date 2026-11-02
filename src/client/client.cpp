#include "../../include/client/client.hpp"

Client::Client(){}

int Client::ConnectToServer(std::string& server_ip, uint16_t& port){
    struct sockaddr_in addr;
    // socket file desc for the client
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd == -1){
        std::cerr << "[ERROR] Cannot Create Socket.\n";
        return -1;
    }
    
    if(inet_pton(AF_INET, server_ip.c_str(), &addr.sin_addr) != 1){
        std::cerr << "[ERROR] Cannot Convert Address " << server_ip << ":" << port << " To Network Bytes.\n";
        return -2;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return -3;
    }

    return fd;
}

ssize_t Client::SendData(int fd, std::string& data){
    uint32_t prefix = htonl(data.length());

    std::string msg = std::to_string(prefix) + " " + data;

    ssize_t bytes_sent = send(fd, msg.c_str(), msg.length(), 0);

    return bytes_sent;
}

int Client::ReceiveData(){
    return -1;
}

void Client::CommandInterface(){
    std::cout << "Commands:\n";
    std::cout << "\t[+] INGEST <Filepath> <Server IP Address>:<Server Port>\n";
    std::cout << "\t[+] QUERY <Server IP Address>:<Server Port> SEARCH_DATE <Date>\n";
    std::cout << "\t[+] QUERY <Server IP Address>:<Server Port> SEARCH_HOST <Hostname>\n";
    std::cout << "\t[+] QUERY <Server IP Address>:<Server Port> SEARCH_DAEMON <Daemon>\n";
    std::cout << "\t[+] QUERY <Server IP Address>:<Server Port> SEARCH_SEVERITY EMERGENCY/ALERT/CRITICAL/ERROR/WARNING/NOTICE/INFORMATIONAL/DEBUG\n";
    std::cout << "\t[+] QUERY <Server IP Address>:<Server Port> SEARCH_KEYWORD <Keywords>\n";
    std::cout << "\t[+] QUERY <Server IP Address>:<Server Port> COUNT_KEYWORD <Keywords>\n";
    std::cout << "\t[+] PURGE <Server IP Address>:<Server Port>\n";
    std::cout << "Tip: Use /help to show this list of commands.\n";
}

void Client::Ingest(std::string& ip_address, uint16_t& port){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchDate(std::string& ip_address, uint16_t& port, std::string& date){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchHost(std::string& ip_address, uint16_t& port, std::string& hostname){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchDaemon(std::string& ip_address, uint16_t& port, std::string& daemon){
    int fd = ConnectToServer(ip_address, port);
    
    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchSeverity(std::string& ip_address, uint16_t& port, std::string& severity){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchKeyword(std::string& ip_address, uint16_t& port, std::string& keyword){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::CountKeyword(std::string& ip_address, uint16_t& port, std::string& keyword){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::Purge(std::string& ip_address, uint16_t& port){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}


void Client::Start(){
    while(1){
        std::string command;
        
        CommandInterface();

        std::cout << "Client> ";
        std::cin >> command; 

        if(command == "/h"){
            std::system("clear");
            continue;
        }

        std::string ip = "127.0.0.1";
        uint16_t port = 5000;

        Ingest(ip, port);
    }
}