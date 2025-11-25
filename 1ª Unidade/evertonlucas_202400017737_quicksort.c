#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct
{
    char nome[4];
    int trocas;
    int id;
} ResultadoTeste;

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

void swap(int *a, int *b, int *trades)
{
    int temp = *a;
    *a = *b;
    *b = temp;
    (*trades)++;
}

int pseudoRandom(int *array, int low, int high)
{
    if (high < low) return low;
    return low + abs(array[low]) % (high - low);
}

int mediana(int *array, int low, int mid, int high)
{
    int a = array[low];
    int b = array[mid];
    int c = array[high];

    if ((a <= b && b <= c) || (c <= b && b <= a))
        return mid;
    else if ((b <= a && a <= c) || (c <= a && a <= b))
        return low;
    else
        return high;
}

int lomuto(int *array, int low, int high, int pivotValue, int *trades)
{
    int begin = low;
    for (int i = low; i < high; i++) {
        if (array[i] <= pivotValue) {
            swap(&array[begin], &array[i], trades);
            begin++;
        }
    }
    swap(&array[begin], &array[high], trades);
    return begin;
}

int lomutoPadrao(int *array, int low, int high, int *trades)
{
    return lomuto(array, low, high, array[high], trades);
}

int lomutoMediana(int *array, int low, int high, int *trades)
{
    if (low >= high) return low;
    int mid = low + (high - low) / 2;
    int pivotIndex = mediana(array, low, mid, high);
    swap(&array[pivotIndex], &array[high], trades);
    return lomuto(array, low, high, array[high], trades);
}

int lomutoRandom(int *array, int low, int high, int *trades)
{
    int pivot = pseudoRandom(array, low, high);
    swap(&array[pivot], &array[high], trades);
    return lomuto(array, low, high, array[high], trades);
}

int hoare(int *array, int low, int high, int pivot, int *trades)
{
    int i = low - 1;
    int j = high + 1;
    while (1) {
        do { i++; } while (array[i] < pivot);
        do { j--; } while (array[j] > pivot);
        if (i >= j) return j;
        swap(&array[i], &array[j], trades);
    }
}

int hoarePadrao(int *array, int low, int high, int *trades)
{
    return hoare(array, low, high, array[low], trades);
}

int hoareMediana(int *array, int low, int high, int *trades)
{
    if (low >= high) return low;
    int mid = low + (high - low) / 2;
    int pivotIndex = mediana(array, low, mid, high);
    (*trades)++;
    return hoare(array, low, high, array[pivotIndex], trades);
}

int hoareRandom(int *array, int low, int high, int *trades)
{
    int pivotIndex = pseudoRandom(array, low, high);
    return hoare(array, low, high, array[pivotIndex], trades);
}

void quickSort(int *array, int low, int high, int method, int *trades)
{
    (*trades)++;
    if (low < high) {
        int mid;
        switch (method) {
            case 1: mid = lomutoPadrao(array, low, high, trades); break;
            case 2: mid = lomutoMediana(array, low, high, trades); break;
            case 3: mid = lomutoRandom(array, low, high, trades); break;
            case 4: mid = hoarePadrao(array, low, high, trades); break;
            case 5: mid = hoareMediana(array, low, high, trades); break;
            case 6: mid = hoareRandom(array, low, high, trades); break;
            default: mid = lomutoPadrao(array, low, high, trades);
        }

        if (method >= 4) { // Hoare partition retorna j
            quickSort(array, low, mid, method, trades);
            quickSort(array, mid + 1, high, method, trades);
        } else { // Lomuto
            quickSort(array, low, mid - 1, method, trades);
            quickSort(array, mid + 1, high, method, trades);
        }
    }
}

int rodarTeste(int *dadosOriginais, int *arrayCopia, int tamanho, int method) {
    int trades = 0;
    for (int i = 0; i < tamanho; i++) arrayCopia[i] = dadosOriginais[i];
    if (tamanho > 1) quickSort(arrayCopia, 0, tamanho - 1, method, &trades);
    return trades;
}

SetArrays lerDados(FILE* arquivo)
{
    SetArrays resultado = {NULL, 0};
    int qtdArrays;
    fscanf(arquivo, "%d", &qtdArrays);
    if (qtdArrays <= 0) return resultado;

    Array *arrays = malloc(sizeof(Array) * qtdArrays);
    if (!arrays)
    {
        fprintf(stderr, "Erro de alocação.\n");
        return resultado;
    }

    // Lendo os arrays
    for (int i = 0; i < qtdArrays; ++i) {
        // Lendo o tamanho do array
        fscanf(arquivo, "%d", &arrays[i].size);
        // Alocando memória para o array
        if (arrays[i].size > 0)
        {
            arrays[i].array = malloc(sizeof(int) * arrays[i].size);
            if (!arrays[i].array) {
                fprintf(stderr, "Erro de alocação.\n");
                for (int k = 0; k < i; ++k) if (arrays[k].array) free(arrays[k].array);
                free(arrays);
                return resultado;
            }
        } else {
            arrays[i].array = NULL;
        }

        // Lendo os elementos do array
        for (int j = 0; j < arrays[i].size; ++j)
        {
            fscanf(arquivo, "%d", &arrays[i].array[j]);
        }
    }

    resultado.arrays = arrays;
    resultado.qtdArrays = qtdArrays;
    return resultado;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Abrindo arquivos
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");

    SetArrays dadosLidos = lerDados(input);

    ResultadoTeste resultados[6] = {
        {"LP", 0, 1},
        {"LM", 0, 2},
        {"LA", 0, 3},
        {"HP", 0, 4},
        {"HM", 0, 5},
        {"HA", 0, 6}
    };
    
    for (int i = 0; i < dadosLidos.qtdArrays; ++i) {
        int tamanho = dadosLidos.arrays[i].size;
        int *arrayCopia = malloc(sizeof(int) * tamanho);
        if (!arrayCopia) {
            fprintf(stderr, "Erro de alocação.\n");
            break;
        }

        for (int j = 0; j < 6; ++j) {
            resultados[j].trocas = rodarTeste(dadosLidos.arrays[i].array, arrayCopia, tamanho, resultados[j].id);
        }

        fprintf(output, "Array %d:\n", i + 1);
        for (int j = 0; j < 6; ++j) {
            fprintf(output, "%s: %d trocas\n", resultados[j].nome, resultados[j].trocas);
        }
        fprintf(output, "\n");

        free(arrayCopia);
    }

    // Fechando arquivos
    fclose(input);
    fclose(output);

    // Liberando memória alocada
    free(dadosLidos.arrays);

    // Finalizando o programa
    return 0;
}