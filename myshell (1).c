// myshell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "myshell.h"

// Show current directory in the prompt
void print_prompt() {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    printf("%s> ", cwd);
}

// Parse command input into arguments and extract redirection info
void parse_input(char *input, char **args, int *background,
                 char **input_file, char **output_file, int *append) {
    *background = 0;
    *input_file = NULL;
    *output_file = NULL;
    *append = 0;

    int i = 0;
    char *token = strtok(input, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        if (strcmp(token, "&") == 0) {
            *background = 1;
        } else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            *input_file = token;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " \t\n");
            *output_file = token;
            *append = 1;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            *output_file = token;
            *append = 0;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

// Return 1 if command is internal
int is_internal_command(const char *cmd) {
    const char *commands[] = {
        "cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", NULL
    };
    for (int i = 0; commands[i]; i++) {
        if (strcmp(cmd, commands[i]) == 0) return 1;
    }
    return 0;
}

// Main shell loop
int main(int argc, char *argv[]) {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    int background;
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;

    // Set shell=full path to myshell
    char shell_path[PATH_MAX];
    realpath(argv[0], shell_path);
    setenv("shell", shell_path, 1);

    //  Welcome banner
    printf("\n----------------OUR SHELL---------------------\n");
    printf("----- REEM AFANA ** Noor El-Kurd ** Abeer Al-Sibakhi -----\n");
    printf("Type 'help' for the user manual\n");
    printf("to show information about the command you want.\n\n");

    // Use batch file if provided
    FILE *input_stream = stdin;
    if (argc == 2) {
        input_stream = fopen(argv[1], "r");
        if (!input_stream) {
            perror("Batch file error");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        if (input_stream == stdin)
            print_prompt();

        if (!fgets(input, sizeof(input), input_stream))
            break;  // Ctrl+D or EOF

        // Parse input and redirection info
        parse_input(input, args, &background, &input_file, &output_file, &append);
        if (args[0] == NULL) continue;

        // Redirect input/output if needed
        int in = dup(STDIN_FILENO);
        int out = dup(STDOUT_FILENO);

        if (input_file) freopen(input_file, "r", stdin);
        if (output_file) {
            if (append)
                freopen(output_file, "a", stdout);
            else
                freopen(output_file, "w", stdout);
        }

        // Execute command
        if (is_internal_command(args[0])) {
            run_internal_command(args);
        } else {
            run_external_command(args, shell_path, background);
        }

        // Restore standard input/output
        fflush(stdout);
        dup2(in, STDIN_FILENO);
        dup2(out, STDOUT_FILENO);
        close(in);
        close(out);
    }

    if (input_stream != stdin) fclose(input_stream);
    return 0;
}

