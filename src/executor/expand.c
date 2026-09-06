#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/executor.h"
#include "../../include/symbol.h"

char *expand_variables(const char *arg) {
    if (strchr(arg, '$') == NULL) {
        return strdup(arg);
    }

    char *result = strdup("");
    if (!result) {
        return NULL;
    }

    const char *position = arg;
    const char *start = position;

    while (*position) {
        if (*position == '$') {
            if (position > start) {
                size_t currentLen = strlen(result);
                size_t appendLen = position - start;
                char *newResult = realloc(result, currentLen + appendLen + 1);
                if (!newResult) {
                    free(result);
                    return NULL;
                }
                result = newResult;
                strncat(result, start, appendLen);
            }
            position++;
            
            char name[256];
            int i = 0;
            if (*position == '{') {
                position++;
                while (*position && *position != '}') {
                    if (i < 255) {
                        name[i++] = *position;
                    }
                    position++;
                }
                if (*position == '}') {
                    position++;
                }
                name[i] = '\0';
            } else {
                while (*position && (isalnum(*position) || *position == '_')) {
                    if (i < 255) {
                        name[i++] = *position;
                    }
                    position++;
                }
                name[i] = '\0';
            }

            char *val = get_symbol_value(name);
            if (!val) {
                val = "";
            }

            size_t currentLen = strlen(result);
            size_t appendLen = strlen(val);
            char *newResult = realloc(result, currentLen + appendLen + 1);
            if (!newResult) {
                free(result);
                return NULL;
            }
            result = newResult;
            strcat(result, val);
            
            start = position;
        } else {
            position++;
        }
    }

    if (position > start) {
        size_t currentLen = strlen(result);
        size_t appendLen = position - start;
        char *newResult = realloc(result, currentLen + appendLen + 1);
        if (!newResult) {
            free(result);
            return NULL;
        }
        result = newResult;
        strncat(result, start, appendLen);
    }

    return result;
}
