#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Procedimento para trocar dois elementos de posição
void swap(int *a, int *b)
{
    int temp = *a; // Armazena o valor de a em uma variável temporária
    *a = *b;       // Atribui o valor de b para a
    *b = temp;     // Atribui o valor temporário (original de a) para b
}

// Função que retorna um índice pseudoaleatório entre low e high
int pseudoRandom(int low, int high)
{
    return low + rand() % (high - low + 1);
}

// Função para encontrar o índice do valor mediano entre três elementos
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

// Função de partição usando o método Lomuto (v)
int lomuto(int *array, int low, int high, int pivotValue)
{
    int begin = low; // Armazena o índice do menor elemento

    int pivotIndex = low; // Inicializa o índice do pivô
    // Encontra o índice do pivô com base no valor do pivô
    for (int i = low; i <= high; i++) {
        if (array[i] == pivotValue) {
            pivotIndex = i;
            break;
        }
    }
    swap(&array[pivotIndex], &array[high]);

    // Loop para particionar o array
    for(int i = low; i < high; i++)
    {
        // Se o elemento atual é menor ou igual ao pivô faz a troca
        if(array[i] <= pivotValue)
        {
            // Faz a troca de posição entre array[begin] com array[i]
            swap(&array[begin], &array[i]);
            begin++; // Incrementa o índice do menor elemento
        }
    }

    // Coloca o pivô na posição correta
    swap(&array[begin], &array[high]);

    return begin;
}

// Função de partição usando o método Lomuto
int lomutoPadrao(int *array, int low, int high)
{
    return lomuto(array, low, high, array[high]);
}

// Função de partição usando o método Lomuto com Mediana
int lomutoMediana(int *array, int low, int high)
{
    // Trata o caso trivial em que há menos de 2 elementos
    if (high - low < 2)
        return lomuto(array, low, high, array[low]);

    int mid = low + (high - low) / 2; // Calcula o índice do elemento do meio
    // Escolhe o pivô usando a mediana entre os elementos: low, mid, high
    int pivotIndex = mediana(array, low, mid, high);

    return lomuto(array, low, high, array[pivotIndex]);
}

// Função de partição usando o método Lomuto com pivô Aleatório
int lomutoRandom(int *array, int low, int high)
{
    int pivot = pseudoRandom(low, high); // Gera um índice aleatório
    swap(&array[pivot], &array[high]); // Move o pivô para o final

    return lomuto(array, low, high, array[high]);
}

// Função de partição usando o método Hoare
int hoare(int *array, int low, int high, int pivot)
{
    int i = low - 1; // Índice para o menor elemento
    int j = high + 1; // Índice para o maior elemento

    // Loop infinito até que os índices se cruzem
    while(1)
    {
        // Incrementa i até encontrar um elemento maior ou igual ao pivô
        do {
            i++;
        } while(array[i] < pivot);

        // Decrementa j até encontrar um elemento menor ou igual ao pivô
        do {
            j--;
        } while(array[j] > pivot);

        // Se os índices se cruzaram, retorna j
        if(i >= j)
            return j;

        // Faz a troca de array[i] com array[j]
        swap(&array[i], &array[j]);
    }
}

// Função de partição usando o método Hoare
int hoarePadrao(int *array, int low, int high)
{
    return hoare(array, low, high, array[low]);
}

// Função de partição usando o método Hoare com Mediana
int hoareMediana(int *array, int low, int high)
{
    // Trata o caso trivial em que há menos de 2 elementos
    if (high - low < 2)
        return hoare(array, low, high, array[low]);

    int mid = low + (high - low) / 2; // Calcula o índice do elemento do meio
    // Escolhe o pivô usando Mediana de Três nos elementos: low, mid, high
    int pivotIndex = mediana(array, low, mid, high);

    return hoare(array, low, high, array[pivotIndex]);
}

// Função de partição usando o método Hoare com pivô Aleatório
int hoareRandom(int *array, int low, int high)
{
    int pivot = pseudoRandom(low, high); // Gera um índice aleatório entre low e high
    
    return hoare(array, low, high, array[pivot]);
}

// Procedimento principal do Quick Sort
void quickSort(int *array, int low, int high, int method)
{
    // Verifica se a partição é válida
    if(low < high)
    {
        int mid;
        // Seleciona o método de partição com base na escolha
        switch(method)
        {
            case 1:
                mid = lomutoPadrao(array, low, high);
                break;
            case 2:
                mid = lomutoMediana(array, low, high);
                break;
            case 3:
                mid = lomutoRandom(array, low, high);
                break;
            case 4:
                mid = hoarePadrao(array, low, high);
                break;
            case 5:
                mid = hoareMediana(array, low, high);
                break;
            case 6:
                mid = hoareRandom(array, low, high);
                break;
            default:
                mid = lomutoPadrao(array, low, high);
        }

        // Chama recursivamente o Quick Sort para as duas metades
        if(method >= 4)
        {
            quickSort(array, low, mid, method); // Ordena a parte esquerda do pivô
            quickSort(array, mid + 1, high, method); // Ordena a parte direita do pivô
        } else
        {
            quickSort(array, low, mid - 1, method); // Ordena a parte esquerda do pivô
            quickSort(array, mid + 1, high, method); // Ordena a parte direita do pivô
        }
    }
}

int main()
{
    // Exemplo de uso do Quick Sort
    int arr1[] = {38, 27, 43, 3, 9, 82, 10};
    int arr2[] = {38, 27, 43, 3, 9, 82, 10};
    int arr3[] = {38, 27, 43, 3, 9, 82, 10};
    int arr4[] = {38, 27, 43, 3, 9, 82, 10};
    int arr5[] = {38, 27, 43, 3, 9, 82, 10};
    int arr6[] = {38, 27, 43, 3, 9, 82, 10};
    int n = sizeof(arr1) / sizeof(arr1[0]); // Calcula o tamanho do array
    // Imprime o array original
    printf("Array Original: \n");
    for(int i = 0; i < n; i++)
        printf("%d ", arr1[i]);
    /* Ordena o array usando diferentes métodos de Quick Sort */
    // Lomuto Padrão
    quickSort(arr1, 0, n - 1, 1);
    printf("\nArray Ordenado pelo Lomuto Padrao: \n");
    for(int i = 0; i < n; i++)
        printf("%d ", arr1[i]);
    
    // Lomuto Mediana
    quickSort(arr2, 0, n - 1, 2);
    printf("\nArray Ordenado pelo Lomuto Mediana: \n");
    for(int i = 0; i < n; i++)
        printf("%d ", arr2[i]);

    // Lomuto Aleatorio
    quickSort(arr3, 0, n - 1, 3);
    printf("\nArray Ordenado pelo Lomuto Aleatorio: \n");
    for(int i = 0; i < n; i++)
        printf("%d ", arr3[i]);
    
    // Hoare Padrão
    quickSort(arr4, 0, n - 1, 4);
    printf("\nArray Ordenado pelo Hoare Padrao: \n");
    for(int i = 0; i < n; i++)
        printf("%d ", arr4[i]);
    
    // Hoare Mediana
    quickSort(arr5, 0, n - 1, 5);
    printf("\nArray Ordenado pelo Hoare Mediana: \n");
    for(int i = 0; i < n; i++)
        printf("%d ", arr5[i]);
    
    // Hoare Aleatorio
    quickSort(arr6, 0, n - 1, 6);
    printf("\nArray Ordenado pelo Hoare Aleatorio: \n");
    for(int i = 0; i < n; i++)
        printf("%d ", arr6[i]);
    printf("\n\n");

    return 0;
}