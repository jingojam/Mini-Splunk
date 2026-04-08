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
			
			// event is on the server socket (new connection requested to be established)
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
					
					// read all available byteS of data from the network
					int bytes = recv(fd, temp_buf, sizeof(temp_buf), 0);
					
					// no data
					if(bytes <= 0){
						DisconnectClient(fd);
						clients.erase(it);
						continue;
					}
					
					//store into the client's persistent buffer
					it->second.buffer.append(temp_buf, bytes);
					
					string message;
					
					// process the buffer one message at a time
					while(ExtractMessage(&it->second, &message)){
						
						// ingest client flag is not set, so it's querying only
						if(it->second.ingest == false){
							vector<string> tokens = Tokenize(message);
							
							// safety check to avoid undefiend behavior due to empty commands
							if(tokens.empty()) continue; 

							auto cmd_it = command_type.find(tokens[0]);
							
							// INGEST COMMAND so set the client flag
							if(cmd_it != command_type.end() && cmd_it->second == 0){
								it->second.ingest = true;
							}
							
							// QUERY COMMAND
							else if(cmd_it != command_type.end() && cmd_it->second == 1){
								// make sure that it has enough tokens to avoid crashing
								if(tokens.size() >= 4){ 
									auto type_it = query_type.find(tokens[2]);
									string keyword;
									
									for(size_t i = 3; i < tokens.size(); i++){
										if(i != tokens.size()){
											keyword += " ";
										}
											
										keyword += tokens[i];
									}
									
									RemoveQuotes(&keyword);
									cout << keyword << "\n";
									
									// make sre the query type exists
									if(type_it != query_type.end()){
										int q_type = type_it->second;
										string search_term = keyword;
										
										std::thread query_thread([this, fd, q_type, search_term]() {
											//temporary vector to hold the results
											vector<string> query_results; 
											
											if(q_type == 0){
												SearchDate(search_term, &query_results);
											} else if(q_type == 1){
												SearchHost(search_term, &query_results);
											} else if(q_type == 2){
												SearchDaemon(search_term, &query_results);
											} else if(q_type == 3){
												SearchSeverity(search_term, &query_results);
											} else if(q_type == 4){
												SearchKeyword(search_term, &query_results);
											} else if(q_type == 5){
												size_t count;
												CountKeyword(search_term, &count);
												string res = "\"" + search_term + "\"" + " found in " + to_string(count) + " logs.";
												this->SendData(fd, res);
											}
											
											//send the results back to the client
											if(q_type != 5){
												for(size_t j = 0; j < query_results.size(); j++){
													this->SendData(fd, query_results[j]);
												}
											}
											
											//let client know that's all
											this->SendData(fd, "END_QUERY");
											cout << "[STATUS] Query thread finished. Sent " << query_results.size() << " logs.\n";
										});
										
										query_thread.join();
									}
								}
							}
							
							// PURGE COMMAND
							else if(cmd_it != command_type.end() && cmd_it->second == 2){
								cout << "[STATUS] Executing PURGE command...\n";
								
								std::thread purge_thread([this, fd]() {
									Purge(); 
									this->SendData(fd, "PURGE_COMPLETE"); 
									cout << "[STATUS] Purge thread finished.\n";
								});
								
								purge_thread.join();
							}
							
						} 
						// client wants to INGEST logs (client ingest flag is set)
						else{
							if(message == "DONE"){
								it->second.ingest = false;
								
								// assign a thread for ingest
								std::thread ingest_thread(Ingest, std::move(it->second.logs));
								ingest_thread.join();
								
								cout << "[STATUS] Ingested Logs.\n";
							} else{
								//store normal log lines in temporary vector
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

void Server::SendData(int sockfd, string data){
    string message = to_string(data.length()) + " " + data;
    
    const char* ptr = message.c_str();
    size_t total_len = message.length();
    size_t total_sent = 0;

    while(total_sent < total_len){
        ssize_t sent = send(sockfd, ptr + total_sent, total_len - total_sent, 0);
        
        if(sent == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                continue;
            }
            return; // real error
        }
        
        total_sent += sent;
    }
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

void Server::DisconnectClient(int clientfd){
	close(clientfd);
}