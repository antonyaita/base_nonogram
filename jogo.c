#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "util.h" 

// definicoes janela
#define LARGURA_JANELA 800
#define ALTURA_JANELA 800
#define TITULO_JANELA "Nonogram"

// definicoes tabuleiro
#define LINHAS 8
#define COLUNAS 8
#define ALTURA_CELULA 50
#define LARGURA_CELULA 50
#define LARGURA_TABULEIRO LARGURA_CELULA * COLUNAS
#define ALTURA_TABULEIRO ALTURA_CELULA * LINHAS
#define MARGIN_HORIZONTAL (int) ((LARGURA_JANELA / 2) - (LARGURA_TABULEIRO / 2))
#define MARGIN_VERTICAL  (int) ((ALTURA_JANELA / 2) - (ALTURA_TABULEIRO / 2))

#define LIMPA_HORIZONTAL 50
#define LIMPA_VERTICAL 50

#define RESET_HORIZONTAL 50
#define RESET_VERTICAL 120

#define CLASSICO 0
#define NORMAL 1

// prototipos das funcoes
void inicia_tabuleiro(Celula tabuleiro[][COLUNAS]);
void desenha_tabuleiro(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS]);
void verifica_clique(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu);
void escreve_numeros(Tela *t, int *numeros, int qtd, Ponto inicio, bool horizontal);
void cicla_estado_celula(Celula *c);

void gera_gabarito(int gabarito[][COLUNAS]);
int conta_blocos(int *linha, int tamanho, int *blocos);
int verifica_venceu(int gabarito[][COLUNAS], Celula tabuleiro[][COLUNAS]);

int main(int argc, char **argv) {
    int tecla_pressionada;
    int venceu = 0;
    int modo_jogo;
    Tela t;

    int gabarito_nonogram[LINHAS][COLUNAS];
    Celula tabuleiro_nonogram[LINHAS][COLUNAS];

    srand(time(NULL));
    gera_gabarito(gabarito_nonogram);

    inicia_tabuleiro(tabuleiro_nonogram);

    inicia_tela(&t, LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);

    while ((tecla_pressionada = codigo_tecla(&t)) != ALLEGRO_EVENT_DISPLAY_CLOSE) {
        limpa_tela(&t);

        if(!venceu)
        {
            desenha_tabuleiro(&t, tabuleiro_nonogram, gabarito_nonogram);
            Retangulo botao_limpar = { 
                {LIMPA_HORIZONTAL, LIMPA_VERTICAL}, {120, 50}, VAZIO
            };

            desenha_retangulo(botao_limpar, &t, false);

            Ponto texto_limpa = {
                LIMPA_HORIZONTAL + 25,
                LIMPA_VERTICAL + 15
            };

            escreve_texto(&t, texto_limpa, "LIMPAR");
            verifica_clique(&t, tabuleiro_nonogram, gabarito_nonogram, &venceu);


            Retangulo botao_reset = {
                {RESET_HORIZONTAL, RESET_VERTICAL}, {120, 50}, VAZIO
            };

            desenha_retangulo(botao_reset, &t, false);

            Ponto texto_reset = {
                RESET_HORIZONTAL + 30,
                RESET_VERTICAL + 15
            };

            escreve_texto(&t, texto_reset, "RESET");
            verifica_clique(&t, tabuleiro_nonogram, gabarito_nonogram, &venceu);
        }
        else
        {
            Retangulo caixa = { {200, 250}, {400, 200}};
            desenha_retangulo(caixa, &t, false);
            
            Ponto p = {280, 310};
            escreve_texto(&t, p, "VOCE VENCEU!");

            Ponto subtitulo = {220, 360};
            escreve_texto(&t, subtitulo, "Pressione R para reiniciar");
        }

        mostra_tela();
        
        // espera 30 ms antes de atualizar a tela
        espera(30);
    }
    
    finaliza_tela(&t);

    return 0;
}

void gera_gabarito(int gabarito[][COLUNAS])
{
    for (int i = 0 ; i< LINHAS ; i++)
    {
        for (int j = 0; j < COLUNAS; j++)
        {
            gabarito[i][j] = rand() % 2;
        }
    }
}

int conta_blocos(int *linha, int tamanho, int *blocos) 
{
    int qtd = 0;
    int contagem = 0;

    for (int i = 0; i < tamanho; i++) {
        if (linha[i] == 1) {
            contagem++;
        } else {
            if (contagem > 0) {
                blocos[qtd] = contagem;
                qtd++;
                contagem = 0;
            }
        }
    }

    if (contagem > 0) {
        blocos[qtd] = contagem;
        qtd++;
    }
    
    if (qtd == 0) {
        blocos[0] = 0;
        qtd = 1;
    }

    return qtd;
}

int verifica_venceu(int gabarito[][COLUNAS], Celula tabuleiro[][COLUNAS])
{
    for (int i = 0; i < LINHAS; i++)  {
        for (int j = 0; j < COLUNAS; j++) {
            if(gabarito[i][j] == 1 && tabuleiro[i][j].estado != COLORIDO)
            {
                return 0;
            }

            else if(gabarito[i][j] == 0 && tabuleiro[i][j].estado == COLORIDO)
            {
                return 0;
            }
        }
    }

    return 1;
}

