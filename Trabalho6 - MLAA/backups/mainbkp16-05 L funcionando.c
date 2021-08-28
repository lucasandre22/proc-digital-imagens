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

void computaBorda(Imagem* img, Imagem* bordas) {
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
    salvaImagem(bordas, "minhasBordas.bmp");
}

/*void classificaPixelsBordas(Imagem* img, Imagem* corners) {
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
            }else
            {
                //corners->dados[0][i][j] = img->dados[0][i][j];
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
    destroiImagem(shapes);
    //salvaImagem(shapes, "shapes.bmp");
}*/

/*void achaShapes(Imagem* img, Imagem* corners, SHAPES* shapes) {
    int x_aux = 1;
    int y_aux = 1;
    //varre horizontal
    for(int i = 1; i < img->altura - 1; i++) {
        for(int j = 1; j < img->largura - 1; j++) {
            if((img->dados[0][i][j] == PRETO && img->dados[0][i+1][j] == BRANCO) || (img->dados[0][i][j] == PRETO && img->dados[0][i-1][j] == BRANCO)) {//arrumar isso se for mexer com imagens diferntes de preto e branco
                x_aux += (x_aux <= 6); //x vai até 5
            }
            if(corners->dados[0][i][j] == PONTA) {
                shapes->quantidade++;
                if(shapes->quantidade == 1)
                    shapes->l_shapes = malloc(sizeof(L_SHAPE));
                else
                    shapes->l_shapes = realloc(shapes->l_shapes, shapes->quantidade*(sizeof(L_SHAPE)));
                printf("%d\n", shapes->quantidade);
                shapes->l_shapes[shapes->quantidade-1].corneri = i;
                printf("%d\n", shapes->quantidade);
                shapes->l_shapes[shapes->quantidade-1].cornerj = j;
                shapes->l_shapes[shapes->quantidade-1].jFinal = j-x_aux;//como é horizontal, preciso somente da coluna inicial
                img->dados[0][i][j] = shapes->quantidade-1;
                
                int max_y = x_aux > 2 ? 2 : 5;
                for(int h = i+1; h < img->altura - 1; h++) {
                    if((img->dados[0][h][j] == PRETO && img->dados[0][h][j+1] == BRANCO) || (img->dados[0][h][j] == PRETO && img->dados[0][h][j-1] == BRANCO)) //arrumar isso também se for mexer com imagens que somente as bordas são pretas
                        y_aux += (y_aux < max_y);

                    if(y_aux == 5) { //da pra eu otimizar, achar logo um pixel de ponta aqui tb, e ja contar o y
                        break;
                    }
                }
                shapes->l_shapes[shapes->quantidade-1].iFinal = i+y_aux;
                y_aux = 0;
                x_aux = 0;
            }
        }
    }
}*/

//22:28 13/05
/*void achaShapes(Imagem* bordas, SHAPES* shapes) {
    Imagem* indices = criaImagem(bordas->largura, bordas->altura, bordas->n_canais);
    int x_aux = 1;
    int y_aux = 1;
    //varre horizontal
    for(int i = 1; i < bordas->altura - 1; i++) {
        for(int j = 1; j < bordas->largura - 1; j++) {
            if(bordas->dados[0][i][j] == BORDA) {
                x_aux += (x_aux <= 5); //x vai até 5
            }
            

            if(bordas->dados[0][i][j] == CORNER) {
                shapes->quantidade++;
                if(shapes->quantidade == 1)
                    shapes->l_shapes = malloc(sizeof(L_SHAPE));
                else
                    shapes->l_shapes = realloc(shapes->l_shapes, shapes->quantidade*(sizeof(L_SHAPE)));
                shapes->l_shapes[shapes->quantidade-1].corneri = i;
                shapes->l_shapes[shapes->quantidade-1].cornerj = j;

                if(x_aux == 1) {
                    int h;
                    for(h = j+1; h < j+6 && h < bordas->largura; j++) {
                        if(bordas->dados[0][i][j] == BORDA) {
                            x_aux += (x_aux <= 5); //x vai até 5
                        }
                        else if(bordas->dados[0][i][h] == CORNER) {
                            j = h-1;
                            break;
                        }
                        else {
                            shapes->l_shapes[shapes->quantidade-1].jFinal = j + x_aux;
                            x_aux = 1;
                            break;
                        }       
                    }
                    continue;
                }
                shapes->l_shapes[shapes->quantidade-1].jFinal = j-(x_aux-1);//como é horizontal, preciso somente da coluna inicial
                indices->dados[0][i][j] = shapes->quantidade-1;
                x_aux = 1;
            }
        }
    }
    for(int i = 1; i < bordas->largura - 1; i++) {
        for(int j = 1; j < bordas->altura - 1; j++) {
            if(bordas->dados[0][j][i] == BORDA) {
                y_aux += (y_aux <= 5);
            }
            if(bordas->dados[0][j][i] == CORNER) {
                int aux = indices->dados[0][j][i];
                if(abs(shapes->l_shapes[aux].jFinal - shapes->l_shapes[aux].cornerj) >= 2 && y_aux >= 2) {
                    y_aux = 2;
                }
                shapes->l_shapes[aux].iFinal = j;
            }
        }
    }
    printf("Quantidade shapes: %d\n", shapes->quantidade);
    destroiImagem(indices);
}*/


