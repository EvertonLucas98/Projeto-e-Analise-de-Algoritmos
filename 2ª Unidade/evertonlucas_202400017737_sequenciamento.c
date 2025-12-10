#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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
    int totalGenes;
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
    int qtdDoencas=0, totalGenes=0, idx=0;
    
    // Lendo tamanho dos subgenes
    fscanf(arquivo, "%d", &dadosArquivo.tamanhoSubGenes);
    // Lendo DNA com limite para evitar overflow
    fscanf(arquivo, "%s", dadosArquivo.dna);
    // Lendo quantidade de doenças
    fscanf(arquivo, "%d", &qtdDoencas);
    // Armazenando a quantidade de doenças
    dadosArquivo.qtdDoencas = qtdDoencas;
    // Alocando memória para o array de doenças
    dadosArquivo.doencas = malloc(qtdDoencas * sizeof(Doenca));

    // Lendo cada doença e seus respectivos genes
    for (int i = 0; i < qtdDoencas; ++i)
    {
        // Lendo nome da doença e quantidade de genes
        fscanf(arquivo, "%s %d", dadosArquivo.doencas[i].nomeDoenca, &dadosArquivo.doencas[i].qtdGenes);
        // Alocando memória para os genes da doença
        int qtdGenes = dadosArquivo.doencas[i].qtdGenes;
        dadosArquivo.doencas[i].genes = malloc(qtdGenes * sizeof(Gene));
        totalGenes += qtdGenes;
        // Lendo cada gene
        for (int j = 0; j < qtdGenes; ++j)
            fscanf(arquivo, "%s", dadosArquivo.doencas[i].genes[j].nomeGene);
    }

    // Armazenando nomes de todos os genes em um array separado
    dadosArquivo.totalGenes = totalGenes;
    dadosArquivo.nomesGenes = malloc(totalGenes * sizeof(char*));
    for (int i = 0; i < qtdDoencas; i++)
    {
        for (int j = 0; j < dadosArquivo.doencas[i].qtdGenes; j++)
        {
            // aloca string
            dadosArquivo.nomesGenes[idx] = malloc((strlen(dadosArquivo.doencas[i].genes[j].nomeGene) + 1) * sizeof(char));  
            // copia nome do gene
            strcpy(dadosArquivo.nomesGenes[idx], dadosArquivo.doencas[i].genes[j].nomeGene);
            // incrementa índice
            idx++;
        }
    }

    return dadosArquivo;
}

// Computa array de prefixos (LPS) para o KMP
void computarLPS(const char* padrao, int M, int* lps)
{
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < M) {
        if (padrao[i] == padrao[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

// Função KMP para buscar o maior prefixo do padrão no DNA
int buscarKMP(const char* dna, const char* padrao, int L, int posInicial, int *posEncontrada)
{
    int N = strlen(dna);
    int M = strlen(padrao);
    
    if (M == 0) return 0;
    
    // Aloca vetor LPS dinamicamente para o padrão atual
    int *lps = (int*)malloc(sizeof(int) * M);
    computarLPS(padrao, M, lps);

    int i = posInicial; // index for text
    int j = 0; // index for pattern

    while (i < N) {
        if (padrao[j] == dna[i]) {
            j++;
            i++;
        }

        // Se encontramos um prefixo do tamanho total do padrão
        if (j == M) {
            // Match completo encontrado
            if (posEncontrada) *posEncontrada = i - j;
            free(lps);
            return M; // Retorna M pois é >= L com certeza
        }
        // Se houve mismatch após j matches
        else if (i < N && padrao[j] != dna[i]) {
            // Verificar se o que combinamos até agora (j) é suficiente
            // O algoritmo original parava na primeira ocorrencia >= L
            // Aqui, antes de resetar o 'j', verificamos se já atingimos L
            if (j >= L) {
                if (posEncontrada) *posEncontrada = i - j;
                free(lps);
                return j;
            }

            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }

    // Verifica se o que foi combinado no final é suficiente
    if (j >= L) {
        if (posEncontrada) *posEncontrada = i - j;
        free(lps);
        return j;
    }

    free(lps);
    return 0;
}

// Função para calcular a compatibilidade entre o DNA e um gene
int calcularCompatibilidade(const char* DNA, char* gene, const int L)
{
    int tamanhoDNA = strlen(DNA);
    int tamanhoGene = strlen(gene);
    if (tamanhoDNA == 0 || tamanhoGene == 0) return 0;

    int matchs = 0;
    int dnaBuscaPos = 0;

    while (matchs < tamanhoGene)
    {
        char *subGene = gene + matchs;
        // Se o que sobrou do gene for menor que L, não tem como dar match válido
        if ((int)strlen(subGene) < L) break; 

        int posEncontrada = -1;
        // Busca eficiente
        int matchsEncontrados = buscarKMP(DNA, subGene, L, dnaBuscaPos, &posEncontrada);

        if (matchsEncontrados <= 0) break;

        matchs += matchsEncontrados;
        // Avança no DNA para depois do match encontrado
        dnaBuscaPos = posEncontrada + matchsEncontrados;
        
        if (dnaBuscaPos >= tamanhoDNA) break;
    }
    
    // Calcula o percentual de compatibilidade
    double percentual = ((double)matchs * 100.0) / (double)tamanhoGene;

    return (int)(percentual + (percentual < 0.0 ? -0.5 : 0.5));
}

// Função para diagnosticar a doença com base no DNA e seus genes
int diagnosticarDoenca(const char* DNA, Gene* genes, int numGenes, int L)
{
    if (!DNA || !genes || numGenes <= 0 || L <= 0) return 0;
    int genes_detectados = 0;

    for (int i = 0; i < numGenes; i++)
    {
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

// Função para escrever os resultados no arquivo de saída
void escreverResultados(FILE* output, Resultado* resultados, int qtdResultados)
{
    for (int i = 0; i < qtdResultados; i++)
    {
        fprintf(output, "%s->%d%%", resultados[i].nomeDoenca, resultados[i].probabilidade);
        if (i < qtdResultados - 1) fprintf(output, "\n");
    }
}

int main(int argc, char *argv[])
{
    clock_t start = clock();
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
        for (int i = 0; i < dadosArquivo.qtdDoencas; i++)
        {
            Doenca doencaAtual = dadosArquivo.doencas[i];
            int resultado = diagnosticarDoenca(dadosArquivo.dna, doencaAtual.genes, doencaAtual.qtdGenes, dadosArquivo.tamanhoSubGenes);
            strcpy(resultados[i].nomeDoenca, doencaAtual.nomeDoenca);
            resultados[i].probabilidade = resultado;
        }

        // Ordena resultados antes de escrever
        ordenarResultados(resultados, dadosArquivo.qtdDoencas);
        // Escreve resultados no arquivo de saída
        escreverResultados(output, resultados, dadosArquivo.qtdDoencas);
        // Libera memória dos resultados
        free(resultados);
    }

    clock_t end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %f segundos\n", cpu_time_used);
    
    fclose(input);
    fclose(output);

    return 0;
}