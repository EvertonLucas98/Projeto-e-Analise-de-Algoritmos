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

DoencaArray leituraArquivo(FILE* arquivo)
{
    DoencaArray doencasArray;
    fscanf(arquivo, "%u", &doencasArray.tamanhoSubcadeias);
    fscanf(arquivo, "%30s", doencasArray.dna);

    uint32_t qtdDoencas;
    fscanf(arquivo, "%u", &qtdDoencas);

    Doenca* doencas = (Doenca*) malloc(qtdDoencas * sizeof(Doenca));

    for (uint32_t i = 0; i < qtdDoencas; ++i)
    {
        fscanf(arquivo, "%19s %u", doencas[i].nome, &doencas[i].qtdSubcadeias);
        doencas[i].subcadeias = (Subcadeia*) malloc(doencas[i].qtdSubcadeias * sizeof(Subcadeia));

        for (uint32_t j = 0; j < doencas[i].qtdSubcadeias; ++j)
        {
            fscanf(arquivo, "%39s", doencas[i].subcadeias[j].subcadeia);
        }
    }

    doencasArray.doencas = doencas;
    doencasArray.qtdDoencas = qtdDoencas;
    
    return doencasArray;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Uso: %s <arquivo_entrada>\n", argv[0]);
        return 1;
    }

    // Abrindo os arquivos
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");

    // Lendo dados do arquivo
    DoencaArray doencasArray = leituraArquivo(input);
    
    // Fechando os arquivos
    fclose(input);
    fclose(output);

    return 0;
}