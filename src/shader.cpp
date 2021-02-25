#include "shader.h"

bool Shader::glInitialized = false;
std::vector<Shader*> Shader::build_queue;
unsigned Shader::active_glID;

Shader::Shader(std::string id) : id(id) {}

Shader::Shader(std::string vPath, std::string fPath, std::string gPath)
{
	fromFiles(vPath, fPath, gPath);
}

Shader::Shader(std::string _id, std::string vPath, std::string fPath, std::string gPath) : 
	id(_id)
{
	fromFiles(vPath, fPath, gPath);
}

Shader::~Shader() {}

void Shader::fromFiles(std::string vPath, std::string fPath, std::string gPath) {
	vertexPath = vPath;
	fragmentPath = fPath;
	geometryPath = gPath;

	// 1. retrieve the vertex/fragment source code from filePath
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	try{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();		
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();			
		// if geometry shader path is present, also load a geometry shader
		if(!geometryPath.empty())
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (const std::ifstream::failure &e){
		log("Shader file was not sucessfully read. Paths given: \"" + vertexPath + "\", \"" + fragmentPath + "\", \"" + geometryPath + "\"", ERR);
		return;
	}

	build();
}

void Shader::fromSource(std::string vertexCode, std::string fragmentCode, std::string geometryCode) {
	this->vertexCode = vertexCode;
	this->fragmentCode = fragmentCode;
	this->geometryCode = geometryCode;

	vertexPath = "";
	fragmentPath = "";
	geometryPath = "";

	build();
}

void Shader::build() {
	// If Opengl hasn't been initialized yet, building a shader will cause a segfault
	// Instead, store this shader to be built later when Opengl is initialized
	if(!glInitialized) {
		if(std::find(build_queue.begin(), build_queue.end(), this) != build_queue.end())
			build_queue.push_back(this);
		return;
	}

	if(glID)
		glDeleteProgram(glID);

	// Perform substitutions if any
	for(auto it : substitution_map) {
		replaceAll(vertexCode, it.first, it.second);
		replaceAll(fragmentCode, it.first, it.second);
		replaceAll(geometryCode, it.first, it.second);
	}
	
	const char* vShaderCode = vertexCode.c_str();
	const char * fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if(!geometryPath.empty()){
		const char * gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	// shader Program
	glID = glCreateProgram();
	glAttachShader(glID, vertex);
	glAttachShader(glID, fragment);
	if(!geometryPath.empty())
		glAttachShader(glID, geometry);
	glLinkProgram(glID);
	checkCompileErrors(glID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if(!geometryPath.empty())
		glDeleteShader(geometry);
}

// activate the shader
// ------------------------------------------------------------------------
void Shader::use() { 
	glUseProgram(glID); 
}
// utility uniform functions
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, bool value) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform1i(glGetUniformLocation(glID, name.c_str()), (int)value); 
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, int value) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform1i(glGetUniformLocation(glID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, float value) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform1f(glGetUniformLocation(glID, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, const glm::vec2 &value) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform2fv(glGetUniformLocation(glID, name.c_str()), 1, &value[0]); 
}
void Shader::set(const std::string &name, float x, float y) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform2f(glGetUniformLocation(glID, name.c_str()), x, y); 
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, const glm::vec3 &value) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform3fv(glGetUniformLocation(glID, name.c_str()), 1, &value[0]); 
}
void Shader::set(const std::string &name, float x, float y, float z) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform3f(glGetUniformLocation(glID, name.c_str()), x, y, z); 
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, const glm::vec4 &value) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform4fv(glGetUniformLocation(glID, name.c_str()), 1, &value[0]); 
}
void Shader::set(const std::string &name, float x, float y, float z, float w) const{ 
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniform4f(glGetUniformLocation(glID, name.c_str()), x, y, z, w); 
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, const glm::mat2 &mat) const{
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniformMatrix2fv(glGetUniformLocation(glID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, const glm::mat3 &mat) const{
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniformMatrix3fv(glGetUniformLocation(glID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::set(const std::string &name, const glm::mat4 &mat) const{
	if(active_glID != glID)
		glUseProgram(active_glID = glID);
	glUniformMatrix4fv(glGetUniformLocation(glID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void Shader::checkCompileErrors(GLuint shader, std::string type) {
	GLint success;
	GLchar infoLog[1024];
	if(type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::string error;
			error = "Shader failed to compile. Faliure type was: " + type + "\n-------------------------------------------------------\n" + infoLog + "\n-------------------------------------------------------\n";
			log(error, CRIT);
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if(!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::string error;
			error = "Shader failed to link. Faliure type was: " + type + "\n-------------------------------------------------------\n" + infoLog + "\n-------------------------------------------------------\n";
			log(error, CRIT);
		}
	}
}

void Shader::init() {
	glInitialized = true;
	
	for(Shader* s : build_queue) {
		s->build();
	}
}