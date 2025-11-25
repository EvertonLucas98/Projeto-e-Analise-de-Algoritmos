#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_CNPJ_LENGTH 32
#define MAX_CODE_LENGTH 32

typedef struct
{
    char codigo[MAX_CODE_LENGTH];
    char cnpj[MAX_CNPJ_LENGTH];
    int peso;
    int qtd;
    int temDivergenciaCNPJ;
} Container;

typedef struct
{
    Container *containers;
    int qtd;
} ContainerArray;

// Função para ler os dados dos contêineres do arquivo
ContainerArray lerDados(FILE* arquivo)
{
    // Inicializa o array de containers
    ContainerArray resultado = {NULL, 0};
    // Lê a quantidade de containers
    int n;

    fscanf(arquivo, "%d", &n); // Lê o número de containers

    // Alocação dinâmica de memória para os containers
    Container *dadosContainer = malloc(sizeof(Container) * n);
    // Verifica se a alocação foi bem-sucedida
    if (!dadosContainer)
    {
        printf("Erro de alocação.\n");
        return resultado;
    }

    // Leitura dos dados dos containers
    for (int i = 0; i < n; ++i)
    {
        // Realiza a leitura e verifica se foi bem-sucedida
        fscanf(arquivo, "%s %s %d", dadosContainer[i].codigo, dadosContainer[i].cnpj, &dadosContainer[i].peso);
    }
    // Inicializa o campo de divergência de CNPJ
    for (int i = 0; i < n; ++i)
        dadosContainer[i].temDivergenciaCNPJ = 0;

    // Adiciona os dados da lista de containers lidos ao resultado
    resultado.containers = dadosContainer;
    // Define a quantidade de containers lidos
    resultado.qtd = n;
    // Retorna o array de containers lidos
    return resultado;
}

// Procedimento de mergesort
void mergeContainers(Container *arr, Container *tmp, int l, int m, int r)
{
    // Índices iniciais para as duas metades e para o array temporário
    int i = l;
    int j = m + 1;
    int k = 0;

    // Mescla as duas metades em ordem
    while (i <= m && j <= r)
    {
        // Compara os códigos dos containers para ordenação
        if (strcmp(arr[i].codigo, arr[j].codigo) <= 0)
        {
            // Se o código do container da primeira metade for menor ou igual, copia para o array temporário
            tmp[k++] = arr[i++];
        } else {
            // Caso o codigo do container da segunda metade seja menor, copia para o array temporário
            tmp[k++] = arr[j++];
        }
    }
    // Copia os elementos restantes de ambas as metades, se houver
    while (i <= m) tmp[k++] = arr[i++];
    while (j <= r) tmp[k++] = arr[j++];

    // Copia os elementos do array temporário de volta para o array original
    for (int t = 0; t < k; ++t)
    {
        arr[l + t] = tmp[t];
    }
}

// Procedimento recursivo de mergesort
void mergesortContainersRec(Container *arr, Container *tmp, int l, int r)
{
    // Caso base da recursão
    if (l >= r) return;
    // Divide o array em duas metades
    int m = l + (r - l) / 2;
    // Chama recursivamente para ordenar as duas metades
    mergesortContainersRec(arr, tmp, l, m);
    mergesortContainersRec(arr, tmp, m + 1, r);
    // Mescla as duas metades ordenadas
    mergeContainers(arr, tmp, l, m, r);
}

// Procedimento principal de mergesort
void mergesortContainers(Container *arr, int n)
{
    // Verifica se o array tem mais de um elemento
    if (n <= 1) return;
    // Aloca memória temporária para a mesclagem
    Container *tmp = malloc(sizeof(Container) * n);
    // Verifica se a alocação foi bem-sucedida
    if (!tmp)
    {
        fprintf(stderr, "Erro de alocação no mergesort.\n");
        return;
    }
    // Chama o procedimento recursivo
    mergesortContainersRec(arr, tmp, 0, n - 1);
    // Libera a memória temporária
    free(tmp);
}

