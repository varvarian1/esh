#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../include/executor.h"

int execute_redirect(ASTNode *node) {
    int fd = node->data.redirect.fd;
    char *filename = node->data.redirect.filename;
    int mode = node->data.redirect.mode; /* 0 - <, 1 - >, 2 - >> */

    int saved_fd = dup(fd);
    if (saved_fd == -1) {
        perror("dup");
        return -1;
    }

    int flags = O_WRONLY | O_CREAT;
    if (mode == 0) {
        flags = O_RDONLY;
    } else if (mode == 2) {
        flags |= O_APPEND;
    } else {
        flags |= O_TRUNC;
    }

    int new_fd = open(filename, flags, 0644);
    if (new_fd == -1) {
        perror("open");
        dup2(saved_fd, fd);
        close(saved_fd);

        return -1;
    }

    if (dup2(new_fd, fd) == -1) {
        perror("dup2");
        close(new_fd);
        dup2(saved_fd, fd);
        close(saved_fd);

        return -1;
    }
    close(new_fd);

    int result = execute(node->data.redirect.child);

    dup2(saved_fd, fd);
    close(saved_fd);

    return result;
}