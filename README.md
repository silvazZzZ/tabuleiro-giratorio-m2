# Labirinto Giratório – M2 (Evolução)

**Universidade do Vale do Itajaí – Escola Politécnica**  
**Curso de Ciência da Computação**  
**Algoritmos e Programação II (22817)**

# Equipe

**Irivam da Silva Junior**

## Versão de desenvolvimento

- **Sistema:** Windows
- **Compilador:** g++ (C++11 ou superior)
- **Compilação:**
  ```bash
  g++ -o labirinto labirinto.cpp -std=c++11
  ```
- **Execução:** a partir da pasta que contém os arquivos `mapaN.csv` e `mapaN.sol`
  ```bash
  ./labirinto
  ```

## Formato do arquivo de jogo salvo (`jogo_salvo.csv`)

O arquivo é escrito pelo jogo ao interromper a partida com ESC. Formato (uma informação por linha, exceto o cenário):

```
n
numeroMapa
orientacao          (0..3)
movimentos
rotacoes
jogador.lin jogador.col
sobJogador          (conteúdo da célula sob o jogador: 0,4,5,6,7…)
titulo
linha0 do cenário (valores separados por vírgula, incluindo o código 2 do jogador)
linha1
…
linha(n-1)
```

O campo `sobJogador` é necessário porque a representação guarda o jogador “por cima” da célula (alavanca, saída, porta etc.). Sem ele, ao sair da alavanca a célula seria perdida.

Se o arquivo estiver ausente, vazio ou corrompido, “Continuar” trata como “sem jogo salvo” e não trava o programa. Ao vencer a fase o arquivo é apagado (`remove`).

## Tabela antes/depois (M1 → M2) – pelo menos 5 funções

| Função M1 | Assinatura M1 | Assinatura M2 | Motivo da mudança |
|-----------|---------------|---------------|-------------------|
| Andar | `void Andar(int mapa[][TAM], int &linha, int &coluna, int &casaAnterior, char tecla, int orientacao, bool &venceu, bool &moveu)` | `void moverJogador(Jogo &j, char tecla, bool &venceu, bool &moveu)` | Todo o estado da partida viaja na struct `Jogo`; evita lista longa de parâmetros soltos. |
| Gravidade | `void Gravidade(int mapa[][TAM], bool &perdeu)` | `void aplicarGravidade(Jogo &j, bool &perdeu)` | Usa `j.n` e `j.cenario`; dimensão variável. |
| Rotacionar | `void Rotacionar(int mapa[][TAM], int &orientacao, int &linha, int &coluna, int &casaAnterior, char tecla, bool &perdeu, bool &girou)` | `void girarDireita(Jogo &j)` / `void girarEsquerda(Jogo &j)` + `processaTecla` | Rotação e atualização da posição do jogador ficam encapsuladas; orientação passa a 0..3. |
| MontarMapa | `void MontarMapa(int mapa[][TAM], int numero)` | `bool carregarMapa(int numero, Jogo &j, string &motivo)` | Mapas lidos de arquivo CSV, validados, com título e dimensão variáveis. |
| Jogar | `void Jogar(int mapa[][TAM], int numero, int &orientacao, …)` | `void jogarPartida(Jogo &j, bool &partidaAtiva)` | Recebe a struct; usa a mesma `processaTecla` da reprodução de soluções. |
| PortaTrancada | `bool PortaTrancada(int tipo, int orientacao)` | `bool portaTrancada(int tipo, int orientacao)` | Mantida com parâmetros simples (orientação 0..3 em vez de graus). |

## Defeitos da M1 corrigidos

1. Estado da partida espalhado em dezenas de parâmetros → centralizado na struct `Jogo`.
2. Mapas hardcoded no código → arquivos `.csv` descobertos em tempo de execução.
3. “Continuar” só em memória → persistido em `jogo_salvo.csv`.
4. Soluções apenas no papel → arquivos `.sol` verificáveis pelo próprio jogo.
5. Sem histórico de vitórias → ranking em `recordes.csv`.
6. Dimensão fixa 11×11 → cada mapa com `n` de 11 até `TAM` (21).
7. Orientação em graus (0/90/180/270) → 0,1,2,3 (múltiplos de 90°), conforme especificação M2.

## Decisões de projeto

1. **Passagem por valor vs referência**  
   Funções que alteram o estado (`aplicarGravidade`, `girarDireita`, `moverJogador`, `processaTecla`) recebem `Jogo &`. Funções que só consultam (`mapaEstavel`, `desenharCenario`, `podePassar`) recebem `const Jogo &` ou por valor quando uma cópia é desejada (ex.: `mapaEstavel` aplica gravidade sobre cópia). Copiar a struct inteira (matriz 21×21) é barato o suficiente para validação e BFS de dica, e garante isolamento.

2. **Posição do jogador vs conteúdo da matriz**  
   Mantemos `jogador` (linha/coluna) e o código `2` na matriz. A posição evita varredura a cada movimento; a matriz é a fonte de verdade visual e para gravidade/rotação. O campo extra `sobJogador` evita inconsistência ao pisar em alavanca/saída/porta.

3. **O que gravar no jogo salvo**  
   Grava-se o cenário completo no estado atual, orientação, posição, `sobJogador`, contadores, número e título do mapa. Nada é recalculado na carga. Gravação ocorre **somente ao sair com ESC** (não a cada tecla), para não prejudicar desempenho e para corresponder à intenção de “partida interrompida”.

4. **Mesmas regras para jogar e reproduzir**  
   Toda tecla passa por `processaTecla(Jogo &, char)`. Tanto o loop do teclado quanto a reprodução de `.sol` (com e sem desenho) usam essa única função. Nenhuma regra está duplicada.

5. **Critério de desempate no ranking**  
   Menor número de movimentos; em empate, menor número de rotações. Motivo: o objetivo principal é chegar com o mínimo de passos; rotações são o recurso “especial” e empatam em segundo critério. A troca de registros usa a função template `trocar`.

## Mapas

| Arquivo | Título | Dimensão | Rot. mín. na solução |
|---------|--------|----------|----------------------|
| mapa1.csv | Primeiro desafio | 11×11 | 1 |
| mapa2.csv | Porta e bloco | 11×11 | 1 |
| mapa3.csv | Duas alavancas | 11×11 | 2 |
| mapa4.csv | Corredor que exige giro | 15×15 | 1 |
| mapa5.csv | Tres giros | 11×11 | 3 |

Todos possuem `.sol` correspondente e passam em “Verificar todos os mapas”.

## Desafios extras implementados

Nenhum neste momento (opcional).

## Menu (M2)

1. Novo jogo  
2. Continuar  
3. Ver solução (inclui “Verificar todos os mapas”)  
4. Recordes  
5. Sobre  
6. Fim  
