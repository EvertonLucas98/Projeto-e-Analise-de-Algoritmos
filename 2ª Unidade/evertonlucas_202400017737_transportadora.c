#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

/*
2
AAA1234 50 100
BBB5C67 2000 12000
5
AB111222333CD 49.99 2 1
EF444555666GH 5000.01 1234 7000
IJ777888999KL 100 49 10
MN000111222OP 65.01 3 125
QR333444555ST 200.01 13 4875

[AAA1234]R$100.00,49KG(98%),10L(10%)->IJ777888999KL
[BBB5C67]R$5265.03,1250KG(63%),12000L(100%)->EF444555666GH,MN000111222OP,QR333444555ST
PENDENTE:R$49.99,2KG,1L->AB111222333CD

5
TRK0001 500 100
TRK0A23 200 120
LOG1234 150 800
CMA9999 350 200
XPT4567 400 600
35
AB111222333CD 49.99 2 1
EF444555666GH 5000.01 123 700
IJ777888999KL 100.00 49 10
MN000111222OP 65.01 3 125
QR333444555ST 200.01 13 48
UV999888777WX 9.99 1 1
YZ123456789AB 249.50 15 20
CD987654321EF 1200.00 95 400
GH135791113KL 15.75 5 2
LM246802468NO 75.00 10 8
OP112233445QR 999.99 80 350
ST556677889UV 0.99 1 1
WX667788990YZ 350.20 30 15
ZA101010101BC 15.00 7 4
BC202020202DE 45.45 20 15
DE303030303FG 2500.00 180 900
FG404040404HI 5.50 2 1
HI505050505JK 130.30 60 30
JK606060606LM 799.99 65 32
LM707070707NO 22.22 9 6
NO808080808PQ 410.00 35 18
PQ909090909RS 60.00 12 9
RS121212121TU 19.95 4 3
TU131313131VW 275.75 220 110
VW141414141XY 1500.50 12 60
XY151515151ZA 5.00 1 1
ZA161616161BC 89.90 18 14
BC171717171DE 49.00 11 7
DE181818181FG 600.00 48 240
FG191919191HI 39.99 6 5
HI202020202JK 999.00 90 42
JK212121212LM 12.34 3 2
LM222222222NO 450.45 40 20
NO232323232PQ 30.00 8 6
*/

typedef struct {
    char codigo[14];
    double valor;
    uint32_t peso, volume;
} Item;

typedef struct {
    Item *itens;
    uint32_t qtd;
    double valorMaximo;
} ItemArray;

typedef struct {
    char placa[8];
    uint32_t peso, volume;
} Veiculo;

typedef struct {
    Veiculo *veiculos;
    uint32_t qtd;
} VeiculosArray;

// Matriz 3D global para uso na programação dinâmica
double ***Matriz3d = NULL;

// Função que aloca uma matriz 3D de doubles
double*** alocarMatriz3D(uint32_t qtdItens, uint32_t capPeso, uint32_t capVolume)
{
    // Alocação da matriz 3D
    double ***matriz3D = (double ***) malloc((qtdItens + 1) * sizeof(double **));
    for (uint32_t i = 0; i <= qtdItens; i++)
    {
        matriz3D[i] = (double **) malloc((capPeso + 1) * sizeof(double *));
        for (uint32_t j = 0; j <= capPeso; j++)
            matriz3D[i][j] = (double *) malloc((capVolume + 1) * sizeof(double));
    }
    return matriz3D;
}

// Função que preenche a tabela 3D para o problema da mochila
void preencherTabela3D(double ***matriz3D, Item *itens, uint32_t qtdItens, uint32_t capPeso, uint32_t capVolume)
{
    for (uint32_t i = 1; i <= qtdItens; i++)
    {
        const uint32_t index = i - 1; // Índice do item atual
        const uint32_t pesoItem = itens[index].peso;
        const uint32_t volumeItem = itens[index].volume;
        const double valorItem = itens[index].valor;
        
        // Otimização: cache de ponteiros para as linhas atuais e anteriores
        double **linhaAtual = matriz3D[i];
        double **linhaAnterior = matriz3D[i - 1];
        
        // Otimização: loop aninhado reorganizado para melhor localidade de cache
        for (uint32_t pesoAtual = 1; pesoAtual <= capPeso; pesoAtual++)
        {
            double *colunaAtual = linhaAtual[pesoAtual];
            double *colunaAnterior = linhaAnterior[pesoAtual];
            
            // Cálculo prévio para evitar operações repetidas
            const int pesoRestante = pesoAtual - pesoItem;
            const bool podeIncluirPeso = (pesoRestante >= 0);
            
            for (uint32_t volumeAtual = 1; volumeAtual <= capVolume; volumeAtual++)
            {
                // Otimização: condições simplificadas e cálculo direto
                if (podeIncluirPeso && (volumeItem <= volumeAtual))
                {
                    const double comItem = linhaAnterior[pesoRestante][volumeAtual - volumeItem] + valorItem;
                    colunaAtual[volumeAtual] = (comItem > colunaAnterior[volumeAtual]) ? comItem : colunaAnterior[volumeAtual];
                }
                else
                {
                    colunaAtual[volumeAtual] = colunaAnterior[volumeAtual];
                }
            }
        }
    }
}

