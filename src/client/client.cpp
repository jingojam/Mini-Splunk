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

void Client::StreamFile(int fd, std::string& filename){
    std::ifstream file(filename);

    if(!file.is_open()){
        std::cerr << "[ERROR] File \"" << filename << "\" Not Found.\n";
        return;
    }
 
    std::string data;

    while(std::getline(file, data)){
        SendData(fd, data);
    }
}

void Client::SendData(int fd, std::string& data){
    size_t len = Prefix(data);
    size_t total_bytes_sent = 0; // how much bytes has been sent so far
    size_t bytes_sent = 0;       // how much bytes was sent for a send iteration
    size_t bytes_to_send = len;  // how much data left to send
    const char* msg = data.c_str();

    while(total_bytes_sent < len){
        bytes_sent = send(fd, msg + total_bytes_sent, bytes_to_send, 0);

        bytes_to_send -= bytes_sent;
        total_bytes_sent += bytes_sent;
    }
}

int Client::ReceiveData(){
    return -1;
}

void Client::CommandInterface(){
    std::cout << "Commands:\n";
    std::cout << "\t[+] INGEST <Server IP Address>:<Server Port> <Filepath>\n";
    std::cout << "\t[+] SEARCH_DATE <Server IP Address>:<Server Port> <Date>\n";
    std::cout << "\t[+] SEARCH_HOST <Server IP Address>:<Server Port> <Hostname>\n";
    std::cout << "\t[+] SEARCH_DAEMON <Server IP Address>:<Server Port> <Daemon>\n";
    std::cout << "\t[+] SEARCH_SEVERITY <Server IP Address>:<Server Port> EMERGENCY/ALERT/CRITICAL/ERROR/WARNING/NOTICE/INFORMATIONAL/DEBUG\n";
    std::cout << "\t[+] SEARCH_KEYWORD <Server IP Address>:<Server Port> <Keywords>\n";
    std::cout << "\t[+] PURGE <Server IP Address>:<Server Port>\n";
    std::cout << "Tip: Use /help to show this list of commands.\n";
}

void Client::Ingest(std::string& ip_address, uint16_t port, std::string& filename){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }

    StreamFile(fd, filename);
}

void Client::SearchDate(std::string& ip_address, uint16_t port, std::string& date){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchHost(std::string& ip_address, uint16_t port, std::string& hostname){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchDaemon(std::string& ip_address, uint16_t port, std::string& daemon){
    int fd = ConnectToServer(ip_address, port);
    
    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchSeverity(std::string& ip_address, uint16_t port, std::string& severity){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::SearchKeyword(std::string& ip_address, uint16_t port, std::string& keyword){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}

void Client::Purge(std::string& ip_address, uint16_t port){
    int fd = ConnectToServer(ip_address, port);

    if(fd < 0){
        std::cerr << "[ERROR] Cannot Connect to Server.\n";
        return;
    }
}


void Client::Start(){
    std::string command;

    while(1){
        CommandInterface();

        std::cout << "Client> ";
        std::getline(std::cin,command); 

        if(command == "/help"){
            std::cout << "\n";
            CommandInterface();
            continue;
        }

        struct ClientCommand client_command;

        ParseClientCommand(client_command, command);

        if(client_command.command == "INGEST"){
            Ingest(client_command.address, client_command.port, client_command.argument);
        }
    }
}