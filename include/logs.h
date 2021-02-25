#pragma once

#include <iostream>
#include <fstream>

extern const char* LOG_PATH;

enum LOG_SEVERITY {
	INFO, WARN, ERR, CRIT
};

void log(std::string message, LOG_SEVERITY severity);
