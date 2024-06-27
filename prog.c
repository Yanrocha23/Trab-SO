#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define POSICAO(I, J, COLUNAS) ((I) * (COLUNAS) + (J))

typedef struct {
    float *matriz;
    const char *arquivo;
    int numLinhas;
    int numColunas;
} LeituraArgs;

typedef struct {
    float *matrizA;
    float *matrizB;
    float *matrizD;
    int numLinhas;
    int numColunas;
} SomaArgs;

typedef struct {
    float *matriz;
    const char *arquivo;
    int numLinhas;
    int numColunas;
} GravacaoArgs;

typedef struct {
    float *matrizD;
    float *matrizC;
    float *matrizE;
    int numLinhas;
    int numColunas;
} MultiplicacaoArgs;

int T;

void leMatrizDeArquivoTextoFunc(LeituraArgs* leituraArgs) {
    FILE *arquivo = fopen(leituraArgs->arquivo, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", leituraArgs->arquivo);
        pthread_exit(NULL);
    }
    for (int i = 0; i < leituraArgs->numLinhas; i++) {
        for (int j = 0; j < leituraArgs->numColunas; j++) {
            fscanf(arquivo, "%f", &leituraArgs->matriz[POSICAO(i, j, leituraArgs->numColunas)]);
        }
    }
    fclose(arquivo);
}

void* leMatrizDeArquivoTexto(void* args) {
    LeituraArgs* leituraArgs = (LeituraArgs*)args;
    leMatrizDeArquivoTextoFunc(leituraArgs);
    pthread_exit(NULL);
}

void somaMatrizesFunc(SomaArgs* somaArgs) {
    for (int i = 0; i < somaArgs->numLinhas; i++) {
        for (int j = 0; j < somaArgs->numColunas; j++) {
            somaArgs->matrizD[POSICAO(i, j, somaArgs->numColunas)] = somaArgs->matrizA[POSICAO(i, j, somaArgs->numColunas)] + somaArgs->matrizB[POSICAO(i, j, somaArgs->numColunas)];
        }
    }
}

void* somaMatrizes(void* args) {
    SomaArgs* somaArgs = (SomaArgs*)args;
    somaMatrizesFunc(somaArgs);
    pthread_exit(NULL);
}

