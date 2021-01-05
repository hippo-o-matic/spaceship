#pragma once

#include "logs.h"

#include <string>

extern const char* CONFIG_PATH;

std::string replaceAll(std::string& input, const std::string& target, const std::string& sub);
