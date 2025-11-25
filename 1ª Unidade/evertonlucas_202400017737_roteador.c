#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
- A empresa de telecomunicações Poxim Tech está construindo um sistema de comunicação, baseada na priorização de pacotes em redes TCP/IP para implementação de qualidade de serviço (QoS)
    - Os dados de cada pacote são sequências de bytes de tamanho variável, mas limitados até 512 bytes
    - Devido às características de roteamento de redes TCP/IP, os níveis de prioridade variam entre 0 (mínimo) e 63 (máximo), considerando que o roteador possui um limite máximo de bytes que consegue processar por vez
    - Sempre que a capacidade de processamento do roteador for atingida ou não for possível receber mais nenhum pacote, é feita a ordenação dos pacotes levando em consideração os seus respectivos níveis de priorização
- Formato de arquivo de entrada
    - [N´umero de pacotes (n)] [Quantidade de bytes]
    - [Prioridade do pacote1] [#m1 Tamanho do pacote] [B1] · · · [Bm1 ]
    - · · ·
    - [Prioridade do pacoten] [#mn Tamanho do pacote] [B1] · · · [Bmn ]
    6 8
    0 3 01 02 03
    2 4 06 07 08 09
    63 2 0F 10
    15 6 11 12 13 14 15 16
    32 5 0A 0B 0C 0D 0E
    11 2 04 05
- Formato de arquivo de saída
    - Quando uma quantidade máxima de dados é recebida, é feita a ordenação dos pacotes armazenados para priorização do roteamento
    |06,07,08,09|01,02,03|
    |0F,10|11,12,13,14,15,16|
    |0A,0B,0C,0D,0E|04,05|
*/

typedef struct {
    int prioridade;
    int size;
    char **data;
} Pacote;

// Procedimento para manter a propriedade do heap
void heapify(Pacote *pacotes, int n, int i) {
    int maior = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;

    if (esq < n && pacotes[esq].prioridade > pacotes[maior].prioridade)
        maior = esq;

    if (dir < n && pacotes[dir].prioridade > pacotes[maior].prioridade)
        maior = dir;

    if (maior != i) {
        Pacote temp = pacotes[i];
        pacotes[i] = pacotes[maior];
        pacotes[maior] = temp;
        heapify(pacotes, n, maior);
    }
}

// Procedimento de ordenação HeapSort
void heapSort(Pacote *pacotes, int n) {
    // Construir heap máximo
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(pacotes, n, i);

    // Extrair elementos do heap um por um
    for (int i = n - 1; i > 0; i--) {
        Pacote temp = pacotes[0];
        pacotes[0] = pacotes[i];
        pacotes[i] = temp;
        heapify(pacotes, i, 0);
    }
}

// Procedimento para liberar memória dos pacotes
void liberarPacotes(Pacote *pacotes, int n) {
    for (int i = 0; i < n; i++) {
        if (pacotes[i].data) {
            for (int j = 0; j < pacotes[i].size; j++) {
                free(pacotes[i].data[j]);
            }
            free(pacotes[i].data);
        }
    }
    free(pacotes);
}

// Procedimento para ler os pacotes do arquivo de entrada
void lendoPacotes(FILE *in, Pacote *pacotes, int N, FILE *out) {
    for (int i = 0; i < N; i++) {
        // Lendo a prioridade e o tamanho do pacote
        if (fscanf(in, "%d %d", &pacotes[i].prioridade, &pacotes[i].size) != 2) {
            fprintf(stderr, "Erro ao ler pacote %d\n", i);
            liberarPacotes(pacotes, N);
            fclose(in);
            fclose(out);
            return;
        }

        // Alocando memória para os dados do pacote
        pacotes[i].data = malloc((size_t)pacotes[i].size * sizeof(char*));
        if (!pacotes[i].data) {
            perror("Erro de alocação para dados do pacote");
            liberarPacotes(pacotes, N);
            fclose(in);
            fclose(out);
            return;
        }

        // Lendo os bytes do pacote
        for (int j = 0; j < pacotes[i].size; j++) {
            pacotes[i].data[j] = malloc(3 * sizeof(char));
            if (!pacotes[i].data[j]) {
                perror("Erro de alocação para byte do pacote");
                liberarPacotes(pacotes, N);
                fclose(in);
                fclose(out);
                return;
            }
            if (fscanf(in, "%2s", pacotes[i].data[j]) != 1) {
                fprintf(stderr, "Erro ao ler byte %d do pacote %d\n", j, i);
                liberarPacotes(pacotes, N);
                fclose(in);
                fclose(out);
                return;
            }
        }
    }
}

// Procedimento para iterar sobre os pacotes e processá-los em lotes
void iterandoPacotes(Pacote *pacotes, int N, int capacidade, FILE *out, int *bytes_atual, int *inicio_lote) {
    for (int i = 0; i < N; i++) {
        // Se adicionar este pacote exceder a capacidade, processar o lote atual
        if (*bytes_atual + pacotes[i].size > capacidade) {
            // Processar o lote atual (do inicio_lote até i-1)
            int tamanho_lote = i - *inicio_lote;
            
            if (tamanho_lote > 0) {
                // Ordenar por prioridade (maior primeiro)
                heapSort(pacotes + *inicio_lote, tamanho_lote);
                
                // Imprimir em ordem decrescente de prioridade
                for (int j = tamanho_lote - 1; j >= 0; j--) {
                    fprintf(out, "|");
                    for (int k = 0; k < pacotes[*inicio_lote + j].size; k++) {
                        if (k > 0) fprintf(out, ",");
                        fprintf(out, "%s", pacotes[*inicio_lote + j].data[k]);
                    }
                }
                fprintf(out, "|\n");
            }
            
            // Iniciar novo lote com o pacote atual
            *inicio_lote = i;
            *bytes_atual = pacotes[i].size;
        } else {
            // Adicionar pacote ao lote atual
            *bytes_atual += pacotes[i].size;
        }
    }
}

int main(int argc, char *argv[]) {
    // Verificação de argumentos
    if (argc != 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Abrindo arquivos
    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    // Ler número de pacotes e capacidade do roteador
    int N, capacidade;
    fscanf(in, "%d %d", &N, &capacidade);

    // Alocando memória para os pacotes
    Pacote *pacotes = malloc((size_t)N * sizeof(Pacote));
    if (!pacotes) {
        perror("Erro de alocação");
        fclose(in);
        fclose(out);
        return 1;
    }

    // Inicializando os pacotes
    for (int i = 0; i < N; i++) {
        pacotes[i].data = NULL;
    }

    // Leitura de todos os pacotes
    lendoPacotes(in, pacotes, N, out);
    

    // Processar pacotes em lotes baseados na capacidade do roteador
    int bytes_atual = 0;
    int inicio_lote = 0;

    // Iterando sobre os pacotes
    iterandoPacotes(pacotes, N, capacidade, out, &bytes_atual, &inicio_lote);
    
    // Processar o último lote (se houver)
    int tamanho_lote = N - inicio_lote;
    if (tamanho_lote > 0) {
        heapSort(pacotes + inicio_lote, tamanho_lote);
        
        for (int j = tamanho_lote - 1; j >= 0; j--) {
            fprintf(out, "|");
            for (int k = 0; k < pacotes[inicio_lote + j].size; k++) {
                if (k > 0) fprintf(out, ",");
                fprintf(out, "%s", pacotes[inicio_lote + j].data[k]);
            }
        }
        fprintf(out, "|\n");
    }

    liberarPacotes(pacotes, N);
    // Fechando arquivos
    fclose(in);
    fclose(out);
    
    // Finalizando o programa
    return 0;
}