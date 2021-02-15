#pragma once

#include "utility.h"
#include "logs.h"

#include "json/json.h"

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <streambuf>

struct ObjectCastException : public std::runtime_error {
    ObjectCastException(std::string type = "") : 
        runtime_error("Object does not inherit from class " + type) {}
};

struct ObjectMissingException : public std::runtime_error {
    ObjectMissingException(std::string id, std::string parent) : 
        runtime_error("The appropriate object could not be found.\nid: \"" + id + "\"\nparent id: \"" + parent +"\"") {}
};

template<class In, class Out>
std::unique_ptr<Out> dynamic_unique_cast(std::unique_ptr<In> ptr) {
	if (In* cast = dynamic_cast<In*>(ptr.get())) {
		std::unique_ptr<Out> result(cast, std::move(ptr.get_deleter()));
		ptr.release();
		return result;
    }
    throw std::bad_cast();
}

class Object {
public:
    typedef std::unique_ptr<Object> ptr;

	Object(std::string id);

	Object(Json::Value);

	virtual ~Object();

	std::string id;
    std::string type;

    /// Component handling

    // Moves an object from one parent into another (Destination << Input)
    template<class T>
    void takeFromRef(std::unique_ptr<T>& o);

    template<class T>
    void take(std::unique_ptr<T> o);

    void operator+=(Object::ptr &o); // Move an object from one parent to another
    void operator+=(std::vector<Object::ptr> &o_vec); // Move a vector of objects from one parent to another
	void operator-=(Object::ptr &o); // Remove an object from an object's components

    Object::ptr& operator[](size_t index); // Operator that returns a component in this object's vector by index
    Object::ptr& operator[](std::string id); // Operator that returns the component in this object's vector that matches the id

    Object::ptr& get(std::string id); // Returns a refrence to an object component by matching the id

    template<class T>
    T& get(std::string id); // Equivalent to get(id)->as<T>() 
    
    template<class T> T* as(); // Returns a pointer to the object as type T, if the object wasn't originally T, throws an ObjectCastException
    
    template<typename... Args> void runup(std::function<void(Object*, Args...)>, Args... args); // Runs a function for parent recursively, working up the tree
    template<typename... Args> void rundown(std::function<void(Object*, Args...)>, Args... args); // Runs a function for each child object recursively, branching for each component. Use lightly


    // A vector that holds objects with no parent, in practice shouldn't hold anything other than Level objects
    // static std::vector<Object::ptr> global;

protected:
	Object* parent = nullptr; // Raw pointer to the objects parent
	std::vector<Object::ptr> components; // Vector of the objects sub-components
	void createComponents(Json::Value items);
};

// A class that registers, creates, and defines new Objects
class ObjFactory {
    typedef std::map<std::string, std::function<Object::ptr(Json::Value)>> map_type;
    struct registered_type {
        std::string token;
        std::function<Object::ptr(Json::Value)> create_f;
        // std::function<???)()> cast_f;
    };

public:
    // template<class T, typename... Args>
    // static Object::ptr newObj(Args...);
    static Object::ptr createObjectJson(std::string const& s, Json::Value json = Json::Value()); // Creates a registered object and returns its unique pointer

	template<class T>
	static bool const registerType(const char* name) {
        std::function<Object::ptr(Json::Value)> create_f([](Json::Value j){ return std::make_unique<T>(j); });
        // std::function<T&(Object::ptr&)> cast_f([](Object::ptr& me){ return me->as<T&>(); }); // Haha I wish
		getMap()->emplace(name, create_f);
		return true;
	}
	
private:
    static std::shared_ptr<map_type> getMap();
    inline static std::shared_ptr<map_type> typemap = nullptr; // The map of all the types and their functions
};


/* Creates a register function for the object and calls it, place inside class definition
* telabrium_obj_reg: a trick using static initialization order to register the type before main()*/
#define REGISTER_OBJECT_TYPE(NAME) inline static bool telabrium_obj_reg = ObjFactory::registerType<NAME>(#NAME)


class BlankObject : public Object {
public:
	BlankObject() = default;
	BlankObject(Json::Value);

private:
	REGISTER_OBJECT_TYPE(BlankObject);
};


template<class T, typename... Args>
static Object::ptr newObj(Args... args) {
    return std::make_unique<T>(args...);
}

template<class T>
void Object::takeFromRef(std::unique_ptr<T>& o) {
    o->parent = this;
    components.push_back(std::move(o));
}

template<class T>
void Object::take(std::unique_ptr<T> o) {
    o->parent = this;
    components.push_back(std::move(dynamic_unique_cast<T, Object>(std::move(o))));
}

template<class T> T* Object::as() {
	T* out = dynamic_cast<T*>(this); // Attempt to cast this to T
	if(out) { // If the object wasn't originally T, it will return a nullptr
		return out;
	} else {
		throw ObjectCastException();
	}
}

template<class T>
T& Object::get(std::string id) {
    return *get(id)->as<T>();
}

// Testing only
// auto Object::get(std::string id) -> decltype(getType(id->type)) {
    
// }


// Recursively runs a function (func) up the object hierarchy, each parent calls func(args) and then has their parent run func()
template<typename... Args>
void Object::runup(std::function<void(Object*, Args...)> func, Args... args) {
    func(this, args...);
    if(parent)
        parent->runup<Args...>(func, args...);
}

template<typename... Args>
void Object::rundown(std::function<void(Object*, Args...)> func, Args... args) {
    func(this, args...);
    if(!components.empty()) {
        for(auto& it : components) {
            it->rundown<Args...>(func, args...);
        }
    }
}
