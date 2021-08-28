#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pdi.h"

#define INPUT_IMAGE "eyehand.bmp"
#define LARGURA_JANELA 3
#define ALTURA_JANELA 3
#define CANAIS_IMAGEM 1

/*============================================================================*/

void filtroMediaIngenuo(Imagem* img, Imagem* out, int largura, int altura, int canais);
void filtroMediaSeparavel(Imagem* img, Imagem* out, int largura, int altura, int canais);
void imagemIntegral(Imagem* img, Imagem* out, int largura, int altura, int canais);

int main ()
{
    Imagem* img;
    Imagem* img_out1;
    Imagem* img_out2;
    Imagem* img_out3;
    Imagem* img_out4;

    img = abreImagem (INPUT_IMAGE, CANAIS_IMAGEM);
    img_out1 = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    img_out2 = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    img_out4 = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    img_out3 = abreImagem("essa.bmp",1);
    //printf("%f %f\n", img->dados[0][0][0], img_out3->dados[0][0][0]);
    //printf("%f %f\n", img->dados[0][0][1], img_out3->dados[0][0][1]);
    //float lala = (img->dados[0][0][0] + img->dados[0][0][1] + img->dados[0][1][0] + img->dados[0][1][1])/4;
    //float lala2 = (img->dados[0][0][0] + img->dados[0][0][1])/2;
    //float lala3 = (img->dados[0][0][0] + img->dados[0][1][0])/2;
    //printf("%f %f %f\n", lala, lala2, lala3);

    //printf("%f %f %f\n %f %f %f\n %f %f %f\n", img->dados[0][0][0], img->dados[0][0][1], img->dados[0][0][2], img->dados[0][1][0], img->dados[0][1][1], img->dados[0][1][2], img->dados[0][2][0],
           //img->dados[0][2][1], img->dados[0][2][2]);

    filtroMediaIngenuo(img, img_out1, LARGURA_JANELA, ALTURA_JANELA, CANAIS_IMAGEM);
    filtroMediaSeparavel(img, img_out2, LARGURA_JANELA, ALTURA_JANELA, CANAIS_IMAGEM);
    imagemIntegral(img,img_out4,LARGURA_JANELA, ALTURA_JANELA, CANAIS_IMAGEM);
    //img = abreImagem ("flowers.bmp", 3); // Imagem colorida.
    //salvaImagem (img, "flowers2.bmp");
    //destroiImagem (img);
    //printf("Minha imagem: %f Imagem do professor: %f\n", img_out4->dados[0][0][img->largura-1], img_out3->dados[0][0][img->largura-1]);
    //printf("Minha imagem: %f Imagem do professor: %f\n", img_out4->dados[0][50][50], img_out3->dados[0][50][50]);

    //img = abreImagem ("eyehand.bmp", 1);
    //img2 = clonaImagem(img);

    salvaImagem (img_out1, "img-out-ingenuo.bmp");
    salvaImagem (img_out2, "img-out-separavel.bmp");
    salvaImagem (img_out4, "img-out-separavel_integral.bmp");
    destroiImagem (img);
    destroiImagem (img_out1);
    destroiImagem (img_out2);
    destroiImagem (img_out3);
    destroiImagem (img_out4);


    scanf("%c");
    return 0;
}

void filtroMediaIngenuo(Imagem* img, Imagem* out, int largura, int altura, int canais)
{
    float soma = 0;
    for(int h = 0; h < canais; h++) {
        for(int i = altura/2; i < (img->altura-(altura/2)); i++) {
            for(int j = largura/2; j < (img->largura-(largura/2)); j++) {

                for(int k = i-(altura/2); k <= i+(altura/2); k++) {
                    for(int m = j-(largura/2); m <= j+(largura/2); m++)
                        soma += img->dados[h][k][m];
                }

            out->dados[h][i][j] = soma/(largura*altura);
            soma = 0;
            }
        }
    }
}

