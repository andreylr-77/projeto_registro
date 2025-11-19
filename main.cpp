#include <stdio.h>    
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

typedef struct {
    char nomeTime[50];
    char estado[20];
    char anoFundacao[10];
    char status;    
} reg;

void configurar_locale(void);
void limpa_buffer(void);
void ler_string(char *s, int tam);
int tamanho(FILE *arq);
void cadastrar(FILE *arq);
void consultar(FILE *arq);
void gerar_arquivo_texto(FILE *arq);
void excluir(FILE *arq);

void limpa_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ler_string(char *s, int tam) {
    fgets(s, tam, stdin);
    s[strcspn(s, "\n")] = '\0';
}

int tamanho(FILE *arq) {
    long pos = ftell(arq); 
    fseek(arq, 0, SEEK_END);   
    long fim = ftell(arq);     
    fseek(arq, pos, SEEK_SET); 
    return (int)(fim / sizeof(reg));
}

void cadastrar(FILE *arq) {
    reg time;
    time.status = ' ';
    char confirma;

    printf("\n=== CADASTRAR TIME DE FUTEBOL ===\n");
    printf("Registro número: %d\n", tamanho(arq) + 1);

    printf("Nome do Time: ");
    ler_string(time.nomeTime, sizeof(time.nomeTime));

    printf("Estado (UF): ");
    ler_string(time.estado, sizeof(time.estado));

    printf("Ano de Fundação: ");
    ler_string(time.anoFundacao, sizeof(time.anoFundacao));

    printf("Confirmar cadastro (s/n)? ");
    if (scanf("%c", &confirma) != 1) {
        printf("Entrada inválida. Cancelando.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        fseek(arq, 0, SEEK_END);
        fwrite(&time, sizeof(reg), 1, arq);
        fflush(arq);
        printf("Time cadastrado com sucesso!\n");
    } else {
        printf("Cadastro cancelado.\n");
    }
}

void consultar(FILE *arq) {
    int nr;
    reg time;

    printf("\nInforme o código do time: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    int total = tamanho(arq);

    if (nr <= 0 || nr > total) {
        printf("Código inválido! Total = %d\n", total);
        return;
    }

    long pos = (long)(nr - 1) * sizeof(reg);

    if (fseek(arq, pos, SEEK_SET) != 0) {
        printf("[ERRO] Falha no fseek.\n");
        return;
    }

    size_t lido = fread(&time, sizeof(reg), 1, arq);
    if (lido != 1) {
        printf("[ERRO] Falha ao ler registro!\n");
        return;
    }

    printf("\n=== TIME (CÓDIGO %d) ===\n", nr);
    if (time.status == '*')
        printf("Status: EXCLUÍDO LOGICAMENTE\n");

    printf("Nome........: %s\n", time.nomeTime);
    printf("Estado......: %s\n", time.estado);
    printf("Fundado em..: %s\n", time.anoFundacao);
}

void gerar_arquivo_texto(FILE *arq) {
    char nomearq[80];
    reg time;

    int i, total;
    char status_str[12];

    printf("\nGerar Arquivo Texto\n");
    printf("Nome do arquivo (sem extensão): ");
    ler_string(nomearq, sizeof(nomearq));
    strcat(nomearq, ".txt");

    FILE *arqtxt = fopen(nomearq, "w");

    if (!arqtxt) {
        printf("Erro ao criar arquivo texto.\n");
        return;
    }

    fprintf(arqtxt, "RELATÓRIO COMPLETO DE TIMES DE FUTEBOL\n\n");
    fprintf(arqtxt,
            "COD  %-25s %-10s %-15s STATUS\n",
            "NOME DO TIME", "ESTADO", "FUNDADO");
    fprintf(arqtxt,
            "--------------------------------------------------------------------------\n");

    total = tamanho(arq);
    for (i = 0; i < total; i++) {
        fseek(arq, i * sizeof(reg), SEEK_SET);
        fread(&time, sizeof(reg), 1, arq);

        if (time.status == '*')
            strcpy(status_str, "EXCLUIDO");
        else
            strcpy(status_str, "ATIVO");

        fprintf(arqtxt, "%03d %-25s %-10s %-15s %s\n",
                i + 1,
                time.nomeTime,
                time.estado,
                time.anoFundacao,
                status_str);
    }

    fclose(arqtxt);
    printf("\nArquivo '%s' gerado com sucesso!\n", nomearq);
}

void excluir(FILE *arq) {
    int nr;
    char confirma;
    reg time;

    printf("\nInforme o código do time para excluir: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    int total = tamanho(arq);
    if (nr <= 0 || nr > total) {
        printf("Código inválido.\n");
        return;
    }

    long pos = (long)(nr - 1) * sizeof(reg);

    fseek(arq, pos, SEEK_SET);
    fread(&time, sizeof(reg), 1, arq);

    if (time.status == '*') {
        printf("Registro já excluído.\n");
        return;
    }

    printf("Confirmar exclusão (s/n)? ");
    if (scanf("%c", &confirma) != 1) {
        printf("Entrada inválida.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        time.status = '*';
        fseek(arq, pos, SEEK_SET);
        fwrite(&time, sizeof(reg), 1, arq);
        fflush(arq);
        printf("Time excluído com sucesso!\n");
    } else {
        printf("Exclusão cancelada.\n");
    }
}

void configurar_locale(void) {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    const char *locais[] = {
        "pt_BR.UTF-8",
        "pt_BR.utf8",
        "Portuguese_Brazil.1252",
        "Portuguese",
        ""
    };

    for (int i = 0; i < 5; i++) {
        const char *r = setlocale(LC_ALL, locais[i]);
        if (r != NULL) {
            printf("Locale ativo: %s\n", r);
            return;
        }
    }
    printf("Aviso: Locale não pôde ser configurado.\n");
}

int main(void) {
    configurar_locale();

    FILE *arq = fopen("times.dat", "r+b");
    if (!arq) {
        arq = fopen("times.dat", "w+b");
        if (!arq) {
            printf("Erro crítico ao abrir/criar arquivo de dados.\n");
            return 1;
        }
    }

    int op;
    do {
        printf("\n========= GERENCIADOR DE TIMES DE FUTEBOL =========\n");
        printf("1. Cadastrar Time\n");
        printf("2. Consultar Time\n");
        printf("3. Gerar Arquivo Texto\n");
        printf("4. Excluir Time\n");
        printf("5. Sair\n");
        printf("----------------------------------------------------\n");
        printf("Total de registros: %d\n", tamanho(arq));
        printf("Opção: ");

        if (scanf("%d", &op) != 1) {
            printf("Digite um número válido.\n");
            limpa_buffer();
            continue;
        }
        limpa_buffer();

        switch (op) {
            case 1: cadastrar(arq); break;
            case 2: consultar(arq); break;
            case 3: gerar_arquivo_texto(arq); break;
            case 4: excluir(arq); break;
            case 5: printf("Saindo...\n"); break;
            default: printf("Opção inválida!\n");
        }

    } while (op != 5);

    fclose(arq);
    return 0;
}
