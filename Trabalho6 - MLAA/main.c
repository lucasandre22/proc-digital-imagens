#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>
#include <time.h>

//#define INPUT_IMAGE "ex1_binaria.bmp"
//#define INPUT_IMAGE "ex2_csgo.bmp"
//#define INPUT_IMAGE "ex3_battlefield.bmp"
//#define INPUT_IMAGE "ex4_benchmark.bmp"
//#define INPUT_IMAGE "ex5_deadspace.bmp"
#define INPUT_IMAGE "ex6_gtaiv.bmp"
//#define INPUT_IMAGE "ex7_shift.bmp"

#define CANAIS_IMAGEM 3

#define BORDA 0.400f
#define CORNER 0.800f
#define PONTA_FALSA 0.100f

#define THRESHOLD 0.200f

//Defino o tamanho máximo do L a ser procurado
#define MAX_PERNA_MENOR 2 //perna menor
#define MAX_PERNA_MAIOR 6 //perna maior

typedef struct { //todo shape é composto de linha vertical e linha horizontal
    int iFinal; /*Onde o i do shape termina (perna que varia horizontalmente do L_SHAPE)*/
    int jFinal; /*Onde o j do shape termina (perna que varia verticalmente do L_SHAPE)*/

    int corneri; /*Posição i do pixel de corner*/
    int cornerj; /*Posição j do pixel de corner*/

    short int borda_falsa; //bool, se estou analisando imagem borda falsa ou não
}L_SHAPE;

typedef struct {
    int quantidade; /*Quantidade total de shapes*/
    L_SHAPE* l_shapes; /*Vetor de L_SHAPES*/
}SHAPES;

void zeraImagem(Imagem* img) {
    for(int h = 0; h < img->n_canais; h++) {
        for(int i = 0; i < img->altura; i++) {
            for(int j = 0; j < img->largura; j++) {
                img->dados[h][i][j] = 0;
            }
        }
    }
}

//vai achar bordas na imagem
void computaBorda(Imagem* img, Imagem* bordas, Imagem* bordas_falsas) {
    Imagem* dx = criaImagem(img->largura, img->altura, 1);
    Imagem* dy = criaImagem(img->largura, img->altura, 1);
    Imagem* aux = criaImagem(img->largura, img->altura, 1);
    Imagem* copia;
    int quantidade_corners = 0;
    zeraImagem(dx);
    zeraImagem(dy);

    if(CANAIS_IMAGEM == 3) {
        RGBParaCinza(img, aux);
    } 
    else {
        copiaConteudo(img, aux);
    }

    //esquerda pra direita
    for(int i = 1; i < img->altura-1; i++) {
        for(int j = 1; j < img->largura-1; j++) {
            if(fabs(aux->dados[0][i][j] - aux->dados[0][i][j+1]) > THRESHOLD) { //marca pixels de borda, coners tem valor de 2*borda. borda falsa será misturada
                if(aux->dados[0][i][j] < aux->dados[0][i][j+1]) {
                    dx->dados[0][i][j] = BORDA;
                    dx->dados[0][i][j+1] = PONTA_FALSA;
                }
                else {
                    dx->dados[0][i][j+1] = BORDA;
                    dx->dados[0][i][j] = PONTA_FALSA;
                }
            }
        }
    }
    //cima pra baixo
    for(int i = 1; i < img->largura - 1; i++) {
        for(int j = 1; j < img->altura - 1; j++) {
            if(fabs(aux->dados[0][j][i] - aux->dados[0][j+1][i]) > THRESHOLD) {
                if(aux->dados[0][j][i] < aux->dados[0][j+1][i]) {
                    dy->dados[0][j][i] = BORDA;
                    dy->dados[0][j+1][i] = PONTA_FALSA;
                }
                else {
                    dy->dados[0][j+1][i] = BORDA;
                    dy->dados[0][j][i] = PONTA_FALSA;
                }
            }    
        }
    }

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            bordas->dados[0][i][j] = dx->dados[0][i][j] + dy->dados[0][i][j];
            if(bordas->dados[0][i][j] == CORNER)    //defino como corner pixel com valores 2*BORDA, ou seja, foi encontrada na imagem dx e dy
                quantidade_corners++;
            if(bordas->dados[0][i][j] == 2*PONTA_FALSA) {
                bordas->dados[0][i][j] = PONTA_FALSA;
            }

            
            if(dx->dados[0][i][j] == PONTA_FALSA || dy->dados[0][i][j] == PONTA_FALSA) {
                bordas_falsas->dados[0][i][j] = BORDA;
            }
            if(dx->dados[0][i][j] == BORDA || dy->dados[0][i][j] == BORDA) {
                bordas_falsas->dados[0][i][j] = PONTA_FALSA;
            }
            if((dx->dados[0][i][j] + dy->dados[0][i][j]) == 2*PONTA_FALSA) {
                quantidade_corners++;
                bordas_falsas->dados[0][i][j] = CORNER;
            }
        }
    }
    destroiImagem(dx);
    destroiImagem(dy);
    destroiImagem(aux);
    printf("Quantidade de bordas esperadas: %d\n", quantidade_corners);
    salvaImagem(bordas_falsas, "img-bordas-falsas.bmp");
    salvaImagem(bordas, "img-bordas.bmp");
}

