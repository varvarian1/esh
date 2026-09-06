#pragma once
#include "parser.h"

#define BREAK_CODE      -2
#define CONTINUE_CODE   -3

/* Base value for exit status when a process is terminated by a signal.
 * Status = 128 + signal_number (as in POSIX shells).
 */
#define EXIT_SIGNAL_BASE 128

int execute(ASTNode *node);
int execute_script(const char *filename, int argc, char **argv);

int is_executable_script(const char *path);
void free_expanded_argv(char **argv, int argc);
char *expand_variables(const char *arg);
int evaluate_test(int argc, char **argv);

int execute_command(ASTNode *node);
int execute_pipeline(ASTNode *node);
int execute_redirect(ASTNode *node);