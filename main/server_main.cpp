#include <iostream>
#include <string>
#include "../src/server/server.cpp"

int main(int argc, char* argv[]){

    if(argc < 3){
        std::cerr << "ERROR: Insufficient Arguments. Usage: ./server <IP>:<PORT>\n";
    }

    std::string ip = argv[1];
    std::string port = argv[2];


    return 0;
}