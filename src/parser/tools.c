#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/lexer.h"
#include "../../include/parser.h"

/* returns current token without consuming it */
Token *peek(ParserState *ps) {
    if (ps->pos < ps->count) {
        return &ps->tokens[ps->pos];
    }
    return NULL;
}

/* returns next token without consuming it */
Token *peek_next(ParserState *ps) {
    if (ps->pos + 1 < ps->count) {
        return &ps->tokens[ps->pos + 1];
    }
    return NULL;
}

/* consumes and returns current token, advancing position */
Token *advance(ParserState *ps) {
    if (ps->pos < ps->count) {
        return &ps->tokens[ps->pos++];
    }
    return NULL;
}

/* checks if next token matches type; consumes it if true */
int match(ParserState *ps, TokenType type) {
    Token *tok = peek(ps);
    if (tok && tok->type == type) {
        advance(ps);
        return 1;
    }
    return 0;
}

ASTNode *new_command_node(char **argv, int argc) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_COMMAND;
    node->data.command.argv = argv;
    node->data.command.argc = argc;
    return node;
}

ASTNode *new_binary_node(NodeType type, ASTNode *left, ASTNode *right) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = type;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

ASTNode *new_redirect_node(ASTNode *child, int fd, char *filename, int mode) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_REDIRECT;
    node->data.redirect.child = child;
    node->data.redirect.fd = fd;
    node->data.redirect.filename = filename;
    node->data.redirect.mode = mode;
    return node;
}

void free_ast(ASTNode *node) {
    if (!node) {
        return;
    }
    
    switch (node->type) {
        case NODE_COMMAND:
            if (node->data.command.argv) {
                for (int i = 0; node->data.command.argv[i]; i++) {
                    free(node->data.command.argv[i]);
                }
                free(node->data.command.argv);
            }
            break;
            
        case NODE_PIPELINE:
        case NODE_AND:
        case NODE_OR:
        case NODE_SEMICOLON: {
            free_ast(node->data.binary.left);
            free_ast(node->data.binary.right);
            break;
        }

        case NODE_REDIRECT: {
            free_ast(node->data.redirect.child);
            free(node->data.redirect.filename);
            break;
        }

        case NODE_IF: {
            free_ast(node->data.ifNode.condition);
            free_ast(node->data.ifNode.thenBranch);
            if (node->data.ifNode.elseBranch) {
                free_ast(node->data.ifNode.elseBranch);
            }
            break;
        }
        
        case NODE_FOR: {
            free(node->data.forNode.varName);
            for (int i = 0; i < node->data.forNode.wordCount; i++) {
                free(node->data.forNode.wordList[i]);
            }
            
            free(node->data.forNode.wordList);
            free_ast(node->data.forNode.body);
            break;
        }
        
        case NODE_WHILE:
        case NODE_UNTIL:
            free_ast(node->data.whileNode.condition);
            free_ast(node->data.whileNode.body);
            break;

        default: {
            break;
        }
    }
    free(node);
}
