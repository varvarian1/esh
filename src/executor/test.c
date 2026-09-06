#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../../include/executor.h"

int evaluate_test(int argc, char **argv) {
    /* format: [expr] */
    if (argc < 2) return 1; /* false */
    
    /* check that the last argument is "]" */
    int last = argc-1;
    if (strcmp(argv[last], "]") != 0) {
        return 1;
    }

    int expr_argc = last-1;
    if (expr_argc == 0) return 1; /* empty expression -> false */
    
    if (expr_argc == 1) {
        const char *arg = argv[1];
        return (arg[0] != '\0' ? 0 : 1);
    }
    if (expr_argc == 2) {
        /* unary operators (2 arguments: operator + operand) */
        const char *op = argv[1];
        const char *operand = argv[2];
        struct stat st;

        if (strcmp(op, "-f") == 0) {
            return (stat(operand, &st) == 0 && S_ISREG(st.st_mode)) ? 0 : 1;
        }
        else if (strcmp(op, "-d") == 0) {
            return (stat(operand, &st) == 0 && S_ISDIR(st.st_mode)) ? 0 : 1;
        }
        else if (strcmp(op, "-e") == 0) {
            return (stat(operand, &st) == 0) ? 0 : 1;
        }
        else if (strcmp(op, "-x") == 0) {
            return (stat(operand, &st) == 0 && (st.st_mode & S_IXUSR)) ? 0 : 1;
        }
        else if (strcmp(op, "-r") == 0) {
            return (stat(operand, &st) == 0 && (st.st_mode & S_IRUSR)) ? 0 : 1;
        }
        else if (strcmp(op, "-w") == 0) {
            return (stat(operand, &st) == 0 && (st.st_mode & S_IWUSR)) ? 0 : 1;
        }
        else if (strcmp(op, "-z") == 0) {
            return (operand[0] == '\0') ? 0 : 1;
        }
        else if (strcmp(op, "-n") == 0) {
            return (operand[0] != '\0') ? 0 : 1;
        }
        
        return 1; /* unknown operator -> false */
    }
    
    /* Binary operators (3 arguments: ... ) */
    if (expr_argc == 3) {
        const char *left = argv[1];
        const char *op = argv[2];
        const char *right = argv[3];

        /* string comparison */
        if (strcmp(op, "=") == 0) {
            return (strcmp(left, right) == 0) ? 0 : 1;
        }
        if (strcmp(op, "!=") == 0) {
            return (strcmp(left, right) != 0) ? 0 : 1;
        }

        /* numeric comparison */
        int left_num = atoi(left);
        int right_num = atoi(right);

        if (strcmp(op, "-eq") == 0) {
            return (left_num == right_num) ? 0 : 1;
        }
        else if (strcmp(op, "-ne") == 0) {
            return (left_num != right_num) ? 0 : 1;
        }
        else if (strcmp(op, "-lt") == 0) {
            return (left_num < right_num) ? 0 : 1;
        }
        else if (strcmp(op, "-le") == 0) {
            return (left_num <= right_num) ? 0 : 1;
        }
        else if (strcmp(op, "-gt") == 0) {
            return (left_num > right_num) ? 0 : 1;
        }
        else if (strcmp(op, "-ge") == 0) {
            return (left_num >= right_num) ? 0 : 1;
        } 
    }
    
    /* fallback: if no condition matched, return false */
    return 1;
}
