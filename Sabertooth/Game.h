#pragma once
/*
------------------------------------------------------------------------------
 Game.h  –  Declara as estruturas e funções centrais da lógica de jogo
------------------------------------------------------------------------------
 Neste arquivo definimos:
	 • Estruturas (struct) que representam Prédios e Jogadores;
	 • Variáveis globais que guardam o estado do jogo (posição do projétil,
	 pontuação, explosão etc.);
	 • Funções de controle: inicialização, troca de turno, detecção de colisões,
	 atualização do projétil e da explosão.

 A ideia é manter **toda** a lógica do jogo isolada deste cabeçalho + Game.cpp,
 deixando main.cpp focado somente em inicializar OpenGL, tratar entrada de
 usuário e desenhar.
------------------------------------------------------------------------------*/

#include <glm/glm.hpp>
#include <vector>

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                               Estruturas                                  ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/// Representa um prédio: posição da base (canto inferior esquerdo) e tamanho.
struct Building
{
	glm::vec2 pos;	///< posição X,Y da base
	glm::vec2 size; ///< largura e altura
};

/// Representa um jogador: posição, tamanho e pontuação.
struct Player
{
	glm::vec2 pos;	///< canto inferior esquerdo do cubo que representa o corpo
	glm::vec2 size; ///< largura e altura (usamos cubo 3D porém achatado no eixo Z)
	int score = 0;	///< pontos marcados ao acertar o adversário
};

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                           Variáveis Globais                               ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

// Posição atual do projétil (uma esfera branca).
extern float projectileX;
extern float projectileY;
extern bool inFlight;		 ///< indica se o projétil está voando.
extern float angleDeg;	 ///< ângulo de lançamento em graus.
extern float power;			 ///< força de lançamento, unidade arbitrária.
extern float gravity;		 ///< aceleração da gravidade (para baixo).
extern float flightTime; ///< tempo que o projétil já está em voo.

// Controle de turno: 1 ou 2.
extern int currentPlayer;

// Posições de lançamento, atualizadas toda vez que o jogador se move.
extern glm::vec2 launchPositionP1;
extern glm::vec2 launchPositionP2;

// Instâncias globais dos jogadores.
extern Player p1;
extern Player p2;

// Vetor que guarda **todos** os prédios do cenário.
extern std::vector<Building> buildings;

// Variáveis que controlam a animação de explosão.
extern bool showExplosion;
extern float explosionTime;
extern float explosionDuration;
extern float explosionX;
extern float explosionY;

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                         Funções – Protótipos                              ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/// Configura o cenário inicial (prédios, jogadores, projétil).
void initGame();

/// Reposiciona o projétil junto ao jogador atual.
void resetProjectile();

/// Alterna o controle para o outro jogador.
void nextTurn();

/// Ativa a animação de explosão em (x, y).
void triggerExplosion(float x, float y);

/// Verifica sobreposição de duas caixas delimitadoras 2D (eixo‐alinhadas).
bool checkCollisionBB(const glm::vec2 &center1, const glm::vec2 &size1,
											const glm::vec2 &center2, const glm::vec2 &size2);

/// Calcula nova posição do projétil e verifica colisões.
void updateProjectile(float deltaTime);

/// Atualiza o tempo de vida da explosão.
void updateExplosion(float deltaTime);