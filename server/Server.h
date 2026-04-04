/*
	Linux man pages
		- socket():                   https://man7.org/linux/man-pages/man2/socket.2.html
		- inet_ntop():				  https://man7.org/linux/man-pages/man3/inet_ntop.3.html
		- epoll: 					  https://man7.org/linux/man-pages/man7/epoll.7.html,
									  https://medium.com/@m-ibrahim.research/mastering-epoll-the-engine-behind-high-performance-linux-networking-85a15e6bde90
		- bind(): 					  https://man7.org/linux/man-pages/man2/bind.2.html
*/
#ifndef SERVER_H
#define SERVER_H

#define SERVER_WORKERS 8

#include "../parser/Parser.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>

using namespace std;

// shared structures
vector<LogEntry> log_file; // master log list
unordered_map<string, vector<size_t>> date_index;     // date -> master log list indexes
unordered_map<string, vector<size_t>> host_index;     // hostname -> master log list indexes
unordered_map<string, vector<size_t>> daemon_index;   // process -> master log list indexes
unordered_map<string, vector<size_t>> severity_index; // severity level -> master log list indexes
unordered_map<string, vector<size_t>> keyword_index;  // keyword -> master log list indiexes

shared_mutex rw_mutex;

/*
	Server class for handling clients
*/
class Server{ // inherit parsing methods
	private:
		string ip;
		uint16_t port;
		struct sockaddr_in ip_address;

	public:
		Server();
		Server(string ip_address, uint16_t port);
		void Start(); // for initializing the Server
};

#endif