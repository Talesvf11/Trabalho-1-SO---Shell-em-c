#include "terminal.h"

#define MAX_PROGRAMAS_LINHA 5
#define MAX_ARGUMENTOS_PROGRAMA 3

char *acsh_read_line()
{
    char *entrada = NULL;
    size_t size = 0;
    getline(&entrada, &size, stdin);
    // TODO: tratar falha de leitura, liberando memória.
    /* "If *lineptr is set to NULL and *n is set 0 before the call, then getline() will al‐
       locate a buffer for storing the line.  This buffer should be freed by the user pro‐
       gram even if getline() failed." */
    return entrada;
}

char ***acsh_comandos_from_line(char *entrada)
{
    int i = 0;
    char ***comandos = (char ***)malloc(MAX_PROGRAMAS_LINHA * sizeof(char **));
    char *token = strtok(entrada, " ");
    // printf("first token: %s\n", token);
    // Pega cada comando (aka programa)
    while (i < MAX_PROGRAMAS_LINHA)
    {
        comandos[i] = (char **)malloc((MAX_ARGUMENTOS_PROGRAMA +
                                       1 /* Próprio programa */ +
                                       1 /* NULL no final */) * sizeof(char *));
        int j = 0;

        // Pega os argumentos do programa
        while (token != NULL && strcmp(token, "<3") != 0)
        {
            comandos[i][j] = token;
            // printf("debug: [%d][%d] %s\n", i, j, comandos[i][j]);
            j++;
            if (j >= MAX_ARGUMENTOS_PROGRAMA + 2) {
                printf("\nnumero invalido de argumentos\n");
            }
            token = strtok(NULL, " \n");
            // printf("new token read: %s\n", token);
        }
        
        comandos[i][j] = NULL; // Sinaliza o fim da lista de argumentos do programa.
        i++;
        // printf("debug i: %d\n", i);
        if (token == NULL) { // Não tem mais programas para serem executados.
            // Preenche o restante dos "ponteiros para programas" com NULL.
            // Assim, é possível consultar depois quantos programas temos.
            for (int k = i; k < MAX_PROGRAMAS_LINHA; k++) {
                comandos[k] = NULL;
            }
            break;
        }
        else {
            token = strtok(NULL, " ");
            // printf("new token read: %s\n", token);
        }
    }
    // printf("debug i: %d\n", i);
    if (i >= MAX_PROGRAMAS_LINHA) {
        printf("\nnumero invalido de comandos\n");
    }

    return comandos;
}

int qtd_comandos(char *** comandos) {
    int i;
    for (i = 0; i < MAX_PROGRAMAS_LINHA; i++)
        if (comandos[i] == NULL) return i;
        
    return i;
}

void libera_comandos(char *** comandos) {
    for (int i = 0; i < 1; i++) { 
        // printf("debug liberando comandos: %d\n", i);
        free(comandos[i]);
    }
    free(comandos);
}

void acsh_exec(char **args)
{
    pid_t child_pid = fork();

    // Execução em foreground
    if (child_pid == 0)
    {
        execvp(args[0], args);
        perror("acsh");
        exit(1);
    }
    else if (child_pid > 0)
    {
        int status;
        do
        {
            waitpid(child_pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    else
    {
        perror("acsh");
    }
}

void acsh_exit(char **args)
{
    exit(0);
}

void acsh_cd(char **args)
{
    if (args[1] == NULL)
    {
        printf("acsh: cd: missing argument\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("acsh: cd");
        }
    }
}

void RodaTerminal()
{
    while (1)
    {
        printf("acsh> ");
        char * entrada = acsh_read_line();
        char * entrada_original_ptr = entrada;
        char ***comandos = acsh_comandos_from_line(entrada);
        if (!strcmp(entrada, "exit\n"))
        {
            // TODO: finalizar todos os processos de background que ainda estejam rodando.
            printf("\nsaindo\n");
            break;
        }
        else if (!strcmp(comandos[0][0], "cd"))
        {
            printf("\nmudando diretorio\n");
            chdir(comandos[0][1]); // Só é necessário isso?
        }

        //printf("qtd comandos: %d\n", qtd_comandos(comandos));

        else {
            ExecutaComandosExternos(comandos, qtd_comandos(comandos));
        }

        free(entrada);
        libera_comandos(comandos);
    }
}

void ExecutaComandosExternos(char ***comandos, int nComandos)
{
    int foreground = 0;
    if (nComandos == 1)
    {
        // Checa se o último parâmetro é % (para então executar em foreground)
        for (int i = 0; i < 5; i++)
        {
            if (comandos[0][i] == NULL)
                break;

            if (!strcmp(comandos[0][i],"%"))
            {
                pid_t pid = fork();
                foreground = 1;
                if (pid < 0)
                {
                    printf("erro ao tentar criar filho.");
                }
                else if (pid == 0)
                {
                    comandos[0][i] = NULL; // Tira o % dos argumentos

                    execvp(comandos[0][0], comandos[0]);
                    printf("erro ao executar programa");
                    exit(0);

                    /*  char *cmd = "ls";
                        char *argv[3];
                        argv[0] = "ls";
                        argv[1] = "-la";
                        argv[2] = NULL;
                        execvp(cmd, argv); //This will run "ls -la" as if it were a command*/
                }
                else if (pid > 0)
                {
                    int status;
                    if (waitpid(pid, &status, 0) == -1)
                    {
                        printf("erro ao aguardar por termino do filho");
                    }
                }
            }
        }
    }
    if (foreground == 0)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printf("erro ao tentar criar filho.");
        }
        else if (pid == 0)
        {
            if (setsid() == -1)
            {
                printf("erro ao tentar colocar filho em bg");
            }
            for (int i = 1; i < nComandos; i++)
            {
                pid = fork();
                if (pid < 0)
                {
                    printf("erro ao tentar criar filho.");
                }
                if (pid == 0)
                {
                    execvp(comandos[i][0], comandos[i]);
                    printf("erro ao executar programa");
                    exit(0);
                }
            }
            printf("%s", comandos[0][0]);
            execvp(comandos[0][0], comandos[0]);
            printf("erro ao executar programa");
            exit(0);
        }
    }
}