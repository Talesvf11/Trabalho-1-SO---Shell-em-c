#include "terminal.h"
#include <termios.h>

#define MAX_PROGRAMAS_LINHA 5
#define MAX_ARGUMENTOS_PROGRAMA 3

char *acsh_read_line()
{
    char *entrada = NULL;
    size_t size = 0;
    
    if(getline(&entrada, &size, stdin) == -1){
        int i = errno;
        printf("erro: %s\n", strerror(i));
    }
    // TODO: tratar falha de leitura, liberando memória.
    /* "If *lineptr is set to NULL and *n is set 0 before the call, then getline() will al‐
       locate a buffer for storing the line.  This buffer should be freed by the user pro‐
       gram even if getline() failed." */
    return entrada;
}

char ***acsh_comandos_from_line(char *entrada)
{
    int i = 0;
    char *token = strtok(entrada, " \n");
    if(token == NULL) return NULL;

    // printf("first token: %s\n", token);
    // Pega cada comando (aka programa)
    char ***comandos = (char ***)malloc(MAX_PROGRAMAS_LINHA * sizeof(char **));
    while (i < MAX_PROGRAMAS_LINHA)
    {
        comandos[i] = (char **)malloc((MAX_ARGUMENTOS_PROGRAMA +
                                       1 /* Próprio programa */ +
                                       1 /* Possível % */ +
                                       1 /* NULL no final */) * sizeof(char *));
        int j = 0;

        // Pega os argumentos do programa
        while (token != NULL && strcmp(token, "<3") != 0)
        {
            comandos[i][j] = token;
            // printf("debug: [%d][%d] %s\n", i, j, comandos[i][j]);
            j++;
            if (j >= MAX_ARGUMENTOS_PROGRAMA + 2 && strcmp(token, "%")) {
                printf("numero invalido de argumentos\n");
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

void main_sig_handler(int sig){
    printf("Não adianta me enviar o sinal por Ctrl-... . Estou vacinado!!\n");
}

void RodaTerminal()
{
    struct sigaction sa;
    sa.sa_handler = main_sig_handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGTSTP);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);

    //signal(SIGTSTP, main_sig_handler);

    char ***comandos;
    while (1)
    {
        printf("acsh> ");
        char * entrada = acsh_read_line();
        char * entrada_original_ptr = entrada;
        comandos = acsh_comandos_from_line(entrada);
        if(comandos != NULL){
            if (!strcmp(entrada, "exit"))
            {
                int status;
                // TODO: finalizar todos os processos de background que ainda estejam rodando.
                printf("saindo\n");
                exit(0);
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
}

void ExecutaEmForeground (char *** comandos) {
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("erro ao tentar criar filho.");
    }
    else if (pid == 0)
    {
        execvp(comandos[0][0], comandos[0]);
        printf("erro ao executar programa\n");
        exit(0);
    }
    else if (pid > 0)
    {
        struct sigaction sa, old;
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGINT);
        sigaddset(&sa.sa_mask, SIGQUIT);
        sigaddset(&sa.sa_mask, SIGTSTP);
        sa.sa_flags = SA_RESTART;

        sigaction(SIGINT, &sa, &old);
        sigaction(SIGQUIT, &sa, &old);
        sigaction(SIGTSTP, &sa, &old);

        if (waitpid(pid, NULL, 0) == -1)
            printf("erro ao aguardar por termino do filho");
        
        sigaction(SIGINT, &old, NULL);
        sigaction(SIGQUIT, &old, NULL);
        sigaction(SIGTSTP, &old, NULL);
    }
}


void ExecutaComandosExternos(char ***comandos, int nComandos)
{
    int foreground = 0;

    // Checa se o último parâmetro é % (pra então executar em foreground)
    if (nComandos == 1)
    {
        for (int i = 0; i < 5; i++) // Bruno: Porque 5 vezes, especificamente?
        {
            if (comandos[0][i] == NULL)
                break;

            if (!strcmp(comandos[0][i],"%"))
            {
                foreground = 1;
                comandos[0][i] = NULL; // Tira o % dos argumentos
                ExecutaEmForeground(comandos);
            }
        }
    }
    // WARNING: Comandos que escrevem no terminal, quando em background, frequentemente dessincronizam a impressão de "acsh>"
    if (foreground == 0)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printf("erro ao tentar criar filho.");
        }
        else if (pid == 0)
        {
            int f1 = open("/dev/null", O_RDONLY);
            int f2 = open("/dev/null", O_WRONLY);
            int f3 = open("/dev/null", O_WRONLY);
            
            if(dup2(f1, 0) == -1){
                printf("erro");
            }
            if(dup2(f2, 1) == -1){
                printf("erro");
            }
            if(dup2(f3, 2) == -1){
                printf("erro");
            }

            // Cria uma nova sessão com apenas o filho
            if (setsid() == -1)
            {
                printf("erro ao tentar colocar filho em bg");
            }
            // WARNING: Desse jeito, o primeiro processo vai acabar sendo o último a executar.
            for (int i = 1; i < nComandos; i++)
            {
                pid = fork();
                if (pid < 0)
                {
                    printf("erro ao tentar criar filho.");
                }
                else if (pid == 0)
                {
                    execvp(comandos[i][0], comandos[i]);
                    printf("erro ao executar programa");
                    exit(0);
                }
            }
            
            execvp(comandos[0][0], comandos[0]);
            printf("erro ao executar programa");
            exit(0);
        }
    }
}