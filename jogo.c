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
#define LARGURA_TABULEIRO LARGURA_CELULA *COLUNAS
#define ALTURA_TABULEIRO ALTURA_CELULA *LINHAS
#define MARGIN_HORIZONTAL (int)((LARGURA_JANELA / 2) - (LARGURA_TABULEIRO / 2))
#define MARGIN_VERTICAL (int)((ALTURA_JANELA / 2) - (ALTURA_TABULEIRO / 2))

#define LIMPA_HORIZONTAL 50
#define LIMPA_VERTICAL 50

#define RESET_HORIZONTAL 50
#define RESET_VERTICAL 120

#define MENU 0
#define CLASSICO 1
#define NORMAL 2

#define VIDA 3

// prototipos das funcoes
void inicia_tabuleiro(Celula tabuleiro[][COLUNAS]);
void desenha_tabuleiro(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS]);
void verifica_clique_normal(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo, int *vidas);
void verifica_clique(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo);
void escreve_numeros(Tela *t, int *numeros, int qtd, Ponto inicio, bool horizontal);
void cicla_estado_celula(Celula *c, int modo_jogo);

void menu(Tela *t, int *modo_jogo);
void jogo_classico(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo);
void jogo_normal(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo, int *vida);
void gera_gabarito(int gabarito[][COLUNAS]);
int conta_blocos(int *linha, int tamanho, int *blocos);
int verifica_venceu(int gabarito[][COLUNAS], Celula tabuleiro[][COLUNAS]);
int total_preenchidos(int gabarito[][COLUNAS]);
int preenchidos_corretos(int gabarito[][COLUNAS], Celula tabuleiro[][COLUNAS]);

int main(int argc, char **argv)
{
    int tecla_pressionada;
    int venceu = 0;
    int modo_jogo = MENU;
    int vida = VIDA;
    Tela t;
    int gabarito[LINHAS][COLUNAS];
    Celula tabuleiro[LINHAS][COLUNAS];

    inicia_tela(&t, LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);

    srand(time(NULL));
    gera_gabarito(gabarito);

    inicia_tabuleiro(tabuleiro);
    while ((tecla_pressionada = codigo_tecla(&t)) != ALLEGRO_EVENT_DISPLAY_CLOSE)
    {
        limpa_tela(&t);

        switch (modo_jogo)
        {
        case MENU:
            menu(&t, &modo_jogo);
            break;

        case CLASSICO:
            jogo_classico(&t, tabuleiro, gabarito, &venceu, &modo_jogo);
            if (venceu != 0)
            {
                if (tecla_pressionada == ALLEGRO_KEY_R)
                {
                    gera_gabarito(gabarito);
                    inicia_tabuleiro(tabuleiro);

                    vida = VIDA;
                    venceu = 0;
                }

                if (tecla_pressionada == ALLEGRO_KEY_M)
                {
                    modo_jogo = MENU;

                    vida = VIDA;
                    venceu = 0;

                    inicia_tabuleiro(tabuleiro);
                }
            }
            break;

        case NORMAL:
            jogo_normal(&t, tabuleiro, gabarito, &venceu, &modo_jogo, &vida);
            if (venceu != 0)
            {
                if (tecla_pressionada == ALLEGRO_KEY_R)
                {
                    gera_gabarito(gabarito);
                    inicia_tabuleiro(tabuleiro);

                    vida = VIDA;
                    venceu = 0;
                }

                if (tecla_pressionada == ALLEGRO_KEY_M)
                {
                    modo_jogo = MENU;

                    vida = VIDA;
                    venceu = 0;

                    inicia_tabuleiro(tabuleiro);
                }
            }
            break;
        }

        mostra_tela();

        // espera 30 ms antes de atualizar a tela
        espera(30);
    }

    finaliza_tela(&t);

    return 0;
}

void menu(Tela *t, int *modo_jogo)
{
    int largura_botao = 300;
    int altura_botao = 60;

    int x_botao = (LARGURA_JANELA - largura_botao) / 2;

    Ponto titulo = {
        LARGURA_JANELA / 2 - 50,
        200};

    escreve_texto(t, titulo, "NONOGRAM");

    Retangulo botao_classico = {
        {x_botao, 320},
        {largura_botao, altura_botao},
        VAZIO};

    Retangulo botao_normal = {
        {x_botao, 420},
        {largura_botao, altura_botao},
        VAZIO};

    desenha_retangulo(botao_classico, t, false);
    desenha_retangulo(botao_normal, t, false);

    escreve_texto(t, (Ponto){x_botao + 105, 340}, "CLASSICO");
    escreve_texto(t, (Ponto){x_botao + 115, 440}, "NORMAL");

    if (botao_clicado(t))
    {
        Ponto mouse = posicao_mouse(t);

        if (ret_contains_pt(botao_classico, mouse))
        {
            *modo_jogo = CLASSICO;
        }

        if (ret_contains_pt(botao_normal, mouse))
        {
            *modo_jogo = NORMAL;
        }

        t->_botao = false;
    }
}

