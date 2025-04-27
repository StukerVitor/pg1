# Projeto "Gorillas 2D" – Documentação Técnica Explicativa

> **Autores:** Vitor Stüker e Guilherme Nogueira

> **Disciplina:** Processamento Gráfico: Computação Gráfica e Aplicações

---

Visão Geral do Projeto

O Gorillas 3D é um jogo baseado em OpenGL onde dois jogadores alternam turnos lançando projéteis, ajustando ângulo e força para acertar o oponente, evitando prédios no cenário. O fluxo do programa inicia no main.cpp, que configura a janela OpenGL, shaders, geometrias e texturas, processa entradas do usuário e renderiza a cena. A lógica do jogo está em Game.h/cpp, calculando a trajetória do projétil com equações balísticas baseadas em ângulo, força e gravidade. Uma câmera ortográfica fixa e transformações de modelo (translação e escala) posicionam cubos texturizados (jogadores e prédios) e esferas coloridas (projétil e explosão). Colisões são detectadas usando caixas delimitadoras 2D alinhadas aos eixos, verificando impactos com prédios ou o oponente. O design modular separa renderização (Shader), geração de geometria (Geometry) e lógica do jogo (Game), utilizando structs para simplicidade e uma classe Shader para renderização eficiente.

## Sumário Completo

