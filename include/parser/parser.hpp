#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <sstream>
#include <cstdint>

/**
 * Daemon structure
 *  argument field corresponds to
 *  extra inner value enclosed by square brackets
 *  if the log daemon field has any.
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

/**
 * SYslog severity levels
 */
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
    {"emergency", EMERGENCY},
    {"alert", ALERT},
    {"crit", CRITICAL},
    {"critical", CRITICAL},
    {"err", ERROR},
    {"error", ERROR},
    {"warning", WARNING},
    {"warn", WARNING},
    {"notice", NOTICE},
    {"info", INFORMATIONAL},
    {"debug", DEBUG}
};

/**
 * Function signatures
 */
std::vector<std::string> Tokenize(const std::string& log);

struct Log ParseLog(const std::string& log);

std::string StringifyLog(const struct Log& log);

uint8_t InferSeverity(const std::vector<std::string>& message);

bool ValidMessage(std::string& buffer);

size_t Prefix(size_t type, std::string& message);

#endif