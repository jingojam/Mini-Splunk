#include "../../include/parser/parser.hpp"

std::vector<std::string> Tokenize(std::string& log){
    std::stringstream stream(log);
    std::vector<std::string> tokens;
    std::string token;

    while(stream >> token){
        tokens.push_back(token);
    }

    return tokens;
}

struct Log ParseLog(std::string& log){
    std::vector<std::string> log_tokens = Tokenize(log);
    
    
}

std::string StringifyLog(struct Log& log){

}

uint8_t InferSeverity()