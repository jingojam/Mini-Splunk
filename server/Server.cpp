#include "Server.h"

using namespace std;

Server::Server(){}

Server::Server(string ip_address, uint16_t port){
	this->ip = ip_address;
	this->port = port;
	int result = inet_pton(AF_INET, ip.c_str(), &(this->ip_address.sin_addr));
    this->ip_address.sin_family = AF_INET;
    this->ip_address.sin_port = htons(port); // must be in network bytes (big endian)
}

// function for creating a socket instance
//  returns a socket file descriptor, otherwise a negative integer meaning error creating socket
int Server::CreateSocket(){
	int sockfd;
	
	// create a socket instance
	if((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1){
		return -1; // -1 means socket instance was not created succesfully
	}

	// bind the socket to the server IP and port
	if(bind(sockfd, (struct sockaddr*)&this->ip_address, sizeof(this->ip_address)) == -1){
		return -2; // -2 means socket instance was created but failed to bind to server
	}

	// set the socket to listening state
	if(listen(sockfd, SOMAXCONN) == -1){
		return -3; // -3 means socket instance was bind to the server but failed to be set to listening state
	}

	// otherwise if successful return the socket file descriptor
	return sockfd;
}

int Server::EpollSocket(int sockfd){
	struct epoll_event event;
	int epollfd = epoll_create1(0);
	
	if(epollfd == -1){
		return -1; // -1 means epoll instance failed to be created
	}
	
	event.data.fd = sockfd;
	event.events = EPOLLIN; // associated file available for read
	
	// add the socket to interest list
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1){
		return -2; // -2 means epoll failed to add the server socket file descriptor for monitoring
	}
	
	// otherwise return the epoll instance file descriptor
	return epollfd;
}
// function for adding client fds to interest list
int Server::AcceptClient(int epollfd, int sockfd){
	struct sockaddr_in client_address;
	socklen_t client_address_length = sizeof(client_address);
	struct epoll_event event;
	int clientfd;
	Client client;
	  
	// accept the client
	if((clientfd = accept(sockfd, (struct sockaddr*)&client_address, &client_address_length)) != -1){
		// set the client socket to nonblocking
		fcntl(clientfd, F_SETFL, O_NONBLOCK);
			
		event.data.fd = clientfd;
		event.events = EPOLLIN;
			
		// add the client socket to interest list
		if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event) == -1){
			return -1;
		}
		cout << "[STATUS] Client connected at " << inet_ntoa(client_address.sin_addr) << ":" << to_string(client_address.sin_port) << "\n";
		
	} else{
		// if accept returns -1, means no clients connecting, accept returns an errno
		//  and if errno is EAGAIN or EWOULDBLOCK, queue is empty
		if(errno == EAGAIN || errno == EWOULDBLOCK){
			return 0;
		}
	}
	
	return clientfd;
}

void Server::MonitorEvents(int epollfd, int sockfd){
	int fd, fds, clientfd;
	struct epoll_event events[10];
	
	while(true){
		// obtain num of file descriptors ready for I/O
		fds = epoll_wait(epollfd, events, 10, -1);
		
		// handle each event
		for(size_t i = 0; i < fds; i++){
			fd = events[i].data.fd; // get the file descriptor from the epoll event
			
			// event is oon the server socket (new connection requested to be established)
			if(fd == sockfd){
				clientfd = AcceptClient(epollfd, sockfd);
				
				if(clientfd > 0){
					Client new_client;
					new_client.fd = clientfd;
					clients[clientfd] = new_client;
				}
			}
			
			else if(events[i].events & EPOLLIN){
				auto it = clients.find(fd);
				
				if(it != clients.end()){
					char temp_buf[1024];
					
					// 1. Read all available bytes from the network
					int bytes = recv(fd, temp_buf, sizeof(temp_buf), 0);
					
					if(bytes <= 0){
						DisconnectClient(fd);
						clients.erase(it);
						continue;
					}
					
					// Dump the bytes into the client's persistent buffer
					it->second.buffer.append(temp_buf, bytes);
					
					string message;
					
					// 2. Process the buffer one message at a time
					// This loop safely handles state changes mid-buffer!
					while(ExtractMessage(&it->second, &message)){
						
						// Are we currently listening for commands?
						if(it->second.ingest == false){
							vector<string> tokens = Tokenize(message);

							auto cmd_it = command_type.find(tokens[0]);
							
							if(cmd_it != command_type.end() && cmd_it->second == 0){
								it->second.ingest = true;
								cout << "[STATUS] Switching to INGEST mode.\n";
							}
							// Handle PURGE, QUERY, etc. here...
						} 
						else {
							if(message == "DONE"){
								it->second.ingest = false;
								cout << "[STATUS] Finished ingesting. Back to COMMAND mode.\n";
							} 
							else {
								cout << "[LOG] " << message << "\n\n";
								it->second.logs.push_back(message);
							}
						}
					}
				}
			}
		}
	}
}

