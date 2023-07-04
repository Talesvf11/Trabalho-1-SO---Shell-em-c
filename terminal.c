#include "terminal.h"
#include <termios.h>

#define MAX_PROGRAMAS_LINHA 5
#define MAX_ARGUMENTOS_PROGRAMA 3

char *acsh_read_line()
{
    char *entrada = NULL;
    size_t size = 0;
    
    if(getline(&entrada, &size, stdin) == -1)
        free(entrada);
    
    return entrada;
}

char ***acsh_comandos_from_line(char *entrada)
{
    int i = 0;
    char *token = strtok(entrada, " \n");
    if(token == NULL) return NULL;

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
            
            j++;
            if (j >= MAX_ARGUMENTOS_PROGRAMA + 2 && strcmp(token, "%")) {
                printf("numero invalido de argumentos\n");
            }
            token = strtok(NULL, " \n");
            
        }
        
        comandos[i][j] = NULL; // Sinaliza o fim da lista de argumentos do programa.
        i++;
        
        if (token == NULL) { // Não tem mais programas para serem executados.
            // Preenche o restante dos "ponteiros para programas" com NULL.
            // Assim, é possível consultar depois quantos programas temos.
            for (int k = i; k < MAX_PROGRAMAS_LINHA; k++) {
                comandos[k] = NULL;
            }
            break;
        }
        else {
            token = strtok(NULL, " \n");
        }
    }
    
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
        
        free(comandos[i]);
    }
    free(comandos);
}

void main_sig_handler(int sig){
    printf("Não adianta me enviar o sinal por Ctrl-... . Estou vacinado!!\n");
    printf("acsh> ");
    fflush(stdout); // Para garantir que acsh> é imediatamente impresso
}

void RodaTerminal()
{
    // Coloca o handler de SIGINT, SIGQUIT e SIGSTP
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

    int proximo_supervisor = 0;
    int supervisores[10000];

    char ***comandos;
    while (1)
    {
        int wstatus;
        // Toda vez que passar aqui, vai verificar se algum processo Session Leader,
        // daqueles que a gente cria o processo do primeiro comando como, e que é pai
        // dos processos dos outros comandos. Isso fornece uma limpeza de tempos em tempos,
        // o que previne a acumulação de processos Zumbis.
        waitpid(-1, &wstatus, WNOHANG);

        printf("acsh> ");
        char * entrada = acsh_read_line();
        char * entrada_original_ptr = entrada;
        comandos = acsh_comandos_from_line(entrada);
        if(comandos != NULL){
            if (!strcmp(entrada, "exit"))
            {
                int status;
                char mata_processos[50];
                // Pega cada supervisor de sessão, e usa o comando pkill -P para matar ele e os filhos criados por ele (processos que o usuário realmente pediu)
                for (int i = 0; i < proximo_supervisor; i++) {
                    snprintf(mata_processos, sizeof(mata_processos), "pkill -P %d", supervisores[i]);
                    system(mata_processos);
                }
                exit(0);
            }
            else if (!strcmp(comandos[0][0], "cd"))
            {
                printf("%s\n", comandos[0][1]);
                fflush(stdout);
                chdir(comandos[0][1]);
            }

            else {
                ExecutaComandosExternos(comandos, qtd_comandos(comandos), supervisores, &proximo_supervisor);
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

void ExecutaComandosExternos(char ***comandos, int nComandos, int * supervisores, int * proximo_supervisor)
{
    int foreground = 0;
    // Checa se o último parâmetro é % (pra então executar em foreground)
    if (nComandos == 1)
    {
        for (int i = 0; i < 5; i++)
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

            //Coloca o handler de SIGUSR1

            if(nComandos == 1){
                struct sigaction sa;
                sa.sa_handler = SIG_IGN;
                sigemptyset(&sa.sa_mask);
                sigaddset(&sa.sa_mask, SIGUSR1);
                sa.sa_flags = SA_RESTART;

                sigaction(SIGUSR1, &sa, NULL);
            }

            // Cria uma nova sessão com apenas o supervisor
            if (setsid() == -1)
            {
                printf("erro ao tentar colocar filho em bg");
            }
            
            int i;
            for (i = 0; i < nComandos; i++)
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

            int status = 0;
            while (waitpid(0, &status, WUNTRACED) >= 0) {
                if ((WIFSIGNALED(status)) && (WTERMSIG(status) == SIGUSR1)) {
                    killpg(0, SIGTERM);
                }
            }
            exit(0);
            printf("[Cheguei onde não deveria]");
            fflush(stdout);
        }
        else if (pid > 0) 
        {
            supervisores[*proximo_supervisor] = pid;
            (*proximo_supervisor)++;
        }
    }
}
