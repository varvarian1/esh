#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/executor.h"
#include "../../include/symbol.h"
#include "../../include/parser.h"

int execute_script(const char *filename, int argc, char **argv) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return -1;
    }

    set_positional_args(argc, argv);
    set_symbol("0", filename, 0);

    char *content = NULL;
    size_t contentSize = 0;
    char *line = NULL;
    size_t lineLen = 0;
    ssize_t line_read;

    while ((line_read = getline(&line, &lineLen, file)) != -1) {
        char *new_content = realloc(content, contentSize + line_read + 1);
        if (!new_content) {
            perror("realloc");
            free(content);
            free(line);
            fclose(file);
            return -1;
        }
        content = new_content;
        memcpy(content + contentSize, line, line_read);
        contentSize += line_read;
        content[contentSize] = '\0';
    }

    free(line);
    fclose(file);

    if (!content || contentSize == 0) {
        free(content);
        return 0;
    }

    /* skip shebang line if present */
    char *start = content;
    if (contentSize >= 2 && content[0] == '#' && content[1] == '!') {
        start = strchr(content, '\n');
        if (start) {
            start++;  /* skip the newline character */
        } else {
            /* only shebang line, no actual script content */
            free(content);
            return 0;
        }
    }

    if (!start || *start == '\0') {
        free(content);
        return 0;
    }

    int tokenCount = 0;
    Token *tokens = tokenize(start, &tokenCount);
    free(content);

    if (!tokens || tokenCount == 0) {
        return 0;
    }

    ASTNode *ast = parse(tokens, tokenCount);
    if (!ast) {
        fprintf(stderr, "Syntax error in script: %s\n", filename);
        free_tokens(tokens, tokenCount);
        return -1;
    }

    int status = execute(ast);
    
    free_ast(ast);
    free_tokens(tokens, tokenCount);

    return status;
}