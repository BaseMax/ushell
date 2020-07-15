/*
 * @Name: ushell
 * @Author: Max Base
 * @Date: 2020-07-15
 * @Repository: https://github.com/BaseMax/ushell
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void ushell_allocation_error() {
    fprintf(stderr, "ushell: Unable to allocate buffer\n");
    exit(EXIT_FAILURE);
}

int ushell_run(char **args) {
    // https://man7.org/linux/man-pages/man2/waitid.2.html
    // https://stackoverflow.com/questions/47441871/why-should-we-check-wifexited-after-wait-in-order-to-kill-child-processes-in-lin
    // https://www.gnu.org/software/libc/manual/html_node/Process-Completion-Status.html
    // https://www.geeksforgeeks.org/exit-status-child-process-linux/
    int status;
    pid_t pid = fork();
    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("ushell");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0) {
        perror("ushell");
    }
    else {
        do {
            waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

char *ushell_read(void) {
    char *buffer;
    size_t bufsize = 1024;
    buffer = (char *)malloc(bufsize * sizeof(char));// sizeof(char) is 1
    if(!buffer) {
        ushell_allocation_error();
    }
    if(getline(&buffer, &bufsize, stdin) == -1) {
        if(feof(stdin)) {
            exit(EXIT_SUCCESS);
        }
        else {
            perror("ushell: getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return buffer;
}

char **ushell_split(char *line) {
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    if(!tokens) {
        ushell_allocation_error();
    }
    char *token = strtok(line, " \t\r\n\a");
    while(token != NULL) {
        tokens[position] = token;
        position++;
        if(position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                ushell_allocation_error();
            }
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

int main(int argc, char **argv, char** env) {
    char *command;
    char **args;
    int status;
    do {
        printf("\x1B[32m");//yellow
        printf("ushell# ");
        printf("\x1B[0m");//normal
        command = ushell_read();
        args = ushell_split(command);
        if(strcmp(command, "exit")==0) {
            status=0;
        }
        else if(args[0]!=NULL){
            status=ushell_run(args);
        }
        free(command);
        free(args);
    } while (status);
    return EXIT_SUCCESS;
}
