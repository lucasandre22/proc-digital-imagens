#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pdi.h"

#define INPUT_IMAGE "flowers.bmp"
#define LARGURA_JANELA 11
#define ALTURA_JANELA 15
#define CANAIS_IMAGEM 3

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

    img = abreImagem (INPUT_IMAGE, CANAIS_IMAGEM);
    img_out1 = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    img_out2 = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    img_out3 = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);

    filtroMediaIngenuo(img, img_out1, LARGURA_JANELA, ALTURA_JANELA, CANAIS_IMAGEM);
    filtroMediaSeparavel(img, img_out2, LARGURA_JANELA, ALTURA_JANELA, CANAIS_IMAGEM);
    imagemIntegral(img,img_out3,LARGURA_JANELA, ALTURA_JANELA, CANAIS_IMAGEM);

    salvaImagem (img_out1, "img-out-ingenuo.bmp");
    salvaImagem (img_out2, "img-out-separavel.bmp");
    salvaImagem (img_out3, "img-out-separavel_integral.bmp");
    destroiImagem (img);
    destroiImagem (img_out1);
    destroiImagem (img_out2);
    destroiImagem (img_out3);

    printf("Pressione qualquer tecla para sair\n");
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
                soma += img->dados[h][i][j]; //salvando soma anterior
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
    float soma = 0;
    Imagem* imagem_aux = criaImagem (img->largura, img->altura, CANAIS_IMAGEM);
    //monta imagem integral
    for(int h = 0; h < canais; h++) {
        for(int i = 0; i < img->altura; i++) {
            for(int j = 0; j < img->largura; j++) {
                soma = img->dados[h][i][j];
                if(j > 0) {
                    soma += imagem_aux->dados[h][i][j-1]; //soma todos valores esquerda
                }

                if(i > 0) {
                    soma += imagem_aux->dados[h][i-1][j]; //soma todos valores da coluna de cima
                    if(j > 0)
                        soma -= imagem_aux->dados[h][i-1][j-1];
                }
                imagem_aux->dados[h][i][j] = soma;
            }
        }
    }
    soma = 0;
    //
    for(int h = 0; h < canais; h++) {
        for(int i = (altura/2), linha = 0; i < (img->altura + altura/2); i++, linha++) {
            int foraDireita = 0; //o quanto a janela saiu pra fora na direita da imagem
            int foraBaixo = i >= img->altura ? (i-(img->altura-1)) : 0; //o quanto saiu pra baixo da imagem
            int divisao = 0;
            for(int j = (largura/2), coluna = 0; j < (img->largura + largura/2); j++, soma = 0, coluna++) {
                int alturaJanela, larguraJanela;

                if(j >= largura)
                    larguraJanela = largura;
                else
                    larguraJanela = j+1;/**Para descobrir tamanho da janela dos cantos quando j < largura ou i < altura, ou seja, quando a janela está pra fora, faço ele receber o index atual + 1,
                                           que é exatamente quantos pixels tanto da largura quanto altura estão "aparecendo" na imagem, então para obter
                                           o tamanho da janela basta multiplicar os dois.**/
                if(i >= altura)
                    alturaJanela = altura;
                else
                    alturaJanela = i+1;

                if(j < img->largura && i < img->altura) {
                    if(j >= largura) //Dava para ter feito um de cada desses ifs, sem duplicar-los logo abaixo no else. Porém achei melhor deste modo para minha melhor interpretação.
                        soma -= imagem_aux->dados[h][i][j-largura]; //esquerda baixo

                    if(i >= altura)
                        soma -= imagem_aux->dados[h][i-altura][j]; //direita cima

                    if(i >= altura && j >= largura)
                        soma += imagem_aux->dados[h][i-altura][j-largura]; //diagonal esquerda cima

                    soma += imagem_aux->dados[h][i][j];
                }else {//Está fora pra direita da imagem ou fora pra baixo da imagem
                    if(j >= img->largura) {
                        foraDireita++;
                        larguraJanela = larguraJanela - foraDireita;
                    }
                    if(i >= img->altura) {
                        alturaJanela = alturaJanela - foraBaixo;
                    }

                    if(j >= largura)
                        soma -= imagem_aux->dados[h][i-foraBaixo][j-largura]; //esquerda baixo

                    if(i >= altura)
                        soma -= imagem_aux->dados[h][i-altura][j-foraDireita]; //direita cima

                    if(i >= altura && j >= largura)
                        soma += imagem_aux->dados[h][i-altura][j-largura]; //diagonal esquerda cima

                    soma += imagem_aux->dados[h][i-foraBaixo][j-foraDireita];
                }

                divisao = alturaJanela*larguraJanela;
                out->dados[h][linha][coluna] = soma/divisao;
            }
        }
    }
    destroiImagem(imagem_aux);
}
