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

void Server::Start(){
	int fd, sockfd, epfd;
	int bindfd, listenfd;
	// result can be used for non-file descriptor value returns from functions
	int result; 
	
	// step 1: initialize server socket
	// SOCK_STREAM for TCP, and non-blocking for `epoll()`
	sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	
	if(sockfd == -1){
		cout << "[ERROR] Failed initializing Server socket." << endl;
		return;
	}
	
	cout << "[STATUS] Successfully initialized Server socket." << endl;
	
	// set options to allow Server to reuse socket in case 
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	
	// step 2: bind server ip and port
	if(bind(sockfd, (struct sockaddr*)&this->ip_address, sizeof(this->ip_address)) != 0){
		cout << "[ERROR] Failed binding Server to " << this->ip << ":" << this->port << endl;
		return;
	}
	
	cout << "[STATUS] Successfully bound Server to " << this->ip << ":" << this->port << endl;
	
	// step 3: server must now listen for incoming connections
	if(listen(sockfd, SOMAXCONN) == -1){
		cout << "[ERROR] Failed to initialize Server listening state." << endl;
		return;
	}
	
	cout << "[STATUS] Server is ready." << endl;
	
	// client handling
	int clientfd, epollfd, count;
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);
	struct epoll_event events[10];
	struct epoll_event event;
	
	event.events = EPOLLIN;
	event.data.fd = sockfd;
	
	epollfd = epoll_create1(0);
	
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
	
	while(1){
		// obtain the number of events the server must monitor
		count = epoll_wait(epollfd, events, 10, -1);
	 
		for(size_t i = 0; i < count; ++i){
			int eventfd = events[i].data.fd;
	
			// if the event happens in the listening socket
			if(eventfd == sockfd){
				// for edge-triggered events, this must be inside a loop
				//  but in the interest of simplicity, server backend uses level-triggered events
				clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen); // accept the client
				
				if(clientfd != -1){
					cout << "[STATUS] Client Connected." << endl;
							
					// set client to non-blocking
					fcntl(clientfd, F_SETFL, (fcntl(clientfd, F_GETFL) | O_NONBLOCK));
							
					// add the client to epoll
					event.events = EPOLLIN;
					event.data.fd = clientfd;
					epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event);
				}
			} 
			
			// if events is a client sending a command to the server (file descriptor has data available to be read)
			else if(events[i].events & EPOLLIN){
				char buffer[4096];
				int read;
				
				read = recv(eventfd, buffer, sizeof(buffer), 0);
				
				string str(buffer);
				// gets the log character length prefix, and strips it from the log string
				size_t expected_length = StripNetworkLog(&str);
				
				LogEntry entry = ParseLog(buffer);
				//cout << entry.date << " " << entry.timestamp << " " << entry.hostname << " " << entry.process << " " << entry.severity << " " << entry.message << endl << endl;
			}
		}
	}

	close(sockfd);
	close(epollfd);
}