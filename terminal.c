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

void acsh_exec(char ** args) {
    pid_t child_pid = fork();

    // Execução em foreground
    if (child_pid == 0) {
        execvp(args[0], args);
        perror("acsh");
        exit(1);
    } else if (child_pid > 0) {
        int status;
        do {
            waitpid(child_pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
        perror("acsh");
    }
}

void acsh_exit(char **args) {
    exit(0);
}

void acsh_cd(char **args) {
    if (args[1] == NULL) {
        printf("acsh: cd: missing argument\n");
    } else {
        if (chdir(args[1]) != 0) {
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
            char ***comandos = (char ***)malloc(MAX_PROGRAMAS_LINHA * sizeof(char **));
            while (i < MAX_PROGRAMAS_LINHA)
            {
                comandos[i] = (char **)malloc((MAX_ARGUMENTOS_PROGRAMA + 2) * sizeof(char *));
                int j = 0;
                while (token != NULL && strcmp(token, "<3") != 0)
                {
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


            else{
                ExecutaComandosExternos(comandos, i);
            }
        }
    }
}

void ExecutaComandosExternos(char ***comandos, int nComandos){
    int foreground = 0;   
    if(nComandos == 1){
        for(int i = 0;i < 5;i++){
            if(comandos[0][i] == NULL) break;
            if(comandos[0][i] == "%%"){
                 pid_t pid = fork();
                 foreground = 1;
                 if(pid < 0){
                    printf("erro ao tentar criar filho.");
                 }
                 if(pid == 0){
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
                 if(pid > 0 ){
                    int status;
                    if(waitpid(pid, &status, 0) == -1){
                        printf("erro ao aguardar por termino do filho");
                    }
                 }
            }
        }
    }
    if(foreground == 0){
            pid_t pid = fork();
            if(pid < 0){
                printf("erro ao tentar criar filho.");
            }
            if(pid == 0){
                if(setsid() == -1){
                    printf("erro ao tentar colocar filho em bg");
                }
                for(int i = 1;i < nComandos;i++){
                    pid = fork();
                    if(pid < 0){
                        printf("erro ao tentar criar filho.");
                    }
                    if(pid == 0){
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