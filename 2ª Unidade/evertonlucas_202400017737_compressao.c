#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <omp.h>

// Estrutura para armazenar o resultado da compressão
typedef struct ResultadoComp {
    int bitsTotal;
    float percentual;
    uint8_t* buffer;
    int bufferTam;
    char algo[4]; // "RLE" ou "HUF"
} ResultadoComp;

typedef struct Dados
{
    uint8_t* dados;
    int sequenciaTam;
} Dados;

typedef struct DadosArquivo
{
    int qtdDados;
    Dados* dados;
} DadosArquivo;

// Estrutura para o nó da árvore de Huffman
typedef struct NoHuffman
{
    uint8_t byte; // Valor do byte
    unsigned int frequencia; // Frequência do byte
    struct NoHuffman* esquerda; // Nó esquerdo
    struct NoHuffman* direita; // Nó direito
} NoHuffman;

// --- Estrutura de Heap (Min-Heap Array) ---
typedef struct Heap {
    NoHuffman** array;
    int tamanho;
    int capacidade;
} Heap;

// Função auxiliar para converter um caractere hexadecimal para seu valor (0-15)
int hexCharParaInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1; // Erro ou caractere não hexadecimal
}

// Função para ler os dados do arquivo de entrada
DadosArquivo lerArquivo(FILE* arquivo)
{
    DadosArquivo dadosArquivo;
    // Lendo quantidade de sequências
    if (fscanf(arquivo, "%d", &dadosArquivo.qtdDados) != 1)
    {
        dadosArquivo.qtdDados = 0;
        dadosArquivo.dados = NULL;
        return dadosArquivo;
    }
    
    // Alocando memória para as sequências
    dadosArquivo.dados = malloc(dadosArquivo.qtdDados * sizeof(Dados));
    
    // Lendo cada sequência
    for (int i = 0; i < dadosArquivo.qtdDados; i++)
    {
        // 1. Lendo tamanho da sequência
        fscanf(arquivo, "%d", &dadosArquivo.dados[i].sequenciaTam);
        int tamanho = dadosArquivo.dados[i].sequenciaTam;

        // 2. Alocando memória para os dados da sequência
        dadosArquivo.dados[i].dados = malloc(tamanho * sizeof(uint8_t));

        // 3. Lendo e convertendo os bytes de forma eficiente (Substituindo o fscanf lento)
        for (int j = 0; j < tamanho; j++) {
            int val_alto = -1;
            int val_baixo = -1;
            char c;

            // Busca pelo dígito hexadecimal alto
            do {
                c = fgetc(arquivo);
                val_alto = hexCharParaInt(c);
            } while (val_alto == -1 && !feof(arquivo));

            // Busca pelo dígito hexadecimal baixo
            do {
                c = fgetc(arquivo);
                val_baixo = hexCharParaInt(c);
            } while (val_baixo == -1 && !feof(arquivo));
            
            // Verifica se as leituras foram bem-sucedidas
            if (val_alto != -1 && val_baixo != -1) {
                // Combina os dois nibbles para formar o byte (e.g., A e 3 = 0xA3)
                dadosArquivo.dados[i].dados[j] = (uint8_t)((val_alto << 4) | val_baixo);
            } else {
                // Tratar erro de formato ou fim de arquivo inesperado
                // Por simplicidade, podemos sair ou logar um erro
                break; 
            }
        }
    }

    return dadosArquivo;
}

// ---------------------- RLE --------------------------

// Função para comprimir dados usando RLE (Run-Length Encoding)
ResultadoComp compressaoRLE(Dados* dados)
{
    ResultadoComp res;
    strcpy(res.algo, "RLE");
    res.buffer = NULL;
    res.bufferTam = 0;

    uint8_t* sequencia = dados->dados;
    int tamanho = dados->sequenciaTam;

    int bitsAntes = tamanho * 8;
    int bitsDepois = 0;
    int bytesDepois = 0;

    if (tamanho == 0) {
        res.bitsTotal = 0;
        res.percentual = 0.0f;
        return res;
    }

    /* ---------- PRIMEIRA PASSAGEM ----------
       Apenas conta bits e bytes gerados
    */
    int count = 1;
    for (int j = 1; j <= tamanho; j++) {
        if (j < tamanho && sequencia[j] == sequencia[j - 1]) {
            count++;
        } else {
            if (count == 1) {
                bitsDepois += 8;
                bytesDepois += 1;
            } else {
                int remaining = count;
                while (remaining > 0) {
                    int chunk = remaining > 255 ? 255 : remaining;
                    bitsDepois += 16;
                    bytesDepois += 2;
                    remaining -= chunk;
                }
            }
            count = 1;
        }
    }

    /* ---------- ALOCAÇÃO ÚNICA ---------- */
    res.buffer = malloc(bytesDepois * sizeof(uint8_t));
    res.bufferTam = bytesDepois;

    /* ---------- SEGUNDA PASSAGEM ----------
       Agora escreve os bytes no buffer
    */
    int pos = 0;
    count = 1;

    for (int j = 1; j <= tamanho; j++) {
        if (j < tamanho && sequencia[j] == sequencia[j - 1]) {
            count++;
        } else {
            if (count == 1) {
                res.buffer[pos++] = sequencia[j - 1];
            } else {
                int remaining = count;
                while (remaining > 0) {
                    int chunk = remaining > 255 ? 255 : remaining;
                    res.buffer[pos++] = (uint8_t)chunk;
                    res.buffer[pos++] = sequencia[j - 1];
                    remaining -= chunk;
                }
            }
            count = 1;
        }
    }

    res.bitsTotal = bitsDepois;
    res.percentual = 100.0f * (float)bitsDepois / (float)bitsAntes;

    return res;
}


