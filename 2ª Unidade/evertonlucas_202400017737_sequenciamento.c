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
void computarLPS(const char* padrao, int M, int* lps)
{
    int len = 0; // comprimento do prefixo mais longo que também é sufixo anterior
    lps[0] = 0; // lps[0] é sempre 0
    int i = 1;

    // O loop calcula lps[i] para i = 1 até M-1
    while (i < M)
    {
        if (padrao[i] == padrao[len])
        {
            len++;
            lps[i] = len;
            i++;
        } else
        { // (padrao[i] != padrao[len])
            if (len != 0)
            {
                len = lps[len - 1]; // Não faça len = 0 diretamente, procure no lps anterior
            } else
            { // Se len == 0
                lps[i] = 0;
                i++;
            }
        }
    }
}

// Busca por KMP
int buscarKMP(const char* DNA, const char* padrao, int L)
{
    int N = strlen(DNA);
    int M = L; // Tamanho do padrão
    int ocorrencias = 0;

    if (N < M) return 0;

    // 1. Pré-processamento: Cria e computa a Tabela LPS
    int* lps = (int*)malloc(M * sizeof(int));
    if (lps == NULL)
    {
        perror("Erro de alocação de memória para LPS");
        return 0;
    }
    computarLPS(padrao, M, lps);

    int i = 0; // Ponteiro para o DNA (Texto)
    int j = 0; // Ponteiro para o Padrão (Subcadeia)

    // Busca KMP Modificada
    while (i < N)
    {
        if (padrao[j] == DNA[i])
        {
            i++;
            j++;
        }

        if (j == M)
        {
            ocorrencias++; // Padrão encontrado
            i = i - j + M; // Força o ponteiro 'i' a pular o padrão encontrado
            j = 0; // Reinicia j para buscar a próxima ocorrência
        } else if (i < N && padrao[j] != DNA[i])
        {
            // Incompatibilidade
            if (j != 0)
            {
                // Não precisa retroceder 'i', apenas 'j' usando a tabela LPS
                j = lps[j - 1];
            } else
            {
                // Se j == 0, avança 'i' no DNA
                i++;
            }
        }
    }

    free(lps);
    return ocorrencias;
}

uint32_t calcularCompatibilidadeGene(const char* DNA, const char* gene, uint32_t L)
{
    uint32_t G_len = strlen(gene);
    
    // Cálculo do Total de Subcadeias (N_TotalSub)
    if (G_len < L) {
        // Gene é menor que o tamanho da subcadeia, impossível
        return 0; 
    }
    uint32_t N_TotalSub = G_len - L + 1; // Número total de subcadeias de tamanho L no gene
    uint32_t N_Detectadas = 0; // Contador de subcadeias detectadas no DNA
    
    // Loop sobre as subcadeias do Gene
    for (uint32_t i = 0; i <= G_len - L; i++) {
        // Extrai a subcadeia
        char subcadeia[L + 1];
        strncpy(subcadeia, gene + i, L);
        subcadeia[L] = '\0'; // Termina a string
        
        // Busca Otimizada e Contagem (KMP)
        int contagem_sub = buscarKMP(DNA, subcadeia, L);
        
        if (contagem_sub > 0) {
            // Se a subcadeia for encontrada pelo menos uma vez, contamos como detectada
            N_Detectadas++;
        }
    }

    // Cálculo da Compatibilidade
    uint32_t compatibilidade = round(((double)N_Detectadas / N_TotalSub)*10)*10;

    return compatibilidade;
}

uint32_t diagnosticarDoenca(const char* DNA, char genes[][20], uint32_t num_genes, uint32_t L)
{
    uint32_t genes_detectados = 0;

    // Loop pelos genes da doença
    for (uint32_t i = 0; i < num_genes; i++) {
        
        // Calcula a compatibilidade deste gene específico
        uint32_t compatibilidade = calcularCompatibilidadeGene(DNA, genes[i], L);
        // Aplica o Limiar de 90%
        if (compatibilidade >= 90)
            genes_detectados++; // O gene conta para o diagnóstico
    }

    // Calcula a Porcentagem Final
    uint32_t probabilidade_doenca = ((double)genes_detectados / num_genes) * 100;

    return probabilidade_doenca;
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
    char* dna = doencasArray.dna;
    uint32_t qtdDoencas = doencasArray.qtdDoencas;
    uint32_t* probabilidades = (uint32_t*) malloc(qtdDoencas * sizeof(uint32_t));

    // Diagnóstico para cada doença
    for (uint32_t i = 0; i < qtdDoencas; i++)
    {
        Doenca doenca = doencasArray.doencas[i];
        uint32_t L = doencasArray.tamanhoSubcadeias;
        // Preparando os genes para diagnóstico
        char genes[doenca.qtdSubcadeias][20];
        for (uint32_t j = 0; j < doenca.qtdSubcadeias; j++)
        {
            strcpy(genes[j], doenca.subcadeias[j].subcadeia);
        }
        uint32_t probabilidade_doenca = diagnosticarDoenca(dna, genes, doenca.qtdSubcadeias, L);
        probabilidades[i] = probabilidade_doenca;
    }

    uint32_t temp;
    // Ordenando os resultados por probabilidade (Bubble Sort)
    for (uint32_t i = 0; i < qtdDoencas - 1; i++)
    {
        for (uint32_t j = 0; j < qtdDoencas - i - 1; j++)
        {
            if (probabilidades[j] < probabilidades[j + 1])
            {
                // Troca de probabilidades
                temp = probabilidades[j];
                probabilidades[j] = probabilidades[j + 1];
                probabilidades[j + 1] = temp;
                // Troca de doenças correspondentes
                Doenca tempDoenca = doencasArray.doencas[j];
                doencasArray.doencas[j] = doencasArray.doencas[j + 1];
                doencasArray.doencas[j + 1] = tempDoenca;
            }
        }
    }

    // Escrevendo resultados no arquivo de saída
    for (uint32_t i = 0; i < qtdDoencas; i++)
    {
        if (i == qtdDoencas - 1)
            fprintf(output, "%s->%u%%", doencasArray.doencas[i].nome, probabilidades[i]);
        else
            fprintf(output, "%s->%u%%\n", doencasArray.doencas[i].nome, probabilidades[i]);
    }
    
    // Fechando os arquivos
    fclose(input);
    fclose(output);

    return 0;
}