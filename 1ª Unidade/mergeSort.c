#include <stdio.h>
#include <stdlib.h>

// Procedimento para mesclar dois subarrays ordenados
void merge(int *vetor, int inicio, int meio, int fim)
{
    // Declara variáveis necessárias
    int *temp, n1, n2, tamanho, i, j, k;
    int fim1 = 0, fim2 = 0; // Flags para indicar o fim dos subarrays
    n1 = inicio; // Início do primeiro subarray
    n2 = meio + 1; // Início do segundo subarray
    tamanho = fim - inicio + 1; // Tamanho total dos dois subarrays

    // Cria dinamicamente um array temporário
    temp = (int*) malloc(tamanho * sizeof(int));

    // Verifica se a alocação foi bem-sucedida
    if (temp != NULL) {
        // Mescla os dois subarrays no array temporário
        for (i = 0; i < tamanho; i++) {
            // Verifica se ambos os subarrays ainda têm elementos
            if (!fim1 && !fim2) {
                // Compara os elementos dos dois subarrays e insere o menor no array temporário
                if (vetor[n1] < vetor[n2])
                    temp[i] = vetor[n1++];
                else
                    temp[i] = vetor[n2++];
                // Verifica se algum dos subarrays chegou ao fim
                if (n1 > meio) fim1 = 1;
                if (n2 > fim) fim2 = 1;
            } else { // Um dos subarrays chegou ao fim
                // Insere os elementos restantes do outro subarray no array temporário
                if (!fim1)
                    temp[i] = vetor[n1++];
                else
                    temp[i] = vetor[n2++];
            }
        }
    }
    // Copia os elementos do array temporário de volta para o array original
    for (j = 0, k = inicio; j < tamanho; j++, k++)
        vetor[k] = temp[j];
    free(temp);
}

// Procedimento principal do Merge Sort
void mergeSort(int *vetor, int inicio, int fim)
{
    // Verifica se o array tem mais de um elemento
    if (inicio < fim)
    {
        // Encontra o ponto médio para dividir o array
        int mid = inicio + (fim - inicio) / 2;
        // Ordena a primeira e a segunda metade
        mergeSort(vetor, inicio, mid);
        // Ordena a segunda metade
        mergeSort(vetor, mid + 1, fim);
        // Mescla as duas metades ordenadas
        merge(vetor, inicio, mid, fim);
    }
}

int main()
{
    // Exemplo de uso do Merge Sort
    int arr[] = {38, 27, 43, 3, 9, 82, 10};
    // Calcula o tamanho do array
    int arr_size = sizeof(arr) / sizeof(arr[0]);

    // Imprime o array desordenado
    printf("Array Desordenado: \n");
    for (int i = 0; i < arr_size; i++)
        printf("%d ", arr[i]);
    printf("\n");

    // Chama a função mergeSort para ordenar o array
    mergeSort(arr, 0, arr_size - 1);
    
    // Imprime o array ordenado
    printf("Array Ordenado: \n");
    for (int i = 0; i < arr_size; i++)
        printf("%d ", arr[i]);
    printf("\n");

    return 0;
}