#include "object.h"

Object::Object(std::string _id) : id(_id) {}

Object::Object(Json::Value j) {
	this->id = j.get("id", "").asString();
	this->type = j.get("type", "BlankObject").asString();

	createComponents(j["components"]);
}

Object::~Object() {
	parent = nullptr;
	for(auto& it : components)
		std::remove(components.begin(), components.end(), it);
}

// Moves an object from one parent into another (Destination << Input)
void Object::operator+=(Object::ptr &o) {
	// o->parent-= o;

	o->parent = this;
	components.push_back(std::move(o));
}

// Moves several objects from one parent to another
void Object::operator+=(std::vector<Object::ptr> &o_vec){
	for(auto it = o_vec.begin(); it != o_vec.end(); it++){
		(*it)->parent = this;
		components.push_back(std::move(*it));
		o_vec.erase(it);
	}
}

// Deletes an object from the parent's vector 
void Object::operator-=(Object::ptr& o) {
	std::vector<Object::ptr>* vec = &(this->components);
	// auto it = std::find_if(vec->begin(), vec->end(), [&](std::unique_ptr<Object> &p){
	// 	return p.get() == o.get();
	// });
	// if(it != vec->end())
	// 	vec->erase(it);

	std::remove(vec->begin(), vec->end(), o);
	o->parent = nullptr;
}

// Returns a refrence to the specified element (a unique pointer)
Object::ptr& Object::get(std::string id) {
	auto index = std::find_if(components.begin(), components.end(), [id](const Object::ptr& o) -> bool { return o->id == id; });
	if(index == components.end()) { // If the component could not be found, throw an exception
		// log("\"" + this->id + "\" does not contain a \"" + id + "\"", WARN); // Log the faliure only the first time
		throw ObjectMissingException(id, this->id);
	}
	return *index; // Try dynamic_cast<unique<T>>& if breaks
}

// Same as above, but 
Object::ptr& Object::operator[](size_t index) {
	return components.at(index);
}

// Equivalent to get(id)
Object::ptr& Object::operator[](std::string id) { 
	return get(id);
}


void Object::createComponents(Json::Value items) {
	for(unsigned i = 0; i < items.size(); i++) {
		Object::ptr o = ObjFactory::createObjectJson(items[i]["type"].asString(), items[i]);
		o->parent = this;
		components.push_back(std::move(o));
	}
}

Object::ptr ObjFactory::createObjectJson(std::string const& s, Json::Value json) {
	ObjFactory::map_type::iterator it = getMap()->find(s); // Find the creation function by key
	if(it == getMap()->end()) { // Check if it exists
		log("Could not find object type \"" + s + "\", type not registered", ERR);
		return nullptr;
	}
	return it->second(json); // Create an object
}

std::shared_ptr<ObjFactory::map_type> ObjFactory::getMap() {
	if(!typemap) { typemap = std::make_shared<ObjFactory::map_type>(); } 
	return typemap;
}

BlankObject::BlankObject(Json::Value j) : Object(j) {}
