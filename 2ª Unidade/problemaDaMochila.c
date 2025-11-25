#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int peso;
    int valor;
} Item;

int max(int a, int b) {
    return (a > b) ? a : b;
}

int mochila(Item itens[], int capacidade, int n) {
    int i, j; // i = índice dos itens (linha), j = capacidade atual da mochila (coluna)
    int **K = (int **) malloc((n + 1)*sizeof(int *)); // Aloca matriz K
    // Aloca memória para cada linha da matriz K
    for (i = 0; i <= n; i++) {
        K[i] = (int *) malloc((capacidade + 1) * sizeof(int));
    }

    // Preenche a matriz K com os valores máximos possíveis para cada subproblema
    for (i = 0; i <= n; i++) {
        // Itera sobre cada capacidade possível da mochila
        for (j = 0; j <= capacidade; j++) {
            // Caso base: se não há itens ou a capacidade é zero, o valor máximo é zero
            if (i == 0 || j == 0) {
                K[i][j] = 0;
            } else if (itens[i - 1].peso <= j) { // Se o peso do item atual é menor ou igual à capacidade atual
                K[i][j] = max(K[i - 1][j], itens[i - 1].valor + K[i - 1][j - itens[i - 1].peso]); // Escolhe o máximo entre incluir ou não o item
            } else {
                K[i][j] = K[i - 1][j]; // Item não pode ser incluído, mantém o valor anterior
            }
        }
    }

    int resultado = K[n][capacidade]; // Valor máximo que pode ser obtido

    // Libera a memória alocada para a matriz K
    for (i = 0; i <= n; i++) {
        free(K[i]);
    }
    free(K);

    return resultado;
}

int main() {
    int n = 3, capacidade = 5;
    Item *itens = (Item *) malloc(n * sizeof(Item));
    itens[0].peso = 1;
    itens[0].valor = 60;
    itens[1].peso = 2;
    itens[1].valor = 100;
    itens[2].peso = 3;
    itens[2].valor = 120;

    
    clock_t inicio = clock(); // Inicia a contagem do tempo
    int valorMaximo = mochila(itens, capacidade, n);
    clock_t fim = clock(); // Finaliza a contagem do tempo
    double tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC; // Calculo do tempo

    printf("\nValor maximo: %d\n", valorMaximo);
    printf("Tempo de execucao: %f segundos\n\n", tempoExecucao);

    free(itens);

    return 0;
}