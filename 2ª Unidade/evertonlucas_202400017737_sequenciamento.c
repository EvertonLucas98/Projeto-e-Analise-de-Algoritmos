#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

// Define uma constante para o tamanho máximo das strings
#define MAX_STR 20 

typedef struct Subcadeia
{
    char subcadeia[MAX_STR];
} Subcadeia;

typedef struct Doenca
{
    char nome[MAX_STR];
    Subcadeia *subcadeias;
    int qtdSubcadeias;
} Doenca;

typedef struct DoencaArray {
    char dna[MAX_STR * 2]; // DNA deve ser maior
    Doenca* doencas;
    int qtdDoencas, tamanhoSubcadeias;
} DoencaArray;

// Função para ler dados do arquivo de entrada
DoencaArray lerArquivo(FILE* arquivo)
{
    // Lendo dados do arquivo e armazenando nos structs
    DoencaArray doencasArray;
    int qtdDoencas;
    
    fscanf(arquivo, "%u", &doencasArray.tamanhoSubcadeias);
    fscanf(arquivo, "%40s", doencasArray.dna);
    fscanf(arquivo, "%u", &qtdDoencas);

    // Alocando memória para o array de doenças
    doencasArray.doencas = (Doenca*) malloc(qtdDoencas * sizeof(Doenca));

    // Lendo cada doença e suas subcadeias
    for (int i = 0; i < qtdDoencas; ++i)
    {
        // Lendo nome da doença e quantidade de subcadeias
        fscanf(arquivo, "%20s %u", doencasArray.doencas[i].nome, &doencasArray.doencas[i].qtdSubcadeias);
        // Alocando memória para o array de subcadeias
        doencasArray.doencas[i].subcadeias = (Subcadeia*) malloc(doencasArray.doencas[i].qtdSubcadeias * sizeof(Subcadeia));
        // Lendo cada subcadeia
        for (int j = 0; j < doencasArray.doencas[i].qtdSubcadeias; ++j)
            fscanf(arquivo, "%20s", doencasArray.doencas[i].subcadeias[j].subcadeia);
    }

    // Preenchendo o struct DoencaArray
    doencasArray.qtdDoencas = qtdDoencas;

    return doencasArray;
}

