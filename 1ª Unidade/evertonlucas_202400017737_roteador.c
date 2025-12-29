#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Pacote
{
    int ordem;
    int tamanho;
    int prioridade;
    unsigned char dados[512];
} Pacote;

typedef struct
{
    int numPacotes; 
    int capacidade;
    Pacote *pacotes;
} Entrada;

// Função para ler os dados do arquivo de entrada
Entrada *lerDados(FILE *entrada)
{
    // Aloca memória para a estrutura de entrada
    Entrada *dados = malloc(sizeof(Entrada));
    if (!dados)
    {
        perror("Erro de alocação");
        fclose(entrada);
        return NULL;
    }

    // Lê o número de pacotes e a capacidade do roteador
    if (fscanf(entrada, "%d %d", &dados->numPacotes, &dados->capacidade) != 2)
    {
        fprintf(stderr, "Erro ao ler cabeçalho\n");
        fclose(entrada);
        free(dados);
        return NULL;
    }

    // Aloca memória para os pacotes
    dados->pacotes = malloc(dados->numPacotes * sizeof(Pacote));
    if (!dados->pacotes)
    {
        perror("Erro de alocação de pacotes");
        fclose(entrada);
        free(dados);
        return NULL;
    }

    // Lê cada pacote
    for (int i = 0; i < dados->numPacotes; i++)
    {
        // Ponteiro para o pacote atual
        Pacote *p = &dados->pacotes[i];

        // Lê prioridade e tamanho
        if (fscanf(entrada, "%d %d", &p->prioridade, &p->tamanho) != 2)
        {
            fprintf(stderr, "Erro ao ler pacote %d\n", i);
            free(dados->pacotes);
            free(dados);
            fclose(entrada);
            return NULL;
        }

        // Define a ordem do pacote
        p->ordem = i;

        // Lê os dados do pacote
        for (int j = 0; j < p->tamanho; j++)
        {
            unsigned int byte;
            fscanf(entrada, "%x", &byte);
            p->dados[j] = (unsigned char) byte;
        }
    }
    
    // Retorna os dados lidos
    return dados;
}

// Procedimento para construir o heap
void heapify(Pacote *heap, int n, int i)
{
    // Inicializa o maior como raiz
    int maior_idx = i;
    // Índices dos filhos
    int esq = (2 * i) + 1;
    int dir = (2 * i) + 2;

    // Verifica se o filho esquerdo é maior que a raiz
    if (esq < n && heap[esq].prioridade < heap[maior_idx].prioridade)
        maior_idx = esq; // Atualiza maior_idx se o filho esquerdo for maior

    // Verifica se o filho direito é maior que o maior até agora
    if (dir < n && heap[dir].prioridade < heap[maior_idx].prioridade)
        maior_idx = dir; // Atualiza maior_idx se o filho direito for maior

    // Se o maior não for a raiz, troca e continua heapificando
    if (maior_idx != i)
    {
        // Troca heap[i] com heap[maior_idx]
        Pacote tmp = heap[i];
        // Realiza a troca
        heap[i] = heap[maior_idx];
        // Completa a troca
        heap[maior_idx] = tmp;
        // Recursivamente heapifica a subárvore afetada
        heapify(heap, n, maior_idx);
    }
}

// Procedimento para construir o heap inicial
void construirHeap(Pacote *heap, int n)
{
    // Começa do último nó não-folha e aplica heapify
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(heap, n, i);
}

void heapSort(Pacote *vetor, int n)
{
    construirHeap(vetor, n);

    for (int i = n - 1; i > 0; i--)
    {
        Pacote tmp = vetor[0];
        vetor[0] = vetor[i];
        vetor[i] = tmp;

        heapify(vetor, i, 0);
    }
}

// Procedimento para processar o buffer de pacotes
void processarBuffer(FILE *output, Pacote *buffer, int qtd)
{
    // 1. Ordena o lote inteiro
    heapSort(buffer, qtd);

    // 2. Percorre o vetor ordenado
    fprintf(output, "|");

    for (int i = 0; i < qtd; i++)
    {
        Pacote p = buffer[i];

        for (int j = 0; j < p.tamanho; j++)
        {
            fprintf(output, "%02X", p.dados[j]);
            if (j + 1 < p.tamanho)
                fprintf(output, ",");
        }
        fprintf(output, "|");
    }

    fprintf(output, "\n");
}

// Procedimento para processar os pacotes conforme a capacidade do roteador
void processarPacotes(Entrada *dados, FILE *output)
{
    Pacote *buffer = malloc(dados->numPacotes * sizeof(Pacote));
    if (!buffer)
    {
        perror("Erro de alocação do buffer");
        return;
    }

    int capacidadeRestante = dados->capacidade;
    int qtdBuffer = 0;

    for (int i = 0; i < dados->numPacotes; i++)
    {
        Pacote *p = &dados->pacotes[i];

        // Se o pacote não cabe no buffer atual,
        // processa tudo que já foi coletado
        if (p->tamanho > capacidadeRestante && qtdBuffer > 0)
        {
            processarBuffer(output, buffer, qtdBuffer);

            // Reinicia o buffer
            qtdBuffer = 0;
            capacidadeRestante = dados->capacidade;
        }

        // Agora ele necessariamente cabe
        buffer[qtdBuffer++] = *p;
        capacidadeRestante -= p->tamanho;
    }

    // Processa o que sobrou no buffer
    if (qtdBuffer > 0)
        processarBuffer(output, buffer, qtdBuffer);

    free(buffer);
}

// Procedimento para liberar memória da estrutura de entrada
void liberarEntrada(Entrada *dados)
{
    if (!dados) return;
    free(dados->pacotes);
    free(dados);
}

int main(int argc, char *argv[])
{
    // Verifica argumentos
    if (argc != 3)
        return 1;

    // Abre arquivos
    FILE *entrada = fopen(argv[1], "r");
    FILE *saida = fopen(argv[2], "w");
    if (!entrada || !saida)
    {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    // Lê os dados de entrada
    Entrada *dados = lerDados(entrada);
    if (!dados)
        return 1;
    
    // Processa os pacotes
    processarPacotes(dados, saida);
    // Libera memória
    liberarEntrada(dados);
    // Fecha os arquivos
    fclose(entrada);
    fclose(saida);

    return 0;
}