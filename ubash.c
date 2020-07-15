/*
 * @Name: ubash
 * @Author: Max Base
 * @Date: 2020-07-15
 * @Repository: https://github.com/BaseMax/ubash
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void ubash_allocation_error() {
    fprintf(stderr, "ubash: Unable to allocate buffer\n");
    exit(EXIT_FAILURE);
}

int ubash_run(char **args) {
    // https://man7.org/linux/man-pages/man2/waitid.2.html
    // https://stackoverflow.com/questions/47441871/why-should-we-check-wifexited-after-wait-in-order-to-kill-child-processes-in-lin
    // https://www.gnu.org/software/libc/manual/html_node/Process-Completion-Status.html
    // https://www.geeksforgeeks.org/exit-status-child-process-linux/
    int status;
    pid_t pid = fork();
    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("ubash");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0) {
        perror("ubash");
    }
    else {
        do {
            waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

char *ubash_read(void) {
    char *buffer;
    size_t bufsize = 1024;
    buffer = (char *)malloc(bufsize * sizeof(char));// sizeof(char) is 1
    if(!buffer) {
        ubash_allocation_error();
    }
    if(getline(&buffer, &bufsize, stdin) == -1) {
        if(feof(stdin)) {
            exit(EXIT_SUCCESS);
        }
        else {
            perror("ubash: getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return buffer;
}

char **ubash_split(char *line) {
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    if(!tokens) {
        ubash_allocation_error();
    }
    char *token = strtok(line, " \t\r\n\a");
    while(token != NULL) {
        tokens[position] = token;
        position++;
        if(position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                ubash_allocation_error();
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
        printf("ubash# ");
        command = ubash_read();
        args = ubash_split(command);
        if(strcmp(command, "exit")==0) {
            status=0;
        }
        else if(args[0]!=NULL){
            status=ubash_run(args);
        }
        free(command);
        free(args);
    } while (status);
    return EXIT_SUCCESS;
}
