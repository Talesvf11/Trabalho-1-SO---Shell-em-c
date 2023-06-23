#include "terminal.h"

#define MAX_PROGRAMAS_LINHA 5
#define MAX_ARGUMENTOS_PROGRAMA 3

char* acsh_read_line() {
    char * entrada = NULL;
    size_t size = 0;
    getline(&entrada, &size, stdin);
    // TODO: tratar falha de leitura, liberando memória.
    /* "If *lineptr is set to NULL and *n is set 0 before the call, then getline() will al‐
       locate a buffer for storing the line.  This buffer should be freed by the user pro‐
       gram even if getline() failed." */
    return entrada;
}

void RodaTerminal()
{
    while (1)
    {
        printf("acsh> ");
        char * entrada = acsh_read_line();
        char *token;
        if (!strcmp(entrada, "exit\n"))
        {
            // TODO: finalizar todos os processos de background que ainda estejam rodando.
            printf("\nsaindo\n");
            break;
        }
        else if (!strcmp(entrada, "cd\n"))
        {
            printf("\nmudando diretorio\n");
        }

        else if ((token = strtok(entrada, " ")) != NULL)
        {
            int i = 0;
            char ***comandos = (char ***)malloc(5 * sizeof(char **));
            while (i < MAX_PROGRAMAS_LINHA)
            {
                int tam = 10;
                comandos[i] = (char **)malloc(tam * sizeof(char *));
                int j = 0;
                while (token != NULL && strcmp(token, "<3") != 0)
                {
                    if (j >= tam)
                    {
                        tam *= 2;
                        comandos[i] = (char **)realloc(comandos[i], tam * sizeof(char *));
                    }
                    comandos[i][j] = (char *)malloc(20 * sizeof(char));
                    comandos[i][j] = strcpy(comandos[i][j], token);
                    // penso que as duas linhas acima poderiam ser substituídas por:
                    // comandos[i][j] = token;
                    // token já é uma string alocada dinamicamente. não precisamos alocar
                    // outra string e copiar o conteúdo de token para ela.
                    j++;
                    token = strtok(NULL, " ");
                }
                comandos[i][j] = NULL;
                i++;
                if (token == NULL)
                    break;
                else
                    token = strtok(NULL, " ");
            }
            if (i > MAX_PROGRAMAS_LINHA)
                printf("\nnumero invalido de comandos\n");

            else if (i == 1)
            {
                printf("\ncomando sozinho:\n");
                int j = 0;
                while (comandos[0][j] != NULL)
                {
                    printf("%s ", comandos[0][j]);
                    j++;
                }
                printf("\n");
            }

            else
            {
                // printf("\ncomandos agrupados:\n");
                // for(int k = 0;k < i;k++){
                //     printf("comando %d:\n", (k + 1));
                //     int j = 0;
                //     while(comandos[k][j] != NULL){
                //         printf("%s ", comandos[k][j]);
                //         j++;
                //     }
                // }
                ExecutaComandosExternos(comandos, i);
            }
        }
    }
}

void ExecutaComandosExternos(char ***comandos, int nComandos)
{
    for (int i = 0; i < nComandos; i++)
    {
        int j = 0;
        int maiorComando = -1;
        while (comandos[i][j] != NULL)
        {
            j++;
            if (strlen(comandos[i][j]) > maiorComando)
                maiorComando = strlen(comandos[i][j]);
        }
        char comando[j][maiorComando];
        for (int k = 0; k < j; k++)
        {
            // strcpy(comando[k], comandos[i][k]);
            // printf("%s ", comando[k]);
        }
    }
}