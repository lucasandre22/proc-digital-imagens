#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>

#define INPUT_IMAGE "2.bmp"
#define CANAIS_IMAGEM 3

void juntaImagens(Imagem* img1, Imagem* img2, Imagem* out, float limiar_g, float limiar_r, float limiar_b);
void separaFundo(Imagem* img, Imagem* out, float limiar_g, float limiar_r, float limiar_b);
void separaVerde(Imagem* img, Imagem* out, float limiar_g, float limiar_r, float limiar_b);

void misturaDuasImagens(Imagem* img1, Imagem* img2, Imagem* out) {
    for(int h = 0; h < 3; h++) {
        for(int i = 0; i < img1->altura; i++) {
            for(int j = 0; j < img2->largura; j++) {
                out->dados[h][i][j] = (0.5 * img1->dados[h][i][j]) + (0.5 * img2->dados[h][i][j]);
            }
        }
    }
    //filtroSobel
}

void teste(Imagem* img) {
    Imagem* mascara_verde = criaImagem(img->largura, img->altura, 3);
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            mascara_verde->dados[0][i][j] = (0.5 * img->dados[0][i][j]) + (0.5 * img->dados[0][i][j]);
        }
    }
    //filtroSobel
}

void montaHSL(Imagem* img) {
    Imagem* img_hsl = criaImagem(img->largura, img->altura, 3);
    Imagem* mascara_hsl = criaImagem(img->largura, img->altura, 1);
    Imagem* canal = criaImagem(img->largura, img->altura, 1);
    RGBParaHSL(img, img_hsl);
    //for(int h = 0; h < 3; h++) {
        for(int i = 0; i < img->altura; i++) {
            for(int j = 0; j < img->largura; j++) {
                canal->dados[0][i][j] = img_hsl->dados[0][i][j];
            }
        }
        //if( h == 0)
            img_hsl->n_canais = 1;
            salvaImagem(img_hsl, "canal-0.bmp");
            img_hsl->n_canais = 3;
        //else if(h == 1)
           // salvaImagem(canal, "canal-1.bmp");
        //else
            //salvaImagem(canal, "canal-2.bmp");
    //}
    
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            //if(img_hsl->dados[0][i][j] < 0.45825 && img_hsl->dados[0][i][j] > 0.2085) //.130 mais aparece
            if(img_hsl->dados[0][i][j] < 0.200)
                mascara_hsl->dados[0][i][j] = 0;
            else
                mascara_hsl->dados[0][i][j] = 1;
        }
    }
    salvaImagem(img_hsl, "img-hsl.bmp");
    //salvaImagem(mascara_hsl, "img-mascara-hsl.bmp");
    destroiImagem(img_hsl);
    destroiImagem(mascara_hsl);
    destroiImagem(canal);
}

//void descobreVerde()

int main ()
{
    Imagem* img;
    
    Imagem* img_mascara;
    Imagem* img_out;
    Imagem* img_fundo;
    Imagem* img_fundo_aux;
    Imagem* teste;
    Imagem* ajuste;
    Imagem* magnitude;
    Imagem* orientacao;

    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);
    img_mascara = criaImagem(img->largura, img->altura, 1);
    img_fundo = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    teste = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    magnitude = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    orientacao = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    montaHSL(img);
    //ajuste = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    //ajustaBrilhoEContraste(img, ajuste, 0.1, 0.9);
    //salvaImagem(ajuste, "img-ajuste.bmp");
    computaGradientes (img, 3, NULL, NULL, magnitude, orientacao);
    salvaImagem(magnitude, "magnitude.bmp");
    salvaImagem(orientacao, "orientacao.bmp");
    destroiImagem(magnitude);
    destroiImagem(orientacao);
    
    img_fundo_aux = abreImagem("fundo.bmp", CANAIS_IMAGEM);
    redimensionaBilinear(img_fundo_aux, img_fundo);
    separaFundo(img, img_mascara, 0,0,0);
    //separaVerde(img, img_mascara, 0,0,0);

    misturaDuasImagens(img, img_fundo, teste);
    salvaImagem(teste, "teste.bmp");
    destroiImagem(teste);

    salvaImagem(img_mascara, "img-mascara.bmp");
    //salvaImagem(img_out, "img-out.bmp");

    destroiImagem(img);
    
    destroiImagem(img_mascara);
    //destroiImagem(img_out);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");
    return 0;
}

