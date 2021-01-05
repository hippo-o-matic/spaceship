#include "utility.h"

const char* LOG_PATH = "";
const char* CONFIG_PATH = "tests/config";

void log(std::string message, LOG_SEVERITY severity) {
	std::string sev;
	switch(severity){
		case INFO:
			sev = "[?] "; // Info or message
		case WARN:
			sev = "[!] "; // Warning, couldn't find model or texture file
		case ERR:
			sev = "[!!] "; // Error, a GL error occured
		case CRIT:
			sev = "[!!!] "; // Critical Error, the engine is going down 
		default:
			sev = "";
	}

	if(LOG_PATH == NULL || LOG_PATH[0] == '\0') {
		std::fstream file(LOG_PATH, std::fstream::out);
		file << sev << message << "\n";
		file.close(); // TODO: dont open and close this constantly
	}

	std::cout << sev << message << "\n";
}

// Replaces all instances of <target> with <result> in <input&>
std::string replaceAll(std::string& input, const std::string& target, const std::string& result) {
	if(target.empty())
		return input;

	size_t pos = 0;
	while((pos = input.find(target, pos)) != std::string::npos) {
		input.replace(pos, result.length(), result);
		pos += result.length();
	}
	return input;
}