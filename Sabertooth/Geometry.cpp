#include "Geometry.h"
#include <array>
#include <cmath>
/*
------------------------------------------------------------------------------
 Geometry.cpp  –  Define o cubo texturizado e gera esferas coloridas.
------------------------------------------------------------------------------*/

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                         Cubo Texturizado (array)                          ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/* O cubo precisa de 6 faces; cada face = 2 triângulos; cada triângulo = 3
	 vértices. 6 × 2 × 3 = 36 vértices. Para cada vértice armazenamos 5 floats:
	 posição XYZ + coord. de textura UV. */
float texturedCubeVertices[36 * 5] = {
		// Face traseira (Z = -0.5)
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		// Face frontal (Z = +0.5)
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		// Face esquerda (X = -0.5)
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		// Face direita (X = +0.5)
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		// Face inferior (Y = -0.5)
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		// Face superior (Y = +0.5)
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                     Geração Procedural de Esfera                          ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

std::vector<float> generateSphereVertices(float radius, int stacks, int slices)
{
	std::vector<float> v;
	v.reserve(stacks * slices * 36); // estimativa
	const float PI = 3.14159265358979323846f;

	for (int i = 0; i < stacks; ++i)
	{
		const float phi1 = PI * i / stacks;
		const float phi2 = PI * (i + 1) / stacks;

		for (int j = 0; j < slices; ++j)
		{
			const float theta1 = 2 * PI * j / slices;
			const float theta2 = 2 * PI * (j + 1) / slices;

			// Função lambda para converter esfera → cartesiano.
			auto polarToXYZ = [&](float phi, float theta)
			{
				const float x = radius * std::sin(phi) * std::cos(theta);
				const float y = radius * std::cos(phi);
				const float z = radius * std::sin(phi) * std::sin(theta);
				return std::array<float, 3>{x, y, z}; };

			const auto P1 = polarToXYZ(phi1, theta1);
			const auto P2 = polarToXYZ(phi2, theta1);
			const auto P3 = polarToXYZ(phi2, theta2);
			const auto P4 = polarToXYZ(phi1, theta2);

			// Cor varia suavemente com latitude/longitude.
			const float c1 = static_cast<float>(i) / stacks;
			const float c2 = static_cast<float>(j) / slices;
			const float R = 0.8f * c1 + 0.2f;
			const float G = 0.6f * (1.0f - c2) + 0.4f;
			const float B = 0.5f * c2 + 0.2f;

			auto pushVertex = [&](const std::array<float, 3> &P)
			{ v.insert(v.end(), {P[0], P[1], P[2], R, G, B}); };

			// Triângulo 1: P1 → P2 → P3
			pushVertex(P1);
			pushVertex(P2);
			pushVertex(P3);
			// Triângulo 2: P1 → P3 → P4
			pushVertex(P1);
			pushVertex(P3);
			pushVertex(P4);
		}
	}
	return v;
}