#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>

#define INPUT_IMAGE "2.bmp"
#define CANAIS_IMAGEM 3

void funcaoMilagrosa(Imagem* img) {
    Imagem* mascara = criaImagem(img->largura, img->altura, 1);
    float diff;
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(fabs((img->dados[2][i][j] - img->dados[0][i][j]) < 0.100)) {
                diff = (img->dados[2][i][j] + img->dados[0][i][j])/2;
                diff = img->dados[1][i][j] - diff;
                mascara->dados[0][i][j] = diff * (img->dados[2][i][j] + img->dados[0][i][j]);
            }
        }
    }
    salvaImagem(mascara, "teste.bmp");

}

void tiraVerde(Imagem* img) {
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) { 
            if(img->dados[1][i][j] > img->dados[0][i][j] && img->dados[1][i][j] > img->dados[2][i][j])
                img->dados[1][i][j] = (img->dados[0][i][j] + img->dados[2][i][j])/2;
        }
    }
}

/*void juntaFundo(Imagem* img, Imagem* mascara, Imagem* fundo) {
    float media;
    tiraVerde(img);
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(mascara->dados[0][i][j] == 1.0f) {
                img->dados[0][i][j] = fundo->dados[0][i][j];
                img->dados[1][i][j] = fundo->dados[1][i][j];
                img->dados[2][i][j] = fundo->dados[2][i][j];
            }
            if(mascara->dados[0][i][j] != 1.0f && mascara->dados[0][i][j] != 0.0f) {
                media = (img->dados[0][i][j] + img->dados[2][i][j])/2;
                img->dados[1][i][j] = media;

                img->dados[0][i][j] = mascara->dados[0][i][j]*img->dados[0][i][j];
                img->dados[1][i][j] = mascara->dados[0][i][j]*img->dados[1][i][j];
                img->dados[2][i][j] = mascara->dados[0][i][j]*img->dados[2][i][j];

                img->dados[0][i][j] += (1.0f-mascara->dados[0][i][j])*fundo->dados[0][i][j];
                img->dados[1][i][j] += (1.0f-mascara->dados[0][i][j])*fundo->dados[1][i][j];
                img->dados[2][i][j] += (1.0f-mascara->dados[0][i][j])*fundo->dados[2][i][j];
            }
        }
    }
    salvaImagem(img, "imagem-foda-wikii.bmp");
}*/

void juntaFundo(Imagem* img, Imagem* mascara, Imagem* fundo) {
    float media;
    //arrumaTransicao(mascara);
    tiraVerde(img);
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(mascara->dados[0][i][j] == 0.0f) {
                img->dados[0][i][j] = fundo->dados[0][i][j];
                img->dados[1][i][j] = fundo->dados[1][i][j];
                img->dados[2][i][j] = fundo->dados[2][i][j];
            }
            else if(mascara->dados[0][i][j] != 1.0f && mascara->dados[0][i][j] != 0.0f) {

                img->dados[0][i][j] = mascara->dados[0][i][j] * img->dados[0][i][j];
                img->dados[1][i][j] = mascara->dados[0][i][j] * img->dados[1][i][j];
                img->dados[2][i][j] = mascara->dados[0][i][j] * img->dados[2][i][j];

                img->dados[0][i][j] += ((1.0f-mascara->dados[0][i][j]) * fundo->dados[0][i][j]);
                img->dados[1][i][j] += ((1.0f-mascara->dados[0][i][j]) * fundo->dados[1][i][j]);
                img->dados[2][i][j] += ((1.0f-mascara->dados[0][i][j]) * fundo->dados[2][i][j]);
            }
        }
    }
    salvaImagem(img, "imagem-foda-key.bmp");
}

void separaFundo(Imagem* img, Imagem* fundo) {
    Imagem* mascara = criaImagem(img->largura, img->altura, 1);
    float a, b;
    a = 1.5f;
    b = 0.5f;
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            //mascara->dados[0][i][j] = (a*(img->dados[0][i][j] + img->dados[2][i][j])) - (b*img->dados[1][i][j]);
            a = (img->dados[0][i][j] + img->dados[2][i][j]);
            mascara->dados[0][i][j] = (3*img->dados[1][i][j]) - a;
        }
    }
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            //mascara->dados[0][i][j] = (a*(img->dados[0][i][j] + img->dados[2][i][j])) - (b*img->dados[1][i][j]);
            mascara->dados[0][i][j] = 1.0f - mascara->dados[0][i][j];
        }
    }

    juntaFundo(img, mascara, fundo);
    salvaImagem(mascara, "imagem-wiki.bmp");
    destroiImagem(mascara);
}

int main ()
{
    Imagem* img;
    Imagem* fundo;
    Imagem* fundo_aux;

    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);
    fundo_aux = abreImagem("fundo.bmp", CANAIS_IMAGEM);
    fundo = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    redimensionaBilinear(fundo_aux, fundo);
    funcaoMilagrosa(img);
    separaFundo(img, fundo);
    //fazMascaraSeparacao(img);
 
    salvaImagem(img, "magnitude.bmp");
    destroiImagem(img);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");
    return 0;
}