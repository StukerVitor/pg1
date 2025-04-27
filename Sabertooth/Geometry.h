#pragma once
/*
------------------------------------------------------------------------------
 Geometry.h  –  Prototipa buffers de geometria genérica (cubo texturizado,
				esfera colorida) usados em toda a aplicação.
------------------------------------------------------------------------------*/

#include <vector>

// Array global – 36 vértices, cada um com 5 floats (x,y,z,u,v).
extern float texturedCubeVertices[36 * 5];

/**
 * @brief Gera vértices de uma esfera simples (malha triangulada).
 * @param radius  Raio da esfera
 * @param stacks  Número de divisões ao longo da latitude
 * @param slices  Número de divisões ao longo da longitude
 * @return        Vetor intercalado (x,y,z,r,g,b) por vértice
 */
std::vector<float> generateSphereVertices(float radius, int stacks, int slices);