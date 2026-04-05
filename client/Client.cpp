#include "Client.h"

using namespace std;

Client::Client(){}

void Client::SendLogFile(string filename, int fd){
	ifstream file(filename);
	string line;
	
	if(file.is_open()){
		while(getline(file, line)){
			int length = line.length();
			string msg = to_string(length) + " " + line; // octet counting
			cout << msg << endl << endl;
			int senddata = send(fd, msg.c_str(), msg.length(), 0);
		}
		file.close();
		cout << "[STATUS] Successfully Ingested File." << endl;
	} else{
		cout << "[ERROR] File not Found." << endl;
	}
}

void Client::CommandInterface(){
	char command_buffer[1024];
	int fd;
	vector<string> command_tokens;
	Address address;
	
	while(1){		
		cout << "Client> ";
		
		cin.getline(command_buffer, 1024);
		
		// convert from char* command input buffer to string
		string command(command_buffer);
		
		// tokenize the command buffer
		command_tokens = Tokenize(command);

		// obtain iterator to the map key-value pair
		auto it = command_type.find(command_tokens[0]);

		// if command is -1 (EXIT)
		if(it != command_type.end() && it->second == -1){
			break;
		}

		// if command type is 0 (INGEST)
		else if(it != command_type.end() && it->second == 0){
			address = ExtractAddress(command_tokens[2]);

			ReadLogFile(command_tokens[1]);
			
			// if(fd = ConnectToServer(address.ip, address.port) != -1){
				
				// string command_msg = command_tokens[0] + " " + command_tokens[1];
				// int senddata = send(fd, command_msg.c_str(), command_msg.length(), 0); 
				
				// SendLogFile(command_tokens[1], fd);
				// // file ingest
			// }
		} 
		
		// other commands..
	}
	close(fd);
	cout << "[STATUS] Disconnected from Server." << endl;
}

int Client::ConnectToServer(string server_ip, uint16_t port){
	int sockfd, connectfd;
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port); 
	int pton = inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd == -1){
		cout << "[ERROR] Error Initializing Client Socket." << endl;
		return -1;
	}
	
	connectfd = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	
	if(connectfd == 0){
		cout << "[STATUS] Successfully Connected to the Server." << endl;
		return sockfd;
	} else{
		cout << "[ERROR] Failed to Connect to the Server." << endl;
	}
	
	return -1;
}