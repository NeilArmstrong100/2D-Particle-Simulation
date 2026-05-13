#include "video.h"
#include "../utility/utility.h"

#include <glad/glad.h>

#include <print>

using namespace engine::renderer;

// Shader
Shader::Shader(const char* vsp, const char* fsp)
{
	id = glCreateProgram();

	int  success;
	char info_log[512];

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string source = read_file(vsp);
	const char* c_source = source.c_str();
	glShaderSource(vertex_shader, 1, &c_source, nullptr);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
		std::println("Failed to compile vertex shader:\n {}", info_log);
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	source = read_file(fsp);
	c_source = source.c_str();
	glShaderSource(fragment_shader, 1, &c_source, nullptr);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
		std::println("Failed to compile fragment shader:\n {}", info_log);
	}

	glAttachShader(id, vertex_shader);
	glAttachShader(id, fragment_shader);
	glLinkProgram(id);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void Shader::use() const
{
	glUseProgram(id);
}

void Shader::del() const
{
	glDeleteProgram(id);
}

// VAO
VAO::VAO()
{
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);
}

void VAO::bind() const
{
	glBindVertexArray(id);
}

void VAO::unbind() const
{
	glBindVertexArray(0);
}

void VAO::del() const
{
	glDeleteVertexArrays(1, &id);
}

// VBO
VBO::VBO(long long int size, float* vertices)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);
}

void VBO::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VBO::unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::del() const
{
	glDeleteBuffers(1, &id);
}