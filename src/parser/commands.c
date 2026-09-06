#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/lexer.h"
#include "../../include/parser.h"

ASTNode *parse_pipeline(ParserState *ps) {
    ASTNode *left = parse_command(ps);
    if (!left) {
        return NULL;
    }

    while (match(ps, TOKEN_PIPE)) {
        ASTNode *right = parse_command(ps);
        if (!right) {
            free_ast(left);
            return NULL;
        }
        left = new_binary_node(NODE_PIPELINE, left, right);
    }
    return left;
}

ASTNode *parse_command(ParserState *ps) {
    char **argv = NULL;
    int argc = 0;
    
    /* collect all words that form the command name + arguments */
    while (1) {
        Token *tok = peek(ps);
        if (!tok || tok->type != TOKEN_WORD) {
            break;
        }

        Token *next = peek_next(ps);
        if (next && (next->type == TOKEN_THEN || 
                     next->type == TOKEN_ELSE || 
                     next->type == TOKEN_FI)) {

            advance(ps);
            argv = realloc(argv, (argc + 1) * sizeof(char *));
            argv[argc++] = strdup(tok->value);
            break;
        }

        advance(ps);
        argv = realloc(argv, (argc + 1) * sizeof(char *));
        argv[argc++] = strdup(tok->value);
    }
    
    if (argc == 0) {
        return NULL;
    }
    argv = realloc(argv, (argc + 1) * sizeof(char *));
    argv[argc] = NULL;

    /* create base command node */
    ASTNode *node = new_command_node(argv, argc);

    /* process redirections that follow the command */
    while (1) {
        Token *tok = peek(ps);
        if (!tok) {
            break;
        }

        int mode;
        if (tok->type == TOKEN_LESS) {
            mode = 0;      /* input redirection (<) */
        } else if (tok->type == TOKEN_GREATER) {
            mode = 1;      /* output redirection (>) */
        } else if (tok->type == TOKEN_DGREAT) {
            mode = 2;      /* append redirection (>>) */
        } else {
            break;
        }
        advance(ps);

        /* expect a filename after the redirection operator */
        tok = peek(ps);
        if (!tok || tok->type != TOKEN_WORD) {
            fprintf(stderr, "Syntax error: expected filename\n");
            free_ast(node);
            return NULL;
        }
        advance(ps);
        char *filename = strdup(tok->value);

        int target_fd;
        if (mode == 0) {
            target_fd = 0;
        } else {
            target_fd = 1;
        }

        /* wrap the existing node with a redirect node */
        ASTNode *new_node = new_redirect_node(node, target_fd, filename, mode);
        node = new_node;
    }

    return node;
}