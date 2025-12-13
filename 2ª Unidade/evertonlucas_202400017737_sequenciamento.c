#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct Gene
{
    char nomeGene[2000];
} Gene;

typedef struct Doenca
{
    char nomeDoenca[100];
    Gene* genes;
    int qtdGenes;
} Doenca;

typedef struct Arquivo
{
    char dna[100000];
    Doenca* doencas;
    char** nomesGenes;
    int qtdDoencas;
    int tamanhoSubGenes;
} Arquivo;

typedef struct Resultado
{
    char nomeDoenca[100];
    int probabilidade;
} Resultado;

// Função para ler dados do arquivo de entrada
Arquivo lerArquivo(FILE* arquivo)
{
    Arquivo dadosArquivo;
    int qtdDoencas = 0, totalGenes = 0;
    
    // Lendo tamanho dos subgenes
    if (fscanf(arquivo, "%d", &dadosArquivo.tamanhoSubGenes) != 1) dadosArquivo.tamanhoSubGenes = 0;
    // Lendo DNA
    if (fscanf(arquivo, "%s", dadosArquivo.dna) != 1) dadosArquivo.dna[0] = '\0';
    // Lendo quantidade de doenças
    if (fscanf(arquivo, "%d", &qtdDoencas) != 1) qtdDoencas = 0;
    
    dadosArquivo.qtdDoencas = qtdDoencas;
    // Alocando memória para o array de doenças
    dadosArquivo.doencas = malloc(qtdDoencas * sizeof(Doenca));

    // Lendo cada doença e seus respectivos genes
    for (int i = 0; i < qtdDoencas; ++i)
    {
        dadosArquivo.doencas[i].qtdGenes = 0;
        fscanf(arquivo, "%s %d", dadosArquivo.doencas[i].nomeDoenca, &dadosArquivo.doencas[i].qtdGenes);
        
        int qtdGenes = dadosArquivo.doencas[i].qtdGenes;
        dadosArquivo.doencas[i].genes = malloc(qtdGenes * sizeof(Gene));
        totalGenes += qtdGenes;
        
        for (int j = 0; j < qtdGenes; ++j)
            fscanf(arquivo, "%s", dadosArquivo.doencas[i].genes[j].nomeGene);
    }

    return dadosArquivo;
}