void gravaMatrizEmArquivoTextoFunc(GravacaoArgs* gravacaoArgs) {
    FILE *arquivo = fopen(gravacaoArgs->arquivo, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", gravacaoArgs->arquivo);
        pthread_exit(NULL);
    }
    for (int i = 0; i < gravacaoArgs->numLinhas; i++) {
        for (int j = 0; j < gravacaoArgs->numColunas; j++) {
            fprintf(arquivo, "%.2f ", gravacaoArgs->matriz[POSICAO(i, j, gravacaoArgs->numColunas)]);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

void* gravaMatrizEmArquivoTexto(void* args) {
    GravacaoArgs* gravacaoArgs = (GravacaoArgs*)args;
    gravaMatrizEmArquivoTextoFunc(gravacaoArgs);
    pthread_exit(NULL);
}

void multiplicacaoMatrizesFunc(MultiplicacaoArgs* multiplicacaoArgs) {
    for (int i = 0; i < multiplicacaoArgs->numLinhas; i++) {
        for (int j = 0; j < multiplicacaoArgs->numColunas; j++) {
            multiplicacaoArgs->matrizE[POSICAO(i, j, multiplicacaoArgs->numColunas)] = 0;
            for (int k = 0; k < multiplicacaoArgs->numColunas; k++) {
                multiplicacaoArgs->matrizE[POSICAO(i, j, multiplicacaoArgs->numColunas)] += multiplicacaoArgs->matrizD[POSICAO(i, k, multiplicacaoArgs->numColunas)] * multiplicacaoArgs->matrizC[POSICAO(k, j, multiplicacaoArgs->numColunas)];
            }
        }
    }
}

void* multiplicacaoMatrizes(void* args) {
    MultiplicacaoArgs* multiplicacaoArgs = (MultiplicacaoArgs*)args;
    multiplicacaoMatrizesFunc(multiplicacaoArgs);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 8) {
        fprintf(stderr, "Uso: %s T n arqA.dat arqB.dat arqC.dat arqD.dat arqE.dat\n", argv[0]);
        return 1;
    }

    T = atoi(argv[1]);
    int n = atoi(argv[2]);
    const char *arquivoA = argv[3];
    const char *arquivoB = argv[4];
    const char *arquivoC = argv[5];
    const char *arquivoD = argv[6];
    const char *arquivoE = argv[7];

    // Alocação de memória para as matrizes
    float *matrizA = (float *)malloc(n * n * sizeof(float));
    float *matrizB = (float *)malloc(n * n * sizeof(float));
    float *matrizC = (float *)malloc(n * n * sizeof(float));
    float *matrizD = (float *)malloc(n * n * sizeof(float));
    float *matrizE = (float *)malloc(n * n * sizeof(float));

    // Verificação de alocação de memória
    if (!matrizA || !matrizB || !matrizC || !matrizD || !matrizE) {
        fprintf(stderr, "Erro na alocação de memória\n");
        return 1;
    }

    pthread_t threads[T];
    int threadIndex = 0;

    // Passo 1: Leitura das matrizes A e B
    LeituraArgs leituraArgsA = {matrizA, arquivoA, n, n};
    LeituraArgs leituraArgsB = {matrizB, arquivoB, n, n};
    if (T == 1) {
        leMatrizDeArquivoTextoFunc(&leituraArgsA);
        leMatrizDeArquivoTextoFunc(&leituraArgsB);
    } else {
        pthread_create(&threads[threadIndex++], NULL, leMatrizDeArquivoTexto, &leituraArgsA);
        pthread_create(&threads[threadIndex++], NULL, leMatrizDeArquivoTexto, &leituraArgsB);
        for (int i = 0; i < threadIndex; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    // Passo 2: Soma das matrizes A e B
    clock_t inicio = clock();
    SomaArgs somaArgs = {matrizA, matrizB, matrizD, n, n};
    if (T == 1) {
        somaMatrizesFunc(&somaArgs);
    } else {
        pthread_create(&threads[0], NULL, somaMatrizes, &somaArgs);
        pthread_join(threads[0], NULL);
    }
    clock_t fim = clock();
    double tempoSoma = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // Passo 3 e Passo 4: Gravação da matriz D e Leitura da matriz C
    GravacaoArgs gravacaoArgsD = {matrizD, arquivoD, n, n};
    LeituraArgs leituraArgsC = {matrizC, arquivoC, n, n};
    if (T == 1) {
        gravaMatrizEmArquivoTextoFunc(&gravacaoArgsD);
        leMatrizDeArquivoTextoFunc(&leituraArgsC);
    } else {
        pthread_create(&threads[0], NULL, gravaMatrizEmArquivoTexto, &gravacaoArgsD);
        pthread_create(&threads[1], NULL, leMatrizDeArquivoTexto, &leituraArgsC);
        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);
    }

    // Passo 5: Multiplicação das matrizes C e D
    inicio = clock();
    MultiplicacaoArgs multiplicacaoArgs = {matrizD, matrizC, matrizE, n, n};
    if (T == 1) {
        multiplicacaoMatrizesFunc(&multiplicacaoArgs);
    } else {
        pthread_create(&threads[0], NULL, multiplicacaoMatrizes, &multiplicacaoArgs);
        pthread_join(threads[0], NULL);
    }
    fim = clock();
    double tempoMultiplicacao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // Passo 6 e Passo 7: Gravação da matriz E e Redução da matriz E
    GravacaoArgs gravacaoArgsE = {matrizE, arquivoE, n, n};
    if (T == 1) {
        gravaMatrizEmArquivoTextoFunc(&gravacaoArgsE);
    } else {
        pthread_create(&threads[0], NULL, gravaMatrizEmArquivoTexto, &gravacaoArgsE);
    }

    // Redução da matriz E e saída do valor na tela
    inicio = clock();
    float reducao = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            reducao += matrizE[POSICAO(i, j, n)];
        }
    }
    fim = clock();
    double tempoReducao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    if (T != 1) {
        pthread_join(threads[0], NULL);
    }

    double tempoTotal = tempoSoma + tempoMultiplicacao + tempoReducao;

    // Exibição dos resultados formatados
    printf("Reducao: %.0f\n", reducao);
    printf("Tempo soma: %.5f segundos.\n", tempoSoma);
    printf("Tempo multiplicacao: %.5f segundos.\n", tempoMultiplicacao);
    printf("Tempo reducao: %.5f segundos.\n", tempoReducao);
    printf("Tempo total: %.5f segundos.\n", tempoTotal);

    // Liberação de memória
    free(matrizA);
    free(matrizB);
    free(matrizC);
    free(matrizD);
    free(matrizE);

    return 0;
}