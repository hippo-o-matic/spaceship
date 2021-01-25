#include "input.h"
#include "logs.h"

std::vector<Input*> Input::active_profiles;
Input* Input::solo_profile;
bool Input::focused = false;
double Input::mouse_x, Input::mouse_y;

bool Input::block_keys = false;
bool Input::block_mouse_buttons = false;
bool Input::block_mouse = false;
bool Input::block_scroll = false;

Input::Input(std::string path) {
	loadConfigFile(path);
}

Input::Input() {}


void Input::loadConfigFile(std::string path) {
	std::ifstream file(path);
	Json::Value j;
	file >> j;
	loadConfig(j);
}

void Input::loadConfig(Json::Value& json) {
	Json::Value b = json["keybinds"];

	// Iterate through the array of keybind assignments
	for(unsigned i = 0; i < b.size(); i++) {
		std::string name = b[i][0].asString();
		auto name_it = std::find_if(binds.begin(), binds.end(), [name](bind b){ return b.name == name; }); // Find the bind by name
		if(name_it != binds.end()) {
			binds[i].key = b[i][1].asInt(); // Set the key stored in json (b[i][1]) to the appropriate bind in the vector
			binds[i].action = b[i][2].asInt();
		}
	}
}

void Input::saveConfigFile(std::string path){
	std::ofstream file(path);
	file << saveConfig();
}

Json::Value Input::saveConfig() {
	Json::Value json;
	for (unsigned i = 0; i < binds.size(); i++) {
		json["keybinds"][i][0] = binds.at(i).name;
		json["keybinds"][i][1] = binds.at(i).key;
		json["keybinds"][i][2] = binds.at(i).action;
	}

	return json;
}


void Input::addBind(const char* name, std::function<void()> func, int key, int action, int type) {
	if(key == GLFW_KEY_UNKNOWN) {
		log("Can't add a bind to an unknown key", ERR); return;
	}

	// If the bind doesn't already exist, push a new one to the binds vector
	if(std::find_if(binds.begin(), binds.end(), [name](bind b){ return b.name == name; }) == binds.end()) { // Find the bind by name
		binds.push_back({name, func, key, action, type});
	} else {
		log("A bind with the id \"" + std::string(name) + "\" already exists", WARN);
	}
		
}

void Input::removeBind(const char* name) {
	binds.erase(std::find_if(binds.begin(), binds.end(), [name](bind b){ return b.name == name; })); // Find the bind by name
}

void Input::removeBind(int key) {
	for(auto it = binds.begin(); it != binds.end(); it++) {
		if(it->key == key)
			binds.erase(it);
	}
}


void Input::editBindFunc(const char* name, std::function<void()> func) {
	auto it = std::find_if(binds.begin(), binds.end(), [name](bind b){ return b.name == name; }); // Find the bind by name
	if(it != binds.end()) {
		it->func = func;
	} else {
		log("Can't find bind \"" + std::string(name) + "\" to edit", ERR);
	}
}

void Input::editBindKey(const char* name, int key, int action) {
	auto it = std::find_if(binds.begin(), binds.end(), [name](bind b){ return b.name == name; }); // Find the bind by name
	if(it != binds.end()) {
		it->key = key; // Assign values if it does
		it->action = (action == -2) ? it->action : action; // If the action is the default (-2), leave it as the same
	} else {
		log("Can't find bind \"" + std::string(name) + "\" to edit", ERR);
	}
}

void Input::setKeyCallback(std::function<void(GLFWwindow*, int, int, int, int)> func) {
	key_c = func;
}
void Input::setMouseButtonCallback(std::function<void(GLFWwindow*, int, int, int)> func) {
	button_c = func;
}
void Input::setMouseCallback(std::function<void(GLFWwindow*, double, double)> func) {
	mouse_c = func;
}
void Input::setScrollCallback(std::function<void(GLFWwindow*, double, double)> func) {
	scroll_c = func;
}
void Input::setFocusCallback(std::function<void(GLFWwindow*, bool)> func){
	focus_c = func;
}
void Input::setFramebufferCallback(std::function<void(GLFWwindow*, int, int)> func) {
	frame_c = func;
}


