def comparar_arquivos(arquivo1, arquivo2):
    """Compara dois arquivos linha a linha e mostra as diferenças."""

    with open(arquivo1, 'r', encoding='utf-8') as f1, \
         open(arquivo2, 'r', encoding='utf-8') as f2:
        
        diferencas = []
        linha_num = 0
        
        while True:
            linha_num += 1
            linha1 = f1.readline()
            linha2 = f2.readline()
            
            # Ambos arquivos terminaram
            if not linha1 and not linha2:
                break
            
            if linha1 != linha2:
                conteudo1 = linha1.rstrip('\n') if linha1 else "[FIM DO ARQUIVO]"
                conteudo2 = linha2.rstrip('\n') if linha2 else "[FIM DO ARQUIVO]"
                diferencas.append((linha_num, conteudo1, conteudo2))
        print("Quantidade de linhas lidas: ", linha_num)
        if not diferencas:
            print("Os arquivos são IGUAIS!")
            return True
        else:
            print(f"Os arquivos são DIFERENTES!\nExiste {len(diferencas)} linha(s) diferente(s)!")
            print("=" * 80)
            # for num, c1, c2 in diferencas:
            #     print(f"Linha {num}:")
            #     print(f"  Arquivo 1: {c1}")
            #     print(f"  Arquivo 2: {c2}")
                # print("-" * 80)
            return False

# Exemplo de uso
comparar_arquivos("arq1.txt", "arq2.txt")