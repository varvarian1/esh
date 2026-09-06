#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/lexer.h"
#include "../../include/parser.h"

ASTNode *parse_if(ParserState *ps) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_IF;

    node->data.ifNode.condition = parse_condition(ps);
    if (!node->data.ifNode.condition) {
        fprintf(stderr, "Syntax error: expected condition after 'if'\n");
        free(node);
        return NULL;
    }

    /* allow optional ';' after condition */
    match(ps, TOKEN_SEMICOLON);
    
    if (!match(ps, TOKEN_THEN)) {
        fprintf(stderr, "Syntax error: expected `then`\n");
        free_ast(node->data.ifNode.condition);
        free(node);
        return NULL;
    }

    node->data.ifNode.thenBranch = parse_list(ps);
    if (!node->data.ifNode.thenBranch) {
        free_ast(node->data.ifNode.condition);
        free(node);
        return NULL;
    }
    
    /* check for `else` */
    node->data.ifNode.elseBranch = NULL;
    if (match(ps, TOKEN_ELSE)) {
        node->data.ifNode.elseBranch = parse_list(ps);
        if (!node->data.ifNode.elseBranch) {
            free_ast(node->data.ifNode.condition);
            free_ast(node->data.ifNode.thenBranch);
            free(node);
            return NULL;
        }
    }

    if (!match(ps, TOKEN_FI)) {
        fprintf(stderr, "Syntax error: expected 'fi'\n");
        free_ast(node->data.ifNode.condition);
        free_ast(node->data.ifNode.thenBranch);
        if (node->data.ifNode.elseBranch) {
            free_ast(node->data.ifNode.elseBranch);
        }
        free(node);
        return NULL;
    }
    
    return node;
}

ASTNode *parse_condition(ParserState *ps) {
    ASTNode *left = NULL;

    while (1) {
        Token *tok = peek(ps);
        if (!tok) {
            break;
        }

        if (tok->type == TOKEN_FI || tok->type == TOKEN_ELSE || tok->type == TOKEN_DONE || tok->type == TOKEN_EOF) {
            break;
        }

        if (tok->type == TOKEN_IF) {
            fprintf(stderr, "Syntax error: nested if not allowed in condition\n");
            if (left){
                free_ast(left);
            }
            
            return NULL;
        }

        ASTNode *right = parse_pipeline(ps);
        if (!right) {
            if (left) {
                free_ast(left);
            }

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

        if (tok->type == TOKEN_AND_IF || tok->type == TOKEN_OR_IF) {
            advance(ps);
            continue;
        }

        break;
    }

    return left;
}
