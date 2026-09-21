#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int id;              // Identificador da música
    int minutos;         // Minutos da duração
    int segundos;        // Segundos da duração
    int tempoTotal;      // Tempo em segundos (para cálculos)
} Musica;

typedef struct {
    int tempoTotalMinutos;  // Tempo total em minutos (ambos os lados)
    int quantidadeMusicas;  // Quantidade de músicas disponíveis
    Musica *musicas;        // Array de músicas
} Tape;

/*
 * Converte minutos e segundos para segundos totais
 */
int converterParaSegundos(int minutos, int segundos) {
    return minutos * 60 + segundos;
}

/*
 * Converte segundos de volta para minutos e segundos
 */
void converterParaMiSeg(int totalSegundos, int *minutos, int *segundos) {
    *minutos = totalSegundos / 60;
    *segundos = totalSegundos % 60;
}

/*
 * Lê um conjunto de teste do arquivo
 */
bool lerTestCase(FILE *arquivo, Tape *tape) {
    // Ler tempo total em minutos e quantidade de músicas
    if (fscanf(arquivo, "%d %d", &tape->tempoTotalMinutos, &tape->quantidadeMusicas) != 2) {
        return false;
    }

    // Alocar memória para as músicas
    tape->musicas = (Musica *)malloc(tape->quantidadeMusicas * sizeof(Musica));
    if (tape->musicas == NULL) {
        printf("Erro: falha na alocação de memória!\n");
        return false;
    }

    // Ler cada música
    for (int i = 0; i < tape->quantidadeMusicas; i++) {
        int m, s;
        if (fscanf(arquivo, "%d %d", &m, &s) != 2) {
            printf("Erro: falha ao ler a música %d!\n", i + 1);
            free(tape->musicas);
            return false;
        }

        tape->musicas[i].id = i + 1;
        tape->musicas[i].minutos = m;
        tape->musicas[i].segundos = s;
        tape->musicas[i].tempoTotal = converterParaSegundos(m, s);
    }

    return true;
}

/*
 * Lê todas as entradas do arquivo tape.in
 */
int lerEntrada(char *nomeArquivo, Tape **vetorTapes) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        printf("Erro: não foi possível abrir o arquivo %s!\n", nomeArquivo);
        return 0;
    }

    // Ler número de testes
    int numTestes;
    if (fscanf(arquivo, "%d", &numTestes) != 1) {
        printf("Erro: não foi possível ler o número de testes!\n");
        fclose(arquivo);
        return 0;
    }

    // Alocar memória para o vetor de Tapes
    *vetorTapes = (Tape *)malloc(numTestes * sizeof(Tape));
    if (*vetorTapes == NULL) {
        printf("Erro: falha na alocação de memória para testes!\n");
        fclose(arquivo);
        return 0;
    }

    // Ler cada teste
    int testesLidos = 0;
    for (int i = 0; i < numTestes; i++) {
        if (lerTestCase(arquivo, &(*vetorTapes)[i])) {
            testesLidos++;
        } else {
            break;
        }
    }

    fclose(arquivo);
    return testesLidos;
}

/*
 * Exibe o resultado de um teste
 */
void exibirResultado(int numCaso, Tape *tape, int *indicesMusicasLadoA, int quantidadeLadoA, int tempoLadoA, int tempoLadoB) {
    printf("Caso:%d\n", numCaso);

    // Verificar se há solução válida
    if (quantidadeLadoA == -1) {
        printf("Impossivel gravar as musicas nessa fita.\n\n");
        return;
    }

    printf("Lado A\n");
    for (int i = 0; i < quantidadeLadoA; i++) {
        int idx = indicesMusicasLadoA[i];
        Musica m = tape->musicas[idx];
        printf("%dm %ds\n", m.minutos, m.segundos);
    }

    printf("Lado B\n");
    
    // Cria um array de booleanos para marcar quais músicas estão no Lado A
    bool *emLadoA = (bool *)calloc(tape->quantidadeMusicas, sizeof(bool));
    for (int i = 0; i < quantidadeLadoA; i++) {
        emLadoA[indicesMusicasLadoA[i]] = true;
    }

    // Imprime as músicas do Lado B (as que não estão no Lado A)
    for (int i = 0; i < tape->quantidadeMusicas; i++) {
        if (!emLadoA[i]) {
            Musica m = tape->musicas[i];
            printf("%dm %ds\n", m.minutos, m.segundos);
        }
    }

    printf("\n");
    free(emLadoA);
}

/*
 * Libera a memória alocada para um Tape
 */
