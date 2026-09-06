#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../include/executor.h"
#include "../../include/symbol.h"
#include "../../include/history.h"

int execute_command(ASTNode *node) {
    char **origin_argv = node->data.command.argv;
    int argc = node->data.command.argc;

    if (argc == 0) {
        return 0;
    }
    
    /* expand variables */
    char **expanded_argv = malloc((argc+1) * sizeof(char*));
    for (int i = 0; i < argc; i++) {
        expanded_argv[i] = expand_variables(origin_argv[i]);
    }
    expanded_argv[argc] = NULL;
    char **argv = expanded_argv;

    if (is_executable_script(argv[0])) {
        int status = execute_script(argv[0], argc-1, &argv[1]);
        free_expanded_argv(expanded_argv, argc);
        return status;
    }

    if (strcmp(argv[0], "exit") == 0) {
        int code;
        if ((argc > 1)) {
            code = atoi(argv[1]);
        } else {
            code = 0;
        }
        free_expanded_argv(expanded_argv, argc);
        exit(code);
    }
    if (strcmp(argv[0], "cd") == 0) {
        const char *path = (argc > 1) ? argv[1] : getenv("HOME");
        if (chdir(path) != 0) {
            perror("cd");
            free_expanded_argv(expanded_argv, argc);
            return 1;
        }

        free_expanded_argv(expanded_argv, argc);
        return 0;
    }
    if (strcmp(argv[0], "history") == 0) {
        show_shell_history();
        free_expanded_argv(expanded_argv, argc);
        return 0;
    }
    if (strcmp(argv[0], "export") == 0) {
        for (int i = 1; i < argc; i++) {
            char *eq = strchr(argv[i], '=');
            if (eq) {
                *eq = '\0';
                set_symbol(argv[i], eq+1, 1);
                *eq = '=';
            } else {
                set_symbol(argv[i], get_symbol_value(argv[i]) ? get_symbol_value(argv[i]) : "", 1);
            }
        }

        export_environment();
        free_expanded_argv(expanded_argv, argc);
        return 0;
    }
    if (strcmp(argv[0], "unset") == 0) {
        for (int i = 1; i < argc; i++) {
            unset_symbol(argv[i]);
        }

        export_environment();
        free_expanded_argv(expanded_argv, argc);
        return 0;
    }

    if (strcmp(argv[0], "break") == 0) {
        free_expanded_argv(expanded_argv, argc);
        return BREAK_CODE;
    }
    if (strcmp(argv[0], "continue") == 0) {
        free_expanded_argv(expanded_argv, argc);
        return CONTINUE_CODE;
    }

    if (strcmp(argv[0], "[") == 0) {
        int status = evaluate_test(argc, argv);
        free_expanded_argv(expanded_argv, argc);
        
        return status;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        free_expanded_argv(expanded_argv, argc);
        return -1;
    }
    if (pid == 0) {
        export_environment();
        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}
