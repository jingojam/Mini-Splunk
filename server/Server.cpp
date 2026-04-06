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

// INGEST function for parsing and indexing of log entries
void Server::Ingest(){
	// acquire a unique lock for exclusive access (WRITE to shared data)
	unique_lock lock(worker_mutex);
	// critical section
	// lock is automatically released
}

// PURGE function for deleting log entries
void Server::Purge(){
	// acquire a unique lock for exclusive access (WRITE to shared data)
	unique_lock lock(worker_mutex);
	/*
		critical section: erase() indexes and central log list
	*/
	// lock is automatically released
}

// function for date lookips
vector<LogEntry> Server::SearchDate(string date){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	vector<LogEntry> logs;
	
	// try to find the date key
	auto it = this->date_index.find(date);

	// if date key is found
	if(it != this->date_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(this->log_file[it->second[i]]);
		}
	}
	
	// lock is automatically released
	return logs;
}

// function for hostname lookups
vector<LogEntry> Server::SearchHost(string hostname){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	vector<LogEntry> logs;
	
	// try to find the hostname key
	auto it = this->host_index.find(hostname);
	
	// if hostname key is found
	if(it != this->host_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(this->log_file[it->second[i]]);
		}
	}
	
	// lock is automatically released
	return logs;
}

// function for process/daemon lookups
vector<LogEntry> Server::SearchDaemon(string process_name){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	vector<LogEntry> logs;
	
	// try to find the process name key
	auto it = this->daemon_index.find(process_name);
	
	// if process key is found
	if(it != this->daemon_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(this->log_file[it->second[i]]);
		}
	}
	
	// lock is automatically released
	return logs;
}

// function for severity level lookups
vector<LogEntry> Server::SearchSeverity(string severity){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	vector<LogEntry> logs;
	
	// try to find the severity key
	auto it = this->severity_index.find(severity);
	
	// if severity key is found
	if(it != this->severity_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(this->log_file[it->second[i]]);
		}
	}
	
	// lock is automatically released
	return logs;
}

// function for keyword lookups
vector<LogEntry> Server::SearchKeyword(string keyword){
	// acquire (P()) a shared lock
	shared_lock lock(worker_mutex);
	vector<LogEntry> logs;

	// critical section
	// obtain an iterator to the first token in the keyword
	auto it = this->keyword_index.find(Tokenize(keyword)[0]);
	
	// if it exists
	if(it != this->keyword_index.end()){
		// for every syslog mapped to contain the first word
		for(size_t i = 0; i < it->second.size(); i++){

			// store if the entire queried string is a substring in the log
			if(this->log_file[it->second[i]].message.find(keyword) != string::npos){
				logs.push_back(this->log_file[it->second[i]]);
			}
			
		}
	}

	// lock is automatically released (V())
	return logs;
}

// function for keyword counting
size_t Server::CountKeyword(string keyword){
	// simply return the size using SearchHost() vector
	return static_cast<size_t>(SearchHost(keyword).size());
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

void Server::AcceptClients(int epollfd, int sockfd){
	struct sockaddr_in client_address;
	socklen_t client_address_length = sizeof(client_address);
	struct epoll_event event;
	int clientfd;
	Client client;
	
	// accept the client
	if((clientfd = accept(sockfd, (struct sockaddr*)&client_address, &client_address_length)) != -1){
		client.fd = clientfd;
		client.command = "";
			
		// store the client for further processing
		this->clients.push_back(client);
			
		// set the client socket to nonblocking
		fcntl(clientfd, F_SETFL, O_NONBLOCK);
			
		event.data.fd = clientfd;
		event.events = EPOLLIN;
			
		// add the client socket to interest list
		if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event) == -1){
			return;
		}
		cout << "[STATUS] Client connected at " << inet_ntoa(client_address.sin_addr) << ":" << to_string(client_address.sin_port) << "\n";
		
	} else{
		// if accept returns -1, means no clients connecting, accept returns an errno
		//  and if errno is EAGAIN or EWOULDBLOCK, queue is empty
		if(errno == EAGAIN || errno == EWOULDBLOCK){
			return;
		}
	}
}

void Server::MonitorEvents(int epollfd, int sockfd){
	int fd, fds;
	struct epoll_event events[10];
	
	while(true){
		// obtain num of file descriptors ready for I/O
		fds = epoll_wait(epollfd, events, 10, -1);
		
		// handle each event
		for(size_t i = 0; i < fds; i++){
			fd = events[i].data.fd; // get the file descriptor from the epoll event
			
			// event is oon the server socket (new connection requested to be established)
			if(fd == sockfd){
				AcceptClients(epollfd, sockfd);
				break;
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