void separaFundo(Imagem* img, Imagem* mascara, float limiar_g, float limiar_r, float limiar_b) {
    Imagem* magnitude = criaImagem(img->largura, img->altura, 1);
    Imagem* orientacao = criaImagem(img->largura, img->altura, 1);
    Imagem* escala_cinza = criaImagem(img->largura, img->altura, 1);
    Imagem* aux = criaImagem(img->largura, img->altura, 1);
    Imagem* mascara2 = criaImagem(img->largura, img->altura, 1);
    //RGBParaCinza(img, escala_cinza);

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(img->dados[1][i][j] > img->dados[0][i][j] && img->dados[1][i][j] > img->dados[2][i][j]) {
                mascara2->dados[0][i][j] = 0;
            }
            else
                mascara2->dados[0][i][j] = img->dados[1][i][j];

            mascara->dados[0][i][j] = img->dados[1][i][j] - (0.65*img->dados[0][i][j] + 0.65*img->dados[2][i][j]);
            // mascara->dados[0][i][j] = img->dados[1][i][j] - (0.65*img->dados[0][i][j] + 0.3*img->dados[2][i][j]);
            //if(img->dados[1][i][j] > 0.2f)
                //mascara->dados[0][i][j] = img->dados[1][i][j];
           // else
                //mascara->dados[0][i][j] = 0;
            
            escala_cinza->dados[0][i][j] = img->dados[1][i][j];
        }
    }
    float media = 0;
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            media += mascara->dados[0][i][j];
        }
    }
    media = media/(img->largura*img->largura);
    printf("media = %f", media);
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(mascara->dados[0][i][j] > media)
                mascara->dados[0][i][j] = 0;
            else
                mascara->dados[0][i][j] = 1;
        }
    }
    /*for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(mascara->dados[0][i][j] > media)
                mascara->dados[0][i][j] = 0;
            else
                mascara->dados[0][i][j] = 1;
        }
    }*/
    computaGradientes (escala_cinza, 1, NULL, NULL, magnitude, orientacao);

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(mascara->dados[0][i][j] == 1) {
                aux->dados[0][i][j] = ((img->dados[1][i][j] + img->dados[0][i][j] + img->dados[2][i][j])/2);
            }
            else
            {
                aux->dados[0][i][j] = (1-img->dados[1][i][j]) + img->dados[0][i][j] + img->dados[2][i][j];
            }
        }
    }

    /*for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            mascara2->dados[0][i][j] += mascara->dados[0][i][j];
        }
    }*/

    salvaImagem(mascara2, "img-mascara2.bmp");

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            mascara->dados[0][i][j] += 2*magnitude->dados[0][i][j];
        }
    }
    //salvaImagem(aux, "aux.bmp");
    salvaImagem(magnitude, "img-mascara-magnitude.bmp");
    salvaImagem(mascara, "img-mascara.bmp");
    destroiImagem(magnitude);
    destroiImagem(orientacao);
}

void juntaImagens(Imagem* img1, Imagem* img2, Imagem* out, float limiar_g, float limiar_r, float limiar_b) {

    for(int i = 0; i < img1->altura; i++) {
        for(int j = 0; j < img2->largura; j++) {
            if(img1->dados[1][i][j] >= 0.99) { //fundo verde

            }
                                                    //fundo verde + parte da imagem
        }
    }

}

void separaVerde(Imagem* img, Imagem* out, float limiar_g, float limiar_r, float limiar_b) {
    float media_verde = 0;
    float media_vermelho = 0;
    float media_azul = 0;
    int qtd = 0;
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            media_vermelho += img->dados[0][i][j];
            media_azul += img->dados[1][i][j];
        }
    }
    media_vermelho = media_vermelho/(img->altura*img->largura);
    media_azul = media_azul/(img->altura*img->largura);
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            //if(img->dados[0][i][j] < media_vermelho && img->dados[2][i][j] < media_azul)
            if(img->dados[1][i][j] - img->dados[0][i][j] > 70 && img->dados[1][i][j] - img->dados[2][i][j] > 70)
                out->dados[0][i][j] = 1;
            else
                out->dados[0][i][j] = 0;
        }
    }
}