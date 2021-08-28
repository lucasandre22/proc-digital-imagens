#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pdi.h"

#define INPUT_IMAGE "Wind Waker GC.BMP"
#define CANAIS_IMAGEM 3


//variaveis p/ filtros, sigma para o gaussiano porém utiliza-se na media tambem, devidamente aumentado. É aplicado o filtro QTD_FILTRAR vezes na mesma máscara, com sigmas diferentes.
//Wind Waker GC.bmp
#define SIGMA 10 //valores legais: 10, 15
#define QTD_FILTRAR 4 //valores legais: 4, 5
#define LIMIAR 0.5

//GT2.BMP
/*#define SIGMA 10 
#define QTD_FILTRAR 4
#define LIMIAR 0.520*/

#define ALFA 0.8 //valores multiplicadores na junção final da imagem, achei boa essa relação
#define BETA 0.2

/*============================================================================*/

void binarizaLuminancia(Imagem* img, Imagem* img_hsl, Imagem* img_out, float limiar);
Imagem* borraGaussiana(Imagem* img, Imagem* img_out, Imagem* buffer, float sigma, int n);
Imagem* juntaImagens(Imagem* img, Imagem* img_filtrada, Imagem* img_out, float a, float b);
Imagem* borraMedia(Imagem* img_mascara, Imagem* img_out, Imagem* buffer, float sigma, int n);

int main ()
{
    Imagem* img;
    Imagem* img_hsl;
    Imagem* img_mascara;
    Imagem* img_filtrada_gaussiana;
    Imagem* img_bloom_gaussiana;
    Imagem* img_filtrada_media;
    Imagem* img_bloom_media;
    Imagem* buffer;

    img = abreImagem (INPUT_IMAGE, CANAIS_IMAGEM);
    buffer = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    img_hsl = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    img_mascara = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);

    img_bloom_gaussiana = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    img_filtrada_gaussiana = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    img_filtrada_media = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    img_bloom_media = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);

    RGBParaHSL(img,img_hsl);
    
    binarizaLuminancia(img, img_hsl, img_mascara, LIMIAR);

    //posso fazer sem retorno essas funções porém ficou de mais fácil visualização deste modo
    img_filtrada_gaussiana = borraGaussiana(img_mascara, img_filtrada_gaussiana, buffer, SIGMA, QTD_FILTRAR);
    img_bloom_gaussiana = juntaImagens(img, img_filtrada_gaussiana, img_bloom_gaussiana, ALFA, BETA);
    img_filtrada_media = borraMedia(img_mascara, img_filtrada_media, buffer, SIGMA, QTD_FILTRAR); //vou multiplicar sigma * 1,5 e fazer QTD_FILTRAR vezes
    img_bloom_media = juntaImagens(img, img_filtrada_media, img_bloom_media, ALFA, BETA);
    
    salvaImagem(img_filtrada_gaussiana, "img-filtrada-gaussiana.bmp");
    salvaImagem(img_bloom_gaussiana, "img-bloom-gaussiana.bmp");
    salvaImagem(img_filtrada_media, "img-filtrada-media.bmp");
    salvaImagem(img_bloom_media, "img-bloom-media.bmp");
    salvaImagem(img_hsl, "img-hsl.bmp");
    salvaImagem(img_mascara, "img_mascara.bmp");
    
    destroiImagem(img);
    destroiImagem(img_hsl);
    destroiImagem(img_mascara);
    destroiImagem(img_filtrada_gaussiana);
    destroiImagem(img_bloom_gaussiana);
    destroiImagem(img_filtrada_media);
    destroiImagem(img_bloom_media);
    destroiImagem(buffer);
    
    printf("Pressione qualquer tecla para sair\n");
    scanf("%c");
    return 0;
}

void binarizaLuminancia(Imagem* img, Imagem* img_hsl, Imagem* img_out, float limiar)
{
    float soma = 0;
    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(img_hsl->dados[2][i][j] > limiar) {
                img_out->dados[0][i][j] = img->dados[0][i][j];
                img_out->dados[1][i][j] = img->dados[1][i][j];
                img_out->dados[2][i][j] = img->dados[2][i][j];
            }
        }
    }
}

Imagem* borraGaussiana(Imagem* img_mascara, Imagem* img_out, Imagem* buffer, float sigma, int n) 
{
    Imagem** imagens = malloc(n*sizeof(Imagem*));

    for(int i = 0; i < n; i++, sigma *= 2) {
        printf("Filtragem Gaussiana sigma %.0f...", sigma);
        imagens[i] = clonaImagem(img_mascara);
        filtroGaussiano (imagens[i], imagens[i], sigma, sigma, buffer);
        printf("Completada!\n", sigma);
    }

    for(int h = 0; h < 3; h++) {
        for(int i = 0; i < img_mascara->altura; i++) {
            for(int j = 0; j < img_mascara->largura; j++) {
                img_out->dados[h][i][j] = 0;
                for(int k = 0; k < n; k++) {
                    img_out->dados[h][i][j] +=  imagens[k]->dados[h][i][j];
                }
                if(img_out->dados[h][i][j] >= 1.0f)
                    img_out->dados[h][i][j] = 0.9999999f;
            }
        }
    }

    for(int i = 0; i < n; i++) {
       destroiImagem(imagens[i]);
    }
    free(imagens);
    return img_out;
}

Imagem* borraMedia(Imagem* img_mascara, Imagem* img_out, Imagem* buffer, float sigma, int n)
{
    int janela = (sigma * 1.5);
    if(janela % 2 == 0)
        janela++;
    Imagem** imagens = malloc(n*sizeof(Imagem*));

    for(int i = 0; i < n; i++) {
        imagens[i] = clonaImagem(img_mascara);
        printf("Filtragem media com janela %d cinco vezes...", janela);
        for(int j = 0; j < 5; j++) {
            blur (imagens[i], imagens[i], janela, janela, buffer); 
        }
        printf("Completada!\n");
        janela = (janela*2)-1;
    }
    
    for(int h = 0; h < 3; h++) {
        for(int i = 0; i < img_mascara->altura; i++) {
            for(int j = 0; j < img_mascara->largura; j++) {
                img_out->dados[h][i][j] = 0;
                for(int k = 0; k < n; k++) {
                    img_out->dados[h][i][j] +=  imagens[k]->dados[h][i][j];
                }
                if(img_out->dados[h][i][j] >= 1.0f)
                    img_out->dados[h][i][j] = 0.9999999f;
            }
        }
    }

    for(int i = 0; i < n; i++) {
       destroiImagem(imagens[i]);
    }
    free(imagens);
    return img_out;
}

Imagem* juntaImagens(Imagem* img, Imagem* img_filtrada, Imagem* img_out, float a, float b)
{
    for(int h = 0; h < 3; h++) {
        for(int i = 0; i < img->altura; i++) {
            for(int j = 0; j < img->largura; j++) {
                img_out->dados[h][i][j] = ((a*img->dados[h][i][j]) + (b*img_filtrada->dados[h][i][j]));
                if(img_out->dados[h][i][j] >= 1.0f) {
                    img_out->dados[h][i][j] = 0.99999f;
                }
            }
        }
    }
    return img_out;
}

