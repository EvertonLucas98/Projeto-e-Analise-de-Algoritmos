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

// Função para ler dados do arquivo
SetArrays lerDados(FILE* arquivo)
{
    // Inicializa resultado
    SetArrays resultado = {NULL, 0};
    int qtdArrays;
    // Lê a quantidade de arrays
    if (fscanf(arquivo, "%d", &qtdArrays) != 1 || qtdArrays <= 0)
        return resultado;

    // Aloca memória para os arrays
    Array *arrays = malloc(sizeof(Array) * qtdArrays);
    if (!arrays)
        return resultado;

    // Lê cada array
    for (int i = 0; i < qtdArrays; ++i)
    {
        // Lê o tamanho do array
        if (fscanf(arquivo, "%d", &arrays[i].size) != 1 || arrays[i].size < 0)
        {
            arrays[i].array = NULL;
            continue;
        }
        
        // Aloca memória para o array
        if (arrays[i].size > 0)
        {
            arrays[i].array = malloc(sizeof(int) * arrays[i].size);
            if (!arrays[i].array) {
                for (int k = 0; k <= i; ++k) {
                    if (arrays[k].array) free(arrays[k].array);
                }
                free(arrays);
                return resultado;
            }
        } else
        {
            arrays[i].array = NULL;
        }

        // Lê os elementos do array
        for (int j = 0; j < arrays[i].size; ++j)
            if (fscanf(arquivo, "%d", &arrays[i].array[j]) != 1)
                arrays[i].array[j] = 0;
    }

    resultado.arrays = arrays;
    resultado.qtdArrays = qtdArrays;

    return resultado;
}

// Procedimento para trocar dois elementos com contagem
void swap(int *a, int *b, Estatisticas *stats)
{
    // Incrementa o contador de trocas
    stats->trocas++;
    // Troca os valores
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Função que retorna o índice da mediana de três
int mediana(int *array, int low, int mid, int high)
{
    // Obtém os valores dos três elementos
    int a = array[low], b = array[mid], c = array[high];

    // Compara para encontrar a mediana
    if ((a <= b && b <= c) || (c <= b && b <= a))
        return mid;
    else if ((b <= a && a <= c) || (c <= a && a <= b))
        return low;
    else
        return high;
}

// Função QuickSort Lomuto Padrão
int lomutoPadrao(int *array, int low, int high, Estatisticas *stats)
{
    // Usa o último elemento como pivô
    int pivo = array[high];
    // Índice do menor elemento
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
    // Calcula o tamanho do subarray
    int n = high - low + 1;
    // Calcula índices para mediana de três
    int idx1 = low + (n/4);
    int idx2 = low + (n/2);
    int idx3 = low + (3*n/4);
    // Seleciona o pivô como a mediana de três
    int pivoIdx = mediana(array, idx1, idx2, idx3);

    // Move o pivô para o final
    swap(&array[high], &array[pivoIdx], stats);
    
    return lomutoPadrao(array, low, high, stats);
}

// Função para Lomuto com pivô aleatório
int lomutoRandom(int *array, int low, int high, Estatisticas *stats)
{
    // Escolhe um pivô aleatório e o move para o final
    swap(&array[high], &array[low + abs(array[low]) % (high - low + 1)], stats);
    
    return lomutoPadrao(array, low, high, stats);
}

// Função para Hoare Padrão
int hoarePadrao(int *array, int low, int high, Estatisticas *stats)
{
    // Escolhe o primeiro elemento como pivô
    int pivo = array[low];
    // Escolhe o índice inicial e final
    int i = low - 1;
    int j = high + 1;

    // Loop infinito até os índices se cruzarem
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

        // Troca os elementos fora de ordem
        swap(&array[i], &array[j], stats);
    }
}

// Função para Hoare com pivô mediana de três
int hoareMediana(int *array, int low, int high, Estatisticas *stats)
{
    // Calcula o tamanho do subarray
    int n = high - low + 1;
    // Calcula índices para mediana de três
    int idx1 = low + (n/4);
    int idx2 = low + (n/2);
    int idx3 = low + (3*n/4);
    // Seleciona o pivô como a mediana de três
    int pivoIdx = mediana(array, idx1, idx2, idx3);

    // Move o pivô para o início
    swap(&array[low], &array[pivoIdx], stats);
    
    return hoarePadrao(array, low, high, stats);
}

