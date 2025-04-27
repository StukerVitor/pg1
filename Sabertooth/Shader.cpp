#include "Shader.h"
#include <iostream>
/*
------------------------------------------------------------------------------
 Shader.cpp  –  Implementação da classe utilitária de shader.
------------------------------------------------------------------------------*/

Shader::Shader(const char *vs, const char *fs)
{
	programID = createShaderProgram(vs, fs);
}

Shader::~Shader()
{
	glDeleteProgram(programID);
}

void Shader::use() { glUseProgram(programID); }

GLuint Shader::getProgramID() const { return programID; }

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                    Funções Estáticas de Implementação                     ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

GLuint Shader::createShaderProgram(const char *vs, const char *fs)
{
	auto compile = [&](GLenum type, const char *src)
	{
		GLuint id = glCreateShader(type);
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);
		checkCompileErrors(id, (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"));
		return id;
	};

	GLuint vert = compile(GL_VERTEX_SHADER, vs);
	GLuint frag = compile(GL_FRAGMENT_SHADER, fs);

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);
	checkCompileErrors(prog, "PROGRAM");

	glDeleteShader(vert);
	glDeleteShader(frag);
	return prog;
}

void Shader::checkCompileErrors(GLuint object, const std::string &stage)
{
	GLint ok;
	if (stage != "PROGRAM")
	{
		glGetShaderiv(object, GL_COMPILE_STATUS, &ok);
		if (!ok)
		{
			char log[1024];
			glGetShaderInfoLog(object, 1024, nullptr, log);
			std::cerr << "[Shader] Erro de compilação (" << stage << ")\n"
								<< log << "\n";
		}
	}
	else
	{
		glGetProgramiv(object, GL_LINK_STATUS, &ok);
		if (!ok)
		{
			char log[1024];
			glGetProgramInfoLog(object, 1024, nullptr, log);
			std::cerr << "[Shader] Erro de linkagem\n"
								<< log << "\n";
		}
	}
}