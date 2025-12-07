#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define uma constante para o tamanho máximo das strings
#define MAX_STR 1002

typedef struct
{
    char nomeGene[MAX_STR];
    int qtdSubGenes;
} Gene;

typedef struct
{
    char nomeDoenca[9];
    Gene *genes;
    int qtdGenes;
} Doenca;

typedef struct
{
    char dna[MAX_STR * 10];
    Doenca* doencas;
    char** nomesGenes;
    int qtdDoencas, tamanhoSubGenes, totalGenes;
} Arquivo;

typedef struct
{
    char nomeDoenca[9];
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
    fscanf(arquivo, "%10002s", dadosArquivo.dna);
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
        fscanf(arquivo, "%9s %d", dadosArquivo.doencas[i].nomeDoenca, &dadosArquivo.doencas[i].qtdGenes);
        int qtdGenes = dadosArquivo.doencas[i].qtdGenes;
        dadosArquivo.doencas[i].genes = malloc(qtdGenes * sizeof(Gene));
        totalGenes += qtdGenes;
        // Lendo cada gene
        for (int j = 0; j < qtdGenes; ++j)
        {
            fscanf(arquivo, "%1001s", dadosArquivo.doencas[i].genes[j].nomeGene);
        }
    }

    dadosArquivo.totalGenes = totalGenes;
    dadosArquivo.nomesGenes = malloc(totalGenes * sizeof(char*));
    for (int i = 0; i < qtdDoencas; ++i)
    {
        for (int j = 0; j < dadosArquivo.doencas[i].qtdGenes; ++j)
        {
            // aloca string
            dadosArquivo.nomesGenes[idx] = malloc(1002);  
            // copia nome do gene
            strcpy(dadosArquivo.nomesGenes[idx], dadosArquivo.doencas[i].genes[j].nomeGene);
            // incrementa índice
            idx++;
        }
    }

    return dadosArquivo;
}

