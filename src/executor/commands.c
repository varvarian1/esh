#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../include/executor.h"
#include "../../include/symbol.h"
#include "../../include/history.h"

#define MAX_ALIAS_EXPANSIONS 10

/* Split a string into words by whitespace.
 * Returns a NULL-terminated array of strdup'ed strings.
 * The caller owns both the array and the strings. */
static char **split_words(const char *str, int *count) {
    char **words = NULL;
    int n = 0;
    const char *p = str;

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        const char *start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        size_t len = p - start;

        char *w = malloc(len + 1);
        if (!w) break;
        memcpy(w, start, len);
        w[len] = '\0';

        char **tmp = realloc(words, (n + 1) * sizeof(char *));
        if (!tmp) { free(w); break; }
        words = tmp;
        words[n++] = w;
    }

    *count = n;
    return words;
}

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

    int depth = 0;
    while (depth < MAX_ALIAS_EXPANSIONS) {
        char *alias_val = get_alias_value(argv[0]);
        if (!alias_val) break;

        int awc;
        char **aw = split_words(alias_val, &awc);
        if (awc == 0) { free(aw); break; }

        int new_argc = awc + (argc - 1);
        char **new_argv = malloc((new_argc + 1) * sizeof(char *));
        if (!new_argv) {
            for (int i = 0; i < awc; i++) free(aw[i]);
            free(aw);
            break;
        }

        /* Copy alias words and original args (except argv[0]) into new_argv */
        int k = 0;
        for (int i = 0; i < awc; i++) new_argv[k++] = aw[i];
        for (int i = 1; i < argc; i++) new_argv[k++] = argv[i];
        new_argv[new_argc] = NULL;

        /* Free old command name and old array (strings are moved to new_argv) */
        free(argv[0]);
        free(aw);
        free(expanded_argv);

        expanded_argv = new_argv;
        argv = expanded_argv;
        argc = new_argc;
        depth++;
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

    if (strcmp(argv[0], "alias") == 0) {
        if (argc == 1) {
            print_aliases();
        } else {
            for (int i = 1; i < argc; i++) {
                char *eq = strchr(argv[i], '=');
                if (eq) {
                    *eq = '\0';
                    set_alias(argv[i], eq+1);
                    *eq = '=';
                } else {
                    char *val = get_alias_value(argv[i]);
                    if (val) { printf("alias %s='%s'\n", argv[i], val); }
                }
            }
        }

        free_expanded_argv(expanded_argv, argc);
        return 0;
    }

    if (strcmp(argv[0], "unalias") == 0) {
        for (int i = 1; i < argc; i++) {
            unset_alias(argv[i]);
        }

        free_expanded_argv(expanded_argv, argc);
        return 0;
    }

    if (strcmp(argv[0], "[") == 0) {
        int status = evaluate_test(argc, argv);
        free_expanded_argv(expanded_argv, argc);
        
        return status;
    }

    if (strchr(argv[0], '=') != NULL) {
        char *eq = strchr(argv[0], '=');
        *eq = '\0';
        set_symbol(argv[0], eq + 1, 0);
        *eq = '=';
        free_expanded_argv(expanded_argv, argc);
        return 0;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        free_expanded_argv(expanded_argv, argc);
        return -1;
    }
    if (pid == 0) {
        /* Child process */
        export_environment();
        execvp(argv[0], argv);
        
        /* If execvp failed because it's a script without a shebang,
         * fall back to interpreting it with esh but still
         * inside the child, so the parent remains isolated. */
        if (errno == ENOEXEC) {
            int status = execute_script(argv[0], argc - 1, &argv[1]);
            exit(status);
        }
        
        perror("execvp");
        exit(1);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        free_expanded_argv(expanded_argv, argc);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}
