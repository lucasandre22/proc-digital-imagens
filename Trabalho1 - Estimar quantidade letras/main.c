/*============================================================================*/
/* Exemplo: segmentação de uma imagem em escala de cinza.                     */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu                                                     */
/* Universidade Tecnológica Federal do Paraná                                 */
/*============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pdi.h"

/*============================================================================*/




// TODO: ajuste estes parâmetros!

//para documento, não achei valores para ficar 100%
/*#define INPUT_IMAGE "documento-3mp.bmp"
#define NEGATIVO 1
#define THRESHOLD 0.390
#define ALTURA_MIN 5
#define LARGURA_MIN 1
#define N_PIXELS_MIN 10*/

//para arroz
#define INPUT_IMAGE "arroz.bmp"
#define THRESHOLD 0.810f
#define NEGATIVO 0
#define ALTURA_MIN 5
#define LARGURA_MIN 5
#define N_PIXELS_MIN 50


/*============================================================================*/

typedef struct
{
    float label; // Rótulo do componente.
    Retangulo roi; // Coordenadas do retângulo envolvente do componente.
    int n_pixels; // Número de pixels do componente.

} Componente;

/*============================================================================*/

void binariza (Imagem* in, Imagem* out, float threshold);
int rotula (Imagem* img, Componente** componentes, int largura_min, int altura_min, int n_pixels_min);

/*============================================================================*/

int main ()
{
    int i;

    // Abre a imagem em escala de cinza, e mantém uma cópia colorida dela para desenhar a saída.
    Imagem* img = abreImagem (INPUT_IMAGE, 1);
    if (!img)
    {
        printf ("Erro abrindo a imagem.\n");
        exit (1);
    }

    Imagem* img_out = criaImagem (img->largura, img->altura, 3);
    cinzaParaRGB (img, img_out);

    // Segmenta a imagem.
    if (NEGATIVO)
        inverte (img, img);
    binariza (img, img, THRESHOLD);
    salvaImagem (img, "01 - binarizada.bmp");

    Componente* componentes;
    int n_componentes;
    clock_t tempo_inicio = clock ();
    n_componentes = rotula (img, &componentes, LARGURA_MIN, ALTURA_MIN, N_PIXELS_MIN);
    clock_t tempo_total = clock () - tempo_inicio;

    printf ("Tempo: %d\n", (int) tempo_total);
    printf ("%d componentes detectados.\n", n_componentes);

    //Mostra os objetos encontrados.
    for (i = 0; i < n_componentes; i++) {
            desenhaRetangulo (componentes [i].roi, criaCor (1,0,0), img_out);
    }

    salvaImagem (img_out, "02 - out.bmp");

    // Limpeza.
    free (componentes);
    destroiImagem (img_out);
    destroiImagem (img);
    scanf("%c");
    return (0);
}

/*----------------------------------------------------------------------------*/
/** Binarização simples por limiarização.
 *
 * Parâmetros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               binariza cada canal independentemente.
 *             Imagem* out: imagem de saída. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             float threshold: limiar.
 *
 * Valor de retorno: nenhum (usa a imagem de saída). */

