#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>







#define MAX_LIMIT 256


int main(int argc, char *argv[]) {

    //bool exit_program =  false;
   // char* quit_program = false;
    char input[MAX_LIMIT];
    pid_t pid;
    printf("\nWelcome...\n\n");
    //fgets(input,MAX_LIMIT,stdin);
    //scanf("%[^\n]%*c",input);
    //printf("%s\n",input);
    while(1) {
        printf("[Quash]$ ");
        fflush(stdout);
        fgets(input,MAX_LIMIT,stdin);

        if(input=="ls") {
            pid = fork();
            if(pid==0) {
                execlp("/bin/ls", "ls", NULL);
                printf("This is the child process");
            }
        }

    }

    printf("I'm exiting . . .");
    return 0;

}