void filtroMediaSeparavel(Imagem* img, Imagem* out, int largura, int altura, int canais)
{
    float soma = 0;
    Imagem* imagem_aux = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    for(int h = 0; h < canais; h++) {
        for(int i = 0; i < img->altura; i++) {

            for(int j = 0; j < img->largura; j++) {
                soma += img->dados[h][i][j];
                if(j >= (largura-1)) {
                    imagem_aux->dados[h][i][(j-(largura/2))] = soma;
                    soma -= img->dados[h][i][j-(largura-1)];
                }

            }
            soma = 0;
        }
    }
    //soma = 0;
    for(int h = 0; h < canais; h++) {
        for(int i = (largura/2); i < (img->largura-(largura/2)); i++) {
            for(int j = 0; j < img->altura; j++) {
                soma += imagem_aux->dados[h][j][i];
                if(j >= (altura-1)) {
                    out->dados[h][(j-(altura/2))][i] = (soma/(largura*altura));
                    soma -= imagem_aux->dados[h][j-(altura-1)][i];
                }
            }
            soma = 0;
        }
    }

    destroiImagem(imagem_aux);
}

void imagemIntegral(Imagem* img, Imagem* out, int largura, int altura, int canais)
{
    double soma = 0; //+ precisão
    Imagem* imagem_aux = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    for(int h = 0; h < canais; h++) {
        for(int i = 0; i < img->altura; i++) {
            for(int j = 0; j < img->largura; j++) {
                soma = img->dados[h][i][j];
                if(j > 0) {
                    soma += imagem_aux->dados[h][i][j-1];
                }

                if(i > 0) {
                    soma += imagem_aux->dados[h][i-1][j]; //soma a de cima
                    if(j > 0)
                        soma -= imagem_aux->dados[h][i-1][j-1];
                }
                imagem_aux->dados[h][i][j] = soma;
            }
        }
    }
    //printf("0 0 %f \n", imagem_aux->dados[0][0][0]);
    //printf("Altura %d Largura %f\n", img->altura, img->largura);
    soma = 0;
    for(int h = 0; h < canais; h++) {
        for(int i = (altura/2), linha = 0; i < (img->altura + altura/2); i++, linha++) {
            int foraDireita = 0;
            int foraBaixo = i >= img->altura ? (i-(img->altura-1)) : 0;
            int divisao = 0;
            for(int j = (largura/2), coluna = 0; j < (img->largura + largura/2); j++, soma = 0, coluna++) {
                int aux1, aux2;

                if(j >= largura)
                    aux2 = largura;
                else
                    aux2 = j+1; //para descobrir tamanho da janela dos cantos quando j < largura, faço ele receber o j + 1
                if(i >= altura)
                    aux1 = altura;
                else
                    aux1 = i+1;

                if(j < img->largura && i < img->altura) {
                    if(j >= largura)
                        soma -= imagem_aux->dados[h][i][j-largura];
                    if(i >= altura)
                        soma -= imagem_aux->dados[h][i-altura][j];
                    if(i >= altura && j >= largura)
                        soma += imagem_aux->dados[h][i-altura][j-largura];
                    soma += imagem_aux->dados[h][i][j];
                }else {
                    if(j >= img->largura) {
                        foraDireita++;
                        aux2 = aux2 - foraDireita;
                    }
                    if(i >= img->altura) {
                        aux1 = aux1 - foraBaixo;
                    }
                   // printf("%d %d", i-foraBaixo, j-foraDireita);

                    //if(j >= img->largura || i >= img->altura && j >= largura) {
                    if(i >= img->altura && j >= largura || j >= img->largura) {
                        soma -= imagem_aux->dados[h][i-foraBaixo][j-largura]; //esquerda baixo
                        //printf(" passou esquerda");
                    }

                    if(i >= altura) {
                        soma -= imagem_aux->dados[h][i-altura][j-foraDireita]; //direita cima
                        //printf(" direita cima");
                    }

                    //if(i >= altura && j >= img->largura || i >= img->largura && j >= largura) {
                    if(i >= altura && j >= largura) {
                        //printf(" esquerda cima");
                        soma += imagem_aux->dados[h][i-altura][j-largura];
                    }


                    soma += imagem_aux->dados[h][i-foraBaixo][j-foraDireita];
                    //printf(" somou! %f \n", soma/(aux1*aux2));
                }








                divisao = aux1*aux2;
                if(i == (altura/2) && j == (largura/2))
                    printf("divisao %d", divisao);
                out->dados[h][linha][coluna] = soma/divisao;
            }
        }
    }

    //soma esquerda, direita e diminui a diagonal cima esquerda
}



    /*for(int h = 0; h < canais; h++) {
        int foraBaixo = 0;
        for(int i = (altura/2), linha = 0; i < (img->altura + altura/2); i++, largura++, linha++) {
            int foraDireita = 0;
            for(int j = (largura/2), coluna = 0, divisao = 0; j < (img->largura + largura/2); j++, soma = 0, coluna++) {
                int aux1, aux2, aux3;

                if(j >= largura)
                    aux2 = largura;
                else
                    aux2 = j+1; //para descobrir tamanho da janela dos cantos quando j < largura, faço ele receber o j + 1
                if(i >= altura)
                    aux1 = altura;
                else
                    aux1 = i+1;

                if(i < img->largura && j < img->altura) {
                    if(j >= largura)
                        soma -= imagem_aux->dados[h][i][j-largura];
                    if(i >= altura)
                        soma -= imagem_aux->dados[h][i-altura][j];
                    if(i >= altura && j >= largura)
                        soma += imagem_aux->dados[h][i-altura][j-largura];
                    soma += imagem_aux->dados[h][i][j];
                }
                /*else {
                    if(j >= img->largura) {
                        foraDireita++;
                        aux2 = aux2 - foraDireita;
                    }
                    if(i >= img->altura) {
                        foraBaixo++;
                        aux1 = aux1 - foraBaixo;
                    }

                    if(j >= img->largura)
                        soma -= imagem_aux->dados[h][i-foraBaixo][j-largura]; //esquerda baixo
                    if(i >= img->altura)
                        soma -= imagem_aux->dados[h][i-altura][j-foraDireita];
                    if(i >= img->altura && j >= img->largura)
                        soma += imagem_aux->dados[h][i-altura][j-largura];

                    /*if(j >= img->largura-(largura/2))
                        soma -= imagem_aux->dados[h][i][j-largura];
                    if(i >= img->altura-(altura/2))
                        soma -=
                    if(i >= img->altura-(altura/2) && j >= img->largura-(largura/2))

                    soma += imagem_aux->dados[h][i-foraBaixo][j-foraDireita];
                }








                //out->dados[h][i-(altura/2)][j-(largura/2)] = (imagem_aux->dados[h][i][j] - imagem_aux->dados[h][i-altura][j] - imagem_aux->dados[h][i][j-largura] + imagem_aux->dados[h][i-altura][j-largura])/(largura*altura);
                divisao = aux1*aux2;
                out->dados[h][linha][coluna] = soma/divisao;
            }
        }
    }
    printf(" lala\n");
    //soma esquerda, direita e diminui a diagonal cima esquerda
}*/



