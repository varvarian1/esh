#pragma once

typedef struct Symbol
{
    char *name;
    char *value;
    int exported; // 1 if exported to environment, 0 otherwise
    struct Symbol *next;
} Symbol;

typedef struct Alias
{
    char *name;
    char *value;
    struct Alias *next;
} Alias;

int set_alias(const char *name, const char *value);
int unset_alias(const char *name);
char *get_alias_value(const char *name);
void print_aliases(void);

void import_environment(void);
Symbol *get_symbol(const char *name);
int set_symbol(const char *name, const char *value, int exported);
int unset_symbol(const char *name);
char *get_symbol_value(const char *name);
void set_positional_args(int argc, char **argv);
char *get_positional_arg(int index);
int get_positional_argc(void);
void export_environment(void);
int get_positional_argc(void);
