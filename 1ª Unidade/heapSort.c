#include <stdio.h>
#include <stdlib.h>

void criarHeap(int *array, int i, int f)
{
    int aux = array[i];
    int j = 2 * i + 1;

    while (j <= f)
    {
        if (j < f)
            if (array[j] < array[j + 1])
                j = j + 1;
        if (aux < array[j])
        {
            array[i] = array[j];
            i = j;
            j = 2 * i + 1;
        }
        else
            j = f + 1;
    }
    array[i] = aux;
}

void heapSort(int *array, int n)
{
    int i, aux;
    for (i = (n - 1) / 2; i >= 0; i--)
        criarHeap(array, i, n - 1);
    for (i = n - 1; i >= 1; i--)
    {
        aux = array[0];
        array[0] = array[i];
        array[i] = aux;
        criarHeap(array, 0, i - 1);
    }
}

int main(int argc, char *argv[])
{
    int array[] = {38, 27, 43, 3, 9, 82, 10};

    // Ordena com HeapSort
    heapSort(array, 7);

    printf("Array ordenado (7 elementos):\n");
    for (int i = 0; i < 7; i++)
        printf("%d ", array[i]);
    return 0;
}
