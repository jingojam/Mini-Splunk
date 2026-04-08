#ifndef PARSER_H
#define PARSER_H

#include <sstream>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

using namespace std;

/*
	Parser.h/cpp serves the log structure parsing, string manip
	
	facilitates:
		- parsing and structuring log info from raw log string
		- mapping of command types
		- mapping several log-centric data
*/

typedef struct Process{
	string name;
	string arguments;
} Process;

// log entry structure
typedef struct LogEntry{
	// parsed attributes from a log line
	string date;
	string timestamp;
	string hostname;
	Process process;
	string severity;
	string message; // maybe keep this as a token list (vector<string>)
} LogEntry;

// address structure (ip address, port)
typedef struct Address{
	string ip;
	uint16_t port;
} Address;

// for mapping severity levels based on index
static const string severity_level[] = {
	"EMERGENCY", // index 0: level 0 (emergency)
	"ALERT",
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFORMATIONAL",
	"DEBUG"
};

// for mapping string keywords to severity
static const unordered_map<string, int> keyword_to_severity = {
    // emergency
    {"panic", 0}, {"emerg", 0}, 
    
	// alert
	{"alert", 1}, {"immediate", 1},

	// critical
    {"crit", 2}, {"critical", 2}, {"fatal", 2},
    
    // error
    {"error", 3}, {"fail", 3}, {"failed", 3}, {"failure", 3}, {"err", 3},
    
    // warning
    {"warning", 4}, {"warn", 4}, {"invalid", 4}, {"denied", 4}, 
    {"refused", 4}, {"unknown", 4}, {"timeout", 4},
    
    // notice
    {"notice", 5}, {"significant", 5},
	
	// informational
    {"info", 6}, {"accepted", 6}, {"opened", 6}, {"closed", 6},
	
	//debug
    {"debug", 7}, {"verbose", 7}
};

// command type map for faster command identification
// useful for integer instead of string for comparions
static const unordered_map<string, int> command_type = {
	{"EXIT", -1},
	{"INGEST", 0},
	{"QUERY", 1},
	{"PURGE", 2}
};

// query type map
static const unordered_map<string, int> query_type = {
	{"SEARCH_DATE", 0},
	{"SEARCH_HOST", 1},
	{"SEARCH_DAEMON", 2},
	{"SEARCH_SEVERITY", 3},
	{"SEARCH_KEYWORD", 4},
	{"COUNT_KEYWORD", 5}
};

// function prototypes	

void RemoveQuotes(string* str);
	
string ToLower(string str);

vector<string> Tokenize(string str);
		
string InferSeverity(vector<string> message);
		
Address ExtractAddress(string address);
		
LogEntry ParseLog(string log);

string ParseLog(LogEntry entry);

#endif