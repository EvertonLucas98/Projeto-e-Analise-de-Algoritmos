#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <stdint.h>

/* Padrão de tipos por tamanho */
// Busca em cadeias por força bruta
void forca_bruta(int32_t* R, char* T, char* P) {
    // Tamanhos das cadeias T e P
    int32_t n = strlen(T), m = strlen(P);
    // Iterando na cadeia T até o índice n - m
    for(int32_t s = 0; s <= n - m; s++) {
        // Contador de diferenças
        uint32_t d = 0;
        // Iterando na cadeia P enquanto d for zero
        for(int32_t i = 0; i < m && d == 0; i++) {
            // Caso os símbolos sejam diferentes
            if(P[i] != T[s + i]) d++;
        }
        // O índice da combinação é armazenado
        if(d == 0) inserir(R, s);
    }
}

// Busca em cadeias com Rabin-Karp
void rabin_karp(int32_t* R, char* T, char* P, uint32_t
b, uint32_t q)
{
    // Variáveis auxiliares
    int32_t n = strlen(T), m = strlen(P), h = (int32_t)pow(b, m - 1) % q, t = 0, p = 0;
    // Pré-processamento numérico das cadeias p e t
    for(int32_t i = 0; i < m; i++)
    {
        p = (b * p + v(P[i])) % q;
        t = (b * t + v(T[i])) % q;
    }
    // Iterando na cadeia T até o índice n - m
    for(int32_t s = 0; s <= n - m; s++) {
        // Comparando o padrão com a cadeia
        if((p == t) && igual(P, m, T, s))
            // Armazenando índice de combinação
            inserir(R, s);
        // Deslocamento numérico
        t = (b * (t - v(T[s]) * h) + v(T[s + m])) % q;
    }
}

// Procedimento de cálculo da tabela de transição
void calcular_tabela(int32_t* k, char* P)
{
    // i = sufixo , j = prefixo
    for(int32_t i = 1, j = -1; i < strlen(P); i++) {
        // Prefixo e sufixo diferentes
        while(j >= 0 && P[j + 1] != P[i])
            j = k[j]; // Retorno de estado
        // Combinação de prefixo e sufixo
        if(P[j + 1] == P[i])
            j++; // Avanço de estado
        // Atualização da transição do estado
        k[i] = j;
    }
}

// Busca por KMP
void KMP(int32_t* k, int32_t* R, char* T, char* P)
{
    // Pré-processamento
    int32_t n = strlen(T), m = strlen(P);
    calcular_tabela(k, P);
    // Iterando na cadeia T
    for(int32_t i = 0, j = -1; i < n; i++)
    {
        // Retorno de estado
        while(j >= 0 && P[j + 1] != T[i]) j = k[j];
        // Avanço de estado
        if(P[j + 1] == T[i]) j++;
        // Combinação do padrão
        if(j == m - 1)
        {
            inserir(R, i - m + 1);
            j = k[j];
        }
    }
}
