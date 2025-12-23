#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char nome[3];
    int custo;
} MetodoResultado;

typedef struct
{
    int *array;
    int size;
} Array;

typedef struct
{
    Array *arrays;
    int qtdArrays;
} SetArrays;

// Estrutura para estatísticas
typedef struct
{
    int trocas;
    int chamadas;
} Estatisticas;

/*
[6]:LP(15),HP(16),LM(19),HM(19),HA(20),LA(22)
[4]:LP(10),HP(10),LM(11),LA(11),HM(12),HA(12)
[7]:HP(17),LM(18),LP(23),HM(26),HA(27),LA(30)
[10]:LM(28),HP(28),LP(33),HA(35),HM(37),LA(38)

[1842]:HP(8369),HA(10267),HM(10661),LM(14028),LP(14405),LA(17752)
[1039]:HP(4487),HA(5615),HM(5807),LM(7148),LA(7967),LP(8170)
*/

// Função para ler dados do arquivo
SetArrays lerDados(FILE* arquivo)
{
    SetArrays resultado = {NULL, 0};
    int qtdArrays;
    
    if (fscanf(arquivo, "%d", &qtdArrays) != 1 || qtdArrays <= 0) {
        return resultado;
    }

    Array *arrays = malloc(sizeof(Array) * qtdArrays);
    if (!arrays) {
        return resultado;
    }

    for (int i = 0; i < qtdArrays; ++i) {
        if (fscanf(arquivo, "%d", &arrays[i].size) != 1 || arrays[i].size < 0) {
            arrays[i].array = NULL;
            continue;
        }
        
        if (arrays[i].size > 0) {
            arrays[i].array = malloc(sizeof(int) * arrays[i].size);
            if (!arrays[i].array) {
                for (int k = 0; k <= i; ++k) {
                    if (arrays[k].array) free(arrays[k].array);
                }
                free(arrays);
                return resultado;
            }
        } else {
            arrays[i].array = NULL;
        }

        for (int j = 0; j < arrays[i].size; ++j) {
            if (fscanf(arquivo, "%d", &arrays[i].array[j]) != 1) {
                arrays[i].array[j] = 0;
            }
        }
    }

    resultado.arrays = arrays;
    resultado.qtdArrays = qtdArrays;

    return resultado;
}