// Procedimento para computar o array LPS (Longest Prefix Suffix)
void computarLPS(const char* padrao, int M, int* lps)
{
    int len = 0;
    lps[0] = 0;
    int i = 1;

    while (i < M)
    {
        if (padrao[i] == padrao[len])
        {
            len++;
            lps[i] = len;
            i++;
        } else
        {
            if (len != 0)
            {
                len = lps[len - 1];
            } else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

// Função KMP para buscar ocorrências de um padrão em uma string DNA
int buscarKMP(const char* DNA, const char* padrao, int L)
{
    int N = strlen(DNA); // Tamanho do DNA
    int M = L; // Tamanho do padrão (subcadeia)
    int ocorrencias = 0; // Contador de ocorrências encontradas

    // Caso base: se o padrão for maior que o DNA, não há ocorrências
    if (N < M) return 0;

    // Alocando memória para o array LPS
    int* lps = (int*)malloc(M * sizeof(int));
    if (lps == NULL) return 0;
    
    // Computando o array LPS para o padrão
    computarLPS(padrao, M, lps);

    int i = 0; // Índice para DNA
    int j = 0; // Índice para padrão

    // Percorrendo o DNA
    while (i < N)
    {
        // Se os caracteres coincidem, avançar ambos os índices
        if (padrao[j] == DNA[i])
        {
            i++;
            j++;
        }

        // Se todo o padrão foi encontrado
        if (j == M)
        {
            ocorrencias++; // Incrementar o contador de ocorrências
            i = i - j + M; // Avançar i para continuar a busca
            j = 0; // Reiniciar j para o próximo possível match
        } else if (i < N && padrao[j] != DNA[i]) // Mismatch após j matches
        {
            // Ajustar índice j baseado no array LPS
            if (j != 0)
                j = lps[j - 1]; // Usar LPS para evitar comparações desnecessárias
            else
                i++; // Avançar i se j é 0
        }
    }

    free(lps);

    return ocorrencias;
}

int calcularCompatibilidadeGene(const char* DNA, const char* gene, int L)
{
    int G_len = strlen(gene); // Tamanho do gene
    // Caso base: se o gene for menor que L, não há compatibilidade
    if (G_len < L) return 0;
    
    int N_TotalSub = G_len - L + 1; // Total de subpartes de tamanho L no gene
    int N_Detectadas = 0; // Contador de subpartes detectadas no DNA
    
    // Percorrer todas as subpartes de tamanho L no gene
    for (int i = 0; i <= G_len - L; i++) {
        char subcadeia[MAX_STR]; // Armazena a subparte atual
        strncpy(subcadeia, gene + i, L); // Copia L caracteres do gene
        subcadeia[L] = '\0'; // Adiciona o terminador nulo
        int contagem_sub = buscarKMP(DNA, subcadeia, L); // Usando KMP para busca
        // Se a subparte foi encontrada no DNA, incrementa o contador
        if (contagem_sub > 0) {
            N_Detectadas++;
        }
    }

    // Calcula a porcentagem de compatibilidade
    double percent = round(((double)N_Detectadas / N_TotalSub)*10)*10;
    
    return (int)round(percent); 
}

int diagnosticarDoenca(const char* DNA, Subcadeia* genes, int num_genes, int L)
{
    int genes_detectados = 0;

    for (int i = 0; i < num_genes; i++) {
        int compatibilidade = calcularCompatibilidadeGene(DNA, genes[i].subcadeia, L);
        
        if (compatibilidade >= 90)
            genes_detectados++; 
    }

    double resultado = ((double)genes_detectados / num_genes) * 100.0;
    return (int)round(resultado);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Uso: %s <input> <output>\n", argv[0]);
        return 1;
    }

    // Abrindo os arquivos
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");

    // Lendo os dados do arquivo
    DoencaArray doencasArray = lerArquivo(input);
    
    int* probabilidades = (int*) malloc(doencasArray.qtdDoencas * sizeof(int));

    for (int i = 0; i < doencasArray.qtdDoencas; i++)
    {
        Doenca doenca = doencasArray.doencas[i];
        probabilidades[i] = diagnosticarDoenca(
            doencasArray.dna, 
            doenca.subcadeias, 
            doenca.qtdSubcadeias, 
            doencasArray.tamanhoSubcadeias
        );
    }

    for (int i = 0; i < doencasArray.qtdDoencas - 1; i++)
    {
        for (int j = 0; j < doencasArray.qtdDoencas - i - 1; j++)
        {
            // Sort descending: If current is smaller than next, swap
            if (probabilidades[j] < probabilidades[j + 1])
            {
                int tempVal = probabilidades[j];
                probabilidades[j] = probabilidades[j + 1];
                probabilidades[j + 1] = tempVal;

                Doenca tempDoenca = doencasArray.doencas[j];
                doencasArray.doencas[j] = doencasArray.doencas[j + 1];
                doencasArray.doencas[j + 1] = tempDoenca;
            }
        }
    }

    for (int i = 0; i < doencasArray.qtdDoencas; i++)
    {
        fprintf(output, "%s->%u%%", doencasArray.doencas[i].nome, probabilidades[i]);
        if (i < doencasArray.qtdDoencas - 1) fprintf(output, "\n");
    }
    
    // Limpando memória
    for(int i=0; i < doencasArray.qtdDoencas; i++) {
        free(doencasArray.doencas[i].subcadeias);
    }
    free(doencasArray.doencas);
    free(probabilidades);

    // Fechando os arquivos
    fclose(input);
    fclose(output);

    return 0;
}