void jogo_classico(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo)
{
    if (!(*venceu))
    {
        desenha_tabuleiro(t, tabuleiro, gabarito);
        escreve_texto(t, (Ponto){50, 20}, "MODO CLASSICO");
        int centro_tabuleiro = MARGIN_HORIZONTAL + (LARGURA_TABULEIRO / 2);
        char progresso[50];

        sprintf(progresso, "Progresso: %d/%d", preenchidos_corretos(gabarito, tabuleiro), total_preenchidos(gabarito));

        escreve_texto(t, (Ponto){centro_tabuleiro - 62, MARGIN_VERTICAL - 110}, progresso);

        Retangulo botao_limpar = {{LIMPA_HORIZONTAL, LIMPA_VERTICAL}, {120, 50}, VAZIO};

        desenha_retangulo(botao_limpar, t, false);

        Ponto texto_limpa = {
            LIMPA_HORIZONTAL + 25,
            LIMPA_VERTICAL + 15};

        escreve_texto(t, texto_limpa, "LIMPAR");

        Retangulo botao_reset = {{RESET_HORIZONTAL, RESET_VERTICAL}, {120, 50}, VAZIO};

        desenha_retangulo(botao_reset, t, false);

        Ponto texto_reset = {
            RESET_HORIZONTAL + 30,
            RESET_VERTICAL + 15};

        escreve_texto(t, texto_reset, "RESET");

        Retangulo botao_menu = {
            {MARGIN_HORIZONTAL + (LARGURA_TABULEIRO / 2) - 60,
             MARGIN_VERTICAL + ALTURA_TABULEIRO + 30},
            {120, 50},
            VAZIO};

        desenha_retangulo(botao_menu, t, false);

        escreve_texto(t, (Ponto){botao_menu.pos.x + 30, botao_menu.pos.y + 15}, "MENU");

        verifica_clique(t, tabuleiro, gabarito, venceu, modo_jogo);
    }
    else
    {
        Retangulo caixa = {{200, 250}, {400, 200}};
        desenha_retangulo(caixa, t, false);

        Ponto p = {330, 310};
        escreve_texto(t, p, "VOCE VENCEU!");

        Ponto subtitulo = {290, 360};
        escreve_texto(t, subtitulo, "Pressione R para reiniciar");
    }
}

