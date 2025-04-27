#include "Game.h"
#include <iostream>
#include <cmath>
/*
------------------------------------------------------------------------------
 Game.cpp  –  Implementa a lógica de jogo declarada em Game.h
------------------------------------------------------------------------------*/

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                      Definição das Variáveis Globais                      ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

float projectileX = 0.0f;
float projectileY = 0.0f;
bool inFlight = false;
float angleDeg = 45.0f;
float power = 5.0f;
float gravity = 9.8f; ///< força para baixo
float flightTime = 0.0f;

int currentPlayer = 1; ///< começa com Jogador 1
glm::vec2 launchPositionP1(-8.0f, 1.5f);
glm::vec2 launchPositionP2(8.0f, 1.5f);

Player p1{glm::vec2(-8.5f, 1.0f), glm::vec2(1.0f, 1.0f), 0};
Player p2{glm::vec2(8.0f, 1.0f), glm::vec2(1.0f, 1.0f), 0};

std::vector<Building> buildings; ///< prédios são inseridos em initGame()

// Explosão
bool showExplosion = false;
float explosionTime = 0.0f;
float explosionDuration = 0.5f;
float explosionX = 0.0f;
float explosionY = 0.0f;

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                             Funções Auxiliares                            ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * @brief Converte graus em radianos (wrapper para glm::radians).
 */
static float degToRad(float deg) { return glm::radians(deg); }

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                          Implementação das Funções                        ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

void initGame()
{
	/* Cria três prédios – poderíamos gerar aleatoriamente; usamos valores
		 fixos para simplicidade. A posição é a base, então altura sobe em Y. */
	buildings.push_back({{-3.0f, 0.0f}, {2.0f, 3.0f}}); // prédio 1
	buildings.push_back({{0.0f, 0.0f}, {2.0f, 5.0f}});	// prédio 2
	buildings.push_back({{3.0f, 0.0f}, {2.0f, 4.0f}});	// prédio 3

	resetProjectile();
}

void resetProjectile()
{
	inFlight = false;
	flightTime = 0.0f;

	const glm::vec2 start = (currentPlayer == 1) ? launchPositionP1
																							 : launchPositionP2;
	projectileX = start.x;
	projectileY = start.y;
}

void nextTurn()
{
	currentPlayer = (currentPlayer == 1) ? 2 : 1;
	resetProjectile();
	std::cout << "Agora é a vez do Jogador " << currentPlayer << "!\n";
}

void triggerExplosion(float x, float y)
{
	showExplosion = true;
	explosionTime = 0.0f;
	explosionX = x;
	explosionY = y;
}

bool checkCollisionBB(const glm::vec2 &center1, const glm::vec2 &size1,
											const glm::vec2 &center2, const glm::vec2 &size2)
{
	// Calcula extents (metade da largura/altura) de cada caixa.
	const glm::vec2 half1 = size1 * 0.5f;
	const glm::vec2 half2 = size2 * 0.5f;

	const float leftA = center1.x - half1.x;
	const float rightA = center1.x + half1.x;
	const float bottomA = center1.y - half1.y;
	const float topA = center1.y + half1.y;

	const float leftB = center2.x - half2.x;
	const float rightB = center2.x + half2.x;
	const float bottomB = center2.y - half2.y;
	const float topB = center2.y + half2.y;

	// Se A está completamente à esquerda de B OU completamente à direita -> sem colisão
	if (rightA < leftB || leftA > rightB)
		return false;
	// Se A está completamente abaixo de B OU acima -> sem colisão
	if (topA < bottomB || bottomA > topB)
		return false;

	return true; // caso contrário as caixas se sobrepõem
}

void updateProjectile(float dt)
{
	// Se projétil ainda não foi disparado, ele acompanha o jogador atual.
	if (!inFlight)
	{
		glm::vec2 base = (currentPlayer == 1) ? p1.pos : p2.pos;
		projectileX = base.x + 0.5f; // meio do cubo do jogador
		projectileY = base.y + 0.5f;
		return; // nada mais a fazer nesta chamada
	}

	// Avança tempo de voo
	flightTime += dt;

	// Converte ângulo para radianos e define direção (p1 atira → direita, p2 → esquerda).
	const float rad = degToRad(angleDeg);
	const float dir = (currentPlayer == 1) ? +1.0f : -1.0f;

	// Posição inicial no instante do disparo.
	const glm::vec2 start = (currentPlayer == 1) ? launchPositionP1
																							 : launchPositionP2;

	// Equações de movimento (sem resistência do ar):
	projectileX = start.x + dir * power * std::cos(rad) * flightTime;
	projectileY = start.y + power * std::sin(rad) * flightTime - 0.5f * gravity * flightTime * flightTime;

	// ------------------------------
	// 1. Colisão com prédios
	// ------------------------------
	for (const Building &b : buildings)
	{
		glm::vec2 centerB = b.pos + b.size * 0.5f;
		glm::vec2 sizeB = b.size;
		glm::vec2 projCenter(projectileX, projectileY);
		glm::vec2 projSize(0.4f); // esfera ≈ caixa de 0.4×0.4

		if (checkCollisionBB(centerB, sizeB, projCenter, projSize))
		{
			std::cout << "Colidiu em um prédio!\n";
			triggerExplosion(projectileX, projectileY);
			nextTurn();
			return; // encerra a verificação – projétil já parou
		}
	}

	// ------------------------------
	// 2. Colisão com o oponente
	// ------------------------------
	Player &target = (currentPlayer == 1) ? p2 : p1;
	glm::vec2 centerT = target.pos + target.size * 0.5f;
	glm::vec2 sizeT = target.size;
	glm::vec2 projCenter(projectileX, projectileY);
	glm::vec2 projSize(0.4f);

	if (checkCollisionBB(centerT, sizeT, projCenter, projSize))
	{
		std::cout << "Acertou o Jogador " << ((currentPlayer == 1) ? 2 : 1) << "!\n";
		(currentPlayer == 1 ? p1.score : p2.score)++;
		std::cout << "Placar -> P1=" << p1.score << "  P2=" << p2.score << "\n";
		triggerExplosion(projectileX, projectileY);
		nextTurn();
		return;
	}

	// ------------------------------
	// 3. Saiu da arena?
	// ------------------------------
	const bool offMap = (projectileX < -12.0f || projectileX > 12.0f ||
											 projectileY < -5.0f || projectileY > 15.0f);
	if (offMap)
	{
		std::cout << "Projétil saiu do mapa.\n";
		nextTurn();
	}
}

void updateExplosion(float dt)
{
	if (!showExplosion)
		return;

	explosionTime += dt;
	if (explosionTime >= explosionDuration)
		showExplosion = false; // encerra animação
}