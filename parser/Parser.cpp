#include "Parser.h"

string ToLower(string str){
	// transform string using tolower() lambda function
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c){return static_cast<char>(tolower(c));});
	return str;
}

/*
	Tokenizes a string by splitting the string via space delimeter
	
	example:
	log: Feb 15 00:54:54 SYSSVR1 sshd[262715]: Failed password for root from 130.12.181.97 port 20066 ssh2
	idx: 0   1  2        3       4             5      6        7   8    9    10            11   12    13
*/
vector<string> Tokenize(string str){
	stringstream ss(str);
	string word;
	vector<string> tokens;
	
	// for every string from the stream store as a token (space delimiter)
	while(ss >> word){
		tokens.push_back(word);
	}
	
	return tokens;
}

string InferSeverity(vector<string> message){
	int severity = 7; // default is debug
	
	for(size_t i = 5; i < message.size(); ++i){
		// obtain a value from the severity map using the key in the index
		string token = ToLower(message[i]);
		
		// locate the key value to search for
		auto it = keyword_to_severity.find(token);
		
		// if the iterator is not past-the-end
		if(it != keyword_to_severity.end()){
			// if the severity is higher (lower in value), set the new severity
			if(it->second < severity){
				severity = it->second;
			}
		}
	}
	
	return severity_level[severity];
}

// function ro extract ip address and port from a string argument
//  e.g., "192.168.254.118:5460", returns `Address` struct
Address ExtractAddress(string address_token){
	Address address;
	size_t colon_index = address_token.find(':');
	
	if(colon_index != string::npos){
		// ip address is first set of characters until the colon `:`
		address.ip = address_token.substr(0, colon_index);
		// port is everything else after the colon
		string port = address_token.substr(colon_index + 1);
		// convert the port to uint16_t
		address.port = static_cast<uint16_t>(stoi(port));
	}
	
	return address;
}

// function for structuring log data 
LogEntry ParseLog(string log){
	vector<string> log_tokens = Tokenize(log);
	LogEntry entry;
	
	entry.date = log_tokens[0] + " " + log_tokens[1];
	entry.timestamp = log_tokens[2];
	entry.hostname = log_tokens[3];
	
	// get the process value (strip of extra arguments)
	size_t open_bracket_index = log_tokens[4].find('['); // first instance of `[` (e.g., "sshd[...")
	size_t close_bracket_index = log_tokens[4].find(']'); // first insance of `]` (e.g., "sshd[...]")	
	size_t colon_index = log_tokens[4].find(':'); // first instance of `:` (e.g., "sshd:...")
	
	entry.process.name = "";
	entry.process.arguments = "";
	
	if(open_bracket_index != string::npos){
		// process name is the string right before the arguments ([...]) part, like sshd
		entry.process.name = log_tokens[4].substr(0, open_bracket_index); // indexes until the bracket
		
		if(close_bracket_index != string::npos){
			// arguments includes strings right after the process, like "[...]"
			entry.process.arguments = log_tokens[4].substr(open_bracket_index, close_bracket_index + 1);
		}
	}	
	
	// otherwise, there are some cases where a colon is in the affix of the process name
	else if(colon_index != string::npos){ 
		entry.process.name = log_tokens[4].substr(0, colon_index); //consider string until the colon
		entry.process.arguments = ":";
	}
	
	// or just a process name without any affix (unusual)
	else{
		entry.process.name = log_tokens[4];
	}
	
	entry.severity = InferSeverity(log_tokens);
	
	entry.message = "";
	
	// reconstruct original message string throuhg concatenation
	for(size_t i = 5; i < log_tokens.size(); i++){
		if(i != log_tokens.size() - 1){
			entry.message += " ";
		}
		
		entry.message += log_tokens[i];
	}
	
	return entry;
}

string ParseLog(LogEntry entry){
	return entry.date + " " + entry.timestamp + " " + entry.hostname + " " + entry.process.name + entry.process.arguments + " " + entry.severity + " " + entry.message;
}