void jogo_normal(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo, int *vida)
{
    if (*venceu == 0)
    {
        int centro_tabuleiro = MARGIN_HORIZONTAL + (LARGURA_TABULEIRO / 2);
        char coracoes[20] = "";

        for (int i = 0; i < *vida; i++)
        {
            strcat(coracoes, "<3 ");
        }

        Cor vermelho = {1, 0, 0};
        define_cor(t, vermelho);

        escreve_texto(t, (Ponto){centro_tabuleiro - 35, MARGIN_VERTICAL - 110}, coracoes);

        Cor preto = {0, 0, 0};
        define_cor(t, preto);

        desenha_tabuleiro(t, tabuleiro, gabarito);
        escreve_texto(t, (Ponto){50, 20}, "MODO NORMAL");

        Retangulo botao_limpar = {{LIMPA_HORIZONTAL, LIMPA_VERTICAL}, {120, 50}, VAZIO};

        desenha_retangulo(botao_limpar, t, false);
        Ponto texto_limpa = {
            LIMPA_HORIZONTAL + 25,
            LIMPA_VERTICAL + 15};

        escreve_texto(t, texto_limpa, "LIMPAR");

        Retangulo botao_reset = {{RESET_HORIZONTAL, RESET_VERTICAL}, {120, 50}, VAZIO};

        desenha_retangulo(botao_reset, t, false);

        Ponto texto_reset = {
            RESET_HORIZONTAL + 30,
            RESET_VERTICAL + 15};

        escreve_texto(t, texto_reset, "RESET");

        Retangulo botao_menu = {
            {MARGIN_HORIZONTAL + (LARGURA_TABULEIRO / 2) - 60,
             MARGIN_VERTICAL + ALTURA_TABULEIRO + 30},
            {120, 50},
            VAZIO};

        desenha_retangulo(botao_menu, t, false);

        escreve_texto(t, (Ponto){botao_menu.pos.x + 30, botao_menu.pos.y + 15}, "MENU");

        verifica_clique_normal(t, tabuleiro, gabarito, venceu, modo_jogo, vida);
    }
    else if (*venceu == 1)
    {
        Retangulo caixa = {{200, 250}, {400, 200}};
        desenha_retangulo(caixa, t, false);

        Ponto p = {330, 310};
        escreve_texto(t, p, "VOCE VENCEU!");

        Ponto subtitulo_reinicio = {290, 360};
        escreve_texto(t, subtitulo_reinicio, "Pressione R para Reiniciar");

        Ponto subtitulo_menu = {290, 380};
        escreve_texto(t, subtitulo_menu, "Pressione M para Menu");
    }
    else if (*venceu == -1)
    {
        Retangulo caixa = {{200, 250}, {400, 200}};
        desenha_retangulo(caixa, t, false);

        Ponto p = {330, 310};
        escreve_texto(t, p, "VOCE PERDEU!");

        Ponto subtitulo_reinicio = {290, 360};
        escreve_texto(t, subtitulo_reinicio, "Pressione R para Reiniciar");

        Ponto subtitulo_menu = {290, 380};
        escreve_texto(t, subtitulo_menu, "Pressione M para Menu");
    }
}

void gera_gabarito(int gabarito[][COLUNAS])
{
    for (int i = 0; i < LINHAS; i++)
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

    for (int i = 0; i < tamanho; i++)
    {
        if (linha[i] == 1)
        {
            contagem++;
        }
        else
        {
            if (contagem > 0)
            {
                blocos[qtd] = contagem;
                qtd++;
                contagem = 0;
            }
        }
    }

    if (contagem > 0)
    {
        blocos[qtd] = contagem;
        qtd++;
    }

    if (qtd == 0)
    {
        blocos[0] = 0;
        qtd = 1;
    }

    return qtd;
}

int verifica_venceu(int gabarito[][COLUNAS], Celula tabuleiro[][COLUNAS])
{
    for (int i = 0; i < LINHAS; i++)
    {
        for (int j = 0; j < COLUNAS; j++)
        {
            if (gabarito[i][j] == 1 && tabuleiro[i][j].estado != COLORIDO)
            {
                return 0;
            }

            else if (gabarito[i][j] == 0 && tabuleiro[i][j].estado == COLORIDO)
            {
                return 0;
            }
        }
    }

    return 1;
}

int total_preenchidos(int gabarito[][COLUNAS])
{
    int total = 0;

    for (int i = 0; i < LINHAS; i++)
    {
        for (int j = 0; j < COLUNAS; j++)
        {
            if (gabarito[i][j] == 1)
            {
                total++;
            }
        }
    }

    return total;
}

int preenchidos_corretos(int gabarito[][COLUNAS], Celula tabuleiro[][COLUNAS])
{
    int corretos = 0;

    for (int i = 0; i < LINHAS; i++)
    {
        for (int j = 0; j < COLUNAS; j++)
        {
            if (gabarito[i][j] == 1 &&
                tabuleiro[i][j].estado == COLORIDO)
            {
                corretos++;
            }
        }
    }

    return corretos;
}

// inicia tabuleiro do jogo, definindo as posicoes de cada celula
void inicia_tabuleiro(Celula tabuleiro[][COLUNAS])
{
    for (int i = 0; i < LINHAS; i++)
    {
        for (int j = 0; j < COLUNAS; j++)
        {
            // coordenadas de cada celula (retangulo)
            int x = MARGIN_HORIZONTAL + (LARGURA_CELULA * j);
            int y = MARGIN_VERTICAL + (ALTURA_CELULA * i);
            // inicializacao da celula com coordenadas (ponto), tamanho e o estado (por padrao = vazio)
            Celula c = {{x, y}, {ALTURA_CELULA, LARGURA_CELULA}, VAZIO};
            tabuleiro[i][j] = c;
        }
    }
}

