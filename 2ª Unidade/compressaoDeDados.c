#include <stdio.h>
#include <stdint.h>

/* Codificação de Huffman */

// Estrutura do nó
typedef struct no {
    // Frequência
    uint32_t F;
    // Código do símbolo
    char S;
    // Nó direito
    no* D;
    // Nó esquerdo
    no* E;
} no;

// Estrutura da fila de prioridade mínima
typedef struct fila_p_min {
    // Array de nós
    no** A;
    // Tamanho atual
    uint32_t tamanho_atual;
    // Capacidade máxima
    uint32_t capacidade_maxima;
} fila_p_min;

// Construção da árvore de prefixos (trie)
no* construir_arvore(uint32_t H[], uint32_t n)
{
    // Criação de fila de prioridade mínima
    fila_p_min* fpm = criar_fila_p_min();
    // Inserindo símbolos não nulos na fila
    for(uint32_t i = 0; i < n; i++)
        if(H[i]) inserir(fpm, H[i], i, NULL, NULL);
    // Combinação dos nós com menor frequência
    while(tamanho(fpm) > 1)
    {
        no* x = extrair_min(fpm);
        no* y = extrair_min(fpm);
        inserir(fpm, x->F + y->F, '\0', x, y);
    }
    // Retornando a raiz da árvore
    return extrair_min(fpm);
}

// Procedimento de compactação dos dados
void compactar(char* C, char* E, char* T)
{
    // Anexando codificação na saída compactada C
    for(uint32_t i = 0; i < strlen(E); i++)
        anexar(C, T[E[i]]);
}