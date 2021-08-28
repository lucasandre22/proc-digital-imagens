#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>

#define INPUT_IMAGE "0.bmp"
#define BACKGROUND "fundo.bmp"
#define CANAIS_IMAGEM 3

void tiraVerdeImagemOriginal(Imagem* img, Imagem* hsl) {
    //estima pelo valor considerado verde no hue
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) { 
            //se hsl acusa que é verde
            if(hsl->dados[0][i][j] >= 50 && hsl->dados[0][i][j] <= 140) {
                //se é mesmo verde
                if((img->dados[1][i][j] > img->dados[0][i][j] && img->dados[1][i][j] > img->dados[2][i][j])) {
                    //se não é uma mistura do verde com alguma outra cor, se possuem valores muito próximos quer dizer que é mistura então não deve ser considerado (estava tendo problemas com amarelo antes)
                    //constante 0.050 é meio aleatória, só estimei um valor de diferença mínima p/ não ser mistura
                    if((img->dados[1][i][j] - img->dados[0][i][j]) > 0.050 && (img->dados[1][i][j] - img->dados[2][i][j]) > 0.050) {
                        //img->dados[1][i][j] = (img->dados[0][i][j] + img->dados[2][i][j]) / 2;
                        img->dados[1][i][j] = img->dados[0][i][j] > img->dados[2][i][j] ? img->dados[0][i][j] : img->dados[2][i][j];
                    }
                }
            }
        }
    }
    //se ainda tiver verde... faço ele receber o maior valor dos outros canais
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(img->dados[1][i][j] > img->dados[0][i][j] && img->dados[1][i][j] > img->dados[2][i][j]) {
                //if((img->dados[1][i][j] - img->dados[0][i][j]) > 0 && (img->dados[1][i][j] - img->dados[2][i][j]) > 0) {
                    img->dados[1][i][j] = img->dados[0][i][j] > img->dados[2][i][j] ? img->dados[0][i][j] : img->dados[2][i][j];
                //}
            }
        }
    }
    salvaImagem(img, "img-depois-tirar-verde.bmp");
}

void arrumaTransicao(Imagem* img, Imagem* mascara) {
    Imagem* magnitude = criaImagem(img->largura, img->altura, 1);
    Imagem* orientacao = criaImagem(img->largura, img->altura, 1);
    Imagem* hsl = criaImagem(img->largura, img->altura, 3);
    Imagem* mascara_borrada = criaImagem(img->largura, img->altura, 1);
    
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            magnitude->dados[0][i][j] = 0.0f;
        }
    }

    computaGradientes(mascara, 7, NULL, NULL, magnitude, orientacao);
    destroiImagem(orientacao);

    RGBParaHSL(img, hsl);

    tiraVerdeImagemOriginal(img, hsl);

    filtroGaussiano(mascara, mascara_borrada, 1, 1, NULL);
    
    
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(magnitude->dados[0][i][j] != 0.0f) {
                mascara->dados[0][i][j] = mascara_borrada->dados[0][i][j];
            }
        }
    }

    destroiImagem(hsl);
    destroiImagem(mascara_borrada);
    destroiImagem(magnitude);
}

void arrumaTransicaoEJuntaFundo(Imagem* img, Imagem* mascara, Imagem* fundo) {
    float peso1;
    float peso2;

    arrumaTransicao(img, mascara);
    
    for(int h = 0; h < 3; h++) {
        for(int i = 0; i < img->altura; i++) {
            for(int j = 0; j < img->largura; j++) {
                peso1 = mascara->dados[0][i][j] * fundo->dados[h][i][j];
                peso2 = (1.0f-mascara->dados[0][i][j]) * img->dados[h][i][j];
                img->dados[h][i][j] = peso1 + peso2;
            }
        }
    }

}

void criaMascaraBinaria(Imagem* img, Imagem* mascara, Imagem* fundo) {
    //Imagem* mascara = criaImagem(img->largura, img->altura, 1);
    float media_diferenca_verde = 0;
    int count = 0;
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(img->dados[1][i][j] > img->dados[0][i][j] && img->dados[1][i][j] > img->dados[2][i][j]) {
                media_diferenca_verde += img->dados[1][i][j] - ((img->dados[0][i][j] + img->dados[2][i][j])/2); //faço media da diferença entre verde e os outros canais, quando é verde.
                count++;
            }
        }
    }

    media_diferenca_verde = media_diferenca_verde/count;

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(img->dados[1][i][j] > img->dados[0][i][j] && img->dados[1][i][j] > img->dados[2][i][j]) {
                if(fabs(img->dados[0][i][j] - img->dados[2][i][j]) < 0.300) //quando é verde, eu sei que os outros canais têm valores parecidos, constante 0.300 veio de algumas tentativas
                    
                    //aqui estimo a diferença entre o canal verde e os demais, dividi a média por 2 para não ser muito alto
                    //para ser fundo
                    if(fabs(img->dados[1][i][j] - img->dados[0][i][j]) > media_diferenca_verde/2.0 && fabs(img->dados[1][i][j] - img->dados[2][i][j]) > media_diferenca_verde/2.0) {
                        mascara->dados[0][i][j] = 1;
                    }
                       
            } else
                mascara->dados[0][i][j] = 0;
        }
    }
}

int main ()
{
    Imagem* img;
    Imagem* fundo;
    Imagem* fundo_aux;
    Imagem* mascara;

    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);
    fundo_aux = abreImagem(BACKGROUND, CANAIS_IMAGEM);
    fundo = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    mascara = criaImagem(img->largura, img->altura, 1);
    redimensionaBilinear(fundo_aux, fundo);

    criaMascaraBinaria(img, mascara, fundo);
    arrumaTransicaoEJuntaFundo(img, mascara, fundo);
 
    salvaImagem(img, "img-final.bmp");
    salvaImagem(mascara, "img-mascara.bmp");
    destroiImagem(mascara);
    destroiImagem(img);
    destroiImagem(fundo_aux);
    destroiImagem(fundo);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");

    return 0;
}