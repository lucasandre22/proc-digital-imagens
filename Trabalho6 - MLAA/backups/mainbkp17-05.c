#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>

#define INPUT_IMAGE "testezinho.bmp"
#define CANAIS_IMAGEM 1
#define LIMITE_INFERIOR 0.020
#define PRETO 0.0f
#define BRANCO 1.0f
#define PONTA 0.500f


#define BORDA 0.400f
#define CORNER 0.800f
#define PONTA_FALSA 0.100f

#define THRESHOLD 0.050f

//Defino o tamanho máximo do L a ser procurado
#define MAX_L_MENOR 2 //perna menor
#define MAX_L_MAIOR 5 //perna maior

typedef struct { //todo shape é componto de linha vertical e linha horizontal
    //int iInicial; //próprio i do corner é o i inicial
    //int jInicial;

    int iFinal; /*Onde o i do shape termina (perna que varia horizontalmente do L_SHAPE)*/
    int jFinal; /*Onde o j do shape termina (perna que varia verticalmente do L_SHAPE)*/

    int corneri; /*Posição i do pixel de corner*/
    int cornerj; /*Posição j do pixel de corner*/

    int pixel_merge;
    //int pixel_merge; //guarda se é o de cima, de baixo, esquerda ou direita os pixels que vão ser mergidos
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

void computaBorda(Imagem* img, Imagem* bordas, Imagem* bordas_falsas) {
    Imagem* dx = criaImagem(img->largura, img->altura, img->n_canais);
    Imagem* dy = criaImagem(img->largura, img->altura, img->n_canais);
    zeraImagem(dx);
    zeraImagem(dy);

    for(int i = 1; i < img->altura-1; i++) {
        for(int j = 1; j < img->largura-1; j++) {
            if(fabs(img->dados[0][i][j] - img->dados[0][i][j+1]) > THRESHOLD) { //marca pixels de borda, coners tem valor de 2*borda. borda falsa será misturada
                if(img->dados[0][i][j] < img->dados[0][i][j+1]) {
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
    for(int i = 1; i < img->largura - 1; i++) {
        for(int j = 1; j < img->altura - 1; j++) {
            if(fabs(img->dados[0][j][i] - img->dados[0][j+1][i]) > THRESHOLD) {
                if(img->dados[0][j][i] < img->dados[0][j+1][i]) {
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
            if(bordas->dados[0][i][j] == 2*PONTA_FALSA) {
                bordas->dados[0][i][j] = PONTA_FALSA;
            }
            if(dx->dados[0][i][j] == PONTA_FALSA || dy->dados[0][i][j] == PONTA_FALSA) {
                bordas_falsas->dados[0][i][j] = BORDA;
                if((dx->dados[0][i][j] + dy->dados[0][i][j]) == )
                //if(dx->dados[0][i][j] == PONTA_FALSA && dy->dados[0][i][j] == PONTA_FALSA) {
                    //bordas_falsas->dados[0][i][j] += BORDA;
                //}
            }
            if(dx->dados[0][i][j] == BORDA || dy->dados[0][i][j] == BORDA) {
                bordas_falsas->dados[0][i][j] = PONTA_FALSA;
            }
            
        }
    }

    /*for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(bordas->dados[0][i][j] != CORNER) {
                bordas->dados[0][i][j] = 0;
            }
        }
    }*/
    destroiImagem(dx);
    destroiImagem(dy);
    salvaImagem(bordas_falsas, "minhasBordasFalsas.bmp");
    salvaImagem(bordas, "minhasBordas.bmp");
}

void achaShapes(Imagem* bordas, SHAPES* shapes) {
    int j, i, h;
    Imagem* aux = criaImagem(bordas->largura, bordas->altura, 1);
    zeraImagem(aux);
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
                aux->dados[0][i][j] = CORNER;
            }
        }
    }
    salvaImagem(aux,"cornerss.bmp");

    for(h = 0; h < shapes->quantidade; h++) { //aqui eu quero navegar pelos shapes e marcar o L deles
        int perna_menor = 1;
        int quantidade_atual = 0;
        int limite;
        int jFinal;
        int iFinal;
        //aux->dados[0][shapes->l_shapes[h].corneri][shapes->l_shapes[h].cornerj] = CORNER;

        //só entra em dois desses ifs, para cada shape
        if(bordas->dados[0][shapes->l_shapes[h].corneri][shapes->l_shapes[h].cornerj+1] == BORDA) { //direita
            limite = perna_menor == 0 ? 1 : 4;
            for(j = shapes->l_shapes[h].cornerj+1; j < bordas->largura && quantidade_atual < 4 && bordas->dados[0][shapes->l_shapes[h].corneri][j] == BORDA; j++) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                aux->dados[0][shapes->l_shapes[h].corneri][j] = BORDA;
                quantidade_atual++;
            }
            if(quantidade_atual > 2)
                perna_menor = 0;
            jFinal = shapes->l_shapes[h].cornerj + (quantidade_atual);
            quantidade_atual = 0;
        }
        else if(bordas->dados[0][shapes->l_shapes[h].corneri][shapes->l_shapes[h].cornerj-1] == BORDA) { //esquerda
            limite = perna_menor == 0 ? 1 : 4;
            for(j = shapes->l_shapes[h].cornerj-1; j >= 0 && quantidade_atual < limite && bordas->dados[0][shapes->l_shapes[h].corneri][j] == BORDA; j--) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                aux->dados[0][shapes->l_shapes[h].corneri][j] = BORDA;
                quantidade_atual++;
            }
            if(quantidade_atual > 2)
                perna_menor = 0;
            jFinal = shapes->l_shapes[h].cornerj - (quantidade_atual);
            quantidade_atual = 0;
        }
        if(bordas->dados[0][shapes->l_shapes[h].corneri+1][shapes->l_shapes[h].cornerj] == BORDA) { //baixo
            limite = perna_menor == 0 ? 1 : 4;
            for(i = shapes->l_shapes[h].corneri+1; i < bordas->altura && quantidade_atual < limite && bordas->dados[0][i][shapes->l_shapes[h].cornerj] == BORDA; i++) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                aux->dados[0][i][shapes->l_shapes[h].cornerj] = BORDA;
                quantidade_atual++;
            }
            if(quantidade_atual > 2)
                perna_menor = 0;
            iFinal = shapes->l_shapes[h].corneri + (quantidade_atual);
            quantidade_atual = 0;
        }
        else if(bordas->dados[0][shapes->l_shapes[h].corneri-1][shapes->l_shapes[h].cornerj] == BORDA) { //cima
            limite = perna_menor == 0 ? 1 : 4;
            for(i = shapes->l_shapes[h].corneri-1; i >= 0 && quantidade_atual < limite && bordas->dados[0][i][shapes->l_shapes[h].cornerj] == BORDA; i--) {  //if para se chegar a 5 ou se chegar ao final da largura da imagem
                aux->dados[0][i][shapes->l_shapes[h].cornerj] = BORDA;
                quantidade_atual++;
            }
            if(quantidade_atual > 2)
                perna_menor = 0;
            iFinal = shapes->l_shapes[h].corneri - (quantidade_atual);
            quantidade_atual = 0;
        }

        shapes->l_shapes[h].jFinal = jFinal;
        shapes->l_shapes[h].iFinal = iFinal;
        perna_menor = 1;
    }
    printf("quantidade: %d\n", shapes->quantidade);
    salvaImagem(aux, "L_marcado.bmp");
    destroiImagem(aux);
}



void marcaShapes(Imagem* img, SHAPES* shapes) {
    Imagem* aux = criaImagem(img->largura, img->altura, 1);
    zeraImagem(aux);
    for(int h = 0; h < shapes->quantidade; h++) {
        printf("%d\n", h);
        aux->dados[0][shapes->l_shapes[h].corneri][shapes->l_shapes[h].cornerj] = 1;

        if(shapes->l_shapes[h].jFinal < shapes->l_shapes[h].cornerj) {
            for(int j = shapes->l_shapes[h].jFinal; j < shapes->l_shapes[h].cornerj; j++) {
                aux->dados[0][shapes->l_shapes[h].corneri][j] = 0.200;
            }
        }
        else {
            for(int j = shapes->l_shapes[h].cornerj+1; j <= shapes->l_shapes[h].jFinal; j++) {
                aux->dados[0][shapes->l_shapes[h].corneri][j] = 0.200;
            }
        }

        if(shapes->l_shapes[h].iFinal < shapes->l_shapes[h].corneri) {
            for(int i = shapes->l_shapes[h].iFinal; i < shapes->l_shapes[h].corneri; i++) {
                aux->dados[0][i][shapes->l_shapes[h].cornerj] = 0.200;
            }
        }
        else {
            for(int i = shapes->l_shapes[h].corneri+1; i <= shapes->l_shapes[h].iFinal; i++) {
                aux->dados[0][i][shapes->l_shapes[h].cornerj] = 0.200;
            }
        }
    }
    salvaImagem(aux, "imagem_com_L.bmp");
}

void calculaPesos(float pesos[][5][5]) { //acessa linha, coluna
    //L com 
    for(int i = 0; i < 2; i++) 
        for(int j = 0; j < 5; j++) 
            for(int h = 0; h < 5; h++) 
                pesos[i][j][h] = 0;

    //MENOR PERNA DO L IGUAL A 1
    //somente um pixel na maior perna
    pesos[0][0][0] = 0.125;
    //dois pixels na maior perna
    pesos[0][1][0] = 0.25;
    //tres pixels na maior perna
    pesos[0][2][0] = 1/24;
    pesos[0][2][1] = 1/3;
    //quatro pixels na maior perna
    pesos[0][3][0] = 0.125;
    pesos[0][3][1] = 3/8;
    //cinco pixels na maior perna
    pesos[0][4][0] = 1/24;//
    pesos[0][4][1] = 3/8;//
    pesos[0][5][1] = 1/24;

    //MENOR PERNA DO L IGUAL A 2
    //somente um pixel na maior perna
    pesos[1][0][0] = 0.25;
    //dois pixels na maior perna
    pesos[1][1][0] = 0.500;
    //tres pixels na maior perna
    pesos[1][2][0] = 2/24;
    pesos[1][2][1] = 2/3;
    //quatro pixels na maior perna
    pesos[1][3][0] = 0.25;
    pesos[1][3][1] = 6/8;
    //cinco pixels na maior perna
    pesos[1][4][0] = 1/24;//
    pesos[1][4][1] = 3/8;//
    pesos[1][5][1] = 2/24;



}

void merge(Imagem* img, Imagem* bordas, Imagem* bordas_falsas) {
    float pesos[2][5][5];
    calculaPesos(pesos);
}

int main ()
{
    Imagem* img;
    Imagem* bordas;
    Imagem* bordas_falsas;
    //Imagem* magnitude;
    //Imagem* orientacao;
    Imagem* resize;
    Imagem* corners;
    SHAPES shapes;
    shapes.quantidade = 0;

    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);

    //img_edges = criaImagem(img->largura, img->altura, img->n_canais);
    //magnitude = criaImagem(img->largura, img->altura, img->n_canais);
    //orientacao = criaImagem(img->largura, img->altura, img->n_canais);
    //zeraImagem(img_edges);

    corners = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    bordas_falsas = criaImagem(img->largura, img->altura, 1);
    bordas = criaImagem(img->largura, img->altura, 1);
    zeraImagem(bordas);
    zeraImagem(bordas_falsas);
    computaBorda(img, bordas, bordas_falsas);
    //classificaPixelsBordas(img, corners);
    //classificaPixelsBordas(img, bordas);
    //achaShapes(img, corners, &shapes);

    achaShapes(bordas, &shapes);

    //marcaShapes(img, &shapes);

    //computaGradientes (img, 3, NULL, NULL, magnitude, orientacao);
    
    //arrumaMagnitude(magnitude, img_edges);
    //arrumaMagnitude(img, img_edges);
    
    //destroiImagem(orientacao);

    //salvaImagem(img_edges, "bordas.bmp");
    //salvaImagem(magnitude, "magnitude.bmp");
    destroiImagem(img);
    //destroiImagem(img_edges);
    //destroiImagem(magnitude);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");
    return 0;
}