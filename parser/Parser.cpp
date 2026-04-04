#include "Parser.h"

Parser::Parser(){}

/*
	(BUGGY)
	
	takes network data size prefix (<size> <space> <log>) from a log line
	and strips the size prefix from the log pointer
	
	returns data size as a number
*/
size_t Parser::StripNetworkLog(string* str){
	size_t size = 0;
	size_t space_index = str->find(' ');
	
	if(space_index != string::npos){
		size = static_cast<size_t>(stoul(str->substr(0, space_index)));
		str->erase(0, space_index + 1);
	}

	return size;
}

string Parser::ToLower(string str){
	// transform string using tolower() lambda function
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c){return static_cast<char>(tolower(c));});
	return str;
}

/*
	Tokenizes a log entry string by splitting the string 
	via space delimeter
	
	example:
	log: Feb 15 00:54:54 SYSSVR1 sshd[262715]: Failed password for root from 130.12.181.97 port 20066 ssh2
	idx: 0   1  2        3       4             5      6        7   8    9    10            11   12    13
*/
vector<string> Parser::Tokenize(string str){
	stringstream ss(str);
	string word;
	vector<string> tokens;
	
	// for every string from the stream store as a token (space delimiter)
	while(ss >> word){
		tokens.push_back(word);
	}
	
	return tokens;
}

string Parser::InferSeverity(vector<string> message){
	int severity = 7; // default is debug

	for(size_t i = 5; i < message.size(); ++i){
		// obtain a value from the severity map using the key in the index
		string token = ToLower(message[i]);
		
		// locate the key value to search for
		auto it = keyword_to_severity.find(token);
		
		// if the iterator is not past-the-end
		if(it != keyword_to_severity.end()){
			if(it->second < severity){
				severity = it->second;
			}
		}
	}
	
	return severity_level[severity];
}

Address Parser::ExtractAddress(string address_token){
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

LogEntry Parser::ParseLog(string log){
	vector<string> log_tokens = Tokenize(log);
	LogEntry entry;
	
	entry.date = log_tokens[0] + " " + log_tokens[1];
	entry.timestamp = log_tokens[2];
	entry.hostname = log_tokens[3];
	
	// get the process value (strip of extra arguments)
	size_t bracket_index = log_tokens[4].find('['); //process without e.g., sshd[...
	size_t colon_index = log_tokens[4].find(':'); // process e.g., sshd:...
	
	if(bracket_index != string::npos) 
		entry.process = log_tokens[4].substr(0, bracket_index); // indexes until the bracket
	else if(colon_index != string::npos) 
		entry.process = log_tokens[4].substr(0, colon_index); //consider string until the colon
	else
		entry.process = log_tokens[4];
	
	entry.severity = InferSeverity(log_tokens);
	
	entry.message = "";
	
	// reconstruct original message string
	for(size_t i = 5; i < log_tokens.size(); i++){
		if(i+1 < log_tokens.size()){
			entry.message += " ";
		}
		
		entry.message += log_tokens[i];
	}
	
	return entry;
}

/*
	COMMAND FORMATS:
		INGEST <path_to_logfile> <IP>:Port> 
		QUERY <IP>:<Port> SEARCH_DATE <date_string>
		QUERY <IP>:<Port> SEARCH_HOST <hostname>
		QUERY <IP>:<Port> SEARCH_DAEMON <daemon_name>
		QUERY <IP>:<Port> SEARCH_SEVERITY <severity_level>
		QUERY <IP>:<Port> SEARCH_KEYWORD <keyword>
		PURGE <IP>:<Port>
*/