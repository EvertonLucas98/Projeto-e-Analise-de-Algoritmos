#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    char codigo[14];
    float valor;
    uint32_t peso, volume;
    bool usado;
} Item;

typedef struct {
    Item *itens;
    uint32_t qtd;
    float valorMaximo;
} ItemArray;

typedef struct {
    char placa[16];
    uint32_t peso, volume;
} Veiculo;

typedef struct {
    Veiculo *veiculos;
    uint32_t qtd;
} VeiculosArray;

void liberarMatriz3D(float ***m, uint32_t qtdItens, uint32_t capPeso)
{
    for (uint32_t i = 0; i <= qtdItens; ++i)
    {
        for (uint32_t j = 0; j <= capPeso; ++j)
            free(m[i][j]);
        free(m[i]);
    }
    free(m);
}

// Aloca matriz 3D
float*** alocarMatriz3D(uint32_t qtdItens, uint32_t capPeso, uint32_t capVolume)
{
    float ***matriz3D = NULL;
    matriz3D = (float ***) malloc((qtdItens + 1) * sizeof(float **));
    if (!matriz3D) return NULL;

    for (uint32_t i = 0; i <= qtdItens; ++i)
    {
        matriz3D[i] = (float **) malloc((capPeso + 1) * sizeof(float *));
        for (uint32_t j = 0; j <= capPeso; ++j)
            matriz3D[i][j] = (float *) calloc((capVolume + 1), sizeof(float));
    }

    return matriz3D;
}

// Preenche Tabela 3D
void preencherTabela3D(float ***matriz3D, Item *itens, uint32_t qtdItens, uint32_t capPeso, uint32_t capVolume)
{
    // Já inicializado com zeros via calloc
    for (uint32_t i = 1; i <= qtdItens; ++i)
    {
        const uint32_t idx = i - 1;
        const uint32_t pesoItem = itens[idx].peso;
        const uint32_t volumeItem = itens[idx].volume;
        const float valorItem = itens[idx].valor;

        float **camadaAtual = matriz3D[i];
        float **camadaPrev = matriz3D[i - 1];

        for (uint32_t p = 0; p <= capPeso; ++p)
        {
            float *colAtual = camadaAtual[p];
            float *colPrev = camadaPrev[p];

            int pesoRestanteBase = (int)p - (int)pesoItem;
            bool podeIncluirBase = (pesoRestanteBase >= 0);

            for (uint32_t v = 0; v <= capVolume; ++v)
            {
                float melhor = colPrev[v]; // sem incluir
                if (podeIncluirBase && volumeItem <= v)
                {
                    float comItem = camadaPrev[pesoRestanteBase][v - volumeItem] + valorItem;
                    if (comItem > melhor) melhor = comItem;
                }
                colAtual[v] = melhor;
            }
        }
    }
}

// Backtracking: marca itens usados e retorna valor máximo
float backtracking(ItemArray itens, uint32_t qtdItens, uint32_t capPeso, uint32_t capVolume)
{
    // Aloca matriz Principal 3D
    float ***matriz3D = alocarMatriz3D(qtdItens, capPeso, capVolume);
    // Preenche tabela para os itens remanescentes, com capacidades maxPeso x maxVolume
    preencherTabela3D(matriz3D, itens.itens, qtdItens, capPeso, capVolume);
    if (qtdItens == 0) return 0.0f;
    int auxPeso = (int)capPeso;
    int auxVolume = (int)capVolume;
    uint32_t temp = qtdItens;

    float valorMaximo = matriz3D[qtdItens][capPeso][capVolume];

    while (temp > 0)
    {
        // Se auxPeso/auxVolume forem negativos, não há mais itens a considerar
        if (auxPeso < 0 || auxVolume < 0) break;

        if (matriz3D[temp][auxPeso][auxVolume] != matriz3D[temp-1][auxPeso][auxVolume])
        {
            // marca e imprime antes de decrementar temp
            itens.itens[temp-1].usado = true;
            auxPeso -= (int)itens.itens[temp-1].peso;
            auxVolume -= (int)itens.itens[temp-1].volume;
            temp--;
        }
        else
        {
            temp--;
        }
    }
    // Libera matriz 3D
    liberarMatriz3D(matriz3D, qtdItens, capPeso);

    return valorMaximo;
}

// Procedimento para calcular somatórios de peso e volume dos itens usados
void somatorio(Item *itens, uint32_t qtdItens, uint32_t *somatorioPeso, uint32_t *somatorioVolume, uint32_t capPeso, uint32_t capVolume)
{
    for (uint32_t i = 0; i < qtdItens; ++i)
    {
        if (itens[i].usado && *somatorioPeso+itens[i].peso <= capPeso && *somatorioVolume+itens[i].volume <= capVolume)
        {
            *somatorioPeso += itens[i].peso;
            *somatorioVolume += itens[i].volume;
        }
    }
}

// Função para calcular porcentagens
uint32_t calcularPorcentagens(uint32_t valorParcial, uint32_t valorTotal)
{
    if (valorTotal == 0) return 0;
    return (uint32_t) round(((float)valorParcial / (float)valorTotal) * 100.0f);
}

