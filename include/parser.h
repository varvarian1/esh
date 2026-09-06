#pragma once

#include "lexer.h"

typedef struct ParserState
{
    Token *tokens;
    int count;
    int pos;
} ParserState;

typedef enum NodeType
{
    NODE_COMMAND,
    NODE_PIPELINE,
    NODE_REDIRECT,
    NODE_AND,
    NODE_OR,
    NODE_IF,
    NODE_ELSE,
    NODE_FOR,
    NODE_WHILE,
    NODE_UNTIL,
    NODE_SEMICOLON,
} NodeType;

typedef struct ASTNode
{
    NodeType type;
    union
    {
        struct
        {
            char **argv;
            int argc;
        } command;
        
        struct 
        {
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;

        struct
        {
            struct ASTNode *child;

            int fd;
            char *filename;
            int mode;
        } redirect;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *thenBranch;
            struct ASTNode *elseBranch;
        } ifNode;

        struct
        {
            char *varName;
            char **wordList;
            int wordCount;
            struct ASTNode *body;
        } forNode;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *body;
        } whileNode;
    } data;
} ASTNode;

ASTNode *parse(Token *tokens, int tokenCount);
ASTNode *parse_line(const char *line);

Token *peek(ParserState *ps);
Token *peek_next(ParserState *ps);
Token *advance(ParserState *ps);
int match(ParserState *ps, TokenType type);

ASTNode *new_command_node(char **argv, int argc);
ASTNode *new_binary_node(NodeType type, ASTNode *left, ASTNode *right);
ASTNode *new_redirect_node(ASTNode *child, int fd, char *filename, int mode);

ASTNode *parse_pipeline(ParserState *ps);
ASTNode *parse_command(ParserState *ps);
ASTNode *parse_condition(ParserState *ps);
ASTNode *parse_if(ParserState *ps);
ASTNode *parse_for(ParserState *ps);
ASTNode *parse_while(ParserState *ps);
ASTNode *parse_until(ParserState *ps);
ASTNode *parse_list(ParserState *ps);

void free_ast(ASTNode *node);
