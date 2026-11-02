g++ -Wall -std=c++20 -I./include main/server_main.cpp src/server/server.cpp src/parser/parser.cpp -o server
g++ -Wall -std=c++20 -I./include main/client_main.cpp src/client/client.cpp src/parser/parser.cpp -o client