// Busca binária para encontrar um container pelo código
int buscarBinariaContainer(Container *dados, int n, const char *codigo)
{
    // Inicializa os índices de busca
    int menor = 0, maior = n - 1;
    // Realiza a busca binária
    while (menor <= maior)
    {
        // Calcula o índice médio
        int mid = menor + (maior - menor) / 2;
        // Compara o código do container médio com o código buscado
        int cmp = strcmp(dados[mid].codigo, codigo);
        // Ajusta os índices com base na comparação
        if (cmp == 0) return mid;
        // Se o código do container médio for menor, busca na metade direita
        if (cmp < 0) menor = mid + 1;
        // Se o código do container médio for maior, busca na metade esquerda
        else maior = mid - 1;
    }
    return -1;
}

// Procedimento para comparar CNPJs e escrever no arquivo de saída
void compararCNPJs(ContainerArray *originais, ContainerArray *observados, FILE* output)
{
    // Itera sobre os containers originais
    for (int i = 0; i < originais->qtd; i++)
    {
        // Busca o índice do container correspondente nos observados
        int idx = buscarBinariaContainer(observados->containers, observados->qtd, originais->containers[i].codigo);
        // Se encontrado, compara os CNPJs
        if (idx != -1)
        {
            // Se os CNPJs forem diferentes, escreve no arquivo de saída
            if (strcmp(originais->containers[i].cnpj, observados->containers[idx].cnpj) != 0)
            {
                fprintf(output, "%s:%s<->%s\n",  originais->containers[i].codigo,
                                                originais->containers[i].cnpj,
                                                observados->containers[idx].cnpj);
                // Marca que houve divergência de CNPJ
                originais->containers[i].temDivergenciaCNPJ = 1;
            }
        }
    }
}

// Procedimento para comparar pesos e escrever no arquivo de saída
void compararPesos(ContainerArray *originais, ContainerArray *observados, FILE* output)
{
    // Itera sobre os containers originais
    for (int i = 0; i < originais->qtd; i++)
    {
        int idx = buscarBinariaContainer(observados->containers, observados->qtd, originais->containers[i].codigo);
        // Se encontrado e não houve divergência de CNPJ, compara os pesos
        if (idx != -1 && !originais->containers[i].temDivergenciaCNPJ)
        {
            // Armazena os pesos original e observado
            int pesoOriginal = originais->containers[i].peso;
            int pesoObservado = observados->containers[idx].peso;
            // Calcula a diferença absoluta entre os pesos
            int diferenca = abs(pesoOriginal - pesoObservado);
            // Evita divisão por zero
            if (pesoOriginal == 0) {
                continue;
            }
            // Calcula o percentual da diferença
            double percentual = round((double)diferenca / (double)pesoOriginal * 100.0);

            // Se o percentual for maior que 10.0%, escreve no arquivo de saída
            if (percentual > 10.0)
            {
                fprintf(output, "%s:%dkg(%.0f%%)\n",  originais->containers[i].codigo,
                                                    diferenca,
                                                    round(percentual));
            }
        }
    }
}

// Função principal com argumentos de linha de comando
int main(int argc, char* argv[])
{
    // Verificação dos argumentos
    if (argc != 3)
    {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        printf("Exemplo: %s input.txt output.txt\n", argv[0]);
        return 1;
    }

    // Abrindo arquivos
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");

    // Leitura dos dados dos arquivos
    ContainerArray dadosOriginais = lerDados(input);
    ContainerArray dadosObservados = lerDados(input);

    // Ordena somente o array de observados
    if (dadosObservados.qtd > 0) mergesortContainers(dadosObservados.containers, dadosObservados.qtd);

    // Passando o arquivo de saída para as funções
    compararCNPJs(&dadosOriginais, &dadosObservados, output);
    compararPesos(&dadosOriginais, &dadosObservados, output);

    // Fechando arquivos
    fclose(input);
    fclose(output);

    // Liberando memória alocada
    free(dadosOriginais.containers);
    free(dadosObservados.containers);
    
    // Finalizando o programa
    return 0;
}