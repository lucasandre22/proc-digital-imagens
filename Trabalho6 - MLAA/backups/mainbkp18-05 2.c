#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>

#define INPUT_IMAGE "imagemTesteAA.bmp"
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

    short int borda_falsa; //bool
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
    int lala = 0;
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
            if(bordas->dados[0][i][j] == CORNER)
                lala++;
            if(bordas->dados[0][i][j] == 2*PONTA_FALSA) {
                bordas->dados[0][i][j] = PONTA_FALSA;
            }
            /*
            if(dx->dados[0][i][j] == BORDA || dy->dados[0][i][j] == BORDA) {
                bordas_falsas->dados[0][i][j] = BORDA;
            }
            if((dx->dados[0][i][j] + dy->dados[0][i][j]) == 2*BORDA) {
                bordas_falsas->dados[0][i][j] = 0;
            }
            if((dx->dados[0][i][j] + dy->dados[0][i][j]) == 2*PONTA_FALSA)
                bordas_falsas->dados[0][i][j] = 1;*/

            
            if(dx->dados[0][i][j] == PONTA_FALSA || dy->dados[0][i][j] == PONTA_FALSA) {
                bordas_falsas->dados[0][i][j] = BORDA;
            }
            if(dx->dados[0][i][j] == BORDA || dy->dados[0][i][j] == BORDA) {
                bordas_falsas->dados[0][i][j] = PONTA_FALSA;
            }
            if((dx->dados[0][i][j] + dy->dados[0][i][j]) == 2*PONTA_FALSA) {
                lala++;
                bordas_falsas->dados[0][i][j] = CORNER;
            }
                

            
        }
    }
    destroiImagem(dx);
    destroiImagem(dy);
    printf("Bordas esperadas: %d\n", lala);
    salvaImagem(bordas_falsas, "minhasBordasFalsas.bmp");
    salvaImagem(bordas, "minhasBordas.bmp");
}

void achaShapes(Imagem* bordas, SHAPES* shapes, int borda_falsa) {
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
                shapes->l_shapes[shapes->quantidade-1].borda_falsa = borda_falsa;
                aux->dados[0][i][j] = CORNER;
            }
        }
    }
    salvaImagem(aux,"cornerss.bmp");

    for(h = 0; h < shapes->quantidade; h++) { //aqui eu quero navegar pelos shapes e marcar o L deles
        int perna_menor = 1;
        int quantidade_atual = 0;
        int limite;
        int jFinal = shapes->l_shapes[h].cornerj;
        int iFinal = shapes->l_shapes[h].corneri;
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
            //printf("quantidade:%d\n", quantidade_atual);
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
            //printf("quantidade:%d\n", quantidade_atual);
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
            //printf("quantidade:%d\n", quantidade_atual);
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
            //printf("quantidade:%d\n", quantidade_atual);
            quantidade_atual = 0;
        }
        //printf("cornerI=%d iFinal=%d\n", shapes->l_shapes[h].corneri, iFinal);
        //printf("cornerJ=%d JFinal=%d\n", shapes->l_shapes[h].cornerj, jFinal);
        shapes->l_shapes[h].jFinal = jFinal;
        shapes->l_shapes[h].iFinal = iFinal;
        perna_menor = 1;
    }
    printf("quantidade: %d\n", shapes->quantidade);
    salvaImagem(aux, "L_marcado.bmp");
    destroiImagem(aux);
}

void calculaPesos(float pesos[][5][5]) { //acessa linha, coluna
    //L com 
    for(int i = 0; i < 2; i++) 
        for(int j = 0; j < 5; j++) 
            for(int h = 0; h < 5; h++) 
                pesos[i][j][h] = 0;

    //MENOR PERNA DO L IGUAL A 1
    //somente um pixel na maior perna
    pesos[0][0][0] = 0.125f;//certo
    //dois pixels na maior perna
    pesos[0][1][0] = 0.25f;//certo
    //tres pixels na maior perna
    pesos[0][2][0] = 1.f/3.f;//certo
    pesos[0][2][1] = 1.f/24.f;//certo
    //quatro pixels na maior perna
    pesos[0][3][0] = 1.f/3.f;
    pesos[0][3][1] = 5.f/24.f;
    //cinco pixels na maior perna
    pesos[0][4][0] = 3.f/7.f;;//
    pesos[0][4][1] = 7.f/24.f;//
    pesos[0][4][2] = 2.f/24.f;

    //MENOR PERNA DO L IGUAL A 2
    //somente um pixel na maior perna
    pesos[1][0][0] = 0.25f;//certo
    //dois pixels na maior perna
    pesos[1][1][0] = 0.500f;//certo
    //tres pixels na maior perna
    pesos[1][2][0] = 2.f/3.f;
    pesos[1][2][1] = 4.f/24.f;
    //quatro pixels na maior perna
    pesos[1][3][0] = 2.f/3.f;
    pesos[1][3][1] = 8.f/24.f;
    //cinco pixels na maior perna
    pesos[1][4][0] = 3.f/7.f;;//
    pesos[1][4][1] = 7.f/24.f;//
    pesos[1][4][2] = 2.f/24.f;
}

