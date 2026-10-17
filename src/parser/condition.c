#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../include/lexer.h"
#include "../../include/parser.h"

ASTNode *parse_for(ParserState *ps) {
    Token *varTok = peek(ps);
    if (!varTok || varTok->type != TOKEN_WORD) {
        fprintf(stderr, "Syntax error: expected variable name after 'for'\n");
        return NULL;
    }
    advance(ps); // consume variable name

    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) {
        return NULL;
    }

    node->type = NODE_FOR;
    node->data.forNode.varName = strdup(varTok->value);
    node->data.forNode.wordList = NULL;
    node->data.forNode.wordCount = 0;
    node->data.forNode.body = NULL;
    
    /* optional 'in' keyword */
    if (match(ps, TOKEN_IN)) {
        /* collect words until 'do' */
        while (1) {
            Token *tok = peek(ps);
            if (!tok || tok->type == TOKEN_DO || tok->type == TOKEN_SEMICOLON) {
                break;
            }
            if (tok->type != TOKEN_WORD) {
                fprintf(stderr, "Syntax error: expected word in list\n");
                free_ast(node);
                return NULL;
            }
            advance(ps);
            node->data.forNode.wordList = realloc(node->data.forNode.wordList,
                (node->data.forNode.wordCount + 1) * sizeof(char*));
            node->data.forNode.wordList[node->data.forNode.wordCount++] = strdup(tok->value);
        }
    }
    
    /* optional ';' before 'do' */
    match(ps, TOKEN_SEMICOLON);

    if (!match(ps, TOKEN_DO)) {
        fprintf(stderr, "Syntax error: expected 'do'\n");
        free_ast(node);
        return NULL;
    }

    node->data.forNode.body = parse_list(ps);
    if (!node->data.forNode.body) {
        free_ast(node);
        return NULL;
    }

    if (!match(ps, TOKEN_DONE)) {
        fprintf(stderr, "Syntax error: expected 'done'\n");
        free_ast(node->data.forNode.body);
        free_ast(node);
        return NULL;
    }

    return node;
}

ASTNode *parse_while(ParserState *ps) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) {
        return NULL;
    }

    node->type = NODE_WHILE;
    node->data.whileNode.condition = parse_pipeline(ps);
    if (!node->data.whileNode.condition) {
        free(node);
        return NULL;
    }
    
    /* optional ';' after condition */
    match(ps, TOKEN_SEMICOLON);
    
    if (!match(ps, TOKEN_DO)) {
        fprintf(stderr, "Syntax error: expected 'do'\n");    
        
        free_ast(node->data.whileNode.condition);
        free(node);
    
        return NULL;
    }

    node->data.whileNode.body = parse_list(ps);
    if (!node->data.whileNode.body) {
        free_ast(node->data.whileNode.condition);
        free(node);

        return NULL;
    }
    if (!match(ps, TOKEN_DONE)) {
        fprintf(stderr, "Syntax error: expected 'done'\n");
        
        free_ast(node->data.whileNode.condition);
        free_ast(node->data.whileNode.body);
        free(node);

        return NULL;
    }

    return node;
}

ASTNode *parse_until(ParserState *ps) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) {
        return NULL;
    }

    node->type = NODE_UNTIL;
    node->data.whileNode.condition = parse_pipeline(ps);
    if (!node->data.whileNode.condition) {
        free(node);
        return NULL;
    }

    /* optional ';' after condition */
    match(ps, TOKEN_SEMICOLON);

    if (!match(ps, TOKEN_DO)) {
        fprintf(stderr, "Syntax error: expected 'do'\n");
        free_ast(node->data.whileNode.condition);
        free(node);

        return NULL;
    }

    node->data.whileNode.body = parse_list(ps);
    if (!node->data.whileNode.body) {
        free_ast(node->data.whileNode.condition);
        free(node);

        return NULL;
    }

    if (!match(ps, TOKEN_DONE)) {
        fprintf(stderr, "Syntax error: expected 'done'\n");
        free_ast(node->data.whileNode.condition);
        free_ast(node->data.whileNode.body);
        free(node);

        return NULL;
    }

    return node;
}
