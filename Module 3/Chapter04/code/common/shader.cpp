//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 4 - Modern OpenGL - Texture Mapping with 2D images
// Copyrights & Licenses:
//=======================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "shader.hpp"

//helper function for handling file I/O
std::string readSourceFile(const char *path){
	//open the file stream for reading
	std::string code;
	std::ifstream file_stream(path, std::ios::in);

	if(file_stream.is_open()){
		std::string line = "";
		while(getline(file_stream, line))
			code += "\n" + line;
		file_stream.close();
		return code;
	}else{
		printf("Failed to open \"%s\".\n", path);
		return "";
	}
}

//compile the shader program from the source code
void CompileShader(std::string program_code, GLuint shader_id){
	GLint result = GL_FALSE;
	int infolog_length;

	char const * program_code_pointer = program_code.c_str();
	glShaderSource(shader_id, 1, &program_code_pointer , NULL);
	glCompileShader(shader_id);

	//check the shader for successful compilation
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &infolog_length);
	if ( infolog_length > 0 ){
		std::vector<char> error_msg(infolog_length+1);
		glGetShaderInfoLog(shader_id, infolog_length, NULL, &error_msg[0]);
		printf("%s\n", &error_msg[0]);
	}
}


//load the shader programs and return the id
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	//create the vertex and fragment shaders
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	//read the shader and fragment programs into string
	std::string vertex_shader_code = readSourceFile(vertex_file_path);
	if(vertex_shader_code == ""){
		return 0; //failed because empty string
	}
	printf("Compiling Vertex shader : %s\n", vertex_file_path);
	CompileShader(vertex_shader_code, vertex_shader_id);

	//load the fragment program (optional)
	std::string fragment_shader_code = readSourceFile(fragment_file_path);
	if(fragment_shader_code == ""){
		return 0; //failed: empty string
	}

	//compile the fragment shader
	printf("Compiling Fragment shader : %s\n", fragment_file_path);
	CompileShader(fragment_shader_code, fragment_shader_id);

	GLint result = GL_FALSE;
	int infolog_length;

	//link the program
	printf("Linking program\n");
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);

	//check the program and ensure that the program is linked properly
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &infolog_length);
	if ( infolog_length > 0 ){
		std::vector<char> program_error_msg(infolog_length+1);
		glGetProgramInfoLog(program_id, infolog_length, NULL, &program_error_msg[0]);
		printf("%s\n", &program_error_msg[0]);
	}else{
		printf("Linked Successfully\n");
	}

	//flag for delete, and will free all memories
	//when the attached program is deleted
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}