// Procedimento para escrever saída formatada
void escreverOutput(FILE* output, char placa[], float valorMaximo, uint32_t somaPeso, uint32_t porcentagemPeso, uint32_t somaVolume, uint32_t porcentagemVolume, ItemArray itens)
{
    fprintf(output, "[%s]R$%.2f,%dKG(%d%%),%dL(%d%%)->", placa, valorMaximo, somaPeso, porcentagemPeso, somaVolume, porcentagemVolume);

    bool primeiro = true;
    // Imprime códigos na ordem do primeiro adicionado ao último (aqui: do índice 0..qtd-1)
    for (uint32_t i = 0; i < itens.qtd; ++i)
    {
        if (itens.itens[i].usado)
        {
            if (!primeiro) fprintf(output, ",");
            fprintf(output, "%s", itens.itens[i].codigo);
            primeiro = false;
        }
    }
    fprintf(output, "\n");
}

// Função para remover itens usados do array e retorna a nova quantidade
uint32_t removerItensUsados(Item *itens, uint32_t qtd)
{
    uint32_t write = 0;
    for (uint32_t i = 0; i < qtd; ++i)
    {
        if (!itens[i].usado)
        {
            if (write != i) itens[write] = itens[i];
            write++;
        }
    }
    
    return write;
}

// Procedimento principal de processamento
void processarDados(VeiculosArray veiculos, ItemArray itens, FILE* output)
{
    for (uint32_t i = 0; i < veiculos.qtd; i++)
    {
        uint32_t capP = veiculos.veiculos[i].peso;
        uint32_t capV = veiculos.veiculos[i].volume;
        float valorMaximo = backtracking(itens, itens.qtd, capP, capV);
        uint32_t somaPeso = 0, somaVolume = 0;
        somatorio(itens.itens, itens.qtd, &somaPeso, &somaVolume, capP, capV);

        uint32_t porcentagemPeso = calcularPorcentagens(somaPeso, capP);
        uint32_t porcentagemVolume = calcularPorcentagens(somaVolume, capV);

        ItemArray tmpArr = { itens.itens, itens.qtd, 0.0f };
        escreverOutput(output, veiculos.veiculos[i].placa, valorMaximo, somaPeso, porcentagemPeso, somaVolume, porcentagemVolume, tmpArr);

        // Remove itens usados (compacta array). Atualiza qtdItens.
        uint32_t novaQtd = removerItensUsados(itens.itens, itens.qtd);
        itens.qtd = novaQtd;
    }

    // Imprime pendente se houver
    if (itens.qtd > 0)
    {
        float somaValor = 0.0f;
        uint32_t somaPeso = 0, somaVolume = 0;
        for (uint32_t i = 0; i < itens.qtd; ++i)
        {
            somaValor += itens.itens[i].valor;
            somaPeso += itens.itens[i].peso;
            somaVolume += itens.itens[i].volume;
        }

        fprintf(output, "PENDENTE:R$%.2f,%dKG,%dL->", somaValor, somaPeso, somaVolume);
        for (uint32_t i = 0; i < itens.qtd; ++i)
        {
            fprintf(output, "%s", itens.itens[i].codigo);
            if (i + 1 < itens.qtd) fprintf(output, ",");
        }
        fprintf(output, "\n");
    }
}

VeiculosArray lerDadosVeiculo(FILE* arquivo)
{
    VeiculosArray veiculos = {NULL, 0};
    uint32_t n;

    if (fscanf(arquivo, "%u", &n) != 1) return veiculos;

    Veiculo *dadosVeiculo = malloc(sizeof(Veiculo) * n);
    if (!dadosVeiculo) return veiculos;

    for (uint32_t i = 0; i < n; ++i)
        fscanf(arquivo, "%15s %u %u", dadosVeiculo[i].placa, &dadosVeiculo[i].peso, &dadosVeiculo[i].volume);

    veiculos.veiculos = dadosVeiculo;
    veiculos.qtd = n;
    return veiculos;
}

ItemArray lerDadosItem(FILE* arquivo)
{
    ItemArray itens = {NULL, 0, 0.0f};
    uint32_t n;

    if (fscanf(arquivo, "%u", &n) != 1) return itens;

    Item *dadosItem = malloc(sizeof(Item) * n);
    if (!dadosItem) return itens;

    for (uint32_t i = 0; i < n; ++i)
    {
        fscanf(arquivo, "%13s %f %u %u", dadosItem[i].codigo, &dadosItem[i].valor, &dadosItem[i].peso, &dadosItem[i].volume);
        dadosItem[i].usado = false;
    }

    itens.itens = dadosItem;
    itens.qtd = n;
    return itens;
}

int main(int argc, char *argv[])
{
    clock_t start = clock();
    // Verificação de argumentos
    if (argc != 3)
    {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Abrindo os arquivos
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");

    // Lendo os dados
    VeiculosArray dadosVeiculos = lerDadosVeiculo(input);
    ItemArray dadosItens = lerDadosItem(input);

    // Processando os dados
    processarDados(dadosVeiculos, dadosItens, output);
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %.2f segundos\n", time_spent);

    // Fechando os arquivos
    fclose(input);
    fclose(output);

    // Liberando a memória
    free(dadosVeiculos.veiculos);
    free(dadosItens.itens);

    return 0;
}
