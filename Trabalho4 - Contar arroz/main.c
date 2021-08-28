#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdi.h"
#include <windows.h>

#define INPUT_IMAGE "205.bmp"
#define CANAIS_IMAGEM 3

int linhaHorizontal(Imagem* img, ComponenteConexo** componentes, int size_componentes, int* vetor, float* mapa);
int linhaVertical(Imagem* img, ComponenteConexo** componentes, int size_componentes, int* vetor, float* mapa);
int calculaQuantidadeArrozBlob(int* duplicados, ComponenteConexo* componentes, int size);
Imagem* geraImagemArrozColadoDiferenciado(Imagem* img, Imagem* out, int* duplicados, ComponenteConexo* componentes, int size);

int buscaBinaria(float* vetor, int e, int d, float _label)
{
    if (d >= e) {
        int mid = e + (d - e) / 2;
        if (vetor[mid] == _label)
            return mid;
        if (vetor[mid] > _label)
            return buscaBinaria(vetor, e, mid - 1, _label);
        return buscaBinaria(vetor, mid + 1, d, _label);
    }
    return -1;
}

int main ()
{
    Imagem* img;
    Imagem* img_out;
    Imagem* buffer;
    Imagem* escala_cinza;
    Imagem* img_abertura;
    Imagem* img_aux1;
    Imagem* img_aux2;
    Imagem* img_arroz_colado_diferenciado;
    ComponenteConexo* componentes;
    int total;
    int* vetor_arroz_grudado;
    float *mapa;

    //diferentes sizes pro dilate e/ou erode
    Coordenada centro = criaCoordenada(3,3);
    Imagem* kernel = criaKernelCircular(7);

    Coordenada centroD = criaCoordenada(1,1);
    Imagem* kernelD = criaKernelCircular(3);

    Coordenada centroA = criaCoordenada(2,2);
    Imagem* kernelA = criaKernelCircular(5);

    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);
    img_abertura = criaImagem(img->largura, img->altura, 1);
    img_arroz_colado_diferenciado = criaImagem(img->largura, img->altura, 1);
    buffer = criaImagem(img->largura, img->altura, 1);
    img_out = criaImagem(img->largura, img->altura, 1);
    escala_cinza = criaImagem(img->largura, img->altura, 1);
    RGBParaCinza(img, escala_cinza);

    binarizaAdapt(escala_cinza, img_out, 45, 0.126, buffer);
    abertura(img_out, kernel, centro, img_abertura, buffer);

    img_aux1 = clonaImagem(img_abertura);
    img_aux2 = clonaImagem(img_abertura);

    erode(img_aux2, kernel, centro, img_aux1);
    dilata(img_aux1, kernelA, centroA, img_aux2);
    erode(img_aux2, kernelD, centroD, img_out);

    total = rotulaFloodFill(img_out, &componentes, 2, 2, 10);

    vetor_arroz_grudado = malloc(total*sizeof(int));
    mapa = malloc(total*sizeof(int));
    for(int i = 0; i < total; i++) {
        vetor_arroz_grudado[i] = 0;
        mapa[i] = componentes[i].label;
    }

    printf("Imagem %s\n", INPUT_IMAGE);
    printf("Total de arroz sem contar arroz grudado = %d\n", total);

    /*Passa linha horizontal da esquerda para direita na imagem a procura de arroz grudado, se a linha passar por um blob, passar por um pixel preto e voltar
    a passar no mesmo blob, quer dizer que tem arroz grudado no blob. Estime a quantidade de acordo com quantas vezes encontrar pixel preto dentro do blob.
    Essa estimativa não ficou 100%, tratei na função calculaQuantidadeArrozBlob.
    */
    int arrozGrudadoHorizontal = linhaHorizontal(img_out, &componentes, total, vetor_arroz_grudado, mapa);
    //mesma lógica tentando achar na vertical
    int arrozGrudadoVertical = linhaVertical(img_out, &componentes, total, vetor_arroz_grudado, mapa);
    /*vetor_arroz_grudado passa a conter a quantidade estimada de arroz no blob correpondente, porém esse valor somente é usado na verificação se é maior que 0 para ser considerado um blob
      com mais de um arroz, calculado posteriormente na função calculaQuantidadeArrozBlob.*/

    //executa o cálculo da quantidade de arroz grudado baseado na média somente se tiver arroz grudado...
    if((arrozGrudadoHorizontal + arrozGrudadoVertical) > 0) {
        printf("Total final: %d\n", total + calculaQuantidadeArrozBlob(vetor_arroz_grudado, componentes, total));
    }
    //gera imagem de saida, blob com um arroz = 0.500 e blob com mais de um arroz = 1
    img_arroz_colado_diferenciado = geraImagemArrozColadoDiferenciado(img_out, img_arroz_colado_diferenciado, vetor_arroz_grudado, componentes, total);

    salvaImagem(img_arroz_colado_diferenciado, "img-arroz-colado-diferenciado.bmp");
    salvaImagem(img_out, "img-out.bmp");

    destroiImagem(img);
    destroiImagem(img_out);
    destroiImagem(buffer);
    destroiImagem(escala_cinza);
    free(mapa);
    free(vetor_arroz_grudado);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");
    return 0;
}

