#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../../include/lexer.h"
#include "../../include/parser.h"
#include "../../include/symbol.h"

ASTNode *parse_list(ParserState *ps) {
    ASTNode *left = NULL;

    while (1) {
        Token *tok = peek(ps);
        if (!tok) {
            break;
        }

        /* skip empty commands: consecutive ; or \n */
        if (tok->type == TOKEN_SEMICOLON) {
            advance(ps);
            continue;
        }

        /* stop on fi, else, done, or EOF */
        if (tok->type == TOKEN_FI || tok->type == TOKEN_ELSE || tok->type == TOKEN_DONE || tok->type == TOKEN_EOF) {
            break;
        }

        /* if... */
        if (tok->type == TOKEN_IF) {
            advance(ps);
            ASTNode *ifNode = parse_if(ps);
            if (!ifNode) {
                if (left) free_ast(left);
                return NULL;
            }
            if (!left) {
                left = ifNode;
            } else {
                left = new_binary_node(NODE_SEMICOLON, left, ifNode);
            }
            continue;
        }

        /* for... */
        if (tok->type == TOKEN_FOR) {
            advance(ps);
            ASTNode *forNode = parse_for(ps);
            if (!forNode) {
                if (left) free_ast(left);
                return NULL;
            }
            if (!left) {
                left = forNode;
            } else {
                left = new_binary_node(NODE_SEMICOLON, left, forNode);
            }
            continue;
        }
        
        /* while... */
        if (tok->type == TOKEN_WHILE) {
            advance(ps);
            ASTNode *whileNode = parse_while(ps);
            if (!whileNode) {
                if (left) {
                    free_ast(left);
                }
                return NULL;
            }
            if (!left) {
                left = whileNode;
            } else {
                left = new_binary_node(NODE_SEMICOLON, left, whileNode);
            }
            continue;
        }

        /* until... */
        if (tok->type == TOKEN_UNTIL) {
            advance(ps);
            ASTNode *untilNode = parse_until(ps);
            if (!untilNode) {
                if (left) {
                    free_ast(left);
                }
                return NULL;
            }
            if (!left) {
                left = untilNode;
            } else {
                left = new_binary_node(NODE_SEMICOLON, left, untilNode);
            }
            continue;
        }

        ASTNode *right = parse_pipeline(ps);
        if (!right) {
            if (left) free_ast(left);
            return NULL;
        }

        if (!left) {
            left = right;
        } else {
            left = new_binary_node(NODE_SEMICOLON, left, right);
        }

        tok = peek(ps);
        if (!tok) {
            break;
        }

        if (tok->type == TOKEN_SEMICOLON ||
            tok->type == TOKEN_AND_IF ||
            tok->type == TOKEN_OR_IF) {
            advance(ps);
            continue;
        }

        if (tok->type != TOKEN_IF && tok->type != TOKEN_ELSE && tok->type != TOKEN_FI && tok->type != TOKEN_DONE && tok->type != TOKEN_EOF) {
            break;
        }
    }

    return left;
}

ASTNode *parse(Token *tokens, int tokenCount) {
    ParserState ps = { tokens, tokenCount, 0 };
    ASTNode *node = parse_list(&ps);
    Token *tok = peek(&ps);
    
    if (tok && tok->type != TOKEN_EOF) {
        fprintf(stderr, "Syntax error: unexpected token\n");
        free_ast(node);
        return NULL;
    }
    return node;
}

ASTNode *parse_line(const char *line) {
    int tokenCount = 0;
    Token *tokens = tokenize(line, &tokenCount);
    if (!tokens) {
        return NULL;
    }

    ASTNode *ast = parse(tokens, tokenCount);
    free_tokens(tokens, tokenCount);

    return ast;
}