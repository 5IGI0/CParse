#include <stdlib.h>

#include "expr.h"
#include "tokens.h"

void free_expr(cparse_expr_node_t *expr, int flags) {
    int is_first = 1;
    while (expr) {
        if (flags&CPARSE_FREE_EXPR_FREE_TOKENS) {
            for (size_t i = 0; i < expr->token_count; i++) {
                cparse_free_token(&expr->tokens[i], 0);
            }
            free(expr->tokens);
        }
        if (expr->left_operand)
            free_expr(expr->left_operand, flags|CPARSE_FREE_EXPR_FREE_ROOT);
        if (expr->condition)
            free_expr(expr->condition, flags|CPARSE_FREE_EXPR_FREE_ROOT);
        void *tmp = expr->right_operand;
        if ((flags&CPARSE_FREE_EXPR_FREE_ROOT) || is_first == 0) free(expr);
        is_first = 0;
        expr = tmp;
    }
}