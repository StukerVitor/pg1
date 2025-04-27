#pragma once
/*
------------------------------------------------------------------------------
 Shader.h  –  Classe utilitária para compilar, linkar e usar Shaders GLSL.
------------------------------------------------------------------------------*/

#include <GL/glew.h>
#include <string>

class Shader
{
public:
	Shader(const char *vertexSrc, const char *fragmentSrc);
	~Shader();

	/// Ativa (glUseProgram) este programa.
	void use();

	/// Acesso ao ID bruto – útil para definir uniforms.
	[[nodiscard]] GLuint getProgramID() const;

private:
	GLuint programID{0};

	/// Cria, compila e linka o programa.
	static GLuint createShaderProgram(const char *vs, const char *fs);

	/// Verifica e mostra logs de erro (compilação ou linkagem).
	static void checkCompileErrors(GLuint shader, const std::string &stage);
};