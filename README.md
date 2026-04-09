# Mini-Splunk Syslog Analytics Server

## Dependencies
The Project is compiled against `Ubuntu 22.04.5 LTS` using `g++` version `(Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0`.

## Overview
The “Mini-Splunk” Syslog Analytics Server serves as a concurrent utility tool for managing multiple client TCP connections using Linux `epoll` multiplexing simultaneously:
- Handling reception of log file ingests from clients over a network
- Parsing and structuring logs
- Indexing and storing structured logs
- Performing search (query) functions
- Safe deletion of log ingests.
- Sending log results back to Clients

The current iteration of the Server stores all indexed logs primarily into RAM allocated using standard C++ STL data structures. Additionally, `PURGE` commands are effective globally. The Server is inspired by the fundamental mechanisms of the SPIMI approach, which utilizes the reverse index technique for indexing logs, where a keyword or log attribute (search term) is mapped to a list of log indices (postings). So the Server performs search operations on terms at a constant `O(1)` at best, and a linear `O(n)` for obtaining logs mapped to the queried term. 

## Project Directory


## Compiling and the Server Application
The `server/` directory already contains an executable file `server_main.exe` which can be run on a a standard Linux distro. To run it:

