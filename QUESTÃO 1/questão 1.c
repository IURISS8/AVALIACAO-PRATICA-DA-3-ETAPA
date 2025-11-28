#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Conta {
    int numero;
    char titular[50];
    float saldo;
};

const char NOME_ARQUIVO[] = "contas.dat";
const char SENHA_ADMIN[]  = "0000";

// ======================= Func�es auxiliares =======================

void limparEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void inicializarSistema(void) {
    FILE *arq = fopen(NOME_ARQUIVO, "ab");
    if (arq != NULL) {
        fclose(arq);
    }
}

int gerarNumeroContaSequencial(void) {
    FILE *arq = fopen(NOME_ARQUIVO, "rb");
    struct Conta contaTemp;
    int maiorNumero = 0;

    if (arq == NULL) {
        return 1;
    }

    while (fread(&contaTemp, sizeof(struct Conta), 1, arq) == 1) {
        if (contaTemp.numero > maiorNumero) {
            maiorNumero = contaTemp.numero;
        }
    }

    fclose(arq);
    return maiorNumero + 1;
}

// ======================= Opera��es =======================

// 1) Abrir nova conta
void abrirConta(void) {
    FILE *arq;
    struct Conta nova;

    printf("\n=== ABERTURA DE CONTA ===\n");

    nova.numero = gerarNumeroContaSequencial();
    nova.saldo  = 0.0f;

    printf("Numero gerado para a conta: %d\n", nova.numero);

    printf("Informe o nome do titular: ");
    limparEntrada();
    fgets(nova.titular, sizeof(nova.titular), stdin);

    // remover quebra de linha
    size_t tamanho = strlen(nova.titular);
    if (tamanho > 0 && nova.titular[tamanho - 1] == '\n') {
        nova.titular[tamanho - 1] = '\0';
    }

    arq = fopen(NOME_ARQUIVO, "ab");
    if (arq == NULL) {
        printf("Falha ao abrir o arquivo de contas.\n");
        return;
    }

    fwrite(&nova, sizeof(struct Conta), 1, arq);
    fclose(arq);

    printf("Conta criada com sucesso para %s!\n", nova.titular);
}

// 2) Ver dados / extrato simples
void consultarConta(void) {
    FILE *arq;
    struct Conta conta;
    int numeroBuscado;
    int achou = 0;

    printf("\n=== CONSULTA DE CONTA ===\n");
    printf("Digite o numero da conta: ");
    scanf("%d", &numeroBuscado);

    arq = fopen(NOME_ARQUIVO, "rb");
    if (arq == NULL) {
        printf("Nao existem contas cadastradas ainda.\n");
        return;
    }

    while (fread(&conta, sizeof(struct Conta), 1, arq) == 1) {
        if (conta.numero == numeroBuscado) {
            achou = 1;
            printf("\nTitular : %s\n", conta.titular);
            printf("Conta   : %d\n", conta.numero);
            printf("Saldo   : R$ %.2f\n", conta.saldo);
            break;
        }
    }

    if (!achou) {
        printf("Conta nao localizada.\n");
    }

    fclose(arq);
}

// 3) Dep�sito
void realizarDeposito(void) {
    FILE *arq;
    struct Conta conta;
    int numeroDeposito;
    float valor;
    int indiceRegistro = 0;
    int localizado = 0;

    printf("\n=== DEPOSITO EM CONTA ===\n");
    printf("Numero da conta para deposito: ");
    scanf("%d", &numeroDeposito);

    printf("Valor a depositar: ");
    scanf("%f", &valor);

    if (valor <= 0) {
        printf("Valor invalido. Operacao cancelada.\n");
        return;
    }

    arq = fopen(NOME_ARQUIVO, "rb+");
    if (arq == NULL) {
        printf("Nenhuma conta encontrada para realizar deposito.\n");
        return;
    }

    while (fread(&conta, sizeof(struct Conta), 1, arq) == 1) {
        if (conta.numero == numeroDeposito) {
            localizado = 1;
            conta.saldo += valor;

            long pos = indiceRegistro * (long)sizeof(struct Conta);
            fseek(arq, pos, SEEK_SET);
            fwrite(&conta, sizeof(struct Conta), 1, arq);

            printf("Deposito concluido com sucesso!\n");
            printf("Saldo atual da conta %d: R$ %.2f\n", conta.numero, conta.saldo);
            break;
        }
        indiceRegistro++;
    }

    if (!localizado) {
        printf("Conta nao encontrada para deposito.\n");
    }

    fclose(arq);
}

// 4) Saque
void realizarSaque(void) {
    FILE *arq;
    struct Conta conta;
    int numeroSaque;
    float valor;
    int indiceRegistro = 0;
    int localizado = 0;

    printf("\n=== SAQUE EM CONTA ===\n");
    printf("Numero da conta para saque: ");
    scanf("%d", &numeroSaque);

    printf("Valor a sacar: ");
    scanf("%f", &valor);

    if (valor <= 0) {
        printf("Valor invalido. Operacao cancelada.\n");
        return;
    }

    arq = fopen(NOME_ARQUIVO, "rb+");
    if (arq == NULL) {
        printf("Nenhuma conta encontrada para saque.\n");
        return;
    }

    while (fread(&conta, sizeof(struct Conta), 1, arq) == 1) {
        if (conta.numero == numeroSaque) {
            localizado = 1;

            if (conta.saldo < valor) {
                printf("Saldo insuficiente. Saldo atual: R$ %.2f\n", conta.saldo);
                fclose(arq);
                return;
            }

            conta.saldo -= valor;

            long pos = indiceRegistro * (long)sizeof(struct Conta);
            fseek(arq, pos, SEEK_SET);
            fwrite(&conta, sizeof(struct Conta), 1, arq);

            printf("Saque efetuado com sucesso!\n");
            printf("Saldo atual da conta %d: R$ %.2f\n", conta.numero, conta.saldo);
            break;
        }
        indiceRegistro++;
    }

    if (!localizado) {
        printf("Conta nao encontrada para saque.\n");
    }

    fclose(arq);
}