/**int main ()
{
    Imagem* img;
    Imagem* img_out;
    Imagem* buffer;
    Imagem* escala_cinza;
    Imagem* img_abertura;
    Imagem* d;
    Imagem* e;
    Imagem* img_arroz_colado_diferenciado;
    ComponenteConexo* componentes;
    int total;
    int* vetor_arroz_grudado;
    float *mapa;

    //diferentes sizes pro dilate e/ou erode
    Coordenada centro = criaCoordenada(3,3);
    Imagem* kernel = criaKernelCircular(7);

    Coordenada centroD = criaCoordenada(1,1);
    Imagem* kernelD = criaKernelCircular(3);

    Coordenada centroA = criaCoordenada(2,2);
    Imagem* kernelA = criaKernelCircular(5);



    img = abreImagem(INPUT_IMAGE, CANAIS_IMAGEM);
    img_abertura = criaImagem(img->largura, img->altura, 1);
    img_arroz_colado_diferenciado = criaImagem(img->largura, img->altura, 1);
    buffer = criaImagem(img->largura, img->altura, 1);
    img_out = criaImagem(img->largura, img->altura, 1);
    escala_cinza = criaImagem(img->largura, img->altura, 1);
    RGBParaCinza(img, escala_cinza);

    binarizaAdapt(escala_cinza, img_out, 45, 0.125, buffer);
    abertura(img_out, kernel, centro, img_abertura, buffer);

    e = clonaImagem(img_abertura);
    d = clonaImagem(img_abertura);

    erode(e, kernel, centro, d);
    dilata(d, kernelA, centroA, e);
    erode(e, kernelD, centroD, d);

    total = rotulaFloodFill(d, &componentes, 2, 2, 10);

    vetor_arroz_grudado = malloc(total*sizeof(int));
    mapa = malloc(total*sizeof(int));
    for(int i = 0; i < total; i++) {
        vetor_arroz_grudado[i] = 0;
        mapa[i] = componentes[i].label;
    }

    printf("Imagem %s\n", INPUT_IMAGE);
    printf("Total de arroz sem contar arroz grudado = %d\n", total);

    /*Passa linha horizontal da esquerda para direita na imagem a procura de arroz grudado, se a linha passar por um blob, passar por um pixel preto e voltar
    a passar no mesmo blob, quer dizer que tem arroz grudado no blob. Estime a quantidade de acordo com quantas vezes encontrar pixel preto dentro do blob.
    Essa estimativa não ficou 100%, tratei na função calculaQuantidadeArrozBlob.

    int arrozGrudadoHorizontal = linhaHorizontal(d, &componentes, total, vetor_arroz_grudado, mapa);
    //mesma lógica tentando achar na vertical
    int arrozGrudadoVertical = linhaVertical(d, &componentes, total, vetor_arroz_grudado, mapa);
    /*vetor_arroz_grudado passa a conter a quantidade estimada de arroz no blob correpondente, porém esse valor somente é usado na verificação se é maior que 0 para ser considerado um blob
      com mais de um arroz, calculado posteriormente na função calculaQuantidadeArrozBlob.

    //executa o cálculo da quantidade de arroz grudado baseado na média somente se tiver arroz grudado...
    if((arrozGrudadoHorizontal + arrozGrudadoVertical) > 0) {
        printf("Total final: %d\n", total + calculaQuantidadeArrozBlob(vetor_arroz_grudado, componentes, total));
    }
    //gera imagem de saida, blob com um arroz = 0.500 e blob com mais de um arroz = 1
    img_arroz_colado_diferenciado = geraImagemArrozColadoDiferenciado(d, img_arroz_colado_diferenciado, vetor_arroz_grudado, componentes, total);

    salvaImagem(img_arroz_colado_diferenciado, "img-arroz-colado-diferenciado.bmp");
    salvaImagem(d, "img-out.bmp");

    destroiImagem(img);
    destroiImagem(img_out);
    destroiImagem(buffer);
    destroiImagem(escala_cinza);
    free(mapa);
    free(vetor_arroz_grudado);

    printf("Pressione qualquer tecla (e de enter) para sair\n");
    scanf("%c");
    return 0;
}**/

