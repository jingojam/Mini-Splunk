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
	// critical section
	// lock is automatically released
}

// function for date lookips
vector<LogEntry> Server::SearchDate(string date){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	vector<LogEntry> logs;
	
	// try to find the date key
	auto it = date_index.find(date);

	// if date key is found
	if(it != date_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(log_file[it->second[i]]);
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
	auto it = host_index.find(hostname);
	
	// if hostname key is found
	if(it != host_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(log_file[it->second[i]]);
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
	auto it = daemon_index.find(process_name);
	
	// if process key is found
	if(it != daemon_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(log_file[it->second[i]]);
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
	auto it = severity_index.find(severity);
	
	// if severity key is found
	if(it != severity_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			logs.push_back(log_file[it->second[i]]);
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
	auto it = keyword_index.find(Tokenize(keyword)[0]);
	
	// if it exists
	if(it != keyword_index.end()){
		// for every syslog mapped to contain the first word
		for(size_t i = 0; i < it->second.size(); i++){

			// store if the entire queried string is a substring in the log
			if(log_file[it->second[i]].message.find(keyword) != string::npos){
				logs.push_back(log_file[it->second[i]]);
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
	
	// create an epoll instance for multiplexing
	epollfd = epoll_create1(0);
	
	// add the server network socket to epoll
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
				
				//read = recv(eventfd, buffer, sizeof(buffer), 0);
				
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