// inicia tabuleiro do jogo, definindo as posicoes de cada celula
void inicia_tabuleiro(Celula tabuleiro[][COLUNAS]) {
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            // coordenadas de cada celula (retangulo)
            int x = MARGIN_HORIZONTAL + (LARGURA_CELULA * j);
            int y = MARGIN_VERTICAL + (ALTURA_CELULA * i);
            // inicializacao da celula com coordenadas (ponto), tamanho e o estado (por padrao = vazio)
            Celula c = { { x, y }, { ALTURA_CELULA, LARGURA_CELULA }, VAZIO };
            tabuleiro[i][j] = c;
        }
    }
}

// desenha tabuleiro (celulas) e os numeros nas linhas e colunas
void desenha_tabuleiro(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS]) {
    Cor preto = {0, 0, 0};
    Cor vermelho = {1, 0, 0};
    define_cor(t, preto); // usado para as bordas (quando celula nao estiver colorida) e para o preenchimento (quando estiver colorida)

    for (int i = 0; i < LINHAS; i++)  {
        for (int j = 0; j < COLUNAS; j++) {
            // desenha a celula. se estado da celula for colorido, ira pintar o retangulo, senão desenha apenas as bordas
            desenha_retangulo(tabuleiro[i][j], t, tabuleiro[i][j].estado == COLORIDO);
            // se estado da celula for alerta, desenha um x vermelho dentro das bordas do retangulo
            if (tabuleiro[i][j].estado == ALERTA) {
                define_cor(t, vermelho);
                desenha_x_dentro_ret(tabuleiro[i][j], t);
                define_cor(t, preto); // restaura cor
            }
        }
    }

    // uso de claude code 
    for (int i = 0; i < LINHAS; i++) {
        int x = MARGIN_HORIZONTAL - 10;
        int y = MARGIN_VERTICAL + (i * ALTURA_CELULA) + 15;
        Ponto p = { x, y };

        int blocos[COLUNAS];
        int qtd = conta_blocos(gabarito[i], COLUNAS, blocos);
        escreve_numeros(t, blocos, qtd, p, true);
    }

    // uso de claude code 
    for (int j = 0; j < COLUNAS; j++) {
        int x = MARGIN_HORIZONTAL + (j * LARGURA_CELULA) + 15;
        int y = MARGIN_VERTICAL - 10;
        Ponto p = { x, y };

        int coluna[LINHAS];
        for (int i = 0; i < LINHAS; i++)
            coluna[i] = gabarito[i][j];

        int blocos[LINHAS];
        int qtd = conta_blocos(coluna, LINHAS, blocos);
        escreve_numeros(t, blocos, qtd, p, false);
    }
}

// escreve os numeros (vetor), a partir de uma coordenada, horizontalmente ou verticalmente
void escreve_numeros(Tela *t, int *numeros, int qtd, Ponto inicio, bool horizontal) {
    int x = inicio.x, y = inicio.y, offset = 20;
    for (int i = qtd - 1; i >= 0; i--) {
        char buffer[10];
        sprintf(buffer, "%d", numeros[i]);

        if (horizontal) {
            x -= offset;
        } else {
            y -= offset;
        }

        Ponto p = { x, y };
        escreve_texto(t, p, buffer);
    }
}

// verifica se houve um clique em alguma celula no tabuleiro
void verifica_clique(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu) {
    Ponto mouse = posicao_mouse(t);
    printf("mouse=(%.0f, %.0f)\n", mouse.x, mouse.y);
    Retangulo botao_limpar = {
        {LIMPA_HORIZONTAL, LIMPA_VERTICAL},
        {120, 50},
        VAZIO
    };

    Retangulo botao_reset = {
        {RESET_HORIZONTAL, RESET_VERTICAL},
        {120, 50},
        VAZIO
    };


    if (botao_clicado(t)) { // mouse foi clicado
        if(ret_contains_pt(botao_limpar, mouse))
        {
            inicia_tabuleiro(tabuleiro);
            t->_botao = false;
            return;
        }

        if(ret_contains_pt(botao_reset, mouse))
        {
            gera_gabarito(gabarito);
            inicia_tabuleiro(tabuleiro);
            t->_botao = false;
            return;
        }

        int x_clique_tabuleiro = (mouse.x - MARGIN_HORIZONTAL);
        int y_clique_tabuleiro = (mouse.y - MARGIN_VERTICAL);
        
        // verifica se o clique foi dentro do tabuleiro
        if (x_clique_tabuleiro >= 0 && y_clique_tabuleiro >= 0) {
            // descobre qual celula do tabuleiro foi clicada
            int i = (int) (x_clique_tabuleiro / LARGURA_CELULA);
            int j = (int) (y_clique_tabuleiro / ALTURA_CELULA);

            // verifica se eh uma celula valida
            if (i >= 0 && i < LINHAS && j >=0 && j < COLUNAS) {
                cicla_estado_celula(&tabuleiro[j][i]);
                if(verifica_venceu(gabarito, tabuleiro))
                    {
                        *venceu = 1;
                    }
            }
        }
        t->_botao = false; // clique ja foi tratado!
    }
}

// cicla o estado da celula
void cicla_estado_celula(Celula *c) {
    // alternativa: c.estado = (c.estado + 1) % 3;
    // cicla os estados da celula
    switch (c->estado) {
        // se celula esta vazia, fica colorida
        case VAZIO: c->estado = COLORIDO;
            break;
        // se celula esta colorida, fica alerta (x)
        case COLORIDO: c->estado = ALERTA;
            break;
        // se celula esta alerta, volta a ficar vazia
        case ALERTA: c->estado = VAZIO;
            break;
    }
}