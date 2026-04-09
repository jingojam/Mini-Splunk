# Mini-Splunk Syslog Analytics Server

## Dependencies
The Project is compiled against `Ubuntu 22.04.5 LTS` using `g++` version `(Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0`.

## Overview
The “Mini-Splunk” Syslog Analytics Server serves as a concurrent utility tool for managing multiple client network connections using Linux `epoll` simultaneously, handling log file ingests from clients, parsing and structuring logs, indexing and storing structured logs, performing search (query) functions, and safe deletion of log ingests.

The current iteration of the Server stores all indexed logs primarily into RAM allocated using standard C++ STL data structures. Additionally, `PURGE` commands are effective globally. The Server is inspired by the fundamental mechanisms of the SPIMI approach, which utilizes the reverse index technique for indexing logs, so the Server 

## Project Directory


## Compiling and the Server Application
The `server/` directory already contains an executable file `server_main.exe` which can be run on a a standard Linux distro. To run it:

