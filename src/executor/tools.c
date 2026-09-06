#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../../include/executor.h"

int is_executable_script(const char* path) {
    if (!strchr(path, '/')) {
        return 0;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR);
}

void free_expanded_argv(char **argv, int argc) {
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}