// desenha tabuleiro (celulas) e os numeros nas linhas e colunas
void desenha_tabuleiro(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS])
{
    Cor preto = {0, 0, 0};
    Cor vermelho = {1, 0, 0};
    define_cor(t, preto); // usado para as bordas (quando celula nao estiver colorida) e para o preenchimento (quando estiver colorida)

    for (int i = 0; i < LINHAS; i++)
    {
        for (int j = 0; j < COLUNAS; j++)
        {
            // desenha a celula. se estado da celula for colorido, ira pintar o retangulo, senão desenha apenas as bordas
            desenha_retangulo(tabuleiro[i][j], t, tabuleiro[i][j].estado == COLORIDO);
            // se estado da celula for alerta, desenha um x vermelho dentro das bordas do retangulo
            if (tabuleiro[i][j].estado == ALERTA)
            {
                define_cor(t, vermelho);
                desenha_x_dentro_ret(tabuleiro[i][j], t);
                define_cor(t, preto); // restaura cor
            }
        }
    }

    // uso de claude code
    for (int i = 0; i < LINHAS; i++)
    {
        int x = MARGIN_HORIZONTAL - 10;
        int y = MARGIN_VERTICAL + (i * ALTURA_CELULA) + 15;
        Ponto p = {x, y};

        int blocos[COLUNAS];
        int qtd = conta_blocos(gabarito[i], COLUNAS, blocos);
        escreve_numeros(t, blocos, qtd, p, true);
    }

    // uso de claude code
    for (int j = 0; j < COLUNAS; j++)
    {
        int x = MARGIN_HORIZONTAL + (j * LARGURA_CELULA) + 15;
        int y = MARGIN_VERTICAL - 10;
        Ponto p = {x, y};

        int coluna[LINHAS];
        for (int i = 0; i < LINHAS; i++)
            coluna[i] = gabarito[i][j];

        int blocos[LINHAS];
        int qtd = conta_blocos(coluna, LINHAS, blocos);
        escreve_numeros(t, blocos, qtd, p, false);
    }
}

// escreve os numeros (vetor), a partir de uma coordenada, horizontalmente ou verticalmente
void escreve_numeros(Tela *t, int *numeros, int qtd, Ponto inicio, bool horizontal)
{
    int x = inicio.x, y = inicio.y, offset = 20;
    for (int i = qtd - 1; i >= 0; i--)
    {
        char buffer[10];
        sprintf(buffer, "%d", numeros[i]);

        if (horizontal)
        {
            x -= offset;
        }
        else
        {
            y -= offset;
        }

        Ponto p = {x, y};
        escreve_texto(t, p, buffer);
    }
}

// verifica se houve um clique em alguma celula no tabuleiro
void verifica_clique(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo)
{
    Ponto mouse = posicao_mouse(t);
    Retangulo botao_limpar = {
        {LIMPA_HORIZONTAL, LIMPA_VERTICAL},
        {120, 50},
        VAZIO};

    Retangulo botao_reset = {
        {RESET_HORIZONTAL, RESET_VERTICAL},
        {120, 50},
        VAZIO};

    Retangulo botao_menu = {
        {MARGIN_HORIZONTAL + (LARGURA_TABULEIRO / 2) - 60,
         MARGIN_VERTICAL + ALTURA_TABULEIRO + 30},
        {120, 50},
        VAZIO};

    if (botao_clicado(t))
    { // mouse foi clicado
        if (ret_contains_pt(botao_limpar, mouse))
        {
            inicia_tabuleiro(tabuleiro);
            t->_botao = false;
            return;
        }

        if (ret_contains_pt(botao_reset, mouse))
        {
            gera_gabarito(gabarito);
            inicia_tabuleiro(tabuleiro);
            t->_botao = false;
            return;
        }

        if (ret_contains_pt(botao_menu, mouse))
        {
            gera_gabarito(gabarito);
            *modo_jogo = MENU;
            inicia_tabuleiro(tabuleiro);
            *venceu = 0;

            t->_botao = false;
            return;
        }

        int x_clique_tabuleiro = (mouse.x - MARGIN_HORIZONTAL);
        int y_clique_tabuleiro = (mouse.y - MARGIN_VERTICAL);

        // verifica se o clique foi dentro do tabuleiro
        if (x_clique_tabuleiro >= 0 && y_clique_tabuleiro >= 0)
        {
            // descobre qual celula do tabuleiro foi clicada
            int i = (int)(x_clique_tabuleiro / LARGURA_CELULA);
            int j = (int)(y_clique_tabuleiro / ALTURA_CELULA);

            // verifica se eh uma celula valida
            if (i >= 0 && i < LINHAS && j >= 0 && j < COLUNAS)
            {
                cicla_estado_celula(&tabuleiro[j][i], *modo_jogo);
                if (verifica_venceu(gabarito, tabuleiro))
                {
                    *venceu = 1;
                }
            }
        }
        t->_botao = false; // clique ja foi tratado!
    }
}

