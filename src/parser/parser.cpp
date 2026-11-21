#include "../../include/parser/parser.hpp"

/**
 * Converts a string to uppercase and returns
 * an uppercase equivalent of the string
 */
std::string Upper(std::string& src){
    std::string res;
    res.resize(src.length());

    std::transform(src.begin(), src.end(), res.begin(), [](unsigned char c){return std::toupper(c);});

    return res; 
}

/**
 * Tokenizes a string and returns a vector
 * of string tokens
 */
std::vector<std::string> Tokenize(std::string& log){
    std::stringstream stream(log);
    std::vector<std::string> tokens;
    std::string token;

    while(stream >> token){
        tokens.push_back(token);
    }

    return tokens;
}

/**
 * Parses a log and returns a `struct Log`
 */
struct Log ParseLog(std::string& log){
    std::vector<std::string> log_tokens = Tokenize(log);
    struct Log l;

    /**
     * sample log ref
     * 0   1  2        3       4           5       6        ...   
     * Feb 16 02:57:38 SYSSVR2 systemd[1]: Started OpenBSD Secure Shell server per-connection daemon (206.123.145.50:9194).
     */

    l.date = log_tokens[0] + " " + log_tokens[1];
    l.timestamp = log_tokens[2];
    l.host = log_tokens[3];

    //find index of 1st occurrence of the braced argument (open brace)
    size_t open_brace_pos = log_tokens[4].find('[');
        // or if there is a colon
    size_t colon_pos = log_tokens[4].find(':');

    if(open_brace_pos != std::string::npos){
        l.daemon.name = log_tokens[4].substr(0, open_brace_pos);
        
        size_t close_brace_pos = log_tokens[4].find(']');

        // enclosing brace
        if(close_brace_pos != std::string::npos){
            l.daemon.argument = log_tokens[4].substr(open_brace_pos, (close_brace_pos - open_brace_pos) + 1);
        }
    } else if(colon_pos != std::string::npos){
        l.daemon.name = log_tokens[4].substr(0, colon_pos);
        l.daemon.argument = "";
    }

    l.severity = InferSeverity(log_tokens);

    //concatenate tokenized message field
    for(size_t i = 5; i < log_tokens.size(); i++){
        if(i != 5){
            l.message += " ";
        }

        l.message += log_tokens[i];
    }

    return l;
}

/**
 * Returns a concatenated string of a `struct Log`
 */
std::string StringifyLog(struct Log& log){
    return log.date + " " + log.timestamp + " " + log.host + " " + log.daemon.name + log.daemon.argument + " " + log.severity  + " " + log.message;
}

uint8_t InferSeverity(std::vector<std::string>& message){
    //initially assume the most unserious 
    uint8_t severity = DEBUG;

    //check every token for keywords
    for(const std::string& keyword : message){
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

/**
 * Checks the length prefix against the
 * actual length of the message
 *  Returns true if it matches,
 *  Otherwise false
 *
 */
bool ValidMessage(std::string& buffer, size_t& size){
    if(buffer.length() < 1){
        return false;
    }

    // find the length prefix separated byspace
    size_t space_pos = buffer.find(' ');

    // if there is no space delimiter then network message incompllete
    if(space_pos == std::string::npos){
        return false;
    } 

    //extract prefix and raw message
    size_t prefix_length = std::stoul(buffer.substr(0, space_pos));
    size = prefix_length;
    size_t raw_network_message_size = (space_pos + 1) + prefix_length; // the whole message plus the prefix
 
    // if the message is less than the prefix then network message incomplete
    if(buffer.length() < raw_network_message_size){
        return false;
    }

    // message buffer is sufficient
    buffer.erase(0, space_pos + 1);

    return true;
}

/**
 * Computes the length of the message and prepends it
 * as a prefix to the message, and returns the length
 */
size_t Prefix(std::string& message){
    message.insert(0, std::to_string(message.length()) + " ");
    return message.length();
}

int ParseClientCommand(struct ClientCommand& dst, std::string& src){
    std::vector<std::string> tokens = Tokenize(src);
    size_t num_tokens = tokens.size();

    if(num_tokens < 2){
        return -1;
    }

    dst.command = tokens[0];

    size_t colon_pos = tokens[1].find(':');

    if(colon_pos != std::string::npos){
        dst.address = tokens[1].substr(0, colon_pos);
        dst.port = static_cast<uint16_t>(std::stoi(tokens[1].substr(colon_pos + 1)));
    }

    if(num_tokens > 2){
        dst.argument = tokens[2];
    }

    return 0;
}