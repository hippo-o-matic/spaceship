#include "shader.h"

std::vector<unsigned int> Shader::shader_draw_order{};
std::unordered_map<unsigned int, std::shared_ptr<Shader>> Shader::shaders{};
unsigned Shader::active_glID;

std::shared_ptr<Shader> Shader::requestShader(std::string _id, std::string _vertexPath, std::string _fragmentPath, std::string _geometryPath) {
	// Check for duplicate shaders
	// If no id is provided, check to see if any shaders have matching paths, and use that instead
	if(_id == "") {
		std::string vertexPath = _vertexPath;
		std::string fragmentPath = _fragmentPath;
		std::string geometryPath = _geometryPath;

		for(auto pair : shaders) {
			if(pair.second->vertexPath == _vertexPath && pair.second->fragmentPath == _fragmentPath && pair.second->geometryPath == _geometryPath) {
				log("No shader id was given, however a shader already exists with the same paths (\"" + _vertexPath + ", \"" + _fragmentPath + ", \"" + _geometryPath + "\") Please specify an id for the shader", WARN);
				return pair.second;
			}
		}
	} else {
		// If a shader with the same id string exists, return the id of that string
		for(auto pair : shaders) {
			if(pair.second->id == _id) {
				return pair.second;
			}
		}
	}

	auto shader = std::make_shared<Shader>(_id, _vertexPath, _fragmentPath, _geometryPath);

	shaders.emplace(std::pair<unsigned int, std::shared_ptr<Shader>>(shader->glID, shader));
	
	shader_draw_order.push_back(shader->glID);
	return shader;
}

Shader::Shader(std::string vPath, std::string fPath, std::string gPath) : vertexPath(vPath), fragmentPath(fPath), geometryPath(gPath) {
	build();
}

Shader::Shader(std::string _id, std::string vPath, std::string fPath, std::string gPath) : id(_id), vertexPath(vPath), fragmentPath(fPath), geometryPath(gPath) {
	build();
}

Shader::~Shader() {
	auto it = std::find(shader_draw_order.begin(), shader_draw_order.end(), glID);
	if(it != shader_draw_order.end())
		shader_draw_order.erase(it);
}

void Shader::build(){
	if(glID)
		glDeleteProgram(glID);

	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
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