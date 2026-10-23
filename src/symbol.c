#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/symbol.h"

static Alias *aliasTable = NULL;
static Symbol *symbol_table = NULL;
static char **positional_argv = NULL;
static int positional_argc = 0;

/* Import all environment variables into the symbol table.
 *
 * The global variable 'environ' (declared with 'extern') is a NULL-terminated
 * array of strings in the form "NAME=value". This function iterates over it,
 * splits each string at the '=' character, and adds the variable to the symbol
 * table with the 'exported' flag set to 1.
 *
 * We temporarily replace '=' with '\0' to split the string in-place, then
 * restore it after extracting the value. This avoids unnecessary memory
 * allocation.
 */
void import_environment(void) {
    extern char **environ;
    for (char **env = environ; *env; env++) {
        char *equal_ptr = strchr(*env, '=');
        if (equal_ptr) {
            *equal_ptr = '\0';
            set_symbol(*env, equal_ptr + 1, 1);
            *equal_ptr = '=';
        }
    }
}

Symbol *get_symbol(const char *name) {
    Symbol *symbol = symbol_table;
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }

    return NULL;
}

int set_symbol(const char *name, const char *value, int exported) {
    Symbol *symbol = get_symbol(name);
    if (symbol) {
        free(symbol->value);
        symbol->value = strdup(value ? value : "");
        symbol->exported = exported;
        return 0;
    }

    symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->value = strdup(value ? value : "");
    symbol->exported = exported;
    symbol->next = symbol_table;
    
    symbol_table = symbol;
    return 0;
}

int unset_symbol(const char *name) {
    Symbol *prevSymbol = NULL;
    Symbol *symbol = symbol_table;

    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            if (prevSymbol) {
                prevSymbol->next = symbol->next;
            } else {
                symbol_table = symbol->next;
            }

            free(symbol->name);
            free(symbol->value);
            free(symbol);

            return 0;
        }

        prevSymbol = symbol;
        symbol = symbol->next;
    }

    return -1; // not found
}

char *get_symbol_value(const char *name) {
    Symbol *symbol = get_symbol(name);
    if (symbol) {
        return symbol->value;
    } else {
        return NULL;
    }
}

void set_positional_args(int argc, char **argv) {
    positional_argc = argc;
    positional_argv = malloc((argc+1) * sizeof(char*));
    for (int i = 0; i < argc; i++) {
        positional_argv[i] = strdup(argv[i]);
    }
    positional_argv[argc] = NULL;
}

char *get_positional_arg(int index) {
    if (index >= 1 && index <= positional_argc) {
        return positional_argv[index-1];
    }
    return NULL;
}

void export_environment(void) {
    Symbol *symbol = symbol_table;
    while (symbol) {
        if (symbol->exported) {
            setenv(symbol->name, symbol->value, 1);
        }
        symbol = symbol->next;
    }
}

int get_positional_argc(void) {
    return positional_argc;
}

static Alias *find_alias(const char *name) {
    Alias *a = aliasTable;
    while (a) {
        if (strcmp(a->name, name) == 0) return a;
        a = a->next;
    }
    return NULL;
}

int set_alias(const char *name, const char *value) {
    Alias *a = find_alias(name);
    if (a) {
        free(a->value);
        a->value = strdup(value);
        return 0;
    }
    a = malloc(sizeof(Alias));
    if (!a) return -1;
    a->name = strdup(name);
    a->value = strdup(value);
    a->next = aliasTable;
    aliasTable = a;
    return 0;
}

int unset_alias(const char *name) {
    Alias *prev = NULL, *a = aliasTable;
    while (a) {
        if (strcmp(a->name, name) == 0) {
            if (prev) prev->next = a->next;
            else aliasTable = a->next;
            free(a->name);
            free(a->value);
            free(a);
            return 0;
        }
        prev = a;
        a = a->next;
    }
    return -1;
}

char *get_alias_value(const char *name) {
    Alias *a = find_alias(name);
    return a ? a->value : NULL;
}

void print_aliases(void) {
    Alias *a = aliasTable;
    while (a) {
        printf("alias %s='%s'\n", a->name, a->value);
        a = a->next;
    }
}