void binariza (Imagem* in, Imagem* out, float threshold)
{
    // TODO: escreva o código desta função.
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais)
    {
        printf ("ERRO: binariza: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }
    for(int k = 0; k < in->n_canais; k++)
        for(int i = 0; i < in->altura; i++) {
            for(int j = 0; j < in->largura; j++) {
                if(in->dados[k][i][j] > threshold)
                    in->dados[k][i][j] = 1.0f;
                else
                    in->dados[k][i][j] = 0.0f;
            }
        }
}

/** Função recursiva da rotulagem (flood fill recursivo)
 *
 *
 * Parâmetros: Imagem* img: imagem de entrada E saída.
 *             int indiceVetor: representa o componente atual.
 *             int i: posição y do pixel que está sendo analisado.
 *             int j: posição x do pixel que está sendo analisado.
 *             float _label: label que o componente está sendo preenchido,
 *
 * Valor de retorno: o número de componentes conexos encontrados. */

void caminhaRotulando(Imagem* img, Componente** componentes, int indiceVetor, int i, int j, float _label)
{
    if(i >= 0 && i < img->altura && j >= 0 && j < img->largura) {
        img->dados[0][i][j] = _label;
        (*componentes)[indiceVetor].n_pixels++;
        if(i+1 < img->altura && img->dados[0][i+1][j] == 1)
            caminhaRotulando(img,componentes,indiceVetor,i+1,j,_label);
        if(i-1 > 0 && img->dados[0][i-1][j] == 1)
            caminhaRotulando(img,componentes,indiceVetor,i-1,j,_label);
        if(j+1 < img->largura && img->dados[0][i][j+1] == 1)
            caminhaRotulando(img,componentes,indiceVetor,i,j+1,_label);
        if(j-1 > 0 && img->dados[0][i][j-1] == 1)
            caminhaRotulando(img,componentes,indiceVetor,i,j-1,_label);
    }
}

/** Função de busca binária clássica
 *
 *
 * Parâmetros: float* mapaLabel: mapa de labels (cada indice um label).
 *             int e: esquerda.
 *             int d: direita.
 *             float _label: label a procurar.
 *
 * Valor de retorno: o índice do label correspondente no vetor arr. */

int buscaBinaria(float mapaLabel[], int e, int d, float _label)
{
    if (d >= e) {
        int mid = e + (d - e) / 2;
        if (mapaLabel[mid] == _label)
            return mid;
        if (mapaLabel[mid] > _label)
            return buscaBinaria(mapaLabel, e, mid - 1, _label);
        return buscaBinaria(mapaLabel, mid + 1, d, _label);
    }
    return -1;
}

/*============================================================================*/
/* ROTULAGEM                                                                  */
/*============================================================================*/
/** Rotulagem usando flood fill. Marca os objetos da imagem com os valores
 * [0.0001,0.0002,etc].
 *
 * Parâmetros: Imagem* img: imagem de entrada E saída.
 *             Componente** componentes: um ponteiro para um vetor de saída.
 *               Supomos que o ponteiro inicialmente é inválido. Ele irá
 *               apontar para um vetor que será alocado dentro desta função.
 *               Lembre-se de desalocar o vetor criado!
 *             int largura_min: descarta componentes com largura menor que esta.
 *             int altura_min: descarta componentes com altura menor que esta.
 *             int n_pixels_min: descarta componentes com menos pixels que isso.
 *
 * Valor de retorno: o número de componentes conexos encontrados. */

int rotula (Imagem* img, Componente** componentes, int largura_min, int altura_min, int n_pixels_min)
{
    int quantidadeComponentes = 0,i,j;
    float _label = 0.0001; //até 10 mil componentes numa imagem, para mais só mudar esse valor e o sizeVetor
    float *mapaLabel = NULL; //cada índice contem o valor do _label
    int indiceVetor = 0;
    int sizeVetor = 10000; //rotula até 10 mil componentes
    int ruidos = 0;

    (*componentes) = malloc(sizeVetor*sizeof(Componente));
    mapaLabel = malloc(sizeVetor*sizeof(int));

    for(i = 0; i < sizeVetor; i++) {
        mapaLabel[i] = 0.f;
        (*componentes)[i].n_pixels = -1;
        (*componentes)[i].roi.b = -1; //y em baixo
        (*componentes)[i].roi.c = -1;//y em cima
        (*componentes)[i].roi.d = -1;//x direita
        (*componentes)[i].roi.e = img->largura+1;//x esquerda com largura maxima possivel + 1
    }

    for(i = 0; i < img->altura; i++) { //faz o label
        for(j = 0; j < img->largura; j++) {
            if(img->dados[0][i][j] == 1.0) {
                caminhaRotulando(img,componentes,indiceVetor++,i,j,_label);
                mapaLabel[indiceVetor-1] = _label; //faço mapeamento de cada label no mapaLabel
                (*componentes)[indiceVetor].label = _label;
                _label += 0.0001;
            }
        }
    }

    sizeVetor = indiceVetor;
    mapaLabel = realloc(mapaLabel,(sizeVetor*sizeof(float)));

    for(i = 0; i < img->altura; i++) { //acha as coordenadas referente ao retângulo
        for(j = 0; j < img->largura; j++) {
            if(img->dados[0][i][j] != 0) {

                /**
                Utilizei busca binária para acessar o índice correspondente do label no mapaLabel,
                ocorriam alguns bugs se eu acesso desta maneira (mais rapida):
                int u = (int)((img->dados[0][i][j] * 10000.f)-1)
                **/
                int u = buscaBinaria(mapaLabel,0,sizeVetor-1,img->dados[0][i][j]);

                if(u != -1) {//nunca acontecerá, porém só para controle
                    if((*componentes)[u].roi.c == -1) //altura maxima, o primeiro pixel rotulado a aparecer é a altura maxima do componente
                        (*componentes)[u].roi.c = i;

                    if((*componentes)[u].roi.e > j) //j menor é o da esquerda
                        (*componentes)[u].roi.e = j;

                    if((*componentes)[u].roi.b < i) //i maior é baixo
                        (*componentes)[u].roi.b = i;

                    if((*componentes)[u].roi.d < j) //j maior é direita
                        (*componentes)[u].roi.d = j;
                }
            }
        }
    }

    for(i = 0; i < sizeVetor; i++) {
        if((*componentes)[i].n_pixels < n_pixels_min ||
           ((*componentes)[i].roi.b - (*componentes)[i].roi.c) < altura_min ||
           ((*componentes)[i].roi.d - (*componentes)[i].roi.e) < largura_min) {
            ruidos++;
            (*componentes)[i].roi.d = -1; //valor qualquer para a marcação de ruído...
        }
    }

    quantidadeComponentes = sizeVetor - ruidos;

    int aux = 0; //apenas para controle no loop a seguir

    for(i = 0; i < sizeVetor; i++) {
        if((*componentes)[i].roi.d != -1) { //se nao for ruido inclui no vetor de componentes
            (*componentes)[aux++] = (*componentes)[i];
        }
    }
    (*componentes) = realloc(*componentes,(quantidadeComponentes*sizeof(Componente)));
    free(mapaLabel);
    return quantidadeComponentes;
}

/*============================================================================*/
//gcc main.c pdi.c -o exec
