#include <iostream>
#include <string>
#include <cstdint>
#include "Server.cpp"

using namespace std;

// CLI input format
// argv[0]: source filename, argv[1]: ip address, argv[2]: port 
int main(int argc, char** argv){
	string ip = argv[1];
	uint16_t port = static_cast<uint16_t>(stoi(argv[2])); // stoi returns an `int` so explicitly typecast it to unsigned 16-bit integer
	
	Server server(ip, port);
	server.Start();
	
	return 0;
}