//vai achar os L's na imagem
void achaShapes(Imagem* bordas, SHAPES* shapes, int borda_falsa) {
    int j, i, h;

    //crio shapes de acordo com pixels de corner
    for(i = 1; i < bordas->altura - 1; i++) {
        for(j = 1; j < bordas->largura - 1; j++) {
            if(bordas->dados[0][i][j] == CORNER) {
                shapes->quantidade++;
                if(shapes->quantidade == 1)
                    shapes->l_shapes = malloc(sizeof(L_SHAPE));
                else
                    shapes->l_shapes = realloc(shapes->l_shapes, shapes->quantidade*(sizeof(L_SHAPE)));
                shapes->l_shapes[shapes->quantidade-1].corneri = i;
                shapes->l_shapes[shapes->quantidade-1].cornerj = j;
                shapes->l_shapes[shapes->quantidade-1].borda_falsa = borda_falsa;
            }
        }
    }

    for(h = 0; h < shapes->quantidade; h++) { //aqui eu quero navegar pelos shapes e marcar o L deles
        int bool_perna_menor = MAX_PERNA_MAIOR;
        int quantidade_atual = 0;
        int limite;
        int jFinal = shapes->l_shapes[h].cornerj;
        int iFinal = shapes->l_shapes[h].corneri;

        //só entra em dois desses ifs, para cada shape
        //analiso cada corner, procurando pra onde o L aponta
        if(bordas->dados[0][shapes->l_shapes[h].corneri][shapes->l_shapes[h].cornerj+1] == BORDA) { //direita
            for(j = shapes->l_shapes[h].cornerj+1; j < bordas->largura && quantidade_atual < (MAX_PERNA_MAIOR-1) && bordas->dados[0][shapes->l_shapes[h].corneri][j] == BORDA; j++) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                quantidade_atual++;
            }
            if(quantidade_atual > MAX_PERNA_MENOR)
                bool_perna_menor = 0;
            jFinal = shapes->l_shapes[h].cornerj + (quantidade_atual);
            quantidade_atual = 0;
        }
        else if(bordas->dados[0][shapes->l_shapes[h].corneri][shapes->l_shapes[h].cornerj-1] == BORDA) { //esquerda
            limite = bool_perna_menor == 0 ? MAX_PERNA_MENOR-1 : MAX_PERNA_MAIOR-1;
            for(j = shapes->l_shapes[h].cornerj-1; j >= 0 && quantidade_atual < limite && bordas->dados[0][shapes->l_shapes[h].corneri][j] == BORDA; j--) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                quantidade_atual++;
            }
            if(quantidade_atual > MAX_PERNA_MENOR)
                bool_perna_menor = 0;
            jFinal = shapes->l_shapes[h].cornerj - (quantidade_atual);
            quantidade_atual = 0;
        }
        if(bordas->dados[0][shapes->l_shapes[h].corneri+1][shapes->l_shapes[h].cornerj] == BORDA) { //baixo
            limite = bool_perna_menor == 0 ? MAX_PERNA_MENOR-1 : MAX_PERNA_MAIOR-1;
            for(i = shapes->l_shapes[h].corneri+1; i < bordas->altura && quantidade_atual < limite && bordas->dados[0][i][shapes->l_shapes[h].cornerj] == BORDA; i++) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                quantidade_atual++;
            }
            if(quantidade_atual > MAX_PERNA_MENOR)
                bool_perna_menor = 0;
            iFinal = shapes->l_shapes[h].corneri + (quantidade_atual);
            quantidade_atual = 0;
        }
        else if(bordas->dados[0][shapes->l_shapes[h].corneri-1][shapes->l_shapes[h].cornerj] == BORDA) { //cima
            limite = bool_perna_menor == 0 ? MAX_PERNA_MENOR-1 : MAX_PERNA_MAIOR-1;
            for(i = shapes->l_shapes[h].corneri-1; i >= 0 && quantidade_atual < limite && bordas->dados[0][i][shapes->l_shapes[h].cornerj] == BORDA; i--) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                quantidade_atual++;
            }
            iFinal = shapes->l_shapes[h].corneri - (quantidade_atual);
            quantidade_atual = 0;
        }

        shapes->l_shapes[h].jFinal = jFinal;
        shapes->l_shapes[h].iFinal = iFinal;
    }
    printf("Quantidade de shapes: %d\n", shapes->quantidade);
}

