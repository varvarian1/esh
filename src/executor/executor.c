
#include "../../include/executor.h"

int execute(ASTNode *node) {
    if (!node) {
        return 0;
    }

    switch (node->type) {
        case NODE_COMMAND: { return execute_command(node); }
        case NODE_PIPELINE: { return execute_pipeline(node); }
        case NODE_REDIRECT: { return execute_redirect(node); }

        case NODE_AND: {
            int left_ret = execute(node->data.binary.left);
            if (left_ret == BREAK_CODE || left_ret == CONTINUE_CODE) {
                return left_ret;
            }
            if (left_ret != 0) {
                return left_ret;
            }

            int right_ret = execute(node->data.binary.right);
            if (right_ret == BREAK_CODE || right_ret == CONTINUE_CODE) {
                return right_ret;
            }

            return right_ret;
        }
        case NODE_OR: {
            int left_ret = execute(node->data.binary.left);
            if (left_ret == BREAK_CODE || left_ret == CONTINUE_CODE) {
                return left_ret;
            }
            if (left_ret == 0) {
                return 0;
            }

            int right_ret = execute(node->data.binary.right);
            if (right_ret == BREAK_CODE || right_ret == CONTINUE_CODE) {
                return right_ret;
            }

            return right_ret;
        }
        case NODE_IF: {
            int condition = execute(node->data.ifNode.condition);
            if (condition == BREAK_CODE || condition == CONTINUE_CODE) {
                return condition;
            }
            if (condition == 0) {
                int then_ret = execute(node->data.ifNode.thenBranch);
                if (then_ret == BREAK_CODE || then_ret == CONTINUE_CODE) {
                    return then_ret;
                }
                return then_ret;
            } else if (node->data.ifNode.elseBranch) {
                int else_ret = execute(node->data.ifNode.elseBranch);
                if (else_ret == BREAK_CODE || else_ret == CONTINUE_CODE) {
                    return else_ret;
                }
                return else_ret;
            }

            return condition;
        }
        case NODE_SEMICOLON: {
            int left_ret = execute(node->data.binary.left);
            if (left_ret == BREAK_CODE || left_ret == CONTINUE_CODE) {
                return left_ret;
            }

            int right_ret = execute(node->data.binary.right);
            if (right_ret == BREAK_CODE || right_ret == CONTINUE_CODE) {
                return right_ret;
            }

            return right_ret;
        }
        case NODE_FOR: {
            ASTNode *forNode = node;
            int status = 0;
            int lastStatus = 0;

            /* if no word list, iterate over positional parameters ($@) */
            if (forNode->data.forNode.wordCount == 0) {
                int count = get_positional_argc();
                for (int i = 1; i <= count; i++) {
                    char *val = get_positional_arg(i);
                    set_symbol(forNode->data.forNode.varName, val ? val : "", 0);
                    status = execute(forNode->data.forNode.body);
                    if (status == BREAK_CODE) {
                        break;
                    }
                    if (status == CONTINUE_CODE) {
                        continue;
                    }
                    if (status != 0) {
                        lastStatus = status;
                    }
                }
            } else {
                for (int i = 0; i < forNode->data.forNode.wordCount; i++) {
                    set_symbol(forNode->data.forNode.varName,
                               forNode->data.forNode.wordList[i], 0);
                    status = execute(forNode->data.forNode.body);
                    if (status == BREAK_CODE) {
                        break;
                    }
                    if (status == CONTINUE_CODE) {
                        continue;
                    }
                    if (status != 0) {
                        lastStatus = status;
                    }
                }
            }

            return (status == BREAK_CODE) ? 0 : lastStatus;
        }
        
        case NODE_WHILE: {
            int status = 0;
            int lastStatus = 0;
            while (1) {
                int condition = execute(node->data.whileNode.condition);
                if (condition != 0) {
                    break;
                }

                status = execute(node->data.whileNode.body);
                if (status == BREAK_CODE) {
                    break;
                }
                if (status == CONTINUE_CODE) {
                    continue;
                }
                if (status != 0) {
                    lastStatus = status;
                }
            }

            return (status == BREAK_CODE) ? 0 : lastStatus;
        }
        case NODE_UNTIL: {
            int status = 0;
            int lastStatus = 0;
            while (1) {
                int condition = execute(node->data.whileNode.condition);
                if (condition == 0) {
                    break;
                }

                status = execute(node->data.whileNode.body);
                if (status == BREAK_CODE) {
                    break;
                }
                if (status == CONTINUE_CODE) {
                    continue;
                }
                if (status != 0) {
                    lastStatus = status;
                }
            }

            return (status == BREAK_CODE) ? 0 : lastStatus;
        }
        default: {
            return 0;
        }
    }
}