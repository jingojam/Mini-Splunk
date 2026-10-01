# Mini-Splunk Syslog Analytics Server

## Dependencies
The Project is compiled against `Ubuntu 22.04.5 LTS` using `g++` version `(Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0`.

## Overview
The **Mini-Splunk Syslog Analytics Server** serves as a lightweigh utility tool for managing multiple client connections, and provide an interface for:
- Handling reception of log file ingests from clients over a network
- Extracting and structuring [**RFC 3164 (BSD Syslog Protocol)**](https://www.rfc-editor.org/rfc/rfc3164.html) log formats
- Parsing syslogs
- Indexing and storing syslogs
- Performing search functions
- Safe deletion of log ingests
- Sending log results back to Clients

## Details
The current iteration of the Server stores all indexed logs primarily into RAM allocated using standard C++ STL data structures. Additionally, `PURGE` commands are effective globally. The Server is inspired by the fundamental mechanisms of the **Single-Pass In-Memory Indexing (SPIMI)** approach used by **Apache Lucene**, which utilizes an inverted index structure for indexing logs, where a keyword or log attribute (search term) is mapped to a list of log indices (postings). So the Server performs search operations on terms at a constant **O(1)** at best, and a linear **O(n)** for obtaining logs mapped to the queried term. The socket file descriptors of clients are monitored by the Server using the Linux `epoll` event multiplexing mechanism. It allows the Server to monitor a batch of client socket file descriptors (also known as an "interest list") and scale with growing amounts of clients efficiently. 

The Server utilizes a multi-threaded backend, where each task/command sent by a Client is handled by a dedicated lightweight Server worker thread. It manages access to shared data structures through through shared locks for managing concurrent read-only access to the shared resources, and unique locks for mutually-exclusive write operations including destructive Ingest and Purge operations. This architecture maximizes Server processing and facilitate Client requests more responsively.

## Compiling and Running the Server Application
The `server/` directory already contains an executable file `server_main.exe` which can be run on a a standard Linux distro. To run it:
1) Acquire execute permissions to run the executables (may require root `sudo` permissions): `sudo chmod +x server_main.exe`
2) Run `./server_main.exe <server IP address> <server port>`
   
To compile it:
1) Navigate to the project's root directory using the terminal application
2) Run `g++ <executable name>.exe server/server_main.cpp server/ServerBackend.cpp parser/Parser.cpp`

## Compiling and Running the Client Application
The `client/` directory already contains an executable file `client_main.exe`. To run it:
1) Acquire execute permissions to run the executables (may require root `sudo` permissions): `sudo chmod +x client_main.exe`
2) Run `./client_main.exe`
   
To compile it:
1) Navigate to the project's root directory using the terminal application
2) Run `g++ <executable name>.exe client/client_main.cpp parser/Parser.cpp`

## Using the Client CLI
| Function | Command Syntax |
| -------- | -------------- |
| Uploading Logs | `INGEST <file_path> <server IP address>:<server port>` |
| Querying by Date | `QUERY <server IP address>:<server port> SEARCH_DATE "<date_string>"` |
| Querying by Host Name | `QUERY <server IP address>:<server port> SEARCH_HOST “<hostname>”` |
| Querying by Daemon (Process) | `QUERY <server IP address>:<server port> SEARCH_DAEMON “<daemon_name>”` |
| Querying by Severity | `QUERY <server IP address>:<server port> SEARCH_SEVERITY “EMERGENCY/ALERT/CRITICAL/ERROR/WARNING/NOTICE/INFORMATIONAL/DEBUG”`  |
| Keyword Search | `QUERY <server IP address>:<server port> SEARCH_KEYWORD “<keyword/s>"` |
| Keyword Count | `QUERY <server IP address>:<server port> COUNT_KEYWORD “<keyword/s>"` |
| Erasing Logs | `PURGE <server IP address>:<server port>` |

