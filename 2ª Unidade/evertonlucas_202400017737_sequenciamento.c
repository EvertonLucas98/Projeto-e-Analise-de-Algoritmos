#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

/*
input:
3
AAAATTTCGTTAAATTTGAACATAGGGATA
4
ABCDE 3 AAA AAT AAAG
XY1WZ2AB 1 TTTTTTGGGG
H1N1 4 ACTG AACCGGTT AATAAT AAAAAAAAGA
HUEBR 1 CATAGGGATT

output:
XY1WZ2AB->100%
HUEBR->100%
ABCDE->67%
H1N1->25%
*/

typedef struct Subcadeia
{
    char subcadeia[20];
} Subcadeia;

typedef struct Doenca
{
    char nome[20];
    Subcadeia *subcadeias;
    uint32_t qtdSubcadeias;
} Doenca;

typedef struct DoencaArray {
    char dna[40];
    Doenca* doencas;
    uint32_t qtdDoencas, tamanhoSubcadeias;
} DoencaArray;

// Função para ler dados do arquivo de entrada
DoencaArray lerArquivo(FILE* arquivo)
{
    // Lendo dados do arquivo e armazenando nos structs
    DoencaArray doencasArray;
    uint32_t qtdDoencas;
    fscanf(arquivo, "%u", &doencasArray.tamanhoSubcadeias);
    fscanf(arquivo, "%30s", doencasArray.dna);
    fscanf(arquivo, "%u", &qtdDoencas);

    // Alocando memória para o array de doenças
    Doenca* doencas = (Doenca*) malloc(qtdDoencas * sizeof(Doenca));

    // Lendo cada doença e suas subcadeias
    for (uint32_t i = 0; i < qtdDoencas; ++i)
    {
        // Lendo nome da doença e quantidade de subcadeias
        fscanf(arquivo, "%20s %u", doencas[i].nome, &doencas[i].qtdSubcadeias);
        // Alocando memória para o array de subcadeias
        doencas[i].subcadeias = (Subcadeia*) malloc(doencas[i].qtdSubcadeias * sizeof(Subcadeia));
        // Lendo cada subcadeia
        for (uint32_t j = 0; j < doencas[i].qtdSubcadeias; ++j)
            fscanf(arquivo, "%20s", doencas[i].subcadeias[j].subcadeia);
    }

    // Preenchendo o struct DoencaArray
    doencasArray.doencas = doencas;
    doencasArray.qtdDoencas = qtdDoencas;
    
    return doencasArray;
}

// Procedimento para inserir índice na lista de resultados
void inserir(int32_t* R, int32_t s)
{
    for(int32_t i = 0; ; i++)
    {
        if(R[i] == -1)
        {
            R[i] = s;
            break;
        }
    }
}

// Procedimento de cálculo da tabela de transição
void calcular_tabela(int32_t* k, char* P)
{
    // i = sufixo , j = prefixo
    for(int32_t i = 1, j = -1; i < (int32_t)strlen(P); i++) {
        // Prefixo e sufixo diferentes
        while(j >= 0 && P[j + 1] != P[i])
            j = k[j]; // Retorno de estado
        // Combinação de prefixo e sufixo
        if(P[j + 1] == P[i])
            j++; // Avanço de estado
        // Atualização da transição do estado
        k[i] = j;
    }
}

// Busca por KMP
void KMP(int32_t* k, int32_t* R, char* T, char* P)
{
    // Pré-processamento
    int32_t n = strlen(T), m = strlen(P);
    calcular_tabela(k, P);
    // Iterando na cadeia T
    for(int32_t i = 0, j = -1; i < n; i++)
    {
        // Retorno de estado
        while(j >= 0 && P[j + 1] != T[i]) j = k[j];
        // Avanço de estado
        if(P[j + 1] == T[i]) j++;
        // Combinação do padrão
        if(j == m - 1)
        {
            // Armazenando índice de combinação
            inserir(R, i - m + 1);
            j = k[j];
        }
    }
}

int main(int argc, char *argv[])
{
    // Verificação de argumentos
    if (argc != 3)
    {
        printf("Uso: %s <arquivo_entrada>\n", argv[0]);
        return 1;
    }

    // Abrindo os arquivos
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");

    // Lendo dados do arquivo
    DoencaArray doencasArray = lerArquivo(input);

    // Processando cada doença
    
    // Fechando os arquivos
    fclose(input);
    fclose(output);

    return 0;
}