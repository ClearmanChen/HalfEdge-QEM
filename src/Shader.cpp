#include "Shader.h"
#include <glm/fwd.hpp>
#include <glm/fwd.hpp>

Shader::Shader(const GLchar* vertexShaderPath, const GLchar* fragmentShaderPath)
{
	// 1. Read shader source code from file
	std::string vertexShaderContent, fragmentShaderContent;
	std::ifstream fVertexShader, fFragementShader;

	fVertexShader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fFragementShader.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		std::stringstream ssVertexShader, ssFragmentShader;

		fVertexShader.open(vertexShaderPath);
		fFragementShader.open(fragmentShaderPath);
		ssVertexShader << fVertexShader.rdbuf();
		ssFragmentShader << fFragementShader.rdbuf();
		fVertexShader.close();
		fFragementShader.close();
		vertexShaderContent = ssVertexShader.str();
		fragmentShaderContent = ssFragmentShader.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "ERROR::SHADER::FILE_READ_FAILED" << std::endl;
	}

	// 2. Create shader
	const GLchar* vertexShaderCode = vertexShaderContent.c_str();
	const GLchar* fragmentShaderCode = fragmentShaderContent.c_str();

	GLint success_flag;
	GLchar info[512];

	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success_flag);
	if (!success_flag)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, info);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << std::endl
			<< info << std::endl;
	}

	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success_flag);
	if (!success_flag)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, info);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << std::endl
			<< info << std::endl;
	}

	// 3. Create shader program and compile
	this->shaderProgram = glCreateProgram();
	glAttachShader(this->shaderProgram, vertexShader);
	glAttachShader(this->shaderProgram, fragmentShader);
	glLinkProgram(this->shaderProgram);
	glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &success_flag);
	if (!success_flag)
	{
		glGetProgramInfoLog(this->shaderProgram, 512, NULL, info);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
			<< info << std::endl;
	}

	// 4. Cleaning shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
	glDeleteProgram(this->shaderProgram);
}

void Shader::active()
{
	glUseProgram(this->shaderProgram);
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(shaderProgram, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}