// Procedimento que computa array de prefixos (LPS) para o KMP
void computarLPS(const char* padrao, int M, int* lps)
{
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < M) {
        if (padrao[i] == padrao[len])
        {
            len++;
            lps[i] = len;
            i++;
        } else {
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

// Função para calcular a compatibilidade entre o DNA e um gene específico
int calcularCompatibilidade(const char* dna, const char* gene, const int L)
{
    // Tamanho do DNA e do Gene
    int dnaLen = (int)strlen(dna);
    int geneLen = (int)strlen(gene);
    if (dnaLen == 0 || geneLen == 0 || L <= 0) return 0;

    int totalMatchs = 0;
    int geneIdx = 0; // Onde estamos no Gene global
    int dnaIdx = 0;  // Onde estamos no DNA global

    // Aloca vetor LPS uma única vez com o tamanho máximo necessário
    int *lps = (int*)malloc(sizeof(int) * geneLen);
    if (!lps) return 0;
    
    // Enquanto houver gene para processar e DNA para procurar
    while (geneIdx < geneLen && dnaIdx < dnaLen)
    {
        // Define o "padrão" atual como sendo o restante do gene
        const char* subGene = &gene[geneIdx];
        int subGeneLen = geneLen - geneIdx;
        
        // printf("Procurando subGene \"%s\" a partir da pos %d\n", subGene, dnaIdx);

        // Se o que sobrou do gene é menor que L, impossível dar match válido
        if (subGeneLen < L) break;

        // Calcula LPS apenas para o pedaço restante do gene
        computarLPS(subGene, subGeneLen, lps);

        int j = 0; // Índice de match no subGene
        int matchEncontrado = 0;
        int tamMatch = 0;
        int k;

        // Busca KMP no DNA a partir de onde paramos (dnaIdx)
        for (k = dnaIdx; k < dnaLen; k++)
        {
            // Ajusta j usando LPS em caso de mismatch
            while (j > 0 && subGene[j] != dna[k])
                j = lps[j - 1];
            // Se houver match, avança j
            if (subGene[j] == dna[k])
                j++;

            // Assim que encontrarmos um match de tamanho L, paramos de buscar novas ocorrências e tentamos estender este match o máximo possível
            if (j == L)
            {
                // Temos um match validado de tamanho L.
                tamMatch = L;

                // Tenta estender linearmente (verificar próximos caracteres)
                // Verifica limites do DNA e do Gene
                while ((k + 1 < dnaLen) && 
                       (tamMatch < subGeneLen) && 
                       (dna[k + 1] == subGene[tamMatch]))
                {
                    k++;        // Avança no DNA
                    tamMatch++; // Avança no Gene
                }
                // printf("MATCH encontrado! DNA[%d..%d], tam=%d\n", k - tamMatch + 1, k, tamMatch);
                // Match finalizado
                matchEncontrado = 1;
                
                // Atualiza contadores globais
                totalMatchs += tamMatch;
                geneIdx += tamMatch;
                
                // O próximo loop do DNA deve começar DEPOIS deste match
                dnaIdx = k + 1;
                // Sai do for KMP para reiniciar o processo com o resto do gene
                break;
            }
        }

        // Se não achou nenhum match >= L
        if (!matchEncontrado)
        {
            // printf("Nenhum match encontrado para subGene \"%s\" a partir da pos %d.\n", subGene, dnaIdx);
            break; 
        }
    }

    // Libera memória do LPS
    free(lps);
    // Calcula percentual arredondado
    double percentual = ((double)totalMatchs * 100.0) / (double)geneLen;

    return (int)(percentual + (percentual < 0.0 ? -0.5 : 0.5));
}

// Função para diagnosticar a doença com base no DNA e seus genes
int diagnosticarDoenca(const char* DNA, Gene* genes, int numGenes, int L)
{
    if (!DNA || !genes || numGenes <= 0 || L <= 0) return 0;
    int genes_detectados = 0;
    
    for (int i = 0; i < numGenes; i++)
    {
        // printf("Gene: %s\n", genes[i].nomeGene);
        int compatibilidade = calcularCompatibilidade(DNA, genes[i].nomeGene, L);
        if (compatibilidade >= 90) genes_detectados++;
    }

    double resultado = ((double)genes_detectados / (double)numGenes) * 100.0;
    
    return (int)(resultado + 0.5);
}

// Função para ordenar os resultados em ordem decrescente de probabilidade
void ordenarResultados(Resultado* resultados, int qtdResultados)
{
    // Insertion Sort
    for (int i = 1; i < qtdResultados; i++)
    {
        Resultado key = resultados[i];
        int j = i - 1;

        // Move os elementos que são menores que key para uma posição à frente
        while (j >= 0 && resultados[j].probabilidade < key.probabilidade)
        {
            resultados[j + 1] = resultados[j];
            j = j - 1;
        }
        resultados[j + 1] = key;
    }
}

// Procedimento para escrever os resultados no arquivo de saída
void escreverResultados(FILE* output, Resultado* resultados, int qtdResultados)
{
    // printf("\n==================================================\n");
    // printf("\t\tResultados Finais:\n");
    // printf("==================================================\n");
    for (int i = 0; i < qtdResultados; i++)
    {
        fprintf(output, "%s->%d%%", resultados[i].nomeDoenca, resultados[i].probabilidade);
        // printf("%s->%d%%", resultados[i].nomeDoenca, resultados[i].probabilidade);
        if (i < qtdResultados - 1)
        {
            fprintf(output, "\n");
            // printf("\n");
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Uso: %s <input> <output>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Erro ao abrir input");
        return 1;
    }
    
    FILE* output = fopen(argv[2], "w");
    if (!output) {
        perror("Erro ao abrir output");
        fclose(input);
        return 1;
    }

    // Lê os dados do arquivo de entrada
    Arquivo dadosArquivo = lerArquivo(input);
    // Aloca array para resultados
    Resultado* resultados = malloc(dadosArquivo.qtdDoencas * sizeof(Resultado));

    // Diagnóstico para cada doença
    if (dadosArquivo.qtdDoencas > 0)
    {
        // Para cada doença, calcula a probabilidade
        #ifdef _OPENMP
        #pragma omp parallel for
        #endif
        for (int i = 0; i < dadosArquivo.qtdDoencas; i++)
        {
            Doenca doencaAtual = dadosArquivo.doencas[i];
            // printf("\n==================================================\n");
            // printf("\tIniciando analise da Doenca: %s\n", doencaAtual.nomeDoenca);
            // printf("==================================================\n");
            int resultado = diagnosticarDoenca(
                dadosArquivo.dna,
                doencaAtual.genes,
                doencaAtual.qtdGenes,
                dadosArquivo.tamanhoSubGenes
            );
            strcpy(resultados[i].nomeDoenca, doencaAtual.nomeDoenca);
            resultados[i].probabilidade = resultado;
        }

        // Ordena resultados antes de escrever
        ordenarResultados(resultados, dadosArquivo.qtdDoencas);
        // Escreve resultados no arquivo de saída
        escreverResultados(output, resultados, dadosArquivo.qtdDoencas);
        
        free(resultados);
    }

    // Liberação de memória básica
    for(int i=0; i<dadosArquivo.qtdDoencas; i++)
        free(dadosArquivo.doencas[i].genes);
    free(dadosArquivo.doencas);
    // Fechamento dos arquivos
    fclose(input);
    fclose(output);

    return 0;
}