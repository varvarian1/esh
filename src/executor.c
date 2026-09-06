#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "../include/executor.h"
#include "../include/symbol.h"
#include "../include/history.h"

static int execute_command(ASTNode *node);
static int execute_pipeline(ASTNode *node);
static int execute_redirect(ASTNode *node);
static char *expand_variables(const char *arg);
static void free_expanded_argv(char **argv, int argc);
static int evaluate_test(int argc, char **argv);

