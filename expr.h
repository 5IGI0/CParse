#ifndef SIG_LIBCPARRSE_EXPR_H
#define SIG_LIBCPARRSE_EXPR_H

#include "tokens.h"

typedef struct cparse_expr_node_s {
  int type;
  
  // identifier / type names and qualifiers
  size_t            token_count;
  cparse_token_t    *tokens;

  // simple : LEFT_OPERAND + RIGHT_OPERAND
  // call   : LEFT_OPERAND(RIGHT_OPERAND)
  // call   : LEFT_OPERAND()
  // ternary: CONDITION ? LEFT_OPERAND : RIGHT_OPERAND
  // postfix: LEFT_OPERAND++
  // prefix : ++RIGHT_OPERAND
  // cast   : (TOKENS)LEFT_OPERAND
  struct cparse_expr_node_s *condition;
  struct cparse_expr_node_s *left_operand;
  struct cparse_expr_node_s *right_operand;
} cparse_expr_node_t;

int   parse_expr(cparse_token_t *tokens, size_t token_count, cparse_expr_node_t *opts);
void  free_expr(cparse_expr_node_t *expr, int flags);

#define CPARSE_FREE_EXPR_FREE_ROOT    1
#define CPARSE_FREE_EXPR_FREE_TOKENS  2

// process purpose types
#define CPARSE_EXPR_NODE_RAW_TOKEN          -1
#define CPARSE_EXPR_NODE_RAW_CHILD          -2
#define CPARSE_EXPR_NODE_RAW_INDEX          -3
#define CPARSE_EXPR_NODE_UNLINKED_INDEX     -4
#define CPARSE_EXPR_NODE_UNLINKED_SIZEOF    -5
#define CPARSE_EXPR_NODE_UNLINKED_ALIGNOF   -6
#define CPARSE_EXPR_NODE_RAW_IMPLICIT_CHILD -7

#define CPARSE_EXPR_NODE_END                  0
#define CPARSE_EXPR_NODE_IDENTIFIER           1
#define CPARSE_EXPR_NODE_LITERAL              2
#define CPARSE_EXPR_NODE_CHILD                3
#define CPARSE_EXPR_NODE_IMPLICIT_CHILD       4
#define CPARSE_EXPR_NODE_INDEX                5
#define CPARSE_EXPR_NODE_POST_INCREMENT       6
#define CPARSE_EXPR_NODE_POST_DECREMENT       7
#define CPARSE_EXPR_NODE_MEMBER               8
#define CPARSE_EXPR_NODE_PTR_MEMBER           9
#define CPARSE_EXPR_NODE_CALL                 10
#define CPARSE_EXPR_NODE_PRE_INCREMENT        11
#define CPARSE_EXPR_NODE_PRE_DECREMENT        12
#define CPARSE_EXPR_NODE_DEREFERENCE          13
#define CPARSE_EXPR_NODE_UNARY_PLUS           14
#define CPARSE_EXPR_NODE_UNARY_MINUS          15
#define CPARSE_EXPR_NODE_ADDR                 16
#define CPARSE_EXPR_NODE_CAST                 17
#define CPARSE_EXPR_NODE_SIZEOF               18
#define CPARSE_EXPR_NODE_ALIGNOF              19
#define CPARSE_EXPR_NODE_LOGICAL_NOT          20
#define CPARSE_EXPR_NODE_BITWISE_NOT          21
#define CPARSE_EXPR_NODE_BITWISE_AND          22
#define CPARSE_EXPR_NODE_MULT                 23
#define CPARSE_EXPR_NODE_DIV                  24
#define CPARSE_EXPR_NODE_MOD                  25
#define CPARSE_EXPR_NODE_BITWISE_RSHIFT       26
#define CPARSE_EXPR_NODE_BITWISE_LSHIFT       27
#define CPARSE_EXPR_NODE_GREATER              28
#define CPARSE_EXPR_NODE_GREATER_EQ           29
#define CPARSE_EXPR_NODE_LESSER               30
#define CPARSE_EXPR_NODE_LESSER_EQ            31
#define CPARSE_EXPR_NODE_EQUAL                32
#define CPARSE_EXPR_NODE_NOT_EQUAL            33
#define CPARSE_EXPR_NODE_BITWISE_XOR          34
#define CPARSE_EXPR_NODE_BITWISE_OR           35
#define CPARSE_EXPR_NODE_LOGICAL_AND          36
#define CPARSE_EXPR_NODE_LOGICAL_OR           37
#define CPARSE_EXPR_NODE_ASSIGNMENT           38
#define CPARSE_EXPR_NODE_SUM_ASSIGNMENT       39
#define CPARSE_EXPR_NODE_DIF_ASSIGNMENT       40
#define CPARSE_EXPR_NODE_PRODUCT_ASSIGNMENT   41
#define CPARSE_EXPR_NODE_QUOTIENT_ASSIGNMENT  42
#define CPARSE_EXPR_NODE_MOD_ASSIGNMENT       43
#define CPARSE_EXPR_NODE_LSHIFT_ASSIGNMENT    44
#define CPARSE_EXPR_NODE_RSHIFT_ASSIGNMENT    45
#define CPARSE_EXPR_NODE_BITAND_ASSIGNMENT    46
#define CPARSE_EXPR_NODE_BITOR_ASSIGNMENT     47
#define CPARSE_EXPR_NODE_BITXOR_ASSIGNMENT    48
#define CPARSE_EXPR_NODE_COMMA                49
#define CPARSE_EXPR_NODE_PLUS                 50
#define CPARSE_EXPR_NODE_MINUS                51
#define CPARSE_EXPR_NODE_UNINTERPRETED_CHILD  52
#define CPARSE_EXPR_NODE_TERNARY              53

#endif // SIG_LIBCPARRSE_EXPR_H