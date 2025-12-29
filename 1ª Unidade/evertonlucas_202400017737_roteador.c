#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Pacote
{
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
    int raiz = i;
    // Índices dos filhos
    int esq = (2 * i) + 1;
    int dir = (2 * i) + 2;

    // Verifica se o filho esquerdo é maior que a raiz
    if (esq < n && heap[esq].prioridade < heap[raiz].prioridade)
        raiz = esq; // Atualiza raiz se o filho esquerdo for maior

    // Verifica se o filho direito é maior que o maior até agora
    if (dir < n && heap[dir].prioridade < heap[raiz].prioridade)
        raiz = dir; // Atualiza raiz se o filho direito for maior

    // Se o maior não for a raiz, troca e continua heapificando
    if (raiz != i)
    {
        // Troca heap[i] com heap[raiz]
        Pacote tmp = heap[i];
        // Realiza a troca
        heap[i] = heap[raiz];
        // Completa a troca
        heap[raiz] = tmp;
        // Recursivamente heapifica a subárvore afetada
        heapify(heap, n, raiz);
    }
}

// Procedimento para construir o heap inicial
void construirHeap(Pacote *heap, int n)
{
    // Constrói o heap (reorganiza o array)
    for (int i = (n/2) - 1; i >= 0; i--)
        heapify(heap, n, i);
}

// Procedimento para realizar o Heap Sort
void heapSort(Pacote *vetor, int n)
{
    // Constrói o heap inicial
    construirHeap(vetor, n);

    // Um por um extrai elementos do heap
    for (int i = n - 1; i > 0; i--)
    {
        // Move a raiz atual para o final
        Pacote tmp = vetor[0];
        // Move o maior valor para o final
        vetor[0] = vetor[i];
        // Coloca o valor da raiz no final
        vetor[i] = tmp;
        // Heapifica o heap reduzido
        heapify(vetor, i, 0);
    }
}

// Procedimento para processar o buffer de pacotes
void processarBuffer(FILE *output, Pacote *buffer, int qtd)
{
    // Ordena o lote inteiro
    heapSort(buffer, qtd);
    // Percorre o vetor ordenado
    fprintf(output, "|");

    // Imprime os pacotes no formato especificado
    for (int i = 0; i < qtd; i++)
    {
        // Obtém o pacote atual
        Pacote p = buffer[i];

        // Imprime os dados do pacote em formato hexadecimal
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
    // Aloca o buffer para armazenar os pacotes temporariamente
    Pacote *buffer = malloc(dados->numPacotes * sizeof(Pacote));
    if (!buffer)
    {
        perror("Erro de alocação do buffer");
        return;
    }

    // Capacidade restante no buffer
    int capacidadeRestante = dados->capacidade;
    // Quantidade de pacotes no buffer
    int qtdBuffer = 0;

    // Percorre todos os pacotes
    for (int i = 0; i < dados->numPacotes; i++)
    {
        // Ponteiro para o pacote atual
        Pacote *p = &dados->pacotes[i];

        // Se o pacote não cabe, processa tudo que já foi coletado
        if (p->tamanho > capacidadeRestante && qtdBuffer > 0)
        {
            // Processa o buffer atual
            processarBuffer(output, buffer, qtdBuffer);
            // Reinicia o buffer
            qtdBuffer = 0;
            capacidadeRestante = dados->capacidade;
        }

        // Agora ele necessariamente cabe
        buffer[qtdBuffer++] = *p;
        capacidadeRestante -= p->tamanho;
    }

    // Se restou algo no buffer, processa
    if (qtdBuffer > 0)
        processarBuffer(output, buffer, qtdBuffer);

    // Libera o buffer
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