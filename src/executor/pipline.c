#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "../../include/executor.h"

int execute_pipeline(ASTNode *node) {
    struct sigaction sa_old, sa_new;
    sa_new.sa_handler = SIG_IGN;
    sigemptyset(&sa_new.sa_mask);
    sa_new.sa_flags = 0;
    sigaction(SIGPIPE, &sa_new, &sa_old);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        sigaction(SIGPIPE, &sa_old, NULL);
        perror("pipe");

        return -1;
    }

    pid_t left_pid = fork();
    if (left_pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        
        return -1;
    }
    if (left_pid == 0) {
        /* child: execute left command with stdout -> pipe */
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        
        /* For built-in commands, we need to execute them directly */
        /* But they will write to the pipe because we redirected stdout */
        int result = execute_command(node->data.binary.left);
        exit(result);
    }

    pid_t right_pid = fork();
    if (right_pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        kill(left_pid, SIGTERM);
        waitpid(left_pid, NULL, 0);
        return -1;
    }
    if (right_pid == 0) {

        /* child: execute right command with stdin <- pipe */
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        
        int result = execute_command(node->data.binary.right);
        exit(result);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    int left_status, right_status;
    waitpid(left_pid, &left_status, 0);
    waitpid(right_pid, &right_status, 0);

    sigaction(SIGPIPE, &sa_old, NULL);

    if (WIFEXITED(right_status)) {
        return WEXITSTATUS(right_status);
    }
    if (WIFSIGNALED(right_status)) {
        return EXIT_SIGNAL_BASE + WTERMSIG(right_status);
    }
    return -1;
}