int linhaHorizontal(Imagem* img, ComponenteConexo** componentes, int size_componentes, int *vetor, float* mapa)
{
    int total_a_adicionar = 0;
    int passou_preto = 0; //conta quantas vezes passou preto
    int i_salvo = 0;
    float _label;
    int indiceDuplicados = 0;

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(img->dados[0][i][j] != 0) {
                if(passou_preto == 1 && _label == img->dados[0][i][j] && i == i_salvo) {
                    i_salvo = i;
                    total_a_adicionar++;

                    indiceDuplicados = buscaBinaria(mapa, 0, size_componentes-1, _label);

                    if(total_a_adicionar > vetor[indiceDuplicados])
                        vetor[indiceDuplicados] = total_a_adicionar;
                    passou_preto = 0;
                    continue;
                }

                _label = img->dados[0][i][j];
                i_salvo = i;
                passou_preto = 0;
                total_a_adicionar = 0;
            }
            else
                passou_preto = 1;
        }
    }

    total_a_adicionar = 0;
    for(int i = 0; i < size_componentes; i++) {
        total_a_adicionar += vetor[i];
    }
    return total_a_adicionar;
}

int linhaVertical(Imagem* img, ComponenteConexo** componentes, int size_componentes, int* vetor, float* mapa)
{
    int total_a_adicionar = 0;
    int passou_preto = 0; //conta quantas vezes passou preto
    int i_salvo = 0;
    float _label = 0;
    int indiceDuplicados = 0;

    for(int i = 0; i < img->largura; i++) {
        for(int j = 0; j < img->altura; j++) {
            if(img->dados[0][j][i] != 0) {
                if(passou_preto == 1 && _label == img->dados[0][j][i] && i == i_salvo) {
                    i_salvo = i;
                    total_a_adicionar++;

                    indiceDuplicados = buscaBinaria(mapa, 0, size_componentes-1, _label);

                    if(total_a_adicionar > vetor[indiceDuplicados])
                        vetor[indiceDuplicados] = total_a_adicionar;
                    passou_preto = 0;
                    continue;
                }

                _label = img->dados[0][j][i];
                i_salvo = i;
                passou_preto = 0;
                total_a_adicionar = 0;
            }
            else
                passou_preto = 1;
        }
    }

    total_a_adicionar = 0;
    for(int i = 0; i < size_componentes; i++) {
        total_a_adicionar += vetor[i];
    }
    return total_a_adicionar;
}

int calculaQuantidadeArrozBlob(int* duplicados, ComponenteConexo* componentes, int size) {
    int total = 0;
    float media;
    int n = 0;
    int diminuir = 0;

    //faz a media diminuindo a quantidade de blob com arroz grudado
    for(int i = 0; i < size; i++) {
        if(duplicados[i] == 0) {
            total += componentes[i].n_pixels;
        }
        else {
            diminuir++;
        }
    }

    media = total/(size-diminuir);

    //estima a quantidade de arroz dentro do blob grudado
    for(int i = 0; i < size; i++) {
        if(duplicados[i] > 0) {
            if(componentes[i].n_pixels > media && abs(componentes[i].n_pixels - media) > (media/3.2)) { //3.2
                n++;

                //ve quantas vezes o valor total de pixels do blob pode ser dividido em arrozes
                for(int j = 2; (abs((componentes[i].n_pixels / j) - media)) > (media/2.9); j++) { //2.8
                    n++;
                }
            }
        }
    }

    return n;
}

Imagem* geraImagemArrozColadoDiferenciado(Imagem* img, Imagem* out, int* duplicados, ComponenteConexo* componentes, int size) {
    int indice = 0;
    float _label = 0;
    float *vetor_arroz_colado;
    int tamanho = 0;

    for(int i = 0; i < size; i++) {
        if(duplicados[i] > 0)
            tamanho++;
    }

    vetor_arroz_colado = malloc(tamanho*sizeof(float));
    tamanho = 0;

    for(int i = 0; i < size; i++) {
        if(duplicados[i] > 0) {
            vetor_arroz_colado[tamanho++] = componentes[i].label;
        }
    }

    for(int i = 0; i < img->altura; i++) {
        for(int j = 0; j < img->largura; j++) {
            if(img->dados[0][i][j] != 0) {

                indice = buscaBinaria(vetor_arroz_colado, 0, tamanho-1, img->dados[0][i][j]);

                if(indice != -1) {
                    out->dados[0][i][j] = 1;
                }
                else
                    out->dados[0][i][j] = 0.500;
            }
        }
    }
    free(vetor_arroz_colado);
    return out;
}
