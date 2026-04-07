#ifndef CLIENT_H
#define CLIENT_H

#include "../parser/Parser.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;

/*
	Client class for facilitiating commands 
*/
class Client{ // client inherits parsing methods
	private:	
		// network methods
		void SendLogFile(string filename, int fd);
		int ConnectToServer(string ip, uint16_t port);
		
		void SendData(int sockfd, string data);
	
	public:
		Client();
		void CommandInterface();
};

#endif