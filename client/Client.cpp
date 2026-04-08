#include "Client.h"

using namespace std;

Client::Client(){}

void Client::CommandInterface(){
    char command_buffer[1024];
    int fd;
    vector<string> command_tokens;
    Address address;
    
    while(1){       
        cout << "Client> ";
        cin.getline(command_buffer, 1024);
        
        string command(command_buffer);
        command_tokens = Tokenize(command);
		
        if(command_tokens.empty()) continue;

        auto it = command_type.find(command_tokens[0]);

        // EXIT
        if(it != command_type.end() && it->second == -1){
            break;
        }

        // INGEST
        else if(it != command_type.end() && it->second == 0){
            address = ExtractAddress(command_tokens[2]);

            if((fd = ConnectToServer(address.ip, address.port)) != -1){
                SendData(fd, command);
                SendLogFile(command_tokens[1], fd);
                
                close(fd); // close socket when done
            }
        } 
        
        // QUERY
        else if(it != command_type.end() && it->second == 1){
            address = ExtractAddress(command_tokens[1]);
			string keyword;

			for(size_t i = 2; i < command_tokens.size(); i++){
				if(!keyword.empty()){
					keyword += " ";
				}
				keyword += command_tokens[i];
			}
			
			string final_command = command_tokens[0] + " " + command_tokens[1] + " " + keyword;
			
            if((fd = ConnectToServer(address.ip, address.port)) != -1){
                SendData(fd, final_command); 
                
                // Only wait for results on a QUERY!
                ReceiveQueryResults(fd); 
                
                close(fd); // Close socket when query is done
            }
        }
        
        // PURGE
        else if(it != command_type.end() && it->second == 2){
            address = ExtractAddress(command_tokens[1]);

            if((fd = ConnectToServer(address.ip, address.port)) != -1){
                SendData(fd, command);
                // Optional: You could read a "PURGE_COMPLETE" ack here if you wanted
                cout << "[STATUS] Sent PURGE command to server.\n";
                close(fd);
            }
        } 
		
		// unknown command
		else{
			cout << "[ERROR] Unknown command.\n";
		}
    }
    
    cout << "[STATUS] Exiting client." << endl;
}

void Client::SendLogFile(string filename, int fd){
	ifstream file(filename);
	string line;
	
	if(file.is_open()){
		while(getline(file, line)){
			if(!line.empty() && line.back() == '\r'){
                line.pop_back();
            }
			SendData(fd, line);
		}
		file.close();
		
		SendData(fd, "DONE");
		cout << "[STATUS] Successfully Ingested File." << endl;
	} else{
		cout << "[ERROR] File not Found." << endl;
	}
}

void Client::SendData(int sockfd, string data){
    string message = to_string(data.length()) + " " + data;
    
    const char* ptr = message.c_str();
    size_t total_len = message.length();
    size_t total_sent = 0;

    while(total_sent < total_len){
        //offset the pointer by the number of bytes already sent
        size_t sent = send(sockfd, ptr + total_sent, total_len - total_sent, 0);
        
        if(sent == -1){
            return;
        }
        
        total_sent += sent;
    }
}

bool Client::ExtractMessage(string& buffer, string& message){
    size_t space_pos = buffer.find(' ');
        
    if(space_pos == string::npos){ 
        return false; // need more data to find the length
    }

    try{
        string message_prefix = buffer.substr(0, space_pos);
        size_t prefix_length = stoul(message_prefix);
            
        size_t total_expected = space_pos + 1 + prefix_length;

        if(buffer.length() < total_expected){
            return false; // must only consider enough data
        }

        message = buffer.substr(space_pos + 1, prefix_length);
        buffer.erase(0, total_expected);
		
        if(buffer.empty()){
            buffer.clear();
        }
        
        return true;
    } catch(...){
       return false;
    }
}

void Client::ReceiveQueryResults(int sockfd){
    char temp_buf[65536];
    string receive_buffer = "";
    string message;
    bool receiving = true;
    
    cout << "\n--- Query Results ---\n";

    while(receiving){
        int bytes = recv(sockfd, temp_buf, sizeof(temp_buf), 0);
        
        if(bytes <= 0){
            cout << "[ERROR] Server disconnected during query.\n";
            break;
        }

        receive_buffer.append(temp_buf, bytes);

        while(ExtractMessage(receive_buffer, message)){
            
            if(message == "END_QUERY"){
                receiving = false;
                break; 
            } 
            else {
                if (!message.empty() && message.back() == '\r') {
                    message.pop_back(); 
                }
                
                cout << message << endl;
            }
        }
    }
    
    cout << "---------------------\n\n";
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