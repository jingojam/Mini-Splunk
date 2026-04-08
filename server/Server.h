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

#include "ServerBackend.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

/*
	Server.h/cpp facilitates the network operations of the server,
	such as:
		- sending and receiving data from a client
		- monitoring client connections via epoll
*/

typedef struct Client{
	int fd; // client socket file descriptor
	string buffer; // client buffer
	bool ingest = false; // if client wants to ingest
	vector<string> logs;
} Client;

/*
	Server class for handling clients
*/
class Server{
	private:
		// server network infos
		string ip;
		uint16_t port;
		struct sockaddr_in ip_address;
		unordered_map<int, Client> clients;
 
		//network functions
		int CreateSocket();
		int EpollSocket(int sockfd);
		int AcceptClient(int epollfd, int sockfd);
		void MonitorEvents(int epollfd, int sockfd);
		void SendResults(int sockfd, Client client);
		bool ExtractMessage(Client* client, string* message);
		// void ReceiveLogs(Client* client, vector<string>* logs);
		// void ReceiveCommand(Client* client);
		void SendData(int sockfd, string data);
		void DisconnectClient(int clientfd);
		void ReceiveLogData(Client* client);

	public:
		Server();
		Server(string ip_address, uint16_t port);
		void Start(); // for initializing the Server
};

#endif