void calculaPesos(float pesos[][6][5]) { //acessa linha, coluna
    //L com 
    for(int i = 0; i < MAX_PERNA_MENOR; i++)
        for(int j = 0; j < MAX_PERNA_MAIOR; j++)
            for(int h = 0; h < 5; h++)
                pesos[i][j][h] = 0;

    //MENOR PERNA DO L IGUAL A 1
    //somente um pixel na maior perna
    pesos[0][0][0] = 0.125f;
    //dois pixels na maior perna
    pesos[0][1][0] = 0.25f;
    //tres pixels na maior perna
    pesos[0][2][0] = 1.f/3.f;
    pesos[0][2][1] = 2.f/24.f;
    //quatro pixels na maior perna
    pesos[0][3][0] = 1.2f/3.f;
    pesos[0][3][1] = 5.f/24.f;
    //cinco pixels na maior perna
    pesos[0][4][0] = 3.f/7.f;;
    pesos[0][4][1] = 6.f/24.f;
    pesos[0][4][2] = 3.f/24.f;
    //seis pixels na maior perna
    pesos[0][5][0] = 3.2f/7.f;;
    pesos[0][5][1] = 7.f/24.f;
    pesos[0][5][2] = 3.f/24.f;

    //MENOR PERNA DO L IGUAL A 2
    //somente um pixel na maior perna
    pesos[1][0][0] = 0.25f;
    //dois pixels na maior perna
    pesos[1][1][0] = 0.500f;
    //tres pixels na maior perna
    pesos[1][2][0] = 1.4f/3.f;
    pesos[1][2][1] = 4.f/24.f;
    //quatro pixels na maior perna
    pesos[1][3][0] = 1.8f/3.f;
    pesos[1][3][1] = 6.f/24.f;
    //cinco pixels na maior perna
    pesos[1][4][0] = 4.f/7.f;;
    pesos[1][4][1] = 9.f/24.f;
    pesos[1][4][2] = 5.f/24.f;
    //seis pixels na maior perna
    pesos[1][5][0] = 4.2f/7.f;;
    pesos[1][5][1] = 11.f/24.f;
    pesos[1][5][2] = 6.f/24.f;
}

