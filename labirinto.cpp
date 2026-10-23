/*
 * Labirinto Giratorio - M2 (Evolucao)
 * Algoritmos e Programacao II - UNIVALI
 *
 * Compilar no Windows:
 * g++ main.cpp -std=c++11 -o labirinto.exe
 *
 * Executar:
 * .\labirinto.exe
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cctype>
#include <conio.h>
#include <windows.h>

using namespace std;

// ==================== CONSTANTES ====================

const int TAM = 21;
const int MAX_RECORDES = 200;

const int VAZIO    = 0;
const int PAREDE   = 1;
const int JOGADOR  = 2;
const int BLOCO    = 3;
const int ALAVANCA = 4;
const int SAIDA    = 5;
const int PORTA_A  = 6;
const int PORTA_B  = 7;

// ==================== STRUCTS ====================

struct Posicao {
    int lin, col;
};

struct Jogo {
    int cenario[TAM][TAM];
    int n;
    int orientacao;
    Posicao jogador;
    int sobJogador;
    int movimentos, rotacoes;
    int numeroMapa;
    string titulo;
};

struct Recorde {
    string nome;
    int mapa, movimentos, rotacoes;
};

// ==================== TEMPLATE TROCAR ====================

template <typename T>
void trocar(T &a, T &b) {
    T temp = a;
    a = b;
    b = temp;
}

// ==================== TECLADO / TELA ====================

int lerTecla() {
    return _getch();
}

void pausa() {
    cout << "\nPressione uma tecla para continuar...";
    lerTecla();
}

void limpar() {
    system("cls");
}

void esperarMs(int ms) {
    Sleep(ms);
}

// ==================== REGRAS DO JOGO ====================

bool portaTrancada(int tipo, int orientacao) {
    if (tipo == PORTA_A)
        return orientacao == 0 || orientacao == 2;

    if (tipo == PORTA_B)
        return orientacao == 1 || orientacao == 3;

    return false;
}

bool podePassar(const Jogo &j, int lin, int col) {
    if (lin < 0 || lin >= j.n || col < 0 || col >= j.n)
        return false;

    int casa = j.cenario[lin][col];

    if (casa == PAREDE || casa == BLOCO)
        return false;

    if (casa == PORTA_A || casa == PORTA_B)
        return !portaTrancada(casa, j.orientacao);

    return true;
}

bool seguraBloco(const Jogo &j, int lin, int col) {
    if (lin < 0 || lin >= j.n || col < 0 || col >= j.n)
        return true;

    int casa = j.cenario[lin][col];

    return casa == PAREDE ||
           casa == BLOCO ||
           casa == SAIDA ||
           casa == PORTA_A ||
           casa == PORTA_B;
}

void aplicarGravidade(Jogo &j, bool &perdeu) {
    perdeu = false;

    bool caiu = true;

    while (caiu) {
        caiu = false;

        for (int i = j.n - 2; i >= 0; i--) {
            for (int k = 0; k < j.n; k++) {
                if (j.cenario[i][k] == BLOCO) {
                    if (j.cenario[i + 1][k] == JOGADOR) {
                        perdeu = true;
                        return;
                    }

                    if (!seguraBloco(j, i + 1, k)) {
                        j.cenario[i + 1][k] = BLOCO;
                        j.cenario[i][k] = VAZIO;
                        caiu = true;
                    }
                }
            }
        }
    }
}

void copiarJogo(const Jogo &origem, Jogo &destino) {
    destino.n = origem.n;
    destino.orientacao = origem.orientacao;
    destino.jogador = origem.jogador;
    destino.sobJogador = origem.sobJogador;
    destino.movimentos = origem.movimentos;
    destino.rotacoes = origem.rotacoes;
    destino.numeroMapa = origem.numeroMapa;
    destino.titulo = origem.titulo;

    for (int i = 0; i < origem.n; i++) {
        for (int k = 0; k < origem.n; k++) {
            destino.cenario[i][k] = origem.cenario[i][k];
        }
    }
}

bool mapaEstavel(Jogo j) {
    Jogo copia;
    copiarJogo(j, copia);

    bool perdeu = false;
    aplicarGravidade(copia, perdeu);

    for (int i = 0; i < j.n; i++) {
        for (int k = 0; k < j.n; k++) {
            if (copia.cenario[i][k] != j.cenario[i][k])
                return false;
        }
    }

    return true;
}

void girarDireita(Jogo &j) {
    int aux[TAM][TAM];

    for (int i = 0; i < j.n; i++) {
        for (int k = 0; k < j.n; k++) {
            aux[k][j.n - 1 - i] = j.cenario[i][k];
        }
    }

    for (int i = 0; i < j.n; i++) {
        for (int k = 0; k < j.n; k++) {
            j.cenario[i][k] = aux[i][k];
        }
    }

    int novaLin = j.jogador.col;
    int novaCol = j.n - 1 - j.jogador.lin;

    j.jogador.lin = novaLin;
    j.jogador.col = novaCol;

    j.orientacao = (j.orientacao + 1) % 4;
}

void girarEsquerda(Jogo &j) {
    int aux[TAM][TAM];

    for (int i = 0; i < j.n; i++) {
        for (int k = 0; k < j.n; k++) {
            aux[j.n - 1 - k][i] = j.cenario[i][k];
        }
    }

    for (int i = 0; i < j.n; i++) {
        for (int k = 0; k < j.n; k++) {
            j.cenario[i][k] = aux[i][k];
        }
    }

    int novaLin = j.n - 1 - j.jogador.col;
    int novaCol = j.jogador.lin;

    j.jogador.lin = novaLin;
    j.jogador.col = novaCol;

    j.orientacao = (j.orientacao + 3) % 4;
}

void moverJogador(Jogo &j, char tecla, bool &venceu, bool &moveu) {
    venceu = false;
    moveu = false;

    int novaLin = j.jogador.lin;
    int novaCol = j.jogador.col;

    if (tecla == 'W' || tecla == 'w')
        novaLin--;
    else if (tecla == 'S' || tecla == 's')
        novaLin++;
    else if (tecla == 'A' || tecla == 'a')
        novaCol--;
    else if (tecla == 'D' || tecla == 'd')
        novaCol++;
    else
        return;

    if (!podePassar(j, novaLin, novaCol))
        return;

    int destino = j.cenario[novaLin][novaCol];

    j.cenario[j.jogador.lin][j.jogador.col] = j.sobJogador;
    j.sobJogador = destino;

    j.cenario[novaLin][novaCol] = JOGADOR;

    j.jogador.lin = novaLin;
    j.jogador.col = novaCol;

    moveu = true;

    if (destino == SAIDA)
        venceu = true;
}

int processaTecla(Jogo &j, char tecla) {
    if (tecla == 'R' || tecla == 'r')
        return 5;

    if (tecla == 27 || tecla == 'X' || tecla == 'x')
        return 6;

    bool venceu = false;
    bool moveu = false;
    bool perdeu = false;

    if (tecla == 'Q' || tecla == 'q' ||
        tecla == 'E' || tecla == 'e') {

        if (j.sobJogador != ALAVANCA)
            return 0;

        if (tecla == 'E' || tecla == 'e')
            girarDireita(j);
        else
            girarEsquerda(j);

        aplicarGravidade(j, perdeu);

        if (perdeu)
            return 4;

        j.rotacoes++;
        return 2;
    }

    if (tecla == 'W' || tecla == 'w' ||
        tecla == 'A' || tecla == 'a' ||
        tecla == 'S' || tecla == 's' ||
        tecla == 'D' || tecla == 'd') {

        moverJogador(j, tecla, venceu, moveu);

        if (moveu) {
            j.movimentos++;

            if (venceu)
                return 3;

            return 1;
        }

        return 0;
    }

    return -1;
}

// ==================== DESENHO ====================

void desenharCenario(const Jogo &j) {
    for (int i = 0; i < j.n; i++) {
        for (int k = 0; k < j.n; k++) {
            switch (j.cenario[i][k]) {
                case VAZIO:
                    cout << "  ";
                    break;

                case PAREDE:
                    cout << "# ";
                    break;

                case JOGADOR:
                    cout << "@ ";
                    break;

                case BLOCO:
                    cout << "O ";
                    break;

                case ALAVANCA:
                    cout << "A ";
                    break;

                case SAIDA:
                    cout << "S ";
                    break;

                case PORTA_A:
                    cout << (portaTrancada(PORTA_A, j.orientacao)
                             ? "= " : ": ");
                    break;

                case PORTA_B:
                    cout << (portaTrancada(PORTA_B, j.orientacao)
                             ? "| " : "; ");
                    break;

                default:
                    cout << "? ";
                    break;
            }
        }

        cout << endl;
    }
}

void desenharStatus(const Jogo &j) {
    int graus = j.orientacao * 90;

    cout << "Mapa: " << j.numeroMapa
         << " | " << j.titulo
         << " | Direcao: " << graus << " graus"
         << " | Movimentos: " << j.movimentos
         << " | Rotacoes: " << j.rotacoes << endl;
}

void desenharTelaJogo(const Jogo &j) {
    limpar();

    cout << "========================================\n";
    cout << "          LABIRINTO GIRATORIO\n";
    cout << "========================================\n";

    desenharStatus(j);
    cout << endl;

    desenharCenario(j);

    cout << "\nWASD: mover | Q/E: girar | R: reiniciar | ESC: menu\n";
    cout << "Comando: ";
}

// ==================== ARQUIVOS ====================

bool carregarMapa(int numero, Jogo &j, string &motivo) {
    motivo = "";

    string nomeArquivo = "mapa" + to_string(numero) + ".csv";
    ifstream arq(nomeArquivo.c_str());

    if (!arq.is_open()) {
        motivo = "arquivo inexistente";
        return false;
    }

    string linha;

    if (!getline(arq, linha)) {
        motivo = "dimensao ausente";
        arq.close();
        return false;
    }

    stringstream ss(linha);
    string token;

    if (!(ss >> token)) {
        motivo = "dimensao ausente";
        arq.close();
        return false;
    }

    int n;

    try {
        n = stoi(token);
    } catch (...) {
        motivo = "dimensao nao numerica";
        arq.close();
        return false;
    }

    if (n < 11 || n > TAM) {
        motivo = "dimensao fora do intervalo 11.." + to_string(TAM);
        arq.close();
        return false;
    }

    if (!getline(arq, linha)) {
        motivo = "titulo ausente";
        arq.close();
        return false;
    }

    if (!linha.empty() && linha.back() == '\r')
        linha.pop_back();

    if (linha.empty()) {
        motivo = "titulo ausente";
        arq.close();
        return false;
    }

    string titulo = linha;

    int cenarioTemp[TAM][TAM];
    int qtdJogador = 0;
    int qtdSaida = 0;

    Posicao posJogador = {-1, -1};

    for (int i = 0; i < n; i++) {
        if (!getline(arq, linha)) {
            motivo = "menos linhas do que o informado";
            arq.close();
            return false;
        }

        if (!linha.empty() && linha.back() == '\r')
            linha.pop_back();

        stringstream ssLinha(linha);
        string cel;
        int col = 0;

        while (getline(ssLinha, cel, ',')) {
            if (col >= n) {
                motivo = "linha com quantidade de colunas diferente de n";
                arq.close();
                return false;
            }

            int valor;

            try {
                valor = stoi(cel);
            } catch (...) {
                motivo = "valor nao numerico";
                arq.close();
                return false;
            }

            if (valor < 0 || valor > 7) {
                motivo = "valor fora do intervalo 0..7";
                arq.close();
                return false;
            }

            cenarioTemp[i][col] = valor;

            if (valor == JOGADOR) {
                qtdJogador++;
                posJogador.lin = i;
                posJogador.col = col;
            }

            if (valor == SAIDA)
                qtdSaida++;

            col++;
        }

        if (col != n) {
            motivo = "linha com quantidade de colunas diferente de n";
            arq.close();
            return false;
        }
    }

    arq.close();

    if (qtdJogador != 1) {
        motivo = "quantidade de jogadores diferente de 1";
        return false;
    }

    if (qtdSaida < 1) {
        motivo = "ausencia de saida";
        return false;
    }

    j.n = n;
    j.titulo = titulo;
    j.numeroMapa = numero;
    j.orientacao = 0;
    j.movimentos = 0;
    j.rotacoes = 0;
    j.jogador = posJogador;
    j.sobJogador = VAZIO;

    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            j.cenario[i][k] = cenarioTemp[i][k];
        }
    }

    if (!mapaEstavel(j)) {
        motivo = "mapa instavel (bloco no ar)";
        return false;
    }

    return true;
}

int descobrirQuantidadeMapas() {
    int qtd = 0;

    while (true) {
        string nome = "mapa" + to_string(qtd + 1) + ".csv";
        ifstream arq(nome.c_str());

        if (!arq.is_open())
            break;

        arq.close();
        qtd++;
    }

    return qtd;
}

bool carregarSolucao(int numero, string &solucao, string &motivo) {
    motivo = "";

    string nome = "mapa" + to_string(numero) + ".sol";
    ifstream arq(nome.c_str());

    if (!arq.is_open()) {
        motivo = "arquivo .sol inexistente";
        return false;
    }

    if (!getline(arq, solucao)) {
        motivo = "arquivo .sol vazio";
        arq.close();
        return false;
    }

    if (!solucao.empty() && solucao.back() == '\r')
        solucao.pop_back();

    string limpa;

    for (size_t i = 0; i < solucao.size(); i++) {
        if (solucao[i] != ' ' && solucao[i] != '\t')
            limpa += solucao[i];
    }

    solucao = limpa;
    arq.close();

    if (solucao.empty()) {
        motivo = "arquivo .sol vazio";
        return false;
    }

    return true;
}

int executarSolucaoSemDesenho(int numeroMapa, string &mensagem) {
    Jogo j;
    string motivo;

    if (!carregarMapa(numeroMapa, j, motivo)) {
        mensagem = "mapa invalido: " + motivo;
        return 4;
    }

    string solucao;

    if (!carregarSolucao(numeroMapa, solucao, motivo)) {
        mensagem = motivo;
        return 4;
    }

    for (size_t i = 0; i < solucao.size(); i++) {
        char t = solucao[i];
        int r = processaTecla(j, t);

        if (r == -1) {
            mensagem = "tecla invalida na posicao " +
                       to_string(i + 1) + " ('" +
                       string(1, t) + "')";
            return 3;
        }

        if (r == 4) {
            mensagem = "derrota na tecla " + to_string(i + 1);
            return 2;
        }

        if (r == 3) {
            mensagem = "solucao valida | teclas: " +
                       to_string(solucao.size()) +
                       " | movimentos: " + to_string(j.movimentos) +
                       " | rotacoes: " + to_string(j.rotacoes);
            return 0;
        }
    }

    mensagem = "teclas acabaram e o jogador nao chegou a saida";
    return 1;
}

bool salvarJogo(const Jogo &j) {
    ofstream arq("jogo_salvo.csv");

    if (!arq.is_open())
        return false;

    arq << j.n << endl;
    arq << j.numeroMapa << endl;
    arq << j.orientacao << endl;
    arq << j.movimentos << endl;
    arq << j.rotacoes << endl;
    arq << j.jogador.lin << " " << j.jogador.col << endl;
    arq << j.sobJogador << endl;
    arq << j.titulo << endl;

    for (int i = 0; i < j.n; i++) {
        for (int k = 0; k < j.n; k++) {
            if (k > 0)
                arq << ",";

            arq << j.cenario[i][k];
        }

        arq << endl;
    }

    arq.close();
    return true;
}

bool carregarJogoSalvo(Jogo &j) {
    ifstream arq("jogo_salvo.csv");

    if (!arq.is_open())
        return false;

    string linha;

    try {
        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        j.n = stoi(linha);

        if (j.n < 11 || j.n > TAM) {
            arq.close();
            return false;
        }

        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        j.numeroMapa = stoi(linha);

        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        j.orientacao = stoi(linha);

        if (j.orientacao < 0 || j.orientacao > 3) {
            arq.close();
            return false;
        }

        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        j.movimentos = stoi(linha);

        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        j.rotacoes = stoi(linha);

        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        stringstream ss(linha);

        if (!(ss >> j.jogador.lin >> j.jogador.col)) {
            arq.close();
            return false;
        }

        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        j.sobJogador = stoi(linha);

        if (!getline(arq, linha)) {
            arq.close();
            return false;
        }

        if (!linha.empty() && linha.back() == '\r')
            linha.pop_back();

        j.titulo = linha;

        for (int i = 0; i < j.n; i++) {
            if (!getline(arq, linha)) {
                arq.close();
                return false;
            }

            if (!linha.empty() && linha.back() == '\r')
                linha.pop_back();

            stringstream ssLinha(linha);
            string cel;
            int col = 0;

            while (getline(ssLinha, cel, ',')) {
                if (col >= j.n) {
                    arq.close();
                    return false;
                }

                j.cenario[i][col] = stoi(cel);
                col++;
            }

            if (col != j.n) {
                arq.close();
                return false;
            }
        }
    } catch (...) {
        arq.close();
        return false;
    }

    arq.close();
    return true;
}

bool existeJogoSalvoValido() {
    Jogo temp;
    return carregarJogoSalvo(temp);
}

void invalidarJogoSalvo() {
    remove("jogo_salvo.csv");
}

// ==================== RECORDES ====================

bool registrarRecorde(const string &nome, int mapa, int mov, int rot) {
    ofstream arq("recordes.csv", ios_base::app);

    if (!arq.is_open())
        return false;

    arq << nome << "," << mapa << "," << mov << "," << rot << endl;

    arq.close();
    return true;
}

int carregarRecordes(Recorde vet[], int max) {
    ifstream arq("recordes.csv");

    if (!arq.is_open())
        return 0;

    int qtd = 0;
    string linha;

    while (qtd < max && getline(arq, linha)) {
        if (!linha.empty() && linha.back() == '\r')
            linha.pop_back();

        if (linha.empty())
            continue;

        stringstream ss(linha);
        string nome, sMapa, sMov, sRot;

        if (!getline(ss, nome, ','))
            continue;

        if (!getline(ss, sMapa, ','))
            continue;

        if (!getline(ss, sMov, ','))
            continue;

        if (!getline(ss, sRot, ','))
            continue;

        try {
            vet[qtd].nome = nome;
            vet[qtd].mapa = stoi(sMapa);
            vet[qtd].movimentos = stoi(sMov);
            vet[qtd].rotacoes = stoi(sRot);
            qtd++;
        } catch (...) {
            continue;
        }
    }

    arq.close();
    return qtd;
}

void ordenarRecordes(Recorde vet[], int qtd) {
    for (int i = 0; i < qtd - 1; i++) {
        for (int k = 0; k < qtd - 1 - i; k++) {
            bool trocaNecessaria = false;

            if (vet[k].movimentos > vet[k + 1].movimentos) {
                trocaNecessaria = true;
            } else if (
                vet[k].movimentos == vet[k + 1].movimentos &&
                vet[k].rotacoes > vet[k + 1].rotacoes
            ) {
                trocaNecessaria = true;
            }

            if (trocaNecessaria)
                trocar(vet[k], vet[k + 1]);
        }
    }
}

// ==================== FLUXO DO JOGO ====================

void reiniciarPartida(Jogo &j) {
    string motivo;
    carregarMapa(j.numeroMapa, j, motivo);
}

void pedirNomeERegistrar(const Jogo &j) {
    cout << "\nDigite seu nome (ate 15 caracteres, sem virgula) e pressione Enter:\n> ";

    string nome;
    getline(cin, nome);

    string limpo;

    for (size_t i = 0; i < nome.size() && limpo.size() < 15; i++) {
        if (nome[i] != ',')
            limpo += nome[i];
    }

    if (limpo.empty())
        limpo = "Jogador";

    if (registrarRecorde(limpo, j.numeroMapa, j.movimentos, j.rotacoes))
        cout << "Recorde registrado!\n";
    else
        cout << "Erro ao registrar o recorde.\n";

    pausa();
}

void jogarPartida(Jogo &j, bool &partidaAtiva) {
    while (true) {
        desenharTelaJogo(j);

        char tecla = (char)lerTecla();

        int r = processaTecla(j, tecla);

        if (r == 6) {
            salvarJogo(j);

            cout << "\nJogo salvo. Voltando ao menu...\n";

            pausa();
            break;
        }

        if (r == 5) {
            reiniciarPartida(j);
            continue;
        }

        if (r == 4) {
            limpar();

            cout << "========================================\n";
            cout << "         VOCE FOI ESMAGADO!\n";
            cout << "========================================\n";
            cout << "Um bloco caiu sobre o jogador.\n";

            pausa();
            reiniciarPartida(j);
            continue;
        }

        if (r == 3) {
            limpar();

            cout << "========================================\n";
            cout << "              PARABENS!\n";
            cout << "========================================\n\n";

            desenharCenario(j);

            cout << "\nVoce completou o mapa " << j.numeroMapa << "!\n";
            cout << "Movimentos: " << j.movimentos << endl;
            cout << "Rotacoes: " << j.rotacoes << endl;

            invalidarJogoSalvo();
            pedirNomeERegistrar(j);

            partidaAtiva = false;
            break;
        }
    }
}

void reproduzirSolucaoComDesenho(int numeroMapa) {
    Jogo j;
    string motivo;

    if (!carregarMapa(numeroMapa, j, motivo)) {
        cout << "Mapa invalido: " << motivo << endl;
        pausa();
        return;
    }

    string solucao;

    if (!carregarSolucao(numeroMapa, solucao, motivo)) {
        cout << motivo << endl;
        pausa();
        return;
    }

    for (size_t i = 0; i < solucao.size(); i++) {
        desenharTelaJogo(j);

        cout << "\nReproduzindo: tecla " << (i + 1)
             << "/" << solucao.size()
             << " -> '" << solucao[i] << "'\n";

        esperarMs(400);

        int r = processaTecla(j, solucao[i]);

        if (r == -1) {
            cout << "\nTecla invalida na posicao "
                 << (i + 1) << endl;

            pausa();
            return;
        }

        if (r == 4) {
            limpar();
            desenharCenario(j);

            cout << "\nDerrota durante a reproducao na tecla "
                 << (i + 1) << endl;

            pausa();
            return;
        }

        if (r == 3) {
            limpar();

            cout << "========================================\n";
            cout << "         SOLUCAO VALIDA!\n";
            cout << "========================================\n\n";

            desenharCenario(j);

            cout << "\nTotal de teclas: " << solucao.size() << endl;
            cout << "Movimentos: " << j.movimentos << endl;
            cout << "Rotacoes: " << j.rotacoes << endl;

            pausa();
            return;
        }
    }

    limpar();
    desenharCenario(j);

    cout << "\nAs teclas acabaram e o jogador nao chegou a saida.\n";

    pausa();
}

// ==================== MENUS ====================

void menuPrincipal() {
    limpar();

    cout << "========================================\n";
    cout << "       LABIRINTO GIRATORIO - M2\n";
    cout << "========================================\n";
    cout << "1 - Novo jogo\n";
    cout << "2 - Continuar\n";
    cout << "3 - Ver solucao\n";
    cout << "4 - Recordes\n";
    cout << "5 - Sobre\n";
    cout << "6 - Fim\n";
    cout << "========================================\n";
}

void opcaoSobre() {
    limpar();

    cout << "========================================\n";
    cout << "                 SOBRE\n";
    cout << "========================================\n";
    cout << "Equipe:\n";
    cout << "  Irivam da Silva Junior\n";
    cout << "  (complete com os nomes dos colegas)\n\n";
    cout << "Disciplina: Algoritmos e Programacao II\n";
    cout << "Professor: (nome do professor)\n";
    cout << "Mes/Ano: 2025/2026\n\n";
    cout << "OBJETIVO\n";
    cout << "Chegue ate a saida (S) utilizando rotacoes\n";
    cout << "do cenario e a gravidade dos blocos.\n\n";
    cout << "COMANDOS\n";
    cout << "  W A S D  - movimentar\n";
    cout << "  Q / E    - girar o cenario (sobre alavanca)\n";
    cout << "  R        - reiniciar a fase\n";
    cout << "  ESC      - salvar e voltar ao menu\n\n";
    cout << "NOVAS OPCOES DO MENU (M2)\n";
    cout << "  Ver solucao  - reproduz a sequencia que resolve o mapa\n";
    cout << "  Recordes     - top 5 de cada mapa\n";
    cout << "  Continuar    - retoma jogo salvo em arquivo\n";
    cout << "========================================\n";

    pausa();
}

void opcaoNovoJogo(Jogo &j, bool &partidaAtiva) {
    if (existeJogoSalvoValido()) {
        cout << "\nExiste um jogo salvo. Deseja substitui-lo? (S/N): ";

        char conf = (char)lerTecla();
        cout << conf << endl;

        if (conf != 'S' && conf != 's') {
            cout << "Operacao cancelada.\n";
            pausa();
            return;
        }

        invalidarJogoSalvo();
    }

    int qtd = descobrirQuantidadeMapas();

    if (qtd == 0) {
        cout << "\nNenhum mapa encontrado (mapa1.csv, mapa2.csv, ...).\n";
        pausa();
        return;
    }

    limpar();

    cout << "========================================\n";
    cout << "              NOVO JOGO\n";
    cout << "========================================\n";
    cout << "Mapas disponiveis:\n\n";

    bool valido[100] = {false};
    string motivos[100];
    string titulos[100];
    int validosCount = 0;

    for (int i = 1; i <= qtd; i++) {
        Jogo temp;
        string motivo;

        if (i >= 100)
            break;

        if (carregarMapa(i, temp, motivo)) {
            valido[i] = true;
            titulos[i] = temp.titulo;
            validosCount++;

            cout << "  " << i << " - " << temp.titulo
                 << " (" << temp.n << "x" << temp.n << ")\n";
        } else {
            valido[i] = false;
            motivos[i] = motivo;

            cout << "  " << i << " - [INDISPONIVEL] "
                 << motivo << "\n";
        }
    }

    cout << "  0 - Mapa aleatorio (entre os validos)\n\n";
    cout << "Opcao: ";

    char tecla = (char)lerTecla();
    cout << tecla << endl;

    int escolha = tecla - '0';
    int numeroEscolhido = -1;

    if (escolha == 0) {
        if (validosCount == 0) {
            cout << "Nenhum mapa valido.\n";
            pausa();
            return;
        }

        int idx = rand() % validosCount;
        int cont = 0;

        for (int i = 1; i <= qtd && i < 100; i++) {
            if (valido[i]) {
                if (cont == idx) {
                    numeroEscolhido = i;
                    break;
                }

                cont++;
            }
        }
    } else if (escolha >= 1 && escolha <= qtd && escolha < 100) {
        if (!valido[escolha]) {
            cout << "Mapa indisponivel: " << motivos[escolha] << endl;
            pausa();
            return;
        }

        numeroEscolhido = escolha;
    } else {
        cout << "Opcao invalida.\n";
        pausa();
        return;
    }

    string motivo;

    if (!carregarMapa(numeroEscolhido, j, motivo)) {
        cout << "Erro ao carregar: " << motivo << endl;
        pausa();
        return;
    }

    partidaAtiva = true;
    jogarPartida(j, partidaAtiva);
}

void opcaoContinuar(Jogo &j, bool &partidaAtiva) {
    if (!carregarJogoSalvo(j)) {
        cout << "\nNao existe jogo salvo valido.\n";
        pausa();
        return;
    }

    partidaAtiva = true;

    cout << "\nJogo carregado. Retomando...\n";

    pausa();
    jogarPartida(j, partidaAtiva);
}

void opcaoVerSolucao() {
    int qtd = descobrirQuantidadeMapas();

    if (qtd == 0) {
        cout << "\nNenhum mapa encontrado.\n";
        pausa();
        return;
    }

    limpar();

    cout << "========================================\n";
    cout << "            VER SOLUCAO\n";
    cout << "========================================\n";

    for (int i = 1; i <= qtd; i++) {
        Jogo temp;
        string motivo;

        if (carregarMapa(i, temp, motivo)) {
            cout << "  " << i << " - " << temp.titulo << "\n";
        } else {
            cout << "  " << i << " - [invalido] "
                 << motivo << "\n";
        }
    }

    cout << "  0 - Verificar TODOS os mapas (sem desenhar)\n\n";
    cout << "Opcao: ";

    char tecla = (char)lerTecla();
    cout << tecla << endl;

    int escolha = tecla - '0';

    if (escolha == 0) {
        limpar();

        cout << "========================================\n";
        cout << "       VERIFICAR TODOS OS MAPAS\n";
        cout << "========================================\n\n";

        for (int i = 1; i <= qtd; i++) {
            string msg;
            int r = executarSolucaoSemDesenho(i, msg);

            Jogo temp;
            string motivo;
            string titulo = "?";

            if (carregarMapa(i, temp, motivo))
                titulo = temp.titulo;

            cout << "Mapa " << i << " (" << titulo << "): ";

            if (r == 0)
                cout << "OK - " << msg << endl;
            else
                cout << "FALHOU - " << msg << endl;
        }

        pausa();
        return;
    }

    if (escolha < 1 || escolha > qtd) {
        cout << "Opcao invalida.\n";
        pausa();
        return;
    }

    reproduzirSolucaoComDesenho(escolha);
}

void opcaoRecordes() {
    int qtdMapas = descobrirQuantidadeMapas();

    if (qtdMapas == 0) {
        cout << "\nNenhum mapa encontrado.\n";
        pausa();
        return;
    }

    limpar();

    cout << "========================================\n";
    cout << "              RECORDES\n";
    cout << "========================================\n";
    cout << "Escolha o mapa (1.." << qtdMapas << "): ";

    char tecla = (char)lerTecla();
    cout << tecla << endl;

    int mapa = tecla - '0';

    if (mapa < 1 || mapa > qtdMapas) {
        cout << "Mapa invalido.\n";
        pausa();
        return;
    }

    Recorde vet[MAX_RECORDES];
    int total = carregarRecordes(vet, MAX_RECORDES);

    Recorde filtrados[MAX_RECORDES];
    int qtd = 0;

    for (int i = 0; i < total; i++) {
        if (vet[i].mapa == mapa)
            filtrados[qtd++] = vet[i];
    }

    if (qtd == 0) {
        cout << "\nNenhum recorde para o mapa " << mapa << ".\n";
        pausa();
        return;
    }

    ordenarRecordes(filtrados, qtd);

    limpar();

    cout << "========================================\n";
    cout << "     TOP 5 - MAPA " << mapa << "\n";
    cout << "========================================\n";
    cout << "Pos  Nome            Mov  Rot\n";
    cout << "----------------------------------------\n";

    int mostrar = (qtd < 5) ? qtd : 5;

    for (int i = 0; i < mostrar; i++) {
        printf("%2d   %-15s %4d %4d\n",
               i + 1,
               filtrados[i].nome.c_str(),
               filtrados[i].movimentos,
               filtrados[i].rotacoes);
    }

    cout << "========================================\n";

    pausa();
}

// ==================== MAIN ====================

int main() {
    srand((unsigned)time(NULL));

    Jogo jogo;
    bool partidaAtiva = false;
    int opcao;

    do {
        menuPrincipal();

        cout << "Opcao: ";

        char tecla = (char)lerTecla();
        cout << tecla << endl;

        opcao = tecla - '0';

        switch (opcao) {
            case 1:
                opcaoNovoJogo(jogo, partidaAtiva);
                break;

            case 2:
                opcaoContinuar(jogo, partidaAtiva);
                break;

            case 3:
                opcaoVerSolucao();
                break;

            case 4:
                opcaoRecordes();
                break;

            case 5:
                opcaoSobre();
                break;

            case 6:
                limpar();

                cout << "========================================\n";
                cout << "             FIM DE JOGO\n";
                cout << "========================================\n";
                break;

            default:
                cout << "\nOpcao invalida.\n";
                pausa();
                break;
        }

    } while (opcao != 6);

    return 0;
}