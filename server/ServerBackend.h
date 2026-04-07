#ifndef SERVERBACKEND_H
#define SERVERBACKEND_H

#include <thread>
#include <vector>
#include <string>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include "../parser/Parser.h"

using namespace std;
/*
	ServerBackend.h/cpp serves as the central 
	"database" system of the Server, which 
	facilitates:
		- thread target operation logic (indexing, searching, etc.)
		- central storage of indexes
*/

// shared structures
static vector<LogEntry> log_file; // master log list
static unordered_map<string, vector<size_t>> date_index;     // date -> master log list indexes
static unordered_map<string, vector<size_t>> host_index;     // hostname -> master log list indexes
static unordered_map<string, vector<size_t>> daemon_index;   // process -> master log list indexes
static unordered_map<string, vector<size_t>> severity_index; // severity level -> master log list indexes
static unordered_map<string, vector<size_t>> keyword_index;  // keyword -> master log list indiexes

// shared mutex for acquiring shared and unique locks
static shared_mutex worker_mutex;

// function prototypes
void Ingest(const vector<string>& logs);
void Purge();
void SearchDate(const string& date, vector<string>* out_logs);
void SearchHost(const string& hostname, vector<string>* out_logs);
void SearchDaemon(const string& process_name, vector<string>* out_logs);
void SearchSeverity(const string& severity, vector<string>* out_logs);
void SearchKeyword(const string& keyword, vector<string>* out_logs);
void CountKeyword(const string& keyword, size_t* out_count);

#endif