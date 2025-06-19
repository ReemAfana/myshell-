// myshell.h
#ifndef MYSHELL_H
#define MYSHELL_H

#define MAX_INPUT 1024
#define MAX_ARGS 64

// Function declarations
void print_prompt();
void parse_input(char *input, char **args, int *background,
                 char **input_file, char **output_file, int *append);
int is_internal_command(const char *cmd);
void run_internal_command(char **args);
void run_external_command(char **args, const char *myshell_path, int background);

#endif


