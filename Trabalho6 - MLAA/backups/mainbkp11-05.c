#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>

#define INPUT_IMAGE "imagem.bmp"
#define CANAIS_IMAGEM 1
#define LIMITE_INFERIOR 0.020
#define PRETO 0.0f
#define BRANCO 1.0f
#define PONTA 0.500f
#define MAX_L_MENOR 2
#define MAX_L_MAIOR 5

typedef struct { //todo shape é componto de linha vertical e linha horizontal
    int xInicial;
    int xFinal;
    int yInicial;
    int yFinal;

    int cornerx;
    int cornery;
}L_SHAPE;

typedef struct {
    int quantidade;
    L_SHAPE* l_shapes;
}SHAPES;

/*void arrumaMagnitude(Imagem* magnitude, Imagem* bordas) {
    for(int h = 0; h < magnitude->n_canais; h++) {
        for(int i = 0; i < magnitude->altura-1; i++) {
            for(int j = 0; j < magnitude->largura-1; j++) {
                if(fabs(magnitude->dados[h][i][j] - magnitude->dados[h][i][j+1]) > LIMITE_INFERIOR)
                    magnitude->dados[h][i][j] = magnitude->dados[h][i][j] > magnitude->dados[h][i][j+1] ? magnitude->dados[h][i][j] : magnitude->dados[h][i][j+1];
            }
        }
    }

    for(int h = 0; h < magnitude->n_canais; h++) {
        for(int i = 0; i < magnitude->largura; i++) {
            for(int j = 0; j < magnitude->altura - 1; j++) {
                if(fabs(magnitude->dados[h][j][i] - magnitude->dados[h][j+1][i]) > LIMITE_INFERIOR)
                    magnitude->dados[h][j][i] = magnitude->dados[h][j][i] > magnitude->dados[h][j+1][i] ? magnitude->dados[h][j][i] : magnitude->dados[h][j+1][i];
            }
        }
    }
}*/

/*void arrumaMagnitude(Imagem* magnitude, Imagem* bordas) {
    for(int i = 0; i < magnitude->altura-1; i++) {
        for(int j = 0; j < magnitude->largura-1; j++) {
            if(fabs(magnitude->dados[0][i][j] - magnitude->dados[0][i][j+1]) > LIMITE_INFERIOR)
                bordas->dados[0][i][j] = 1.0f;
                //bordas->dados[h][i][j] = magnitude->dados[h][i][j] > magnitude->dados[h][i][j+1] ? magnitude->dados[h][i][j] : magnitude->dados[h][i][j+1];
        }
    }

    for(int i = 0; i < magnitude->largura - 1; i++) {
        for(int j = 0; j < magnitude->altura - 1; j++) {
            if(fabs(magnitude->dados[0][j][i] - magnitude->dados[0][j+1][i]) > LIMITE_INFERIOR)
                bordas->dados[0][j][i] = 1.0f;
                //bordas->dados[h][j][i] = magnitude->dados[h][j][i] > magnitude->dados[h][j+1][i] ? magnitude->dados[h][j][i] : magnitude->dados[h][j+1][i];
        }
    }
}*/

void zeraImagem(Imagem* img) {
    for(int h = 0; h < img->n_canais; h++) {
        for(int i = 0; i < img->altura; i++) {
            for(int j = 0; j < img->largura; j++) {
                img->dados[h][i][j] = 0;
            }
        }
    }
}

/*void meuGradiente(Imagem* img) {
    Imagem* dx = criaImagem(img->largura, img->altura, img->n_canais);
    Imagem* dy = criaImagem(img->largura, img->altura, img->n_canais);
    Imagem* out = criaImagem(img->largura, img->altura, img->n_canais);
    zeraImagem(dx);
    zeraImagem(dy);

    for(int i = 1; i < img->altura-1; i++) {
        for(int j = 1; j < img->largura-1; j++) {
            if(((-1*img->dados[0][i][j-1]) + img->dados[0][i][j+1]) > 0.050) {
                dx->dados[0][i][j] = ((-1*img->dados[0][i][j-1]) + img->dados[0][i][j+1]);
            }
        }
    }
    for(int i = 1; i < img->largura - 1; i++) {
        for(int j = 1; j < img->altura - 1; j++) {
            if(((-1*img->dados[0][j-1][i]) + img->dados[0][j+1][i]) > 0.050) {
                dy->dados[0][j][i] = ((-1*img->dados[0][j-1][i]) + img->dados[0][j+1][i]);
            }    
        }
    }
    salvaImagem(dx, "teste2.bmp");
    salvaImagem(dy, "teste23.bmp");

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            out->dados[0][i][j] = dx->dados[0][i][j] + dy->dados[0][i][j];
        }
    }
    printf("lala");
    salvaImagem(out, "teste.bmp");

}*/

/*void classificaPixelsCorner() {
    int l_normal[2][2];

    l_normal[0][0] = 1;
    l_normal[0][1] = 0;
    l_normal[1][0] = 1;
    l_normal[1][1] = 1;





}*/

void classificaPixelsBordas(Imagem* img, Imagem* corners) {
    Imagem* shapes = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    int h;
    zeraImagem(corners);
    zeraImagem(shapes);
    for(int i = 0; i < img->altura - 1; i++) {
        for(int j = 0, aux = 0; j < img->largura - 1; j++, aux = 0) {
            if(img->dados[0][i][j] != img->dados[0][i][j+1]) {
                if(img->dados[0][i][j] == BRANCO && img->dados[0][i][j+1] == PRETO)
                    h = j+1;
                else
                    h = j;

                aux += (img->dados[0][i-1][h] == img->dados[0][i][h]);
                aux += (img->dados[0][i+1][h] == img->dados[0][i][h]);

                if(aux != 2)
                    corners->dados[0][i][h] = PONTA;
            }    
        }
    }
    
    int qnt;
    for(int i = 0; i < img->altura - 1; i++) {
        for(int j = 0; j < img->largura - 1; j++) {
            if(corners->dados[0][i][j] == PONTA) {
                qnt = 1;
                for(h = j+1; (img->dados[0][i+1][h] == BRANCO && corners->dados[0][i][h] != PONTA) || 
                (img->dados[0][i-1][h] == BRANCO && corners->dados[0][i][h] != PONTA); h++) {
                    shapes->dados[0][i][h] = 1;
                    qnt++;
                }
                j = h;
            }
        }
    }
    salvaImagem(corners, "corners.bmp");
    salvaImagem(shapes, "shapes.bmp");
}



int main ()
{
    Imagem* img;
    Imagem* img_edges;
    Imagem* magnitude;
    Imagem* orientacao;
    Imagem* resize;
    Imagem* corners;
    SHAPES shapes;

    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);

    img_edges = criaImagem(img->largura, img->altura, img->n_canais);
    magnitude = criaImagem(img->largura, img->altura, img->n_canais);
    orientacao = criaImagem(img->largura, img->altura, img->n_canais);
    corners = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    zeraImagem(img_edges);

    classificaPixelsBordas(img, corners);
    computaGradientes (img, 3, NULL, NULL, magnitude, orientacao);
    
    //arrumaMagnitude(magnitude, img_edges);
    //arrumaMagnitude(img, img_edges);
    
    destroiImagem(orientacao);

    salvaImagem(img_edges, "bordas.bmp");
    salvaImagem(magnitude, "magnitude.bmp");
    destroiImagem(img);
    destroiImagem(img_edges);
    destroiImagem(magnitude);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");
    return 0;
}