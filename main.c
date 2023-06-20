#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char* entrada;
    size_t size = 30;
    int lidos;
    entrada = (char*)malloc(size * sizeof(char));
    while(1){
        printf("acsh> ");
        lidos = getline(&entrada, &size, stdin);
        char* token;
        if(!strcmp(entrada, "exit\n")){
            printf("\nsaindo\n");
            break;
        }
        else if(!strcmp(entrada, "cd\n")){
            printf("\nmudando diretorio\n");
        }

        else if((token = strtok(entrada, " ")) != NULL){
            int i = 0;
            char*** comandos = (char***)malloc(5 * sizeof(char**));
            while(i < 5){
                int tam = 10;
                comandos[i] = (char**)malloc(tam * sizeof(char*));          
                int j = 0;
                while(token != NULL && strcmp(token, "<3") != 0){
                    if(j >= tam){
                        tam *= 2;
                        comandos[i] = (char**)realloc(comandos[i], tam * sizeof(char*));
                    }
                    comandos[i][j] = (char*)malloc(20 * sizeof(char));
                    comandos[i][j] = strcpy(comandos[i][j], token);
                    j++;
                    token = strtok(NULL, " ");
                }
                comandos[i][j] = NULL;
                i++;
                if(token == NULL ) break;
                else token = strtok(NULL, " ");
            }
            if(i > 5) printf("\nnumero invalido de comandos\n");

            else if(i == 1){
                printf("\ncomando sozinho:\n");
                int j = 0;
                while(comandos[0][j] != NULL){
                    printf("%s ", comandos[0][j]);
                    j++;
                }
                printf("\n");
            }

            else{
                printf("\ncomandos agrupados:\n");
                for(int k = 0;k < i;k++){
                    printf("comando %d:\n", (k + 1));
                    int j = 0;
                    while(comandos[k][j] != NULL){
                        printf("%s ", comandos[k][j]);
                        j++;
                    }
                }
            }
        }
    }
    return 0;
}