// 5) Transfer�ncia entre contas
void realizarTransferencia(void) {
    FILE *arq;
    struct Conta conta;
    struct Conta origem;
    struct Conta destino;
    int numeroOrigem, numeroDestino;
    float valor;
    int indice = 0;
    int indiceOrigem = -1;
    int indiceDestino = -1;
    int achouOrigem = 0;
    int achouDestino = 0;

    printf("\n=== TRANSFERENCIA ENTRE CONTAS ===\n");
    printf("Conta de ORIGEM : ");
    scanf("%d", &numeroOrigem);
    printf("Conta de DESTINO: ");
    scanf("%d", &numeroDestino);

    if (numeroOrigem == numeroDestino) {
        printf("A conta de origem deve ser diferente da conta de destino.\n");
        return;
    }

    printf("Valor da transferencia: ");
    scanf("%f", &valor);

    if (valor <= 0) {
        printf("Valor invalido. Operacao cancelada.\n");
        return;
    }

    arq = fopen(NOME_ARQUIVO, "rb+");
    if (arq == NULL) {
        printf("Nao ha contas cadastradas para transferencia.\n");
        return;
    }

    while (fread(&conta, sizeof(struct Conta), 1, arq) == 1) {
        if (conta.numero == numeroOrigem) {
            achouOrigem = 1;
            indiceOrigem = indice;
            origem = conta;
        }
        if (conta.numero == numeroDestino) {
            achouDestino = 1;
            indiceDestino = indice;
            destino = conta;
        }
        indice++;
    }

    if (!achouOrigem || !achouDestino) {
        printf("Conta de origem ou destino nao localizada.\n");
        fclose(arq);
        return;
    }

    if (origem.saldo < valor) {
        printf("Saldo insuficiente na conta de origem. Saldo atual: R$ %.2f\n", origem.saldo);
        fclose(arq);
        return;
    }

    origem.saldo  -= valor;
    destino.saldo += valor;

    long posOrigem = indiceOrigem * (long)sizeof(struct Conta);
    fseek(arq, posOrigem, SEEK_SET);
    fwrite(&origem, sizeof(struct Conta), 1, arq);

    long posDestino = indiceDestino * (long)sizeof(struct Conta);
    fseek(arq, posDestino, SEEK_SET);
    fwrite(&destino, sizeof(struct Conta), 1, arq);

    printf("Transferencia executada com sucesso!\n");
    printf("Novo saldo conta origem  (%d): R$ %.2f\n", origem.numero, origem.saldo);
    printf("Novo saldo conta destino (%d): R$ %.2f\n", destino.numero, destino.saldo);

    fclose(arq);
}

// 6) Listar todas as contas (admin)
void listarTodasAsContas(void) {
    char senha[20];
    FILE *arq;
    struct Conta conta;

    printf("\n=== LISTAGEM DE TODAS AS CONTAS (ADMIN) ===\n");
    printf("Informe a senha de administrador: ");
    limparEntrada();
    scanf("%19s", senha);

    if (strcmp(senha, SENHA_ADMIN) != 0) {
        printf("Senha incorreta. Nao foi possivel exibir as contas.\n");
        return;
    }

    arq = fopen(NOME_ARQUIVO, "rb");
    if (arq == NULL) {
        printf("Nenhuma conta cadastrada no sistema.\n");
        return;
    }

    printf("\n--- RELACAO DE CONTAS CADASTRADAS ---\n");
    while (fread(&conta, sizeof(struct Conta), 1, arq) == 1) {
        printf("Conta: %4d | Titular: %-20s | Saldo: R$ %8.2f\n",
               conta.numero, conta.titular, conta.saldo);
    }

    fclose(arq);
}

// ======================= Fun��o principal =======================

int main(void) {
    int opcao;

    inicializarSistema();

    do {
        printf("\n=====================================\n");
        printf("   MENU - SISTEMA BANCARIO\n");
        printf("=====================================\n");
        printf("1 - Abrir nova conta\n");
        printf("2 - Consultar conta (extrato simples)\n");
        printf("3 - Depositar em conta\n");
        printf("4 - Realizar saque\n");
        printf("5 - Transferir entre contas\n");
        printf("6 - Listar todas as contas (admin)\n");
        printf("7 - Encerrar sistema\n");
        printf("Opcao desejada: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                abrirConta();
                break;
            case 2:
                consultarConta();
                break;
            case 3:
                realizarDeposito();
                break;
            case 4:
                realizarSaque();
                break;
            case 5:
                realizarTransferencia();
                break;
            case 6:
                listarTodasAsContas();
                break;
            case 7:
               printf("\nEncerrando o sistema. Ate breve!\n");
                break;
            default:
                printf("Opcao invalida. Tente novamente.\n");
        }

    } while (opcao != 7);

    return 0;
}