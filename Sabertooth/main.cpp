// ============================================================================
// main.cpp  –  Ponto de entrada do Gorillas 3D
// ============================================================================

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <clocale> // UTF‑8 no terminal
#ifdef _WIN32
#include <windows.h> // para SetConsoleOutputCP
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.h"
#include "Geometry.h"
#include "Game.h"

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                      Constantes de Janela / App                          ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
constexpr int WIN_WIDTH = 800;
constexpr int WIN_HEIGHT = 600;
constexpr char TITLE[] = "Gorillas 3D – Universidade";

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                  Substituto simples para std::clamp                       ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static inline float clampf(float v, float mn, float mx)
{
	return (v < mn ? mn : (v > mx ? mx : v));
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                         Handles OpenGL globais                            ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
GLuint bgVAO = 0, bgVBO = 0;
GLuint cubeVAO = 0, cubeVBO = 0;
GLuint sphereVAO = 0, sphereVBO = 0;
GLsizei sphereVertexCount = 0;

GLuint texBG = 0, texBuilding = 0, texP1 = 0, texP2 = 0;

Shader *gShader = nullptr;

struct
{
	GLint model, view, proj;
	GLint useColor, overrideColor, blur, sampler;
} uni;

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                       Utilitário de Carregamento de Textura               ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static GLuint loadTexture(const char *path)
{
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Filtragem e repetição
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int w, h, c;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(path, &w, &h, &c, STBI_rgb_alpha);
	if (!data)
	{
		std::cerr << "Falha ao carregar " << path << "\n";
		return 0;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	return id;
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                     Inicialização da Janela e OpenGL                      ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static GLFWwindow *createWindow()
{
	if (!glfwInit())
	{
		std::cerr << "Erro ao iniciar GLFW.\n";
		return nullptr;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *win = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, TITLE, nullptr, nullptr);
	if (!win)
	{
		std::cerr << "Erro ao criar janela.\n";
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(win);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Erro ao inicializar GLEW.\n";
		return nullptr;
	}

	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return win;
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                    Construção da Geometria (VAOs/VBOs)                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static void buildGeometry()
{
	// Quad de fundo
	float bg[] = {
			-10, -1, -0.9, 0, 0, 10, -1, -0.9, 1, 0, 10, 10, -0.9, 1, 1,
			10, 10, -0.9, 1, 1, -10, 10, -0.9, 0, 1, -10, -1, -0.9, 0, 0};
	glGenVertexArrays(1, &bgVAO);
	glGenBuffers(1, &bgVBO);
	glBindVertexArray(bgVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bg), bg, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Cubo texturizado
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texturedCubeVertices), texturedCubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Esfera (projétil/explosão)
	std::vector<float> sp = generateSphereVertices(0.2f, 16, 16);
	sphereVertexCount = (GLsizei)(sp.size() / 6);
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sp.size() * sizeof(float), sp.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                          Carregamento de Texturas                         ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static void loadAllTextures()
{
	texBG = loadTexture("city_bg.jpg");
	texBuilding = loadTexture("building_texture_2.jpg");
	texP1 = loadTexture("player1_texture.png");
	texP2 = loadTexture("player2_texture.png");
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                           Setup do Shader                                 ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static void createShader()
{
	const char *vs = R"(
        #version 440 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec2 aUV;
        out vec2 vUV;
        uniform mat4 model, view, projection;
        void main() {
            vUV = aUV;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

	const char *fs = R"(
        #version 440 core
        in vec2 vUV;
        out vec4 FragColor;
        uniform sampler2D tex;
        uniform bool useColor;
        uniform bool blur;
        uniform vec3 overrideColor;

        vec3 applyBlur(vec2 uv) {
            vec2 sz = vec2(textureSize(tex, 0));
            vec2 off = 1.0 / sz;
            vec3 col = vec3(0.0);
            for(int y = -1; y <= 1; ++y)
                for(int x = -1; x <= 1; ++x)
                    col += texture(tex, uv + vec2(x, y)*off).rgb;
            return col / 9.0;
        }

        void main() {
            if (useColor) {
                FragColor = vec4(overrideColor, 1.0);
                return;
            }

            vec4 texel = texture(tex, vUV);

            // Transparência real – descarta fundo branco
            if (texel.a < 0.1) discard;

            vec3 rgb = blur ? applyBlur(vUV) : texel.rgb;
            FragColor = vec4(rgb, texel.a);
        }
    )";

	gShader = new Shader(vs, fs);
	gShader->use();

	uni.model = glGetUniformLocation(gShader->getProgramID(), "model");
	uni.view = glGetUniformLocation(gShader->getProgramID(), "view");
	uni.proj = glGetUniformLocation(gShader->getProgramID(), "projection");
	uni.useColor = glGetUniformLocation(gShader->getProgramID(), "useColor");
	uni.overrideColor = glGetUniformLocation(gShader->getProgramID(), "overrideColor");
	uni.blur = glGetUniformLocation(gShader->getProgramID(), "blur");
	uni.sampler = glGetUniformLocation(gShader->getProgramID(), "tex");
	glUniform1i(uni.sampler, 0);

	// Matriz de câmera fixa
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 proj = glm::ortho(-10.0f, 10.0f, -1.0f, 10.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(uni.view, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uni.proj, 1, GL_FALSE, glm::value_ptr(proj));
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                       Entrada de Usuário (Teclado)                        ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static void processInput(GLFWwindow *win, float dt)
{
	const float move = 0.005f;

	// Movimento horizontal somente para o jogador da vez
	if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
		(currentPlayer == 1 ? p1.pos.x : p2.pos.x) -= move;
	if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
		(currentPlayer == 1 ? p1.pos.x : p2.pos.x) += move;

	// Limites de faixa
	if (currentPlayer == 1)
		p1.pos.x = clampf(p1.pos.x, -10.0f, -6.0f);
	else
		p2.pos.x = clampf(p2.pos.x, 6.0f, 9.0f);

	bool changed = false;

	// Ajuste de força e ângulo
	auto adjust = [&](int key, float &var, float inc, float mn, float mx)
	{
		if (glfwGetKey(win, key) == GLFW_PRESS)
		{
			var += inc;
			var = clampf(var, mn, mx);
			changed = true;
		}
	};
	adjust(GLFW_KEY_UP, power, +0.02f, 1.0f, 20.0f);
	adjust(GLFW_KEY_DOWN, power, -0.02f, 1.0f, 20.0f);
	adjust(GLFW_KEY_LEFT, angleDeg, -0.2f, 0.0f, 90.0f);
	adjust(GLFW_KEY_RIGHT, angleDeg, +0.2f, 0.0f, 90.0f);

	if (changed)
		std::cout << "Angle=" << angleDeg << "  Force=" << power << "\n";

	// Disparo
	if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS && !inFlight)
	{
		inFlight = true;
		flightTime = 0.0f;
		if (currentPlayer == 1)
			launchPositionP1 = {p1.pos.x + 0.5f, p1.pos.y + 0.5f};
		else
			launchPositionP2 = {p2.pos.x + 0.5f, p2.pos.y + 0.5f};

		std::cout << "DISPARO do Player " << currentPlayer
							<< " | Angulo=" << angleDeg << " Forca=" << power << "\n";
	}

	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(win, true);
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                         Funções de Desenho Auxiliares                     ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
static void drawQuad(bool blur)
{
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(bgVAO);
	glUniform1i(uni.useColor, GL_FALSE);
	glUniform1i(uni.blur, blur);
	glBindTexture(GL_TEXTURE_2D, texBG);

	glm::mat4 m(1.0f);
	glUniformMatrix4fv(uni.model, 1, GL_FALSE, glm::value_ptr(m));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
}

static void drawCube(const glm::vec2 &pos, const glm::vec2 &size, GLuint tex)
{
	glBindVertexArray(cubeVAO);
	glUniform1i(uni.useColor, GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, tex);

	glm::mat4 m(1.0f);
	m = glm::translate(m, {pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, 0.0f});
	m = glm::scale(m, {size.x, size.y, 1.0f});
	glUniformMatrix4fv(uni.model, 1, GL_FALSE, glm::value_ptr(m));
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void drawSphere(const glm::vec2 &center, float scale, const glm::vec3 &color)
{
	glBindVertexArray(sphereVAO);
	glUniform1i(uni.useColor, GL_TRUE);
	glUniform3fv(uni.overrideColor, 1, &color[0]);

	glm::mat4 m(1.0f);
	m = glm::translate(m, {center.x, center.y, 0.0f});
	m = glm::scale(m, {scale, scale, scale});
	glUniformMatrix4fv(uni.model, 1, GL_FALSE, glm::value_ptr(m));
	glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);

	glUniform1i(uni.useColor, GL_FALSE); // restaura
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                                   main                                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
int main()
{
	// Consoles Windows precisam de setlocale + CP_UTF8 para acentos
	std::setlocale(LC_ALL, "");
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif

	GLFWwindow *window = createWindow();
	if (!window)
		return -1;

	createShader();
	buildGeometry();
	loadAllTextures();
	initGame();

	std::cout << "Controles:\n"
						<< "[A/D] mover | Left/Right ajusta Angulo | Up/Down ajusta Forca | Espaco dispara\n";

	float lastTime = (float)glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		float currTime = (float)glfwGetTime();
		float dt = currTime - lastTime;
		lastTime = currTime;

		processInput(window, dt);

		// Atualiza lógica de jogo
		updateProjectile(dt);
		updateExplosion(dt);

		// Desenha
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawQuad(true);									// fundo com blur
		for (const auto &b : buildings) // prédios
			drawCube(b.pos, b.size, texBuilding);
		drawCube(p1.pos, p1.size, texP1); // jogadores
		drawCube(p2.pos, p2.size, texP2);
		drawSphere({projectileX, projectileY}, 1.0f, {1, 1, 1}); // projétil
		if (showExplosion)
		{
			float t = explosionTime / explosionDuration;
			float r = 0.2f + 1.3f * t;
			drawSphere({explosionX, explosionY}, r, {1.0f, 1.0f - t, 0.0f});
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete gShader;
	glfwTerminate();
	return 0;
}