// Procedimento para trocar dois elementos com contagem
void swap(int *a, int *b, Estatisticas *stats)
{
    stats->trocas++;
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Função que retorna o índice da mediana de três
int mediana(int *array, int low, int high)
{
    // Calcula o índice do elemento do meio
    int mid = low + (high - low) / 2;
    // Obtém os valores dos três elementos
    int a = array[low], b = array[mid], c = array[high];
    // Retorna o índice do valor mediano
    if ((a <= b && b <= c) || (c <= b && b <= a)) return mid;
    if ((b <= a && a <= c) || (c <= a && a <= b)) return low;

    return high;
}

// Função QuickSort Lomuto Padrão (V)
int lomutoPadrao(int *array, int low, int high, Estatisticas *stats)
{
    // Usa o último elemento como pivô
    int pivo = array[high];
    int i = low - 1;

    // Percorre todos os elementos
    for (int j = low; j < high; j++)
        // Verifica se o elemento atual é menor ou igual ao pivô
        if (array[j] <= pivo)
        {
            // Incrementa o índice do menor elemento
            i++;
            // Troca array[i] e array[j]
            swap(&array[i], &array[j], stats);
        }

    // Coloca o pivô na posição correta
    swap(&array[i + 1], &array[high], stats);
    
    return i + 1; // Retorna o índice do pivô
}

// Função para Lomuto com mediana de três
int lomutoMediana(int *array, int low, int high, Estatisticas *stats)
{
    // Se necessário, usa o Lomuto padrão
    if (high - low < 2)
        return lomutoPadrao(array, low, high, stats);

    // Seleciona o pivô como a mediana de três
    int pivoIdx = mediana(array, low, high);

    // Se necessário, move o pivô para o final
    if (pivoIdx != high)
        swap(&array[high], &array[pivoIdx], stats);
    
    return lomutoPadrao(array, low, high, stats);
}

// Função para Lomuto com pivô aleatório (V)
int lomutoRandom(int *array, int low, int high, Estatisticas *stats)
{
    swap(&array[high], &array[low + abs(array[low]) % (high - low + 1)], stats);
    
    return lomutoPadrao(array, low, high, stats);
}

// Função para Hoare Padrão (V)
int hoarePadrao(int *array, int low, int high, Estatisticas *stats)
{
    // Escolhe o primeiro elemento como pivô
    int pivo = array[low];
    // Escolhe o índice inicial e final
    int i = low - 1;
    int j = high + 1;

    while (1)
    {
        // Encontra o elemento à esquerda que deve estar à direita
        do {
            i++;
        } while (array[i] < pivo);

        // Encontra o elemento à direita que deve estar à esquerda
        do {
            j--;
        } while (array[j] > pivo);

        // Se os índices se cruzarem, a partição está concluída
        if (i >= j)
            return j;

        swap(&array[i], &array[j], stats);
    }
}

// Função para Hoare com pivô mediana de três
int hoareMediana(int *array, int low, int high, Estatisticas *stats)
{
    // Se necessário, usa o Hoare padrão
    if (high - low < 2)
        return hoarePadrao(array, low, high, stats);
    
    // Seleciona o pivô como a mediana de três
    int pivoIdx = mediana(array, low, high);

    // Se necessário, move o pivô para o início
    if (pivoIdx != low)
        swap(&array[low], &array[pivoIdx], stats);
    
    return hoarePadrao(array, low, high, stats);
}

// Função para Hoare com pivô aleatório (V)
int hoareRandom(int *array, int low, int high, Estatisticas *stats)
{
    swap(&array[low], &array[low + abs(array[low]) % (high - low + 1)], stats);
    
    return hoarePadrao(array, low, high, stats);
}

// Procedimento Quick Sort
void quickSort(int *array, int low, int high, int method, Estatisticas *stats)
{
    stats->chamadas++;
    
    if(low < high)
    {
        int mid;
        switch(method)
        {
            case 1: mid = lomutoPadrao(array, low, high, stats); break;
            case 2: mid = lomutoMediana(array, low, high, stats); break;
            case 3: mid = lomutoRandom(array, low, high, stats); break;
            case 4: mid = hoarePadrao(array, low, high, stats); break;
            case 5: mid = hoareMediana(array, low, high, stats); break;
            case 6: mid = hoareRandom(array, low, high, stats); break;
            default: break;
        }

        if(method >= 4)
        {
            quickSort(array, low, mid, method, stats);
            quickSort(array, mid + 1, high, method, stats);
        } else
        {
            quickSort(array, low, mid - 1, method, stats);
            quickSort(array, mid + 1, high, method, stats);
        }
    }
}

// Procedimento para mesclar dois subarrays
void merge(MetodoResultado *v, int inicio, int meio, int fim)
{
    int i = inicio;
    int j = meio + 1;
    int k = 0;

    int tamanho = fim - inicio + 1;
    MetodoResultado *aux = malloc(tamanho * sizeof(MetodoResultado));

    while (i <= meio && j <= fim)
    {
        // Estável: mantém ordem em caso de empate
        if (v[i].custo <= v[j].custo)
            aux[k++] = v[i++];
        else
            aux[k++] = v[j++];
    }

    while (i <= meio)
        aux[k++] = v[i++];

    while (j <= fim)
        aux[k++] = v[j++];

    for (i = inicio, k = 0; i <= fim; i++, k++)
        v[i] = aux[k];

    free(aux);
}

// Merge Sort recursivo
void mergeSort(MetodoResultado *v, int inicio, int fim)
{
    if (inicio < fim)
    {
        int meio = inicio + (fim - inicio) / 2;

        mergeSort(v, inicio, meio);
        mergeSort(v, meio + 1, fim);
        merge(v, inicio, meio, fim);
    }
}

// Função para copiar um vetor
int* copiaVetor(int *original, int n)
{
    int *copia = malloc(sizeof(int) * n);
    if (!copia) return NULL;
    for (int i = 0; i < n; i++)
        copia[i] = original[i];
    return copia;
}

// Procedimento para liberar memória
void liberarSetArrays(SetArrays *set)
{
    if (set && set->arrays)
    {
        for (int i = 0; i < set->qtdArrays; ++i)
            if (set->arrays[i].array)
                free(set->arrays[i].array);
        free(set->arrays);
        set->arrays = NULL;
        set->qtdArrays = 0;
    }
}

int main(int argc, char *argv[])
{
    // Verifica argumentos
    if (argc != 3)
    {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Abre arquivos
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    if (!input || !output) {
        printf("Erro ao abrir arquivos.\n");
        return 1;
    }

    // Lê dados do arquivo de entrada
    SetArrays dadosLidos = lerDados(input);
    if (dadosLidos.qtdArrays == 0)
    {
        printf("Nenhum array válido encontrado no arquivo.\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    // Nomes dos métodos na ordem dos casos (1-6)
    char nomesMetodos[6][3] = {"LP", "LM", "LA", "HP", "HM", "HA"};
    
    // Processar cada array
    for (int i = 0; i < dadosLidos.qtdArrays; i++)
    {
        // Armazenar resultados dos métodos
        MetodoResultado resultados[6];
        
        // Testar cada método de ordenação
        for (int method = 0; method < 6; method++)
        {
            // Inicializa estatísticas
            Estatisticas stats = {0, 0};
            // Cria cópia do array original
            int *arrayTemp = copiaVetor(dadosLidos.arrays[i].array, dadosLidos.arrays[i].size);
            // Executa o Quick Sort com o método atual
            quickSort(arrayTemp, 0, dadosLidos.arrays[i].size - 1, method + 1, &stats);
            // Armazena resultados
            strcpy(resultados[method].nome, nomesMetodos[method]);
            // Armazena o número de trocas
            resultados[method].custo = stats.trocas + stats.chamadas;
            // Libera memória temporária
            free(arrayTemp);
        }
        
        // Ordenar de forma estável pelos resultados
        mergeSort(resultados, 0, 5);
        
        // Gerar output no formato especificado
        fprintf(output, "[%d]:", dadosLidos.arrays[i].size);
        for (int m = 0; m < 6; m++) {
            fprintf(output, "%s(%d)", resultados[m].nome, resultados[m].custo);
            if (m < 5) fprintf(output, ",");
        }
        fprintf(output, "\n");
        
        // Também imprimir no console para verificação
        printf("[%d]:", dadosLidos.arrays[i].size);
        for (int m = 0; m < 6; m++) {
            printf("%s(%d)", resultados[m].nome, resultados[m].custo);
            if (m < 5) printf(",");
        }
        printf("\n");
    }

    // Libera memória
    liberarSetArrays(&dadosLidos);
    // Fecha arquivos
    fclose(input);
    fclose(output);

    return 0;
}