void liberarTape(Tape *tape) {
    if (tape->musicas != NULL) {
        free(tape->musicas);
        tape->musicas = NULL;
    }
}

/*
 * Libera a memória alocada para um vetor de Tapes
 */
void liberarVetorTapes(Tape *vetorTapes, int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        liberarTape(&vetorTapes[i]);
    }
    free(vetorTapes);
}

void recuperarSolucao(Musica *musicas, int total, bool **dp, int tempoAlvo, int *ladoA, int *tamA) {
    int j = tempoAlvo;
    *tamA = 0;

    for (int i = total; i > 0 && j > 0; i--) {
        int duracao = musicas[i - 1].tempoTotal;

        // Se a solução sem a música (i - 1) não existia para a soma 'j',
        // significa que a música (i - 1) foi selecionada para o Lado A.
        if (!dp[i - 1][j]) {
            ladoA[*tamA] = i - 1; // Salva o índice (0 a total-1)
            (*tamA)++;
            j -= duracao;
        }
    }
}

/*
 * 3.1 Função Principal de Resolução (Programação Dinâmica)
 * Determina se é possível dividir as músicas em dois lados iguais.
 */
bool resolverTape(Musica *musicas, int total, int tempoTotal, int *ladoA, int *tamA) {
    // 3.4 Casos Especiais: Verificar se a soma total de todas as músicas é igual a tempoTotal * 60
    int somaTotal = 0;
    for (int i = 0; i < total; i++) {
        somaTotal += musicas[i].tempoTotal;
    }

    if (somaTotal != tempoTotal * 60) {
        return false;
    }

    // Passo 1: Calcular o tempo alvo para cada lado em segundos
    int tempoAlvo = (tempoTotal * 60) / 2;

    // 3.4 Casos Especiais: Se alguma música for maior que tempoAlvo, elimina a opção
    for (int i = 0; i < total; i++) {
        if (musicas[i].tempoTotal > tempoAlvo) {
            return false;
        }
    }

    // Passo 2: Criar matriz de programação dinâmica dp[total + 1][tempoAlvo + 1]
    bool **dp = (bool **)malloc((total + 1) * sizeof(bool *));
    if (dp == NULL) return false;

    for (int i = 0; i <= total; i++) {
        dp[i] = (bool *)calloc(tempoAlvo + 1, sizeof(bool));
        if (dp[i] == NULL) {
            for (int k = 0; k < i; k++) free(dp[k]);
            free(dp);
            return false;
        }
    }

    // Caso base: 0 músicas conseguem preencher 0 segundos
    dp[0][0] = true;

    // Passo 3: Preencher a matriz verificando todas as combinações
    for (int i = 1; i <= total; i++) {
        int duracao = musicas[i - 1].tempoTotal;
        for (int j = 0; j <= tempoAlvo; j++) {
            // Manter a solução sem incluir a música i
            dp[i][j] = dp[i - 1][j];

            // Se for possível incluir a música i e obter a soma j
            if (duracao <= j && dp[i - 1][j - duracao]) {
                dp[i][j] = true;
            }
        }
    }

    // Passo 4: Verificar se a partição exata foi atingida
    bool temSolucao = dp[total][tempoAlvo];

    if (temSolucao) {
        // Recuperar as músicas que pertencem ao Lado A
        recuperarSolucao(musicas, total, dp, tempoAlvo, ladoA, tamA);
    }

    // Desalocar a matriz DP
    for (int i = 0; i <= total; i++) {
        free(dp[i]);
    }
    free(dp);

    return temSolucao;
}

int main(int argc, char *argv[]) {
    Tape *vetorTapes = NULL;
    char *nomeArquivo = "tape.in";

    if (argc > 1) {
        nomeArquivo = argv[1];
    }

    int numTestes = lerEntrada(nomeArquivo, &vetorTapes);

    for (int i = 0; i < numTestes; i++) {
        int *ladoA = (int *)malloc(vetorTapes[i].quantidadeMusicas * sizeof(int));
        int tamA = 0;

        bool temSolucao = resolverTape(vetorTapes[i].musicas,
                                       vetorTapes[i].quantidadeMusicas,
                                       vetorTapes[i].tempoTotalMinutos,
                                       ladoA,
                                       &tamA);

        if (temSolucao) {
            exibirResultado(i + 1, &vetorTapes[i], ladoA, tamA);
        } else {
            exibirResultado(i + 1, &vetorTapes[i], NULL, -1);
        }

        free(ladoA);
    }

    liberarVetorTapes(vetorTapes, numTestes);
    return 0;
}
