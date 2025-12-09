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
        // Alocando memória para os genes da doença
        int qtdGenes = dadosArquivo.doencas[i].qtdGenes;
        dadosArquivo.doencas[i].genes = malloc(qtdGenes * sizeof(Gene));
        totalGenes += qtdGenes;
        // Lendo cada gene
        for (int j = 0; j < qtdGenes; ++j)
            fscanf(arquivo, "%1001s", dadosArquivo.doencas[i].genes[j].nomeGene);
    }

    // Armazenando nomes de todos os genes em um array separado
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
    int len = 0; // Comprimento do prefixo anterior
    if (M == 0) return; // Caso especial para padrão vazio
    lps[0] = 0; // lps[0] é sempre 0
    int i = 1; // Índice para percorrer o padrão

    // Loop para preencher o array lps
    while (i < M)
    {
        // Se os caracteres combinam
        if (padrao[i] == padrao[len])
        {
            len++; // Incrementa o comprimento do prefixo
            lps[i] = (int)len; // Define lps[i]
            i++; // Move para o próximo caractere
        } else // Se os caracteres não combinam
        {
            // Verifica se há um prefixo anterior para comparar
            if (len != 0)
                len = (int) lps[len - 1];
            else // Se não houver prefixo anterior
            {
                lps[i] = 0; // Define lps[i] como 0
                i++; // Move para o próximo caractere
            }
        }
    }
}

// Função para buscar o maior prefixo do padrão no DNA usando KMP
int buscarKMP(const char* DNA, const char* padrao, const int L, int posInicial, int *posEncontrada)
{
    int N = (int)strlen(DNA); // Tamanho do DNA
    int M = (int)strlen(padrao); // Tamanho do padrão
    if (N == 0 || M == 0) return 0;
    if (posInicial < 0) posInicial = 0; // Ajusta posição inicial negativa
    if (posInicial >= N) return 0; // Posição inicial inválida
    if (L > M) return 0; // Não é possível obter prefixo de tamanho L

    // Limite superior para pos: basta ter espaço para ao menos L caracteres
    int maxInicio = N - L; // Última posição válida para início de busca
    if (maxInicio < posInicial) return 0; // Não há espaço para busca

    // printf("\nProcurando subGene \"%s\" a partir de pos %d\n", padrao, posInicial);

    // Percorre o DNA a partir da posição inicial até o limite máximo
    for (int pos = posInicial; pos <= maxInicio; pos++)
    {
        int k = 0;
        // Compara enquanto houver caracteres e enquanto combinar
        while (pos + k < N && k < M && DNA[pos + k] == padrao[k])
        {
            k++;
            // Se k atingiu M podemos parar, já é o máximo possível aqui
            if (k == M) break;
        }

        // Se encontramos um prefixo de tamanho mínimo L
        if (k >= L)
        {
            // Verifica se o ponteiro de posição foi fornecido
            if (posEncontrada)
                *posEncontrada = pos;
            // printf("MATCH encontrado! DNA[%d..%d], tam=%d\n", pos, pos + k - 1, k);
            return k;
        }
    }

    // printf("Nenhum match encontrado para \"%s\".\n", padrao);
    return 0;
}

// Função para calcular a compatibilidade entre o DNA e um gene
int calcularCompatibilidade(const char* DNA, char* gene, const int L)
{
    // printf("\n====================================================\n");
    // printf("Iniciando analise do gene: %s\n", gene);
    // printf("====================================================\n");
    int tamanhoDNA = (int)strlen(DNA); // Tamanho do DNA
    int tamanhoGene = (int)strlen(gene); // Tamanho do gene
    if (tamanhoDNA == 0 || tamanhoGene == 0) return 0;

    int matchs = 0; // Quantidade de caracteres combinados
    int dnaBuscaPos = 0; // onde começamos a procurar no DNA

    while (matchs < tamanhoGene)
    {
        char *subGene = gene + matchs; // Ponteiro para o subGene atual
        int restante = (int)strlen(subGene); // Caracteres restantes no gene
        if (restante < L) break; // não há mais subGene com tamanho mínimo

        int posEncontrada = -1;
        // Busca o próximo prefixo do subGene no DNA
        int matchsEncontrados = buscarKMP(DNA, subGene, L, dnaBuscaPos, &posEncontrada);
        // Se não encontrou mais correspondências, sai do loop
        if (matchsEncontrados <= 0) break;
        // Avançamos no gene pelos encontrados
        matchs += matchsEncontrados;
        // Avançamos a posição de busca no DNA para logo após o trecho usado
        dnaBuscaPos = posEncontrada + matchsEncontrados;
        // Se a posição de busca ultrapassar o tamanho do DNA, encerramos
        if (dnaBuscaPos >= tamanhoDNA) break;
    }

    // Calcula o percentual de compatibilidade
    int percentualGene = (int)ceil((double)(matchs * 100) / (double)tamanhoGene);

    return percentualGene;
}

// Função para diagnosticar a doença com base no DNA e seus genes
int diagnosticarDoenca(const char* DNA, Gene* genes, int numGenes, int L)
{
    if (!DNA || !genes || numGenes <= 0 || L <= 0) return 0;
    int genes_detectados = 0; // Contador de genes detectados

    // Verifica cada gene da doença
    for (int i = 0; i < numGenes; i++)
    {
        int compatibilidade = calcularCompatibilidade(DNA, genes[i].nomeGene, L);
        if (compatibilidade >= 90) genes_detectados++;
    }

    // Calcula a probabilidade final da doença
    double resultado = ((double)genes_detectados / (double)numGenes) * 100.0;
    return (int)lround(resultado);
}

// Função para ordenar os resultados em ordem decrescente de probabilidade
void ordenarResultados(Resultado* resultados, int qtdResultados)
{
    // Ordenação estável usando Bubble Sort xD
    for (int i = 0; i < qtdResultados - 1; i++)
    {
        for (int j = 0; j < qtdResultados - i - 1; j++)
        {
            if (resultados[j].probabilidade < resultados[j + 1].probabilidade)
            {
                // Troca os resultados
                Resultado temp = resultados[j];
                resultados[j] = resultados[j + 1];
                resultados[j + 1] = temp;
            }
        }
    }
}

// Função para escrever os resultados no arquivo de saída
void escreverResultados(FILE* output, Resultado* resultados, int qtdResultados)
{
    for (int i = 0; i < qtdResultados; i++)
    {
        if (i == qtdResultados - 1)
            fprintf(output, "%s->%d%%", resultados[i].nomeDoenca, resultados[i].probabilidade);
        else
            fprintf(output, "%s->%d%%\n", resultados[i].nomeDoenca, resultados[i].probabilidade);
    }
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
        Doenca doencaAtual = dadosArquivo.doencas[i];
        int resultado = diagnosticarDoenca(dadosArquivo.dna, doencaAtual.genes, doencaAtual.qtdGenes, dadosArquivo.tamanhoSubGenes);
        // Armazenando resultado
        strcpy(resultados[i].nomeDoenca, doencaAtual.nomeDoenca);
        resultados[i].probabilidade = resultado;
    }

    // Ordenação estável dos resultados em ordem decrescente de probabilidade
    ordenarResultados(resultados, dadosArquivo.qtdDoencas);

    // Escrevendo os resultados no arquivo de saída
    escreverResultados(output, resultados, dadosArquivo.qtdDoencas);

    // Fechando os arquivos
    fclose(input);
    fclose(output);

    return 0;
}