// Função para Hoare com pivô aleatório
int hoareRandom(int *array, int low, int high, Estatisticas *stats)
{
    // Escolhe um pivô aleatório e o move para o início
    swap(&array[low], &array[low + abs(array[low]) % (high - low + 1)], stats);
    
    return hoarePadrao(array, low, high, stats);
}

// Procedimento Quick Sort
void quickSort(int *array, int low, int high, int method, Estatisticas *stats)
{
    // Incrementa o contador de chamadas
    stats->chamadas++;
    
    // Verifica se o subarray tem mais de um elemento
    if(low < high)
    {
        // Índice do pivô após partição
        int mid;

        // Seleciona o método de partição com base no parâmetro
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

        // Hoare
        if(method >= 4)
        {
            quickSort(array, low, mid, method, stats);
            quickSort(array, mid + 1, high, method, stats);
        } else // Lomuto
        {
            quickSort(array, low, mid - 1, method, stats);
            quickSort(array, mid + 1, high, method, stats);
        }
    }
}

// Ordenação estável usando Insertion Sort
void insertionSort(MetodoResultado *arr, int n)
{
    // Percorre cada elemento do array
    for (int i = 1; i < n; i++)
    {
        // Armazena o elemento atual
        MetodoResultado key = arr[i];
        // Índice do elemento anterior
        int j = i - 1;

        // Move os elementos maiores que key para uma posição à frente
        while (j >= 0 && arr[j].custo > key.custo)
        {
            // Move o elemento para a direita
            arr[j + 1] = arr[j];
            // Decrementa o índice
            j--;
        }

        // Insere o elemento na posição correta
        arr[j + 1] = key;
    }
}

// Procedimento para escrever resultados no arquivo
void escreverResultados(FILE* arquivo, MetodoResultado *resultados, int qtdResultados)
{
    // Escreve os resultados no arquivo
    fprintf(arquivo, "[%d]:", qtdResultados);
    for (int m = 0; m < 6; m++)
    {
        fprintf(arquivo, "%s(%d)", resultados[m].nome, resultados[m].custo);
        if (m < 5) fprintf(arquivo, ",");
    }
}

// Procedimento para processar os dados lidos
void processarDados(FILE* output, SetArrays dadosLidos, char nomesMetodos[6][3])
{
    // Tamanho máximo do array para alocação do buffer
    int maxSize = 0;
    // Determina o tamanho máximo entre os arrays
    for (int i = 0; i < dadosLidos.qtdArrays; i++)
        if (dadosLidos.arrays[i].size > maxSize)
            maxSize = dadosLidos.arrays[i].size;
    
    // Aloca buffer para ordenação
    int *buffer = malloc(sizeof(int) * maxSize);
    if (!buffer)
    {
        fprintf(stderr, "Erro ao alocar buffer\n");
        return;
    }

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
            // Copia o array original para o buffer
            memcpy(buffer, dadosLidos.arrays[i].array, sizeof(int) * dadosLidos.arrays[i].size);
            // Executa o QuickSort com o método atual
            quickSort(buffer, 0, dadosLidos.arrays[i].size - 1, method + 1, &stats);
            // Armazena o nome do método
            strcpy(resultados[method].nome, nomesMetodos[method]);
            // Armazena o número de trocas
            resultados[method].custo = stats.trocas + stats.chamadas;
        }
        
        // Ordenar de forma estável pelos resultados
        insertionSort(resultados, 6);
        // Gerar output no formato especificado
        escreverResultados(output, resultados, dadosLidos.arrays[i].size);
        // Nova linha entre os arrays, exceto após o último
        fprintf(output, "\n");
    }

    free(buffer);
}

// Procedimento para liberar memória
void liberarSetArrays(SetArrays *set)
{
    // Verifica se o set é válido
    if (set && set->arrays)
    {
        // Libera cada array individualmente
        for (int i = 0; i < set->qtdArrays; ++i)
            if (set->arrays[i].array)
                free(set->arrays[i].array);
        // Libera o array de arrays
        free(set->arrays);
        // Zera os campos
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
    processarDados(output, dadosLidos, nomesMetodos);
    // Libera memória
    liberarSetArrays(&dadosLidos);
    // Fecha arquivos
    fclose(input);
    fclose(output);

    return 0;
}