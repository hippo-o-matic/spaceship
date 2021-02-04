#pragma once

#include "logs.h"

#include <string>
#include <vector>

extern const char* CONFIG_PATH;

std::string replaceAll(std::string& input, const std::string& target, const std::string& sub);

// From a vector of structs, extracts one field from each struct and returns a vector of copies of them
template<typename Struct_T, typename Field_T>
std::vector<Field_T> extractStructField(std::vector<Struct_T>& vec, Field_T Struct_T::* field) {
	std::vector<Field_T> result;
	for(Struct_T& s : vec) {
		result.emplace_back(s.*field);
	}

	return result;
}