// Função que resolve o problema da mochila usando programação dinâmica
ItemArray mochila3D(Item *itens, uint32_t qtdItens, uint32_t capPeso, uint32_t capVolume)
{
    // Alocação das camadas da matriz 3D
    Matriz3d = alocarMatriz3D(qtdItens, capPeso, capVolume);
    // Preenche a tabela 3D
    preencherTabela3D(Matriz3d, itens, qtdItens, capPeso, capVolume);
    
    ItemArray itensAdd;
    itensAdd.itens = (Item *)malloc(qtdItens * sizeof(Item));
    itensAdd.qtd = 0;
    uint32_t temp = qtdItens;
    int auxPeso = capPeso;
    int auxVolume = capVolume;
    // O resultado final está no canto da última camada da tabela 2D.
    itensAdd.valorMaximo = Matriz3d[qtdItens][capPeso][capVolume];
    
    // Rastreia os itens incluídos na solução ótima
    while (temp > 0)
    {
        // Verifica se os pesos e volumes auxiliares são válidos
        if (auxPeso < 0 || auxVolume < 0)
            break;
        // Inclui o item se o valor atual for diferente do valor na linha anterior
        if (Matriz3d[temp][auxPeso][auxVolume] != Matriz3d[temp-1][auxPeso][auxVolume])
        {
            // Adiciona o item ao array de itens adicionados, sem substituir o anterior
            itensAdd.itens[itensAdd.qtd++] = itens[temp-1];
            auxPeso -= itens[temp-1].peso;
            auxVolume -= itens[temp-1].volume;
            temp--;
        }
        else
        {
            temp--;
        }
    }

    return itensAdd;
}

// Procedimento para realizar um somatório
void somatorio(Item *itens, uint32_t qtdItens, uint32_t *somatorioPeso, uint32_t *somatorioVolume)
{
    for (uint32_t i = 0; i < qtdItens; i++)
    {
        *somatorioPeso += itens[i].peso;
        *somatorioVolume += itens[i].volume;
    }
}

// Função para calcular porcentagens
uint32_t calcularPorcentagens(uint32_t valorParcial, uint32_t valorTotal)
{
    if (valorTotal == 0) return 0;
    return round(((double)valorParcial / (double)valorTotal) * 100.0);
}

// Procedimento para escrever a saída no arquivo
void escreverOutput(FILE* output, char placa[], double valorMaximo, uint32_t somaPeso, uint32_t porcentagemPeso, uint32_t somaVolume, uint32_t porcentagemVolume, ItemArray itensAdd)
{
    fprintf(output, "[%s]:%.2lf,%dKG(%d%%),%dL(%d%%)->", placa, valorMaximo, somaPeso, porcentagemPeso, somaVolume, porcentagemVolume);
    if (valorMaximo == 0)
        fprintf(output, "\n");
    else
    {
        for (int i = itensAdd.qtd - 1; i >= 0; i--)
        {
            if (i == 0)
            fprintf(output, "%s\n", itensAdd.itens[i].codigo);
            else
            fprintf(output, "%s,", itensAdd.itens[i].codigo);
        }
    }
}

