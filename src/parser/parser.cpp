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
    struct Log l;

    /**
     * sample log ref
     * 0   1  2        3       4           5       6        ...   
     * Feb 16 02:57:38 SYSSVR2 systemd[1]: Started OpenBSD Secure Shell server per-connection daemon (206.123.145.50:9194).
     */

    l.date = log_tokens[0] + " " + log_tokens[1];
    l.timestamp = log_tokens[3];

    //find index of 1st occurrence of the braced argument (open brace)
    size_t open_brace_pos = log_tokens[4].find('[');
        // or if there is a colon
    size_t colon_pos = log_tokens[4].find(':');

    if(open_brace_pos != std::string::npos){
        l.daemon.name = log_tokens[4].substr(0, open_brace_pos);
        
        size_t close_brace_pos = log_tokens[4].find(']');

        // enclosing brace
        if(close_brace_pos != std::string::npos){
            l.daemon.argument = log_tokens[4].substr(open_brace_pos, close_brace_pos + 1);
        }
    } else if(colon_pos != std::string::npos){
        l.daemon.name = log_tokens[4].substr(0, colon_pos);
        l.daemon.argument = "";
    }

    l.severity = InferSeverity(log_tokens);

    //concatenate tokenized message field
    for(int i = 5; i < log_tokens.size(); i++){
        if(i != 5){
            l.message += " ";
        }

        l.message += log_tokens[i];
    }

    return l;
}

std::string StringifyLog(struct Log& log){
    return log.date + " " + log.timestamp + " " + log.host + " " + log.daemon.name + log.daemon.argument + " " + log.severity  + " " + message;
}

uint8_t InferSeverity(std::vector<std::string> message){
    //initially assume the most unserious 
    uint8_t severity = DEBUG;

    //check every token for keywords
    for(std::string& keyword : message){
        auto it = SEVERITY_KEYWORDS.find(keyword);

        if(it != SEVERITY_KEYWORDS.end()){
            //  if there's a match, and it's higher than the current level then set it
            if(it->second < severity){
                severity = it->second;
            }
        }
    }

    return severity;
}

struct Command IdentifyCommand(std::string& command){
    struct Command c;
    // look for the first space (first word is before the 1st space)
    size_t space_pos = command.find(' ');

    if(space_pos != std::string::npos){
        auto it = COMMANDS.find(command.substr(0, space_pos));

        if(it != COMMANDS.end()){
            c.type = it->second;
            c.argument = command.substr(space_pos, command.length() - space_pos - 1);
        } else{ 
            c.type = -1;
            c.argumenr = "";
        }
    }

    return c;
}