void verifica_clique_normal(Tela *t, Celula tabuleiro[][COLUNAS], int gabarito[][COLUNAS], int *venceu, int *modo_jogo, int *vidas)
{
    Ponto mouse = posicao_mouse(t);
    // printf("mouse=(%.0f, %.0f)\n", mouse.x, mouse.y);
    Retangulo botao_limpar = {
        {LIMPA_HORIZONTAL, LIMPA_VERTICAL},
        {120, 50},
        VAZIO};

    Retangulo botao_reset = {
        {RESET_HORIZONTAL, RESET_VERTICAL},
        {120, 50},
        VAZIO};

    Retangulo botao_menu = {
        {MARGIN_HORIZONTAL + (LARGURA_TABULEIRO / 2) - 60,
         MARGIN_VERTICAL + ALTURA_TABULEIRO + 30},
        {120, 50},
        VAZIO};

    if (botao_clicado(t))
    { // mouse foi clicado
        if (ret_contains_pt(botao_limpar, mouse))
        {
            inicia_tabuleiro(tabuleiro);

            *vidas = VIDA;
            *venceu = 0;

            t->_botao = false;
            return;
        }

        if (ret_contains_pt(botao_reset, mouse))
        {
            gera_gabarito(gabarito);
            inicia_tabuleiro(tabuleiro);

            *vidas = VIDA;
            *venceu = 0;

            t->_botao = false;
            return;
        }

        if (ret_contains_pt(botao_menu, mouse))
        {
            *modo_jogo = MENU;
            gera_gabarito(gabarito);
            inicia_tabuleiro(tabuleiro);
            *venceu = 0;

            t->_botao = false;
            return;
        }

        int x_clique_tabuleiro = (mouse.x - MARGIN_HORIZONTAL);
        int y_clique_tabuleiro = (mouse.y - MARGIN_VERTICAL);

        // verifica se o clique foi dentro do tabuleiro
        if (x_clique_tabuleiro >= 0 && y_clique_tabuleiro >= 0)
        {
            // descobre qual celula do tabuleiro foi clicada
            int i = (int)(x_clique_tabuleiro / LARGURA_CELULA);
            int j = (int)(y_clique_tabuleiro / ALTURA_CELULA);

            // verifica se eh uma celula valida
            if (i >= 0 && i < LINHAS && j >= 0 && j < COLUNAS)
            {
                Celula *c = &tabuleiro[j][i];

                if (t->_botao_mouse == 1)
                {
                    c->estado = COLORIDO;

                    if (gabarito[j][i] == 0)
                    {
                        (*vidas)--;
                        if (*vidas <= 0)
                        {
                            *venceu = -1;
                        }

                        c->estado = ALERTA;
                    }
                }
                else if (t->_botao_mouse == 2)
                {
                    c->estado = ALERTA;

                    if (gabarito[j][i] == 1)
                    {
                        (*vidas)--;

                        if (*vidas <= 0)
                        {
                            *venceu = -1;
                        }

                        c->estado = COLORIDO;
                    }
                }
                if (verifica_venceu(gabarito, tabuleiro))
                {
                    *venceu = 1;
                }
            }
        }
        t->_botao = false; // clique ja foi tratado!
    }
}

// cicla o estado da celula
void cicla_estado_celula(Celula *c, int modo_jogo)
{
    // alternativa: c.estado = (c.estado + 1) % 3;
    // cicla os estados da celula
    if (modo_jogo == CLASSICO)
    {
        if (c->estado == VAZIO)
        {
            c->estado = COLORIDO;
        }
        else
        {
            c->estado = VAZIO;
        }
    }
    else
    {
        switch (c->estado)
        {
        // se celula esta vazia, fica colorida
        case VAZIO:
            c->estado = COLORIDO;
            break;
        // se celula esta colorida, fica alerta (x)
        case COLORIDO:
            c->estado = ALERTA;
            break;
        // se celula esta alerta, volta a ficar vazia
        case ALERTA:
            c->estado = VAZIO;
            break;
        }
    }
}