// Função para processar os dados e gerar a saída
void processarDados(VeiculosArray veiculos, ItemArray itens, FILE* output)
{
    for (uint32_t i = 0; i < veiculos.qtd; i++)
    {
        // Resolvendo o problema da mochila para o veículo atual
        ItemArray itensAdd = mochila3D(itens.itens, itens.qtd, veiculos.veiculos[i].peso, veiculos.veiculos[i].volume);
        
        // Cálculo do peso e volume total dos itens adicionados
        uint32_t somaPeso=0, somaVolume=0;
        somatorio(itensAdd.itens, itensAdd.qtd, &somaPeso, &somaVolume);
        
        // Calculo das porcentagens
        const uint32_t porcentagemPeso = calcularPorcentagens(somaPeso, veiculos.veiculos[i].peso);
        const uint32_t porcentagemVolume = calcularPorcentagens(somaVolume, veiculos.veiculos[i].volume);
        
        // Escrita dos dados no arquivo de saída
        escreverOutput(output, veiculos.veiculos[i].placa, itensAdd.valorMaximo, somaPeso, porcentagemPeso, somaVolume, porcentagemVolume, itensAdd);
        
        // Removendo os itens alocados da lista de itens disponíveis
        // removerItens(&itens, &itensAdd);
        for (uint32_t j = 0; j < itensAdd.qtd; j++)
        {
            for (uint32_t k = 0; k < itens.qtd; k++)
            {
                if (strcmp(itensAdd.itens[j].codigo, itens.itens[k].codigo) == 0)
                {
                    // Move os vizinhos para preencher o espaço do item removido
                    for (uint32_t l = k; l < itens.qtd - 1; l++)
                    {
                        itens.itens[l] = itens.itens[l + 1];
                    }
                    itens.qtd--;
                    break;
                }
            }
        }
        
        free(itensAdd.itens);
    }
    
    // Itens pendentes
    if (itens.qtd > 0)
    {
        double somaValor = 0.0;
        uint32_t somaPeso = 0;
        uint32_t somaVolume = 0;
        for (uint32_t i = 0; i < itens.qtd; i++)
        {
            somaValor += itens.itens[i].valor;
            somaPeso += itens.itens[i].peso;
            somaVolume += itens.itens[i].volume;
        }
        
        fprintf(output, "PENDENTE:R$%.2f,%dKG,%dL->", somaValor, somaPeso, somaVolume);
        if (itens.qtd == 0)
            fprintf(output, "\n");
        else
            for (uint32_t i = 0; i < itens.qtd; i++)
            {
                fprintf(output, "%s", itens.itens[i].codigo);
                if (i < itens.qtd - 2) fprintf(output, ",");
            }
    }
}

// Função para ler os dados dos veículos
VeiculosArray lerDadosVeiculo(FILE* arquivo)
{
    // Inicializa o array
    VeiculosArray veiculos = {NULL, 0};
    // Lê a quantidade de containers
    uint32_t n;

    fscanf(arquivo, "%d", &n); // Lê o número de veiculos

    // Alocação dinâmica de memória para os veiculos
    Veiculo *dadosVeiculo = malloc(sizeof(Veiculo) * n);
    // Verifica se a alocação foi bem-sucedida
    if (!dadosVeiculo)
    {
        printf("Erro de alocação.\n");
        return veiculos;
    }

    // Leitura dos dados dos veiculos
    for (uint32_t i = 0; i < n; ++i)
        fscanf(arquivo, "%s %d %d", dadosVeiculo[i].placa, &dadosVeiculo[i].peso, &dadosVeiculo[i].volume);
    
    veiculos.veiculos = dadosVeiculo;
    veiculos.qtd = n;

    return veiculos;
}

// Função para ler os dados dos itens
ItemArray lerDadosItem(FILE* arquivo)
{
    // Inicializa o array
    ItemArray itens = {NULL, 0, 0.0};
    // Lê a quantidade de containers
    uint32_t n;
    fscanf(arquivo, "%d", &n); // Lê o número de veiculos

    // Alocação dinâmica de memória para os veiculos
    Item *dadosItem = malloc(sizeof(Item) * n);
    // Verifica se a alocação foi bem-sucedida
    if (!dadosItem)
    {
        printf("Erro de alocação.\n");
        return itens;
    }

    // Leitura dos dados dos veiculos
    for (uint32_t i = 0; i < n; ++i)
        fscanf(arquivo, "%s %lf %d %d", dadosItem[i].codigo, &dadosItem[i].valor, &dadosItem[i].peso, &dadosItem[i].volume);
    
    itens.itens = dadosItem;
    itens.qtd = n;

    return itens;
}

int main(int argc, char *argv[]) {
    // Medição de tempo de execução
    clock_t start = clock();
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
    VeiculosArray dadosVeiculos = lerDadosVeiculo(input);
    ItemArray dadosItens = lerDadosItem(input);

    // Processamento dos dados e escrita no arquivo de saída
    processarDados(dadosVeiculos, dadosItens, output);
    
    // Medição de tempo de execução
    clock_t end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTempo de execucao: %.6f segundos\n\n", cpu_time_used);
    fprintf(output, "\nTempo de execucao: %.6f segundos\n", cpu_time_used);

    // Fechando arquivos
    fclose(input);
    fclose(output);
    
    // Liberando memória alocada
    free(dadosVeiculos.veiculos);
    free(dadosItens.itens);
    for (uint32_t i = 0; i <= dadosItens.qtd; i++)
    {
        for (uint32_t j = 0; j <= dadosVeiculos.veiculos[i % dadosVeiculos.qtd].peso; j++)
            free(Matriz3d[i][j]);
        free(Matriz3d[i]);
    }
    free(Matriz3d);

    return 0;
}