void merge(Imagem* img, Imagem* bordas, Imagem* bordas_falsas, SHAPES* shapes) {
    float pesos[2][5][5];
    Imagem* vetor[2];
    vetor[0] = bordas;
    vetor[1] = bordas_falsas;

    int corneri, cornerj;
    int tamanho_menor, tamanho_maior;
    int tamanhoi;
    int tamanhoj;
    int inegativo = 0;
    int jnegativo = 0;
    int maior;
    float aux;
    float calcula_pesos[4];
    calculaPesos(pesos);

    for(int h = 0; h < shapes->quantidade; h++) {
        corneri = shapes->l_shapes[h].corneri;
        cornerj = shapes->l_shapes[h].cornerj;

        tamanhoi = shapes->l_shapes[h].iFinal > corneri ? shapes->l_shapes[h].iFinal - corneri : corneri - shapes->l_shapes[h].iFinal; //tamanho do L no eixo y
        tamanhoj = shapes->l_shapes[h].jFinal > cornerj ? shapes->l_shapes[h].jFinal - cornerj : cornerj - shapes->l_shapes[h].jFinal; //tamanho do L no eixo x

        inegativo = shapes->l_shapes[h].iFinal > corneri ? 0 : 1;
        jnegativo = shapes->l_shapes[h].jFinal > cornerj ? 0 : 1;
        
        maior = tamanhoi > tamanhoj ? 1 : 0;

        if(tamanhoi > tamanhoj) {
            tamanho_maior = tamanhoi;
            tamanho_menor = tamanhoj;
        } 
        else {
            tamanho_maior = tamanhoj;
            tamanho_menor = tamanhoi;
        }

        //erro quando tamanhoi == tamanhoj;
        if(tamanhoi == tamanhoj) { //quando é um só píxel ou tem 2 de altura e 2 de largura, tratamento diferente
            aux = img->dados[0][corneri][cornerj];
            if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj+1] == PONTA_FALSA) {
                img->dados[0][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri][cornerj+1]) +
                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri][cornerj]);
            }
            else if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj-1] == PONTA_FALSA) {
                img->dados[0][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri][cornerj-1]) +
                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri][cornerj]);
            }
            else if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri-1][cornerj] == PONTA_FALSA) {
                img->dados[0][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri-1][cornerj]) +
                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri][cornerj]);
            }
            else if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri+1][cornerj] == PONTA_FALSA) {
                img->dados[0][corneri][cornerj] = ((pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri+1][cornerj]) +
                                                ((1-pesos[tamanho_menor][tamanho_maior][0]) * img->dados[0][corneri][cornerj]);
            }
            /*int pixels_a_mergir = -1;
            int controla = 0;
            calcula_pesos[0] = (vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj+1] == PONTA_FALSA); //direita
            calcula_pesos[1] = (vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj-1] == PONTA_FALSA); //esquerda
            calcula_pesos[2] = (vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri-1][cornerj] == PONTA_FALSA); //baixo
            calcula_pesos[3] = (vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri+1][cornerj] == PONTA_FALSA); //cima
            aux = pesos[0][0][1] + tamanho_maior * pesos[0][0][1]; //se é um só pixel ou 2 de altura e 2 de largura, pesos diferentes
            for(int i = 0; i < 4; i++) {
                if(calcula_pesos[i] == 1.f) {
                    pixels_a_mergir++;
                    controla++;
                }
                calcula_pesos[i] = 0.0f;
            }
            if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj+1] == PONTA_FALSA) {
                calcula_pesos[0] = aux * img->dados[0][corneri][cornerj+1];
                pixels_a_mergir--;
            } 
            if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj-1] == PONTA_FALSA && pixels_a_mergir > 0) {
                calcula_pesos[1] = aux * img->dados[0][corneri][cornerj-1];
                pixels_a_mergir--;
            }
            if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri+1][cornerj] == PONTA_FALSA && pixels_a_mergir > 0) {
                calcula_pesos[2] = aux * img->dados[0][corneri+1][cornerj];
                pixels_a_mergir--;
            }
            if(vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri-1][cornerj] == PONTA_FALSA && pixels_a_mergir > 0) {
                calcula_pesos[3] = aux * img->dados[0][corneri-1][cornerj];
                pixels_a_mergir--;
            }
            img->dados[0][corneri][cornerj] = (img->dados[0][corneri][cornerj]-(aux*controla)*img->dados[0][corneri][cornerj]);

            for(int i = 0; i < 4; i++) {
                img->dados[0][corneri][cornerj] += calcula_pesos[i];
            }*/
            continue;
        }
        //printf("tamanho_maior:%d tamanho_menor:%d\n", tamanho_maior, tamanho_menor);

        //itero cima ou baixo, eu sei que a perna do L maior é pra cima ou pra baixo
        int coordenada_pixel_misturar;
        if(maior) { //I É MAIOR
            //Antes de começar, preciso saber a direção do pixel que será misturado
            coordenada_pixel_misturar = vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri][cornerj+1] == PONTA_FALSA ? cornerj+1 : cornerj-1;
            if(inegativo) { //i vai negativando
                for(int i = corneri, f = 0; i > shapes->l_shapes[h].iFinal/*+ tamanhoi/2 */; i--, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] != 0) {
                        aux = img->dados[0][i][cornerj];
                        img->dados[0][i][cornerj] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[0][i][coordenada_pixel_misturar]) 
                        + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[0][i][cornerj]);
                    }
                    //printf("peso%d:%f ", f, pesos[tamanho_menor][tamanho_maior][f]);
                }
                //printf("saiu!\n");
            }
            else {
                for(int i = corneri, f = 0; i < shapes->l_shapes[h].iFinal/*+ tamanhoi/2 */; i++, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] > 0.0f) {
                        aux = img->dados[0][i][cornerj];
                        img->dados[0][i][cornerj] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[0][i][coordenada_pixel_misturar]) 
                        + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[0][i][cornerj]);
                    }
                    //printf("peso%d:%f ", f, pesos[tamanho_menor][tamanho_maior][f]);
                }
                //printf("saiu!\n");
            }
        }
        else { //j é maior
            coordenada_pixel_misturar = vetor[shapes->l_shapes[h].borda_falsa]->dados[0][corneri+1][cornerj] == PONTA_FALSA ? corneri+1 : corneri-1;
            if(jnegativo) {
                for(int j = cornerj, f = 0; j > shapes->l_shapes[h].jFinal/*+ tamanhoi/2 */; j--, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] != 0) {
                        aux = img->dados[0][corneri][j];
                        img->dados[0][corneri][j] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[0][coordenada_pixel_misturar][j]) 
                        + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[0][corneri][j]);
                        printf("peso%d:%f ", f, pesos[tamanho_menor][tamanho_maior][f]);
                    }
                    //printf("peso%d:%f ", f, pesos[tamanho_menor][tamanho_maior][f]);
                }
                printf("saiu!\n");
            }
            else {
                for(int j = cornerj, f = 0; j < shapes->l_shapes[h].jFinal/*+ tamanhoi/2 */; j++, f++) { //f controla peso
                    if(pesos[tamanho_menor][tamanho_maior][f] != 0) {
                        aux = img->dados[0][corneri][j];
                        img->dados[0][corneri][j] = (pesos[tamanho_menor][tamanho_maior][f] * img->dados[0][coordenada_pixel_misturar][j]) 
                        + ((1-pesos[tamanho_menor][tamanho_maior][f]) * img->dados[0][corneri][j]);
                    }
                    //printf("peso%d:%f ", f, pesos[tamanho_menor][tamanho_maior][f]);
                }
                //printf("saiu!\n");

            }
        }
    }
    salvaImagem(img, "arquivo.bmp");
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

    achaShapes(bordas, &shapes, 0);
    achaShapes(bordas_falsas, &shapes, 1);
    merge(img, bordas, bordas_falsas, &shapes);

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