1. [Fluxo de Execução da Aplicação](#fluxo-de-execução-da-aplicação)
2. [Descrição Minuciosa dos Arquivos‑Fonte](#descrição-minuciosa-dos-arquivos‑fonte)  
   &nbsp;&nbsp;2.1 [`Geometry.h` e `Geometry.cpp`](#geometry)  
   &nbsp;&nbsp;2.2 [`Shader.h` e `Shader.cpp`](#shader)  
   &nbsp;&nbsp;2.3 [`Game.h` e `Game.cpp`](#game)  
   &nbsp;&nbsp;2.4 [`main.cpp`](#maincpp)
3. [Controle de Entrada do Usuário](#controle-de-entrada-do-usuário)
4. [Pipeline de Renderização OpenGL](#pipeline-de-renderização-opengl)
5. [Física do Projétil e Sistema de Colisões](#física-do-projétil-e-sistema-de-colisões)
6. [Processo de Compilação e Execução Passo a Passo](#processo-de-compilação-e-execução-passo-a-passo)
7. [Sugestões de Expansão e Trabalhos Futuros](#sugestões-de-expansão-e-trabalhos-futuros)

## Fluxo de Execução da Aplicação

```mermaid
graph TD
    Start([Início do programa]) --> InitWindow[createWindow()] --> InitGL[createShader() → buildGeometry() → loadAllTextures()]
    InitGL --> GameInit[initGame() – posiciona jogadores e prédios]
    GameInit --> Loop{Laço principal}
    Loop --> Input[processInput() – lê teclado]
    Input --> Physics[updateProjectile() + updateExplosion()]
    Physics --> Render[Desenha fundo, prédios, jogadores, projétil, explosão]
    Render --> Loop
    Loop -->|ESC ou janela fechada| Shutdown[Limpeza de recursos e término]
```

O laço permanece ativo até que o usuário pressione **Escape** ou feche a janela gráfica.

---

## Descrição Minuciosa dos Arquivos‑Fonte

### <a id="geometry"></a>2.1 `Geometry.h` e `Geometry.cpp`

**Responsabilidade**: Criar as malhas estáticas (cubo texturizado) e procedurais (esfera colorida) que serão enviadas para a placa de vídeo.

#### `texturedCubeVertices`

- São **trinta e seis** vértices (seis faces × dois triângulos × três vértices).
- Cada vértice possui **cinco** valores de ponto flutuante: `x`, `y`, `z`, `u` e `v`.
- O cubo encontra‑se centrado na origem, com aresta igual a `1.0`. Durante o desenho aplicamos escalonamento para transformá‑lo em prédios ou em avatares dos jogadores.

#### `generateSphereVertices(float raio, int stacks, int slices)`

- Implementa duas subdivisões aninhadas que percorrem **parâmetros esféricos** `phi` (latitude) e `theta` (longitude).
- Cada quadrilateral em latitude/longitude é decomposto em **dois** triângulos.
- Retorna um `std::vector<float>` intercalando posição espacial (`x`,`y`,`z`) e cor (`r`,`g`,`b`), dispensando objeto de índice.
- A cor varia suavemente para criar uma percepção de profundidade mesmo sem iluminação.

### <a id="shader"></a>2.2 `Shader.h` e `Shader.cpp`

- `Shader` é uma classe utilitária que **esconde** os detalhes de compilação e linkagem.
- A função estática `createShaderProgram` compila, verifica erros e remove os objetos de shader após o `glLinkProgram`.
- `checkCompileErrors` faz distinção entre falhas de **estágio de shader** e **programa** completo, imprimindo logs detalhados.
- Os **uniform locations** são obtidos em `main.cpp` para evitar chamadas repetitivas a `glGetUniformLocation`.

### <a id="game"></a>2.3 `Game.h` e `Game.cpp`

- Define as estruturas `Building` e `Player`, cada qual contendo **posição**, **tamanho** e, no caso do jogador, **pontuação**.
- Variáveis globais como `projectileX`, `projectileY`, `angleDeg`, `power` e `gravity` armazenam o estado dinâmico do jogo.
- Funções de maior relevância:
  - `initGame()` – insere três prédios fixos e posiciona os avatares nos extremos do cenário.
  - `updateProjectile(float deltaTime)` – resolve a trajetória segundo as equações de movimento retilíneo uniformemente variado no eixo vertical.
  - `checkCollisionBB(...)` – verificação de **Caixa Delimitadora Alinhada aos Eixos** (Axis Aligned Bounding Box) para detectar sobreposição entre o projétil, prédios ou jogadores.
  - `nextTurn()` – alterna o jogador ativo e reposiciona a banana.
  - `triggerExplosion()` – inicializa a animação de expansão da esfera laranja representando a explosão.

### <a id="maincpp"></a>2.4 `main.cpp`

`main.cpp` é segmentado por comentários‑caixa claramente visíveis, facilitando a navegação.

#### Seções principais

| Seção                                       | Conteúdo detalhado                                                                                                                                         |
| ------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Constantes de janela**                    | Largura, altura, título e função auxiliar `clampf` que substitui `std::clamp` para evitar cabeçalho adicional.                                             |
| **Atributos OpenGL globais**                | Identificadores de **Vertex Array Objects** e **Vertex Buffer Objects** para o fundo, cubo e esfera.                                                       |
| `loadTexture()`                             | Carrega imagem com **stb_image**, converte para `GL_RGBA`, gera mipmap e define filtros de minimização e magnificação.                                     |
| `createWindow()`                            | Inicializa GLFW, define a versão do contexto OpenGL, ativa `GLEW`, habilita **teste de profundidade** e **mistura de transparência**.                      |
| `buildGeometry()`                           | Preenche cada VAO/VBO com seus respectivos vértices. Note que a esfera é construída **em tempo de execução** através da função descrita em `Geometry.cpp`. |
| `createShader()`                            | Declara **vertex shader** e **fragment shader** como literais de sequência _raw_ (`R"(`) para evitar arquivos externos, agilizando testes em laboratório.  |
| `processInput(GLFWwindow*, dt)`             | Gerencia todas as teclas de controle, limitando faixa de movimento e valores de força e ângulo com `clampf`.                                               |
| Blocos `drawQuad`, `drawCube`, `drawSphere` | Funções ponte para aplicar `model matrix` específica antes de renderizar cada entidade.                                                                    |
| Laço principal                              | Sequência: entrada → atualização → limpeza de buffers → desenho → `glfwSwapBuffers` e `glfwPollEvents`.                                                    |

---

## Controle de Entrada do Usuário

| Tecla física             | Ação específica                                                       |
| ------------------------ | --------------------------------------------------------------------- |
| **A**                    | Move o jogador ativo para a esquerda dentro de sua metade do cenário. |
| **D**                    | Move o jogador ativo para a direita dentro de sua metade do cenário.  |
| **Seta para a esquerda** | Diminui o ângulo de arremesso (limite inferior 0 grau).               |
| **Seta para a direita**  | Aumenta o ângulo de arremesso (limite superior 90 graus).             |
| **Seta para cima**       | Incrementa a força de disparo (máximo 20 unidades arbitrárias).       |
| **Seta para baixo**      | Decrementa a força de disparo (mínimo 1 unidade).                     |
| **Barra de espaço**      | Arremessa a banana se ela não estiver em voo.                         |
| **Escape**               | Sinaliza encerramento do jogo.                                        |

---

## Pipeline de Renderização OpenGL

1. **Configuração das matrizes**: O projeto utiliza projeção **ortográfica** com valores simétricos para facilitar cálculos de colisão (comparações em coordenadas mundo).
2. **Desenho do fundo**: Feito primeiro, com o teste de profundidade desabilitado para evitar descartes acidentais.
3. **Renderização dos prédios e jogadores**: Cubos texturizados; a textura do predio é compartilhada, porém cada jogador recebe textura própria.
4. **Projétil e Explosão**: Esferas coloridas desenhadas com `useColor = true`; a cor da explosão é animada do amarelo ao vermelho conforme o tempo decorrido.
5. **Efeito de desfoque (blur)**: Implementado no _fragment shader_ via amostragem 3 × 3 sobre o `sampler2D`, aplicado somente à textura de fundo.

---

## Física do Projétil e Sistema de Colisões

### Equações de Movimento Utilizadas

Dados:

- `v0` = `power` (magnitude da velocidade inicial).
- `θ` = `angleDeg` (conversão para radianos em tempo de execução).
- `g` = `gravity` (constante 9,8 m/s²).
- `t` = `flightTime` (tempo acumulado em segundos).

Posições:

```math
x(t) = x_0 ± v_0 · \cos(θ) · t

y(t) = y_0 + v_0 · \sin(θ) · t - \frac{1}{2} · g · t^2
```

O sinal de `±` depende de qual jogador está arremessando: **Jogador 1** dispara para a direita; **Jogador 2** dispara para a esquerda.

### Colisões com Caixa Delimitadora Alinhada

Para cada entidade potencial, calcula‑se:

```math
\text{left}   = center.x - half.x
```

```math
\text{right}  = center.x + half.x
```

```math
\text{bottom} = center.y - half.y
```

```math
\text{top}    = center.y + half.y
```

A colisão ocorre quando **nenhuma** das quatro condições de separação é verdadeira:

1. `rightA < leftB`
2. `leftA  > rightB`
3. `topA   < bottomB`
4. `bottomA> topB`

Caso haja sobreposição, dispara‑se a função `triggerExplosion()` e o turno é trocado.

---