/* ---------------------- Huffman -------------------------- */

Heap* criarHeap(int capacidade) {
    Heap* h = malloc(sizeof(Heap));
    h->tamanho = 0;
    h->capacidade = capacidade;
    h->array = malloc(capacidade * sizeof(NoHuffman*));
    return h;
}

NoHuffman* criarNo(uint8_t byte, int freq) {
    NoHuffman *n = malloc(sizeof(NoHuffman));
    n->byte = byte;
    n->frequencia = freq;
    n->esquerda = NULL;
    n->direita = NULL;
    return n;
}

void trocarNo(NoHuffman** a, NoHuffman** b) {
    NoHuffman* t = *a;
    *a = *b;
    *b = t;
}

// Min-Heapify padrão
void minHeapify(Heap* h, int idx) {
    int menor = idx;
    int esq = 2 * idx + 1;
    int dir = 2 * idx + 2;

    if (esq < h->tamanho && h->array[esq]->frequencia < h->array[menor]->frequencia)
        menor = esq;

    if (dir < h->tamanho && h->array[dir]->frequencia < h->array[menor]->frequencia)
        menor = dir;

    if (menor != idx) {
        trocarNo(&h->array[menor], &h->array[idx]);
        minHeapify(h, menor);
    }
}

void inserirHeap(Heap *h, NoHuffman *novo) {
    int i = h->tamanho++;
    h->array[i] = novo;

    // Fix up
    while (i && h->array[i]->frequencia < h->array[(i - 1) / 2]->frequencia) {
        trocarNo(&h->array[i], &h->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

NoHuffman* extrairMin(Heap *h) {
    if (h->tamanho == 0) return NULL;
    NoHuffman* raiz = h->array[0];
    h->array[0] = h->array[h->tamanho - 1];
    h->tamanho--;
    minHeapify(h, 0);
    return raiz;
}

// Gera tabela de códigos recursivamente
void gerarCodigos(NoHuffman* raiz, int arr[], int top, char codigos[256][256], int tamanhos[256]) {
    if (raiz->esquerda) {
        arr[top] = 0;
        gerarCodigos(raiz->esquerda, arr, top + 1, codigos, tamanhos);
    }
    if (raiz->direita) {
        arr[top] = 1;
        gerarCodigos(raiz->direita, arr, top + 1, codigos, tamanhos);
    }
    // É nó folha
    if (!raiz->esquerda && !raiz->direita) {
        for (int i = 0; i < top; i++) {
            codigos[raiz->byte][i] = arr[i] + '0';
        }
        codigos[raiz->byte][top] = '\0';
        tamanhos[raiz->byte] = top;
    }
}

void liberarArvore(NoHuffman* raiz) {
    if (!raiz) return;
    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    free(raiz);
}

ResultadoComp compressaoHuffman(Dados *dados) {
    ResultadoComp res;
    strcpy(res.algo, "HUF");
    res.buffer = NULL;
    res.bufferTam = 0;

    int tam = dados->sequenciaTam;
    int bitsAntes = tam * 8;
    
    // Frequência
    int freq[256] = {0};
    for (int j = 0; j < tam; j++)
        freq[dados->dados[j]]++;

    Heap* h = criarHeap(256);

    for (int b = 0; b < 256; b++)
        if (freq[b] > 0)
            inserirHeap(h, criarNo((uint8_t)b, freq[b]));

    // Construção da árvore
    while (h->tamanho > 1) {
        NoHuffman *esquerda = extrairMin(h);
        NoHuffman *direita = extrairMin(h);

        NoHuffman *pai = criarNo(0, esquerda->frequencia + direita->frequencia);
        pai->esquerda = esquerda;
        pai->direita = direita;

        inserirHeap(h, pai);
    }
    
    NoHuffman* raiz = extrairMin(h);

    // Gerar Códigos e Empacotar Bits
    char mapaCodigos[256][256]; // Tabela de strings "0101"
    int mapaTamanhos[256] = {0};
    int arrAux[256];
    
    // Caso especial: apenas 1 símbolo (ex: AAAAA) -> Huffman gera 1 bit por simbolo (0)
    if (raiz && !raiz->esquerda && !raiz->direita) {
        // Simbolizamos como bit 0
        mapaCodigos[raiz->byte][0] = '0';
        mapaCodigos[raiz->byte][1] = '\0';
        mapaTamanhos[raiz->byte] = 1;
    } else if (raiz) {
        gerarCodigos(raiz, arrAux, 0, mapaCodigos, mapaTamanhos);
    }

    // Calcular bits totais e gerar buffer
    int bitsTotais = 0;
    for(int i=0; i<tam; i++) bitsTotais += mapaTamanhos[dados->dados[i]];
    
    // Arredondar para bytes completos para alocação
    res.bufferTam = (bitsTotais + 7) / 8;
    if (res.bufferTam == 0 && bitsTotais > 0) res.bufferTam = 1;
    res.buffer = calloc(res.bufferTam, sizeof(uint8_t));

    // Escrever bits no buffer
    int bitPos = 0;
    for (int i = 0; i < tam; i++) {
        uint8_t byte = dados->dados[i];
        char* codigo = mapaCodigos[byte];
        for (int k = 0; codigo[k] != '\0'; k++) {
            if (codigo[k] == '1') {
                // Seta o bit correspondente
                res.buffer[bitPos / 8] |= (1 << (7 - (bitPos % 8)));
            }
            bitPos++;
        }
    }

    int bitsDepois = res.bufferTam * 8;

    res.bitsTotal = bitsDepois;
    res.percentual = 100.0f * (float)bitsDepois / (float)bitsAntes;

    liberarArvore(raiz);
    free(h->array);
    free(h);
    
    return res;
}

void escreverHex(FILE* f, uint8_t* buffer, int tam) {
    for(int i=0; i<tam; i++) {
        fprintf(f, "%02X", buffer[i]);
    }
}

int main(int argc, char *argv[])
{
    clock_t start = clock();
    if (argc != 3)
    {
        printf("Uso: %s <input> <output>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Erro ao abrir input");
        return 1;
    }
    
    FILE* output = fopen(argv[2], "w");
    if (!output) {
        perror("Erro ao abrir output");
        fclose(input);
        return 1;
    }

    DadosArquivo dadosArquivo = lerArquivo(input);
    int qtdDados = dadosArquivo.qtdDados;

    char** saidas = malloc(qtdDados * sizeof(char*));
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 0; i < qtdDados; i++)
    {
        ResultadoComp rle = compressaoRLE(&dadosArquivo.dados[i]);
        ResultadoComp huf = compressaoHuffman(&dadosArquivo.dados[i]);

        char* bufferSaida = malloc(512); // tamanho seguro para saída textual
        int offset = 0;

        if (huf.bitsTotal == rle.bitsTotal)
        {
            offset += snprintf(bufferSaida + offset, 512 - offset, "%d->HUF(%.2f%%)=", i, huf.percentual);
            for (int j = 0; j < huf.bufferTam && offset < 512 - 2; j++)
                offset += snprintf(bufferSaida + offset, 512 - offset, "%02X", huf.buffer[j]);

            offset += snprintf(bufferSaida + offset, 512 - offset, "\n%d->RLE(%.2f%%)=", i, rle.percentual);
            for (int j = 0; j < rle.bufferTam && offset < 512 - 2; j++)
                offset += snprintf(bufferSaida + offset, 512 - offset, "%02X", rle.buffer[j]);
        }
        else
        {
            ResultadoComp* v = (huf.bitsTotal < rle.bitsTotal) ? &huf : &rle;
            offset += snprintf(bufferSaida + offset, 512 - offset, "%d->%s(%.2f%%)=", i, v->algo, v->percentual);
            for (int j = 0; j < v->bufferTam && offset < 512 - 2; j++)
                offset += snprintf(bufferSaida + offset, 512 - offset, "%02X", v->buffer[j]);
        }

        saidas[i] = bufferSaida;

        free(rle.buffer);
        free(huf.buffer);
    }


    for (int i = 0; i < qtdDados; i++)
    {
        fprintf(output, "%s", saidas[i]);
        if (i < qtdDados - 1)
            fprintf(output, "\n");
        free(saidas[i]);
    }
    
    // Liberar memória alocada para os dados
    for (int i = 0; i < qtdDados; i++)
        free(dadosArquivo.dados[i].dados);
    free(dadosArquivo.dados);
    free(saidas);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %.6f segundos\n", time_spent);

    fclose(input);
    fclose(output);

    return 0;
}