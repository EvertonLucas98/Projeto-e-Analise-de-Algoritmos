#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct {
    char codigo[14];
    double valor;
    int peso, volume;
} Item;

typedef struct {
    Item *itens;
    int qtd;
    double valorMaximo;
} ItemArray;

typedef struct {
    char placa[8];
    int peso, volume;
} Veiculo;

typedef struct {
    Veiculo *veiculos;
    int qtd;
} VeiculosArray;

double ***Matriz3d = NULL;

// Função que retorna o maior valor entre dois valores
double max(double a, double b) {
    return (a > b) ? a : b;
}

// Função que aloca uma matriz 3D de doubles
double*** alocarMatriz3D(int qtdItens, int capPeso, int capVolume)
{
    // Alocação da matriz 3D
    double ***matriz3D = (double ***) malloc((qtdItens + 1) * sizeof(double **));
    for (int i = 0; i <= qtdItens; i++)
    {
        matriz3D[i] = (double **) malloc((capPeso + 1) * sizeof(double *));
        for (int j = 0; j <= capPeso; j++)
            matriz3D[i][j] = (double *) malloc((capVolume + 1) * sizeof(double));
    }
    return matriz3D;
}

// Função que preenche a tabela 3D para o problema da mochila
void preencherTabela3D(double ***matriz3D, Item *itens, int qtdItens, int capPeso, int capVolume)
{
    // Preenche o restante da tabela
    for (int i = 0; i <= qtdItens; i++)
    {
        for (int j = 0; j <= capPeso; j++)
        {
            for (int k = 0; k <= capVolume; k++)
            {
                // Se não há itens ou a capacidade é zero, o valor máximo é zero
                if (i == 0 || j == 0 || k == 0)
                    matriz3D[i][j][k] = 0;
                else if (itens[i-1].peso <= j && itens[i-1].volume <= k)
                {
                    matriz3D[i][j][k] = max(matriz3D[i-1][j][k], matriz3D[i-1][j - itens[i-1].peso][k - itens[i-1].volume] + itens[i-1].valor);
                }
                else
                    matriz3D[i][j][k] = matriz3D[i-1][j][k];
            }
        }
    }
}

// Função que resolve o problema da mochila usando programação dinâmica
ItemArray mochila3D(Item *itens, int qtdItens, int capPeso, int capVolume)
{
    // Alocação das camadas da matriz 3D
    Matriz3d = alocarMatriz3D(qtdItens, capPeso, capVolume);
    // Preenche a tabela 3D
    preencherTabela3D(Matriz3d, itens, qtdItens, capPeso, capVolume);
    
    ItemArray itensAdd;
    itensAdd.itens = (Item *)malloc(qtdItens * sizeof(Item));
    itensAdd.qtd = 0;
    int temp = qtdItens;
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
void somatorio(Item *itens, int qtdItens, int *somatorioPeso, int *somatorioVolume)
{
    for (int i = 0; i < qtdItens; i++)
    {
        *somatorioPeso += itens[i].peso;
        *somatorioVolume += itens[i].volume;
    }
}

// Função para calcular porcentagens
int calcularPorcentagens(int valorParcial, int valorTotal)
{
    if (valorTotal == 0) return 0;
    return round(((double)valorParcial / (double)valorTotal) * 100.0);
}

// Procedimento para escrever a saída no arquivo
void escreverOutput(FILE* output, char placa[], double valorMaximo, int somaPeso, int porcentagemPeso, int somaVolume, int porcentagemVolume, ItemArray itensAdd)
{
    fprintf(output, "[%s]:%.2lf,%d(%d%%),%dL(%d%%)->", placa, valorMaximo, somaPeso, porcentagemPeso, somaVolume, porcentagemVolume);
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
    for (int i = 0; i < veiculos.qtd; i++)
    {
        // Resolvendo o problema da mochila para o veículo atual
        ItemArray itensAdd = mochila3D(itens.itens, itens.qtd, veiculos.veiculos[i].peso, veiculos.veiculos[i].volume);
        
        // Cálculo do peso e volume total dos itens adicionados
        int somaPeso=0, somaVolume=0;
        somatorio(itensAdd.itens, itensAdd.qtd, &somaPeso, &somaVolume);
        
        // Calculo das porcentagens
        int porcentagemPeso = calcularPorcentagens(somaPeso, veiculos.veiculos[i].peso);
        int porcentagemVolume = calcularPorcentagens(somaVolume, veiculos.veiculos[i].volume);
        
        // Escrita dos dados no arquivo de saída
        escreverOutput(output, veiculos.veiculos[i].placa, itensAdd.valorMaximo, somaPeso, porcentagemPeso, somaVolume, porcentagemVolume, itensAdd);
        
        // Removendo os itens alocados da lista de itens disponíveis
        // removerItens(&itens, &itensAdd);
        for (int j = 0; j < itensAdd.qtd; j++)
        {
            for (int k = 0; k < itens.qtd; k++)
            {
                if (strcmp(itensAdd.itens[j].codigo, itens.itens[k].codigo) == 0)
                {
                    // Move os vizinhos para preencher o espaço do item removido
                    for (int l = k; l < itens.qtd - 1; l++)
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
        int somaPeso = 0;
        int somaVolume = 0;
        for (int i = 0; i < itens.qtd; i++)
        {
            somaValor += itens.itens[i].valor;
            somaPeso += itens.itens[i].peso;
            somaVolume += itens.itens[i].volume;
        }
        
        fprintf(output, "PENDENTE:R$%.2f,%dKG,%dL->", somaValor, somaPeso, somaVolume);
        if (itens.qtd == 0)
            fprintf(output, "\n");
        else
            for (int i = 0; i < itens.qtd; i++)
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
    int n;

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
    for (int i = 0; i < n; ++i)
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
    int n;
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
    for (int i = 0; i < n; ++i)
        fscanf(arquivo, "%s %lf %d %d", dadosItem[i].codigo, &dadosItem[i].valor, &dadosItem[i].peso, &dadosItem[i].volume);
    
    itens.itens = dadosItem;
    itens.qtd = n;

    return itens;
}

int main(int argc, char *argv[]) {
    // Medição do tempo de execução (início)
    clock_t inicio = clock();
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
    
    // Medição do tempo de execução (fim)
    clock_t fim = clock();
    double tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %.6f segundos\n", tempoExecucao);
    
    // Fechando arquivos
    fclose(input);
    fclose(output);
    
    // Liberando memória alocada
    free(dadosVeiculos.veiculos);
    free(dadosItens.itens);
    for (int i = 0; i <= dadosItens.qtd; i++)
    {
        for (int j = 0; j <= dadosVeiculos.veiculos[i % dadosVeiculos.qtd].peso; j++)
            free(Matriz3d[i][j]);
        free(Matriz3d[i]);
    }
    free(Matriz3d);

    return 0;
}