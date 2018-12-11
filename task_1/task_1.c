#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
int main()
{
    const int MAX_LEN = sysconf(_SC_ARG_MAX)+1;
    const char* STOP_KEYWORD = "exit()";
    char* DELIMETERS = " ,-\n";

    char* com;
    char** com_arg;
    printf("[x] Command to exit emulated bash: %s\n", STOP_KEYWORD);
    while(1)
    {
        com=malloc(sizeof(char)*MAX_LEN);
        printf("\n> ");
        scanf("%[^\n]%*c", com);
        
        if (strcmp(com, STOP_KEYWORD) == 0) {
            exit(0);
        } else {
            int len = strlen(com);
            
            int delim_count = 0;
            for(int i=0; i<len; i++) {
                if(*(com+i)==' ' || *(com+i)==',' || *(com+i)=='-') {
                    delim_count++;
                }
            }

            int offset = 0;
            com_arg = malloc(sizeof(char*)*(delim_count+1));
            for(char *p=strtok(com, DELIMETERS); p!=NULL; p = strtok(NULL, DELIMETERS)) {
                *(com_arg+offset)=p;
                offset++;
            }

            pid_t pid = fork();
            if(pid<0) {
                printf("[x] Fork error :(\n");
                return -1;
            }
            if(pid) {
                int status;
                waitpid(pid, &status, 0);
                printf("[x] Exit code: %d\n", WEXITSTATUS(status));
                free(com_arg);
            }
            else {
                execvp(com_arg[0],com_arg);
                free(com_arg);
            }
        }
        free(com);
    }
    return 0;
}
