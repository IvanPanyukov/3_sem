#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    const int MAX_LEN = sysconf(_SC_ARG_MAX) + 1;
    const char* STOP_KEYWORD = "exit()";
    printf("[x] Command to exit emulated bash: %s\n", STOP_KEYWORD);
    
    char* command;
    while(1) {
        command = malloc(sizeof(char) * MAX_LEN);
        printf("\n> ");
        scanf("%[^\n]%*c", command);
        if (strcmp(command, STOP_KEYWORD) == 0) {
            exit(0);
        }

        char** tokens = malloc(sizeof(char*) * MAX_LEN);
        char* pch = strtok(command, " -,");
        tokens[0] = pch;
        for (int i = 1; pch != NULL; i++) { 
            pch = strtok(NULL, ", ");
            *(tokens + i) = pch;
        }

        pid_t pid = fork();
        if (pid < 0) {
            printf("[x] Fork error :(\n");
        }
        if (pid) {
            int status;
            waitpid(pid, &status, 0);
            printf("[x] Exit code: %d\n", WEXITSTATUS(status));
        }
        if (pid == 0) {
            execvp(tokens[0], tokens);
            exit(0);
        }       

        free(tokens);
        free(command);
    }
}
