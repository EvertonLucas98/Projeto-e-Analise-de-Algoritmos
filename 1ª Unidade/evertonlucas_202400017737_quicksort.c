#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct {
    char nome[3];
    int trocas;
} MetodoResultado;

typedef struct {
    int *array;
    int size;
} Array;

typedef struct {
    Array *arrays;
    int qtdArrays;
} SetArrays;

// Estrutura para estatísticas
typedef struct {
    long long comparacoes;
    long long trocas;
    long long chamadas_recursivas;
} Estatisticas;

// Procedimento para trocar dois elementos com contagem
void swap(int *a, int *b, Estatisticas *stats) {
    int temp = *a;
    *a = *b;
    *b = temp;
    if (stats) stats->trocas++;
}

// Função correta para gerar índice aleatório
int pseudoRandom(int low, int high) {
    if (low >= high) return low;
    return low + rand() % (high - low + 1);
}

// Função mediana
int mediana(int *array, int low, int mid, int high) {
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

// Função de partição Lomuto
int lomuto(int *array, int low, int high, int pivotValue, Estatisticas *stats) {
    int begin = low;
    
    // Encontra o índice do valor do pivô
    int pivotIndex = high;
    for (int i = low; i <= high; i++) {
        if (stats) stats->comparacoes++;
        if (array[i] == pivotValue) {
            pivotIndex = i;
            break;
        }
    }
    
    // Move o pivô para o final se necessário
    if (pivotIndex != high) {
        swap(&array[pivotIndex], &array[high], stats);
    }
    
    // Particiona o array
    for(int i = low; i < high; i++) {
        if (stats) stats->comparacoes++;
        if(array[i] <= pivotValue) {
            if (begin != i) {
                swap(&array[begin], &array[i], stats);
            }
            begin++;
        }
    }
    
    // Coloca o pivô na posição correta
    if (begin != high) {
        swap(&array[begin], &array[high], stats);
    }
    
    return begin;
}

int lomutoPadrao(int *array, int low, int high, Estatisticas *stats) {
    return lomuto(array, low, high, array[high], stats);
}

int lomutoMediana(int *array, int low, int high, Estatisticas *stats) {
    if (low >= high) return low;
    int mid = low + (high - low) / 2;
    int pivotIndex = mediana(array, low, mid, high);
    swap(&array[pivotIndex], &array[high], stats);
    return lomuto(array, low, high, array[high], stats);
}

int lomutoRandom(int *array, int low, int high, Estatisticas *stats) {
    int pivotIndex = pseudoRandom(low, high);
    swap(&array[pivotIndex], &array[high], stats);
    return lomuto(array, low, high, array[high], stats);
}

int hoare(int *array, int low, int high, int pivot, Estatisticas *stats) {
    int i = low - 1;
    int j = high + 1;
    
    while (1) {
        do { 
            i++; 
            if (stats) stats->comparacoes++;
        } while (array[i] < pivot);
        
        do { 
            j--; 
            if (stats) stats->comparacoes++;
        } while (array[j] > pivot);
        
        if (i >= j) return j;
        
        swap(&array[i], &array[j], stats);
    }
}

int hoarePadrao(int *array, int low, int high, Estatisticas *stats) {
    return hoare(array, low, high, array[low], stats);
}

int hoareMediana(int *array, int low, int high, Estatisticas *stats) {
    if (low >= high) return low;
    int mid = low + (high - low) / 2;
    int pivotIndex = mediana(array, low, mid, high);
    return hoare(array, low, high, array[pivotIndex], stats);
}

int hoareRandom(int *array, int low, int high, Estatisticas *stats) {
    int pivotIndex = pseudoRandom(low, high);
    return hoare(array, low, high, array[pivotIndex], stats);
}

// Procedimento Quick Sort
void quickSort(int *array, int low, int high, int method, Estatisticas *stats) {
    if (stats) stats->chamadas_recursivas++;
    
    if(low < high) {
        if (stats) stats->comparacoes++;
        
        int mid;
        switch(method) {
            case 1: mid = lomutoPadrao(array, low, high, stats); break;
            case 2: mid = lomutoMediana(array, low, high, stats); break;
            case 3: mid = lomutoRandom(array, low, high, stats); break;
            case 4: mid = hoarePadrao(array, low, high, stats); break;
            case 5: mid = hoareMediana(array, low, high, stats); break;
            case 6: mid = hoareRandom(array, low, high, stats); break;
            default: mid = lomutoPadrao(array, low, high, stats);
        }

        if(method >= 4) {
            quickSort(array, low, mid, method, stats);
            quickSort(array, mid + 1, high, method, stats);
        } else {
            quickSort(array, low, mid - 1, method, stats);
            quickSort(array, mid + 1, high, method, stats);
        }
    }
}

// Função para comparar métodos para ordenação
int compararMetodos(const void *a, const void *b) {
    const MetodoResultado *ma = (const MetodoResultado *)a;
    const MetodoResultado *mb = (const MetodoResultado *)b;
    return ma->trocas - mb->trocas;
}

SetArrays lerDados(FILE* arquivo) {
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

void liberarSetArrays(SetArrays *set) {
    if (set && set->arrays) {
        for (int i = 0; i < set->qtdArrays; ++i) {
            if (set->arrays[i].array) {
                free(set->arrays[i].array);
            }
        }
        free(set->arrays);
        set->arrays = NULL;
        set->qtdArrays = 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    
    if (!input || !output) {
        printf("Erro ao abrir arquivos.\n");
        return 1;
    }

    SetArrays dadosLidos = lerDados(input);
    
    if (dadosLidos.qtdArrays == 0) {
        printf("Nenhum array válido encontrado no arquivo.\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    // Nomes dos métodos na ordem dos casos (1-6)
    char nomesMetodos[6][3] = {"LP", "LM", "LA", "HP", "HM", "HA"};
    
    for (int i = 0; i < dadosLidos.qtdArrays; i++) {
        MetodoResultado resultados[6];
        
        for (int method = 0; method < 6; method++) {
            Estatisticas stats = {0, 0, 0};
            
            int* arrayTemp = malloc(sizeof(int) * dadosLidos.arrays[i].size);
            for (int j = 0; j < dadosLidos.arrays[i].size; ++j) {
                arrayTemp[j] = dadosLidos.arrays[i].array[j];
            }
            
            quickSort(arrayTemp, 0, dadosLidos.arrays[i].size - 1, method + 1, &stats);
            
            strcpy(resultados[method].nome, nomesMetodos[method]);
            resultados[method].trocas = stats.trocas;
            
            free(arrayTemp);
        }
        
        // Ordenar métodos pelo número de trocas (menor para maior)
        qsort(resultados, 6, sizeof(MetodoResultado), compararMetodos);
        
        // Gerar output no formato especificado
        fprintf(output, "[%d]:", dadosLidos.arrays[i].size);
        for (int m = 0; m < 6; m++) {
            fprintf(output, "%s(%d)", resultados[m].nome, resultados[m].trocas);
            if (m < 5) fprintf(output, ",");
        }
        fprintf(output, "\n");
        
        // Também imprimir no console para verificação
        printf("[%d]:", dadosLidos.arrays[i].size);
        for (int m = 0; m < 6; m++) {
            printf("%s(%d)", resultados[m].nome, resultados[m].trocas);
            if (m < 5) printf(",");
        }
        printf("\n");
    }

    fclose(input);
    fclose(output);
    liberarSetArrays(&dadosLidos);

    return 0;
}