//essa mesmo
/*void achaShapes(Imagem* bordas, SHAPES* shapes) {
    Imagem* indices = criaImagem(bordas->largura, bordas->altura, bordas->n_canais);
    Imagem* aux = criaImagem(bordas->largura, bordas->altura, 1);
    zeraImagem(aux);
    int x_aux = 1;
    int y_aux = 1;
    int aux1;
    int h;
    //varre horizontal
    for(int i = 1; i < bordas->altura - 1; i++) {
        for(int j = 1; j < bordas->largura - 1; j++) {
            if(bordas->dados[0][i][j] == CORNER) {
                indices->dados[0][i][j] = shapes->quantidade;
                shapes->quantidade++;

                if(shapes->quantidade == 1)
                    shapes->l_shapes = malloc(sizeof(L_SHAPE));
                else
                    shapes->l_shapes = realloc(shapes->l_shapes, shapes->quantidade*(sizeof(L_SHAPE)));
                shapes->l_shapes[shapes->quantidade-1].corneri = i;
                shapes->l_shapes[shapes->quantidade-1].cornerj = j;
                aux->dados[0][i][j] = 1;
                aux1 = shapes->quantidade;
                for(h = j+1, x_aux = 1; h < bordas->largura - 1; h++) {
                    if(bordas->dados[0][i][h] != BORDA && bordas->dados[0][i][h] != CORNER) {
                        break;
                    }
                    else if(bordas->dados[0][i][h] == CORNER) {
                        shapes->quantidade++;
                        if(shapes->quantidade == 1)
                            shapes->l_shapes = malloc(sizeof(L_SHAPE));
                        else
                            shapes->l_shapes = realloc(shapes->l_shapes, shapes->quantidade*(sizeof(L_SHAPE)));
                        shapes->l_shapes[shapes->quantidade-1].corneri = i;
                        shapes->l_shapes[shapes->quantidade-1].cornerj = h;
                        shapes->l_shapes[shapes->quantidade-1].jFinal = h-x_aux;
                        
                        aux->dados[0][i][h-x_aux] = 0.200;
                        j = h;
                        break;
                    }
                    x_aux += (x_aux <= 5);//x vai até 5
                    if((x_aux <= 5)) {
                        aux->dados[0][i][h] = 0.200;
                    }
                }
                //x_aux = 1;
                shapes->l_shapes[aux1].jFinal = j+x_aux;
                j = h-1;
            }
            else if(bordas->dados[0][i][j] == BORDA) {
                int tem_corner = 0;
                for(h = j+1, x_aux = 1; h < bordas->largura - 1; h++) {
                    if(bordas->dados[0][i][h] != BORDA && bordas->dados[0][i][h] != CORNER) {
                        break;
                    }
                    else if(bordas->dados[0][i][h] == CORNER) {
                        shapes->quantidade++;
                        if(shapes->quantidade == 1)
                            shapes->l_shapes = malloc(sizeof(L_SHAPE));
                        else
                            shapes->l_shapes = realloc(shapes->l_shapes, shapes->quantidade*(sizeof(L_SHAPE)));
                        shapes->l_shapes[shapes->quantidade-1].corneri = i;
                        shapes->l_shapes[shapes->quantidade-1].cornerj = h;
                        shapes->l_shapes[shapes->quantidade-1].jFinal = h-x_aux;
                        j = h;
                        break;
                    }
                    x_aux += (x_aux <= 5);//x vai até 5
                    if((x_aux <= 5)) {
                        if(aux->dados[0][i][h] == CORNER) {
                            printf("corner!\n");
                        }
                        aux->dados[0][i][h] = 0.200;
                    }
                }
                //x_aux = 1;
            }
            x_aux = 1;
            

            /*if(bordas->dados[0][i][j] == CORNER) {
                
                shapes->l_shapes[shapes->quantidade-1].corneri = i;
                shapes->l_shapes[shapes->quantidade-1].cornerj = j;

                if(x_aux == 1) {
                    
                    for(h = j+1; h < j+6 && h < bordas->largura; j++) {
                        if(bordas->dados[0][i][j] == BORDA) {
                            x_aux += (x_aux <= 5); //x vai até 5
                        }
                        else if(bordas->dados[0][i][h] == CORNER) {
                            j = h-1;
                            break;
                        }
                        else {
                            shapes->l_shapes[shapes->quantidade-1].jFinal = j + x_aux;
                            x_aux = 1;
                            break;
                        }       
                    }
                    continue;
                }
                shapes->l_shapes[shapes->quantidade-1].jFinal = j-(x_aux-1);//como é horizontal, preciso somente da coluna inicial
                
                x_aux = 1;
            } /* aqui
        }
    }
    /*for(int i = 1; i < bordas->largura - 1; i++) {
        for(int j = 1; j < bordas->altura - 1; j++) {
            if(bordas->dados[0][j][i] == BORDA) {
                y_aux += (y_aux <= 5);
            }
            if(bordas->dados[0][j][i] == CORNER) {
                int aux = (int)indices->dados[0][j][i];
                if(abs(shapes->l_shapes[aux].jFinal - shapes->l_shapes[aux].cornerj) >= 2 && y_aux >= 2) {
                    y_aux = 2;
                }
                shapes->l_shapes[aux].iFinal = j;
            }
        }
    } /* aqui
    printf("Quantidade shapes: %d\n", shapes->quantidade);
    destroiImagem(indices);
    salvaImagem(aux, "bordasteste.bmp");
    destroiImagem(aux);
}*/

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
            limite = perna_menor == 0 ? 1 : 5;
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
            limite = perna_menor == 0 ? 1 : 5;
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
            limite = perna_menor == 0 ? 1 : 5;
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
            limite = perna_menor == 0 ? 1 : 5;
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

int main ()
{
    Imagem* img;
    Imagem* bordas;
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
    bordas = criaImagem(img->largura, img->altura, CANAIS_IMAGEM);
    zeraImagem(bordas);
    computaBorda(img, bordas);
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