void Server::Start(){
	// first step is to create a network socket
	int sockfd, epollfd;
	
	if((sockfd = CreateSocket()) < 0){
		
		if(sockfd == -1){
			cout << "[ERROR] Failed to create socket.\n";
		} else if(sockfd == -2){	
			cout << "[ERROR] Failed to bind socket.\n";
		} else{
			cout << "[ERROR] Failed to set socket to listening state.\n";
		}
			
		return;
	} 
	
	cout << "[STATUS] Server is ready.\n";

	if((epollfd = EpollSocket(sockfd)) != -1){
		MonitorEvents(epollfd, sockfd);
	}

	close(sockfd);
	close(epollfd);
}

bool Server::ExtractMessage(Client* client, string* message){
    size_t space_pos = client->buffer.find(' ');
		
    if(space_pos == string::npos){ 
		return false; //need more data to find the length
	}

    try{
		// get the length prefix (just before the first white space)
        string message_prefix = client->buffer.substr(0, space_pos);
        size_t prefix_length = stoul(message_prefix);
            
        // total size = <length of message> + " " + <message>
        size_t total_expected = space_pos + 1 + prefix_length;

		// if message received is shorter than expected
        if(client->buffer.length() < total_expected){
            return false; // must only consider enough data
        }

		//extract the actual log message (without the length prefix)
        *message = client->buffer.substr(space_pos + 1, prefix_length);

        //remove the processed packet from the buffer
        client->buffer.erase(0, total_expected);
        return true;
    } catch(...){
        client->buffer.clear();
        return false;
    }
}

// void Server::ReceiveCommand(Client* client){
    // char temp_buf[1024];
	
    // int bytes = recv(client->fd, temp_buf, sizeof(temp_buf), 0);

    // if(bytes <= 0){
        // return;
    // }

    // client->buffer.append(temp_buf, bytes);

	// string command;

    // while(ExtractMessage(client, &command)){
		// // process it
		// vector<string> tokens = Tokenize(command);
		// auto it = command_type.find(tokens[0]);
		
		// if(it != command_type.end() && it->second == 0){
			// client->ingest = true;
		// }
	// }
// }

// void Server::ReceiveLogs(Client* client, vector<string>* logs){
    // char temp_buf[1024];
	
    // int bytes = recv(client->fd, temp_buf, sizeof(temp_buf), 0);

    // if(bytes <= 0){
        // return;
    // }

    // client->buffer.append(temp_buf, bytes);

	// string log;

	// while(ExtractMessage(client, &log)){
		// // check if the client is signaling the end of the file
		// if(log == "DONE"){
			// client->ingest = false;
		// } 
		// // otherwise, treat it as a normal log line
		// else{
			// cout << log << "\n\n";
			// logs->push_back(log);
		// }
	// }
// }

void Server::DisconnectClient(int clientfd){
	close(clientfd);
}