//FUNFANDO
/*for(int h = 0; h < canais; h++) {
        for(int i = (altura/2), linha = 0; i < img->altura; i++, linha++) {
            for(int j = (largura/2), coluna = 0, divisao = 0; j < img->largura; j++, soma = 0, coluna++) {
                int aux1, aux2;

                if(j >= largura)
                    aux2 = largura;
                else
                    aux2 = j+1; //para descobrir tamanho da janela dos cantos quando j < largura, faço ele receber o j + 1
                if(i >= altura)
                    aux1 = altura;
                else
                    aux1 = i+1;

                if(j >= largura)
                    soma -= imagem_aux->dados[h][i][j-largura];
                if(i >= altura)
                    soma -= imagem_aux->dados[h][i-altura][j];
                if(i >= altura && j >= largura)
                    soma += imagem_aux->dados[h][i-altura][j-largura];
                soma += imagem_aux->dados[h][i][j];






                //out->dados[h][i-(altura/2)][j-(largura/2)] = (imagem_aux->dados[h][i][j] - imagem_aux->dados[h][i-altura][j] - imagem_aux->dados[h][i][j-largura] + imagem_aux->dados[h][i-altura][j-largura])/(largura*altura);
                divisao = aux1*aux2;
                out->dados[h][linha][coluna] = soma/divisao;
            }
        }
    }*/

/*============================================================================*/
