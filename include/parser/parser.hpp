#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stringstream>

/**
 * Daemon structure
 *  argument field corresponds to
 *  extra inner value inside the square brackets
 *   if the log daemon field has any.
 */
struct Daemon{
    std::string name;
    std::string argument;
};

/**
 * Log Structure
 */
struct Log{
    std::string date;
    std::string timestamp;
    std::string host;
    struct Daemon daemon;
    std::string severity;
    std::string message;
};

static constexpr uint8_t EMERGENCY = 0;
static constexpr uint8_t ALERT = 1;
static constexpr uint8_t CRITICAL = 2;
static constexpr uint8_t ERROR = 3;
static constexpr uint8_t WARNING = 4;
static constexpr uint8_t NOTICE = 5;
static constexpr uint8_t INFORMATIONAL = 6;
static constexpr uint8_t DEBUG = 7;

/**
 * reference: https://www.manageengine.com/products/eventlog/logging-guide/syslog/syslog-levels.html
 */
static const std::unordered_map<std::string, uint8_t> SEVERITY_KEYWORDS = {
    {"emerg", EMERGENCY},
    {"alert", ALERT},
    {"crit", CRITICAL},
    {"err", ERROR},
    {"warning", WARNING},
    {"notice", NOTICE},
    {"info", INFORMATIONAL},
    {"debug", DEBUG}
};

std::vector<std::string> Tokenize(std::string& log);
struct Log ParseLog(std::string& log);
std::string StringifyLog(struct Log& log);
uint8_t InferSeverity(std::vector<std::string> message);

#endif