// Procedimento para computar o array LPS (Longest Prefix Suffix)
void computarLPS(const char* padrao, int M, int* lps)
{
    int len = 0; // comprimento do prefixo anterior
    if (M == 0) return;
    lps[0] = 0;     // lps[0] é sempre 0
    int i = 1;   // índice para percorrer o padrão

    while (i < M)
    {
        if (padrao[i] == padrao[len])
        {
            len++;
            lps[i] = (int)len;
            i++;
        }
        else
        {
            if (len != 0)
            {
                len = (int) lps[len - 1];
            }
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

// Função para buscar o maior prefixo do padrão no DNA usando KMP
int buscarKMP(const char* DNA, const char* padrao, const int L, int start_pos, int *pos_found)
{
    if (!DNA || !padrao || L <= 0) return 0;

    int N = (int)strlen(DNA);
    int M = (int)strlen(padrao);

    if (N == 0 || M == 0) return 0;
    if (start_pos < 0) start_pos = 0;
    if (start_pos >= N) return 0;
    if (L > M) return 0; // não é possível obter prefixo de tamanho L

    // limite superior para pos: basta ter espaço para ao menos L caracteres
    int max_start = N - L;
    if (max_start < start_pos) return 0;

    // printf("\nProcurando subGene \"%s\" a partir de pos %d\n", padrao, start_pos);

    for (int pos = start_pos; pos <= max_start; pos++) {
        int k = 0;
        // compara enquanto houver caracteres e enquanto bater
        while (pos + k < N && k < M && DNA[pos + k] == padrao[k]) {
            k++;
            // se k atingiu M podemos parar, já é o máximo possível aqui
            if (k == M) break;
        }
        if (k >= L) {
            if (pos_found) *pos_found = pos;
            // printf("MATCH encontrado! DNA[%d..%d], tam=%d\n", pos, pos + k - 1, k);
            return k;
        }
        // caso contrário, continue para próxima posição
    }

    // printf("Nenhum match encontrado para \"%s\".\n", padrao);
    // Nenhum match mínimo encontrado
    return 0;
}

// Função para calcular a compatibilidade entre o DNA e um gene
int calcularCompatibilidade(const char* DNA, char* gene, const int L)
{
    // printf("\n====================================================\n");
    // printf("Iniciando analise do gene: %s\n", gene);
    // printf("====================================================\n");
    if (!DNA || !gene || L <= 0) return 0;
    int tamanhoDNA = (int)strlen(DNA);
    int tamanhoGene = (int)strlen(gene);
    if (tamanhoDNA == 0 || tamanhoGene == 0) return 0;

    int matchs = 0;
    int dna_search_pos = 0; // onde começamos a procurar no DNA

    while (matchs < tamanhoGene)
    {
        char *subGene = gene + matchs;
        int rem = (int)strlen(subGene);
        if (rem < L) break; // não há mais subGene com tamanho mínimo

        int found_pos = -1;
        int found_k = buscarKMP(DNA, subGene, L, dna_search_pos, &found_pos);
        if (found_k <= 0) {
            // não encontrou segmento mínimo; encerra
            break;
        }

        // avançamos no gene pelos encontrados
        matchs += found_k;
        // avançamos a posição de busca no DNA para logo após o trecho usado
        dna_search_pos = found_pos + found_k;
        
        if (dna_search_pos >= tamanhoDNA) break; // não há mais DNA para procurar
    }

    int percentualGene = (int)ceil((double)(matchs * 100) / (double)tamanhoGene);

    return percentualGene;
}

// Função para diagnosticar a doença com base no DNA e seus genes
int diagnosticarDoenca(const char* DNA, Gene* genes, int num_genes, int L)
{
    if (!DNA || !genes || num_genes <= 0) return 0; // validação defensiva

    int genes_detectados = 0;

    for (int i = 0; i < num_genes; i++)
    {
        // supondo que Gene.nomeGene é uma C-string terminada em '\0'
        int compatibilidade = calcularCompatibilidade(DNA, genes[i].nomeGene, L);
        if (compatibilidade >= 90)
            genes_detectados++;
    }

    double resultado = ((double)genes_detectados / (double)num_genes) * 100.0;
    return (int) lround(resultado);
}

int main(int argc, char *argv[])
{
    // Verificando argumentos de linha de comando
    if (argc != 3)
    {
        printf("Uso: %s <input> <output>\n", argv[0]);
        return 1;
    }

    // Abrindo os arquivos
    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Erro ao abrir arquivo de input");
        return 1;
    }
    FILE* output = fopen(argv[2], "w");
    if (!output) {
        perror("Erro ao abrir arquivo de output");
        fclose(input);
        return 1;
    }

    // Lendo os dados do arquivo
    Arquivo dadosArquivo = lerArquivo(input);

    // Struct para armazenar os resultados dos diagnósticos
    Resultado* resultados = malloc(dadosArquivo.qtdDoencas * sizeof(Resultado));
    
    // Diagnóstico para cada doença
    for (int i = 0; i < dadosArquivo.qtdDoencas; i++)
    {
        // Diagnóstico da doença atual
        Doenca doencaAtual = dadosArquivo.doencas[i];
        int resultado = diagnosticarDoenca(dadosArquivo.dna, doencaAtual.genes, doencaAtual.qtdGenes, dadosArquivo.tamanhoSubGenes);
        // Armazenando resultado
        strcpy(resultados[i].nomeDoenca, doencaAtual.nomeDoenca);
        resultados[i].probabilidade = resultado;
    }

    // Ordenação estável dos resultados em ordem decrescente de probabilidade
    for (int i = 0; i < dadosArquivo.qtdDoencas - 1; i++)
    {
        for (int j = 0; j < dadosArquivo.qtdDoencas - i - 1; j++)
        {
            if (resultados[j].probabilidade < resultados[j + 1].probabilidade)
            {
                Resultado temp = resultados[j];
                resultados[j] = resultados[j + 1];
                resultados[j + 1] = temp;
            }
        }
    }

    // Escrevendo os resultados no arquivo de saída
    for (int i = 0; i < dadosArquivo.qtdDoencas; i++)
    {
        // Evitando quebra de linha desnecessária
        if (i == dadosArquivo.qtdDoencas - 1)
            fprintf(output, "%s->%d%%", resultados[i].nomeDoenca, resultados[i].probabilidade);
        else // 
            fprintf(output, "%s->%d%%\n", resultados[i].nomeDoenca, resultados[i].probabilidade);
    }

    // Fechando os arquivos
    fclose(input);
    fclose(output);

    return 0;
}