void merge(Imagem* img, Imagem* bordas, Imagem* bordas_falsas, SHAPES* shapes) {
    float pesos[MAX_PERNA_MENOR][6][5];
    Imagem* final = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    copiaConteudo(img, final);
    Imagem* vetor[2];//arrumar
    vetor[0] = bordas;
    vetor[1] = bordas_falsas;

    int corneri, cornerj;
    int tamanho_menor, tamanho_maior;
    int tamanhoi, tamanhoj;
    int inegativo = 0, jnegativo = 0;
    int bool_imaior;
    calculaPesos(pesos);

    for(int h = 0; h < shapes->quantidade; h++) {
        corneri = shapes->l_shapes[h].corneri;
        cornerj = shapes->l_shapes[h].cornerj;

        tamanhoi = shapes->l_shapes[h].iFinal > corneri ? shapes->l_shapes[h].iFinal - corneri : corneri - shapes->l_shapes[h].iFinal; //tamanho do L no eixo y
        tamanhoj = shapes->l_shapes[h].jFinal > cornerj ? shapes->l_shapes[h].jFinal - cornerj : cornerj - shapes->l_shapes[h].jFinal; //tamanho do L no eixo x

        inegativo = shapes->l_shapes[h].iFinal > corneri ? 0 : 1;
        jnegativo = shapes->l_shapes[h].jFinal > cornerj ? 0 : 1;
        
        bool_imaior = tamanhoi > tamanhoj ? 1 : 0;

        if(tamanhoi > tamanhoj) {
            tamanho_maior = tamanhoi;
            tamanho_menor = tamanhoj;
        } 
        else {
            tamanho_maior = tamanhoj;
            tamanho_menor = tamanhoi;
        }

        //quando tamanhoi == tamanhoj, calculo é feito diferente;
        if(tamanhoi == tamanhoj) { //quando é um só píxel ou tem 2 de altura e 2 de largura, tratamento diferente
            if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj+1] == PONTA_FALSA) {
                for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                    final->dados[canal][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri][cornerj+1]) +
                                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri][cornerj]);
            }
            else if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj-1] == PONTA_FALSA) {
                for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                    final->dados[canal][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri][cornerj-1]) +
                                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri][cornerj]);
            }
            else if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri-1][cornerj] == PONTA_FALSA) {
                for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                    final->dados[canal][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri-1][cornerj]) +
                                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri][cornerj]);
            }
            else if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri+1][cornerj] == PONTA_FALSA) {
                for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                    final->dados[canal][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri+1][cornerj]) +
                                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[canal][corneri][cornerj]);
            }
            continue;
        }

        //Vejo qual direção do L seguir
        //itero cima ou baixo, eu sei que a perna do L maior é pra cima ou pra baixo
        int coordenada_pixel_misturar;
        if(bool_imaior) { //I É MAIOR
            //Antes de começar, preciso saber a direção do pixel que será misturado
            coordenada_pixel_misturar = vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj+1] == PONTA_FALSA ? cornerj+1 : cornerj-1;
            if(inegativo) { //i vai negativando
                for(int i = corneri, f = 0; i > shapes->l_shapes[h].iFinal && f < MAX_PERNA_MAIOR; i--, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] != 0) {
                        for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                            final->dados[canal][i][cornerj] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[canal][i][coordenada_pixel_misturar]) 
                                                                + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[canal][i][cornerj]);
                    }
                }
            }
            else {
                for(int i = corneri, f = 0; i < shapes->l_shapes[h].iFinal && f < MAX_PERNA_MAIOR; i++, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] != 0.0f) {
                        for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                            final->dados[canal][i][cornerj] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[canal][i][coordenada_pixel_misturar]) 
                                                                + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[canal][i][cornerj]);
                    }
                }
            }
        }
        else { //j é maior
            coordenada_pixel_misturar = vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri+1][cornerj] == PONTA_FALSA ? corneri+1 : corneri-1;
            if(jnegativo) {
                for(int j = cornerj, f = 0; j > shapes->l_shapes[h].jFinal && f < MAX_PERNA_MAIOR; j--, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] != 0) {
                        for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                            final->dados[canal][corneri][j] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[canal][coordenada_pixel_misturar][j]) 
                                                                + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[canal][corneri][j]);
                    }
                }
            }
            else {
                for(int j = cornerj, f = 0; j < shapes->l_shapes[h].jFinal && f < MAX_PERNA_MAIOR; j++, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] != 0) {
                        for(int canal = 0; canal < CANAIS_IMAGEM; canal++)
                            final->dados[canal][corneri][j] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[canal][coordenada_pixel_misturar][j]) 
                                                                + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[canal][corneri][j]);
                    }
                }
            }
        }
    }
    salvaImagem(final, "imagem-final.bmp");
    destroiImagem(final);
}

int main ()
{
    Imagem* img;
    Imagem* bordas;
    Imagem* bordas_falsas;
    SHAPES shapes;
    shapes.quantidade = 0;

    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);

    bordas_falsas = criaImagem(img->largura, img->altura, 1);
    bordas = criaImagem(img->largura, img->altura, 1);

    zeraImagem(bordas_falsas);
    zeraImagem(bordas);
    
    computaBorda(img, bordas, bordas_falsas);
    achaShapes(bordas, &shapes, 0);
    achaShapes(bordas_falsas, &shapes, 1);
    merge(img, bordas, bordas_falsas, &shapes);

    destroiImagem(img);
    destroiImagem(bordas);
    destroiImagem(bordas_falsas);
    free(shapes.l_shapes);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");
    return 0;
}