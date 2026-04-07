#include "ServerBackend.h"

// INGEST function for parsing and indexing of log entries
void Ingest(const vector<string>& logs){
	LogEntry entry;
	vector<string> message_tokens;
	size_t index = 0;
	
	// for every log line
	for(size_t i = 0; i < logs.size(); i++){
		entry = ParseLog(logs[i]); // structure into LogEntry
		message_tokens = Tokenize(entry.message);
		
		// acquire a unique lock for exclusive access (WRITE to shared data)
		//  only lock when reaching critical section (actual write)
		unique_lock lock(worker_mutex);
		
		// critical section
		log_file.push_back(entry);
		
		// update index based on current size of the log list
		index = log_file.size() - 1;
		
		// map the index to the log field
		date_index[entry.date].push_back(index);
		host_index[entry.hostname].push_back(index);
		daemon_index[entry.process.name].push_back(index);
		severity_index[entry.severity].push_back(index);
		
		// map each message token too
		for(string token : message_tokens){
			keyword_index[token].push_back(index);
		}
	}
	// lock is automatically released
}

// PURGE function for deleting log entries
void Purge(){
	// acquire a unique lock for exclusive access (WRITE to shared data)
	unique_lock lock(worker_mutex);
	/*
		critical section: clear indexes and central log list
	*/
	date_index.clear();
	host_index.clear();
	daemon_index.clear();
	severity_index.clear();
	keyword_index.clear();
	log_file.clear();
	// lock is automatically released
}

// function for date lookips
void SearchDate(const string& date, vector<string>* out_logs){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	
	// try to find the date key
	auto it = date_index.find(date);

	// if date key is found
	if(it != date_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			out_logs->push_back(ParseLog(log_file[it->second[i]]));
		}
	}
	// lock is automatically released
}

// function for hostname lookups
void SearchHost(const string& hostname, vector<string>* out_logs){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	
	// try to find the hostname key
	auto it = host_index.find(hostname);
	
	// if hostname key is found
	if(it != host_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			out_logs->push_back(ParseLog(log_file[it->second[i]]));
		}
	}
	// lock is automatically released
}

// function for process/daemon lookups
void SearchDaemon(const string& process_name, vector<string>* out_logs){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	
	// try to find the process name key
	auto it = daemon_index.find(process_name);
	
	// if process key is found
	if(it != daemon_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			out_logs->push_back(ParseLog(log_file[it->second[i]]));
		}
	}
	// lock is automatically released
}

// function for severity level lookups
void SearchSeverity(const string& severity, vector<string>* out_logs){
	// acquire a shared lock
	shared_lock lock(worker_mutex);
	
	// try to find the severity key
	auto it = severity_index.find(severity);
	
	// if severity key is found
	if(it != severity_index.end()){
		// iterate on the index list and add the corresponding indexed log 
		for(size_t i = 0; i < it->second.size(); i++){
			out_logs->push_back(ParseLog(log_file[it->second[i]]));
		}
	}
	// lock is automatically released
}

// function for keyword lookups
void SearchKeyword(const string& keyword, vector<string>* out_logs){
	// acquire (P()) a shared lock
	shared_lock lock(worker_mutex);

	// critical section
	// obtain an iterator to the first token in the keyword
	auto it = keyword_index.find(Tokenize(keyword)[0]);
	
	// if it exists
	if(it != keyword_index.end()){
		// for every syslog mapped to contain the first word
		for(size_t i = 0; i < it->second.size(); i++){

			// store if the entire queried string is a substring in the log
			if(log_file[it->second[i]].message.find(keyword) != string::npos){
				out_logs->push_back(ParseLog(log_file[it->second[i]]));
			}
			
		}
	}
	// lock is automatically released (V())
}

// function for keyword counting
void CountKeyword(const string& keyword, size_t* count){
	// acquire (P()) a shared lock
	shared_lock lock(worker_mutex);
	*count = 0;

	// critical section
	// obtain an iterator to the first token in the keyword
	auto it = keyword_index.find(Tokenize(keyword)[0]);
	
	// if it exists
	if(it != keyword_index.end()){
		// for every syslog mapped to contain the first word
		for(size_t i = 0; i < it->second.size(); i++){

			// store if the entire queried string is a substring in the log
			if(log_file[it->second[i]].message.find(keyword) != string::npos){
				(*count)++;
			}
			
		}
	}
}