// Check each bind and run its function
void Input::process(GLFWwindow* win) {
	std::function<int(GLFWwindow*, int)> check_f = [](GLFWwindow*, int){ return 0; };
	
	for(auto bt = binds.begin(); bt != binds.end(); bt++) {
		if(bt->type == MOUSE_B && !block_mouse_buttons)
			check_f = glfwGetMouseButton;
		if(bt->type == KEY && !block_keys)
			check_f = glfwGetKey;
		
		if(bt->action == INPUT_ONCE) {
			int state = check_f(win, bt->key);
			if(state == GLFW_PRESS) {
				if(!bt->block) {
					bt->func();
					bt->block = true;
				}
			} else if(state == GLFW_RELEASE) {
				bt->block = false;
			}
		} else if(bt->action == INPUT_ONCE_RELEASE) {
			int state = check_f(win, bt->key);
			if(state == GLFW_RELEASE) {
				if(!bt->block) {
					bt->func();
					bt->block = true;
				}
			} else if(state == GLFW_PRESS) {
				bt->block = false;
			}
		}

		if(check_f(win, bt->key) == bt->action)
			bt->func();
	}
}

// Check for input for all active profiles
void Input::processActive(GLFWwindow* win) {
	if(solo_profile != nullptr) {
		solo_profile->process(win);
		return;
	}

	for(auto it = active_profiles.begin(); it != active_profiles.end(); it++) { // Iterate through all active profiles
		(*it)->process(win);
	}
}


// Start tracking inputs from this profile
void Input::activate() {
	active_profiles.push_back(this);
}

// Stop tracking inputs from this profile
void Input::deactivate() {
	active_profiles.erase(std::find(active_profiles.begin(), active_profiles.end(), this));
}

// Make this input the only active one
void Input::activate_solo() {
	solo_profile = this;
}

// Stop only paying attention to the solo input
void Input::undo_solo() {
	solo_profile = nullptr;
}


// Ignore all other profiles while this one is active
// void Input::activate_solo() {
//     active_profiles.clear();
//     active_profiles.push_back(this);
// }

// Callbacks to be passed to GLFW
/////////////////////////////////

// NOTE: keys and mouse buttons should be handled with Input::process, as this callback doesn't continue running when keys are held
void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// TEST: Unfocus on escape
	// if (focused) {
	// 	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	// 		focused = false;
	// }
	if(!block_keys) {
		if(solo_profile != nullptr && solo_profile->key_c) {
			solo_profile->key_c(window, key, scancode, action, mods);
			return;
		}
		
		for(auto it : active_profiles) { // Iterate through all active profiles
			if(it->key_c)
				it->key_c(window, key, scancode, action, mods);
		}
	}
}

void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// Focus the window on click (make mouse disappear)
	// if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
	// 	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	// 		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// }
	if(!block_mouse_buttons) {
		if(solo_profile != nullptr && solo_profile->button_c) {
			solo_profile->button_c(window, button, action, mods);
			return;
		}

		for(auto it : active_profiles) { // Iterate through all active profiles
			if(it->button_c)
				it->button_c(window, button, action, mods);
		}
	}
}

void Input::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	mouse_x = xpos;
	mouse_y = ypos;

	if(focused && !block_mouse) { // Only run mouse events if the window is focused
		if(solo_profile != nullptr && solo_profile->mouse_c) {
			solo_profile->mouse_c(window, xpos, ypos);
			return;
		}
		
		for(auto it : active_profiles) { // Iterate through all active profiles
			if(it->mouse_c)
				it->mouse_c(window, xpos, ypos);
		}
	}
}

void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if(focused && !block_scroll) { // Only run mouse events if the window is focused
		for(auto it : active_profiles) { // Iterate through all active profiles
			if(solo_profile != nullptr && solo_profile->scroll_c) {
				solo_profile->scroll_c(window, xoffset, yoffset);
				return;
			}
			
			if(it->scroll_c)
				it->scroll_c(window, xoffset, yoffset);
		}
	}
}

void Input::focus_callback(GLFWwindow* window, int glfwFocus) {
	focused = glfwFocus == GLFW_TRUE ? true : false; // Avoid implicit conversion because it's icky, make 0 false and 1 true
	
	for(auto it : active_profiles) { // Iterate through all active profiles
		if(it->focus_c)
			it->focus_c(window, focused);
	}
}

void Input::framebuffer_callback(GLFWwindow* window, int width, int height) {
	if(focused) { // Only run mouse events if the window is focused
		for(auto it : active_profiles) { // Iterate through all active profiles
			if(it->frame_c)
				it->frame_c(window, width, height);
		}
	}
}

void Input::assignCallbacks(GLFWwindow* window) {
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowFocusCallback(window, focus_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_callback);
}