
// utility.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/wait.h>
#include "myshell.h"

extern char **environ;

// Handle internal shell commands
void run_internal_command(char **args) {
    char cwd[PATH_MAX];

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd error");
            } else {
                getcwd(cwd, sizeof(cwd));
                setenv("PWD", cwd, 1);
            }
        }
    }

    else if (strcmp(args[0], "clr") == 0) {
        printf("\033[H\033[J");
    }

    else if (strcmp(args[0], "dir") == 0) {
        const char *path = args[1] ? args[1] : ".";
        DIR *dir = opendir(path);
        if (!dir) {
            perror("dir error");
            return;
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            printf("%s\n", entry->d_name);
        }
        closedir(dir);
    }

    else if (strcmp(args[0], "environ") == 0) {
        for (char **env = environ; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    }

    else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    }

    else if (strcmp(args[0], "help") == 0) {
        FILE *fp = fopen("readme", "r");
        if (!fp) {
            perror("help: cannot open readme");
            return;
        }
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            fputs(line, stdout);
        }
        fclose(fp);
    }

    else if (strcmp(args[0], "pause") == 0) {
        printf("Press Enter to continue...");
        while (getchar() != '\n');
    }

    else if (strcmp(args[0], "quit") == 0) {
        exit(0);
    }

    else {
        printf("Unrecognized internal command\n");
    }
}

// Run external programs with fork+exec
void run_external_command(char **args, const char *myshell_path, int background) {
    pid_t pid = fork();

    if (pid == 0) {
        setenv("parent", myshell_path, 1);
        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        if (!background) {
            waitpid(pid, NULL, 0);
        } else {
            printf("Process running in background [PID: %d]\n", pid);
        }
    } else {
        perror("fork failed");
    }
}

