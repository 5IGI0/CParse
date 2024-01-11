#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "tokens.h"
#include "expr.h"

#define DEBUG_LOG(prefix, nodes, count) \
printf("%s", prefix);                   \
for (size_t i = 0; i < count; i++) {    \
    printf("%d:%d, ", (nodes)[i].type, ((nodes)[i].type==CPARSE_EXPR_NODE_RAW_TOKEN)?(nodes)[i].tokens[0].type:0);    \
}                                       \
puts("");        

int parse_expr(cparse_token_t *tokens, size_t token_count, cparse_expr_node_t *opts);
int resolv_unresolved_nodes(cparse_expr_node_t *root);

static inline int easy_fail(cparse_expr_node_t *node, size_t node_count, int ret) {
    for (size_t i = 0; i < node_count; i++) {
        free_expr(&node[i], 0);
    }
    return free(node), ret;
}

static inline int is_value(cparse_expr_node_t node) {
    return (
        node.type != CPARSE_EXPR_NODE_RAW_TOKEN &&
        node.type != CPARSE_EXPR_NODE_ALIGNOF   &&
        node.type != CPARSE_EXPR_NODE_SIZEOF);
}

#define MERGE_LEFT  1
#define MERGE_RIGHT 2
#define MERGE_BOTH  (MERGE_LEFT|MERGE_RIGHT)
static inline int merge_nodes(cparse_expr_node_t *nodes, int *node_count, int *pos, int flags) {
    int tmp_pos = *pos;

    if (flags & MERGE_RIGHT) {
        nodes[tmp_pos].right_operand = malloc(sizeof(*nodes));
        if (nodes[tmp_pos].right_operand == NULL)
            return -__LINE__;
    }

    if (flags & MERGE_LEFT) {
        nodes[tmp_pos].left_operand = malloc(sizeof(*nodes));
        if (nodes[tmp_pos].left_operand == NULL) {
            if (flags & MERGE_RIGHT)
                free(nodes[tmp_pos].right_operand);
            return -__LINE__;
        }
    }

    if (flags & MERGE_RIGHT) {
        memcpy(nodes[tmp_pos].right_operand, &nodes[tmp_pos+1], sizeof(*nodes));
        memmove(&nodes[tmp_pos+1], &nodes[tmp_pos+2], sizeof(*nodes)*(*node_count-(tmp_pos+2)));
        node_count[0]--;
    }

    if (flags & MERGE_LEFT) {
        memcpy(nodes[tmp_pos].left_operand, &nodes[tmp_pos-1], sizeof(*nodes));
        memmove(&nodes[tmp_pos-1], &nodes[tmp_pos], sizeof(*nodes)*(*node_count-tmp_pos));
        pos[0]--;
        tmp_pos--;
        node_count[0]--;
    }
    
    return 0;
}

static inline int count_until_closed(cparse_token_t *tokens, size_t token_count) {
    int rnd_count = 0;
    int sqr_count = 0;

    // TODO: check for open and close order if [ is opened in ( then [ must be closed before )
    for (size_t i = 0; i < token_count; i++) {
        switch (tokens[i].type) {
            case CTOKEN_RND_OPEN_BRKT:  rnd_count++; break;
            case CTOKEN_RND_CLOSE_BRKT: rnd_count--; break;
            case CTOKEN_SQR_OPEN_BRKT:  sqr_count++; break;
            case CTOKEN_SQR_CLOSE_BRKT: sqr_count--; break;
            default:    break;
        }

        if (rnd_count < 0 || sqr_count < 0) return -1;
        else if (rnd_count == 0 && sqr_count == 0)  return i+1;
    }

    return -1;
}

static int  add_implicit_nodes(cparse_expr_node_t *nodes, int *node_count, int *pos) {
    int tmp_pos = *pos;

    if (nodes[tmp_pos].type != CPARSE_EXPR_NODE_RAW_TOKEN ||
        nodes[tmp_pos].tokens[0].type != CTOKEN_QUESTION_MARK) 
        return 0;

    for (size_t i = tmp_pos+2; i < *node_count; i++) {
        if (nodes[i].type == CPARSE_EXPR_NODE_RAW_TOKEN &&
            nodes[i].tokens[0].type == CTOKEN_COLON) {
            nodes[tmp_pos+1].type = CPARSE_EXPR_NODE_RAW_IMPLICIT_CHILD;
            nodes[tmp_pos+1].token_count = nodes[i].tokens-nodes[tmp_pos].tokens-1;
            nodes[tmp_pos+1].tokens = nodes[tmp_pos].tokens+1;
            memmove(&nodes[tmp_pos+2], &nodes[i], sizeof(*nodes)*((*node_count)-i));
            node_count[0] -= i-tmp_pos-2;
            return 0;
        }
    }

    return -__LINE__;
}

static int  parse_opt_level_1(cparse_expr_node_t *nodes, int *node_count, int *pos) {
    int tmp_pos = *pos;

    if (nodes[tmp_pos].type == CPARSE_EXPR_NODE_RAW_TOKEN) {
        int type = nodes[tmp_pos].tokens[0].type;

        if (type == CTOKEN_INCREMENT || type == CTOKEN_DECREMENT) {
            if (
                tmp_pos == 0 ||
                is_value(nodes[tmp_pos-1]) == 0
            )  return 0;

            switch (type) {
                case CTOKEN_INCREMENT: nodes[tmp_pos].type = CPARSE_EXPR_NODE_POST_INCREMENT; break;
                case CTOKEN_DECREMENT: nodes[tmp_pos].type = CPARSE_EXPR_NODE_POST_DECREMENT; break;
            }

            return merge_nodes(nodes, node_count, pos, MERGE_LEFT);
        }

        if (type == CTOKEN_DOT || type == CTOKEN_ARROW) {
            if (
                tmp_pos == 0                                       ||
                tmp_pos == *node_count-1                           ||
                is_value(nodes[tmp_pos-1]) == 0 
            )   return -__LINE__;
            
            switch (type) {
                case CTOKEN_DOT:    nodes[tmp_pos].type = CPARSE_EXPR_NODE_MEMBER; break;
                case CTOKEN_ARROW:  nodes[tmp_pos].type = CPARSE_EXPR_NODE_PTR_MEMBER; break;
            }

            return merge_nodes(nodes, node_count, pos, MERGE_BOTH);
        }
    } else if (nodes[tmp_pos].type == CPARSE_EXPR_NODE_RAW_INDEX) {
        if (
                tmp_pos == 0 ||
                is_value(nodes[tmp_pos-1]) == 0 
        )   return -__LINE__;

        nodes[tmp_pos].type = CPARSE_EXPR_NODE_INDEX;

        return merge_nodes(nodes, node_count, pos, MERGE_LEFT);
    } else if (nodes[tmp_pos].type == CPARSE_EXPR_NODE_RAW_CHILD) {
        if (
                tmp_pos == 0 ||
                is_value(nodes[tmp_pos-1]) == 0 
        )   return 0;

        nodes[tmp_pos].type = CPARSE_EXPR_NODE_CALL;

        // do not parse expression in empty call
        if (nodes[tmp_pos].tokens[1].type != CTOKEN_RND_CLOSE_BRKT) {
            assert(nodes[tmp_pos].right_operand = calloc(1, sizeof(*nodes)));
            int ret = parse_expr(nodes[tmp_pos].tokens+1, nodes[tmp_pos].token_count-2, nodes[tmp_pos].right_operand);
            if (ret != 0) {
                free(nodes[tmp_pos].right_operand);
                nodes[tmp_pos].right_operand = NULL;
                return -1;
            }
            
        }
        
        return merge_nodes(nodes, node_count, pos, MERGE_LEFT);
    }
    return 0;
}

static int  parse_opt_level_2(cparse_expr_node_t *nodes, int *node_count, int *pos) {
    int tmp_pos = *pos;

    if (nodes[tmp_pos].type == CPARSE_EXPR_NODE_RAW_TOKEN) {
        int type = nodes[tmp_pos].tokens[0].type;

        if (type == CTOKEN_INCREMENT || type == CTOKEN_DECREMENT) {
            if (
                tmp_pos == (*node_count) - 1 ||
                is_value(nodes[tmp_pos+1]) == 0
            )  return 0;

            switch (type) {
                case CTOKEN_INCREMENT: nodes[tmp_pos].type = CPARSE_EXPR_NODE_PRE_INCREMENT; break;
                case CTOKEN_DECREMENT: nodes[tmp_pos].type = CPARSE_EXPR_NODE_PRE_DECREMENT; break;
            }

            return merge_nodes(nodes, node_count, pos, MERGE_RIGHT);
        }

        if (
            type == CTOKEN_STAR     ||
            type == CTOKEN_PLUS     ||
            type == CTOKEN_MINUS    ||
            type == CTOKEN_AND_SYM) {
            if (
                tmp_pos == (*node_count)-1 ||
                (tmp_pos != 0 && is_value(nodes[tmp_pos-1]))
            ) return 0;

            switch (type) {
                case CTOKEN_STAR:       nodes[tmp_pos].type = CPARSE_EXPR_NODE_DEREFERENCE; break;
                case CTOKEN_PLUS:       nodes[tmp_pos].type = CPARSE_EXPR_NODE_UNARY_PLUS; break;
                case CTOKEN_MINUS:      nodes[tmp_pos].type = CPARSE_EXPR_NODE_UNARY_MINUS; break;
                case CTOKEN_AND_SYM:    nodes[tmp_pos].type = CPARSE_EXPR_NODE_ADDR; break;
            }

            return merge_nodes(nodes, node_count, pos, MERGE_RIGHT);
        }

        if (
            type == CTOKEN_LOGICAL_NOT ||
            type == CTOKEN_BITWISE_NOT) {
            if (
                tmp_pos == (*node_count)-1 ||
                is_value(nodes[tmp_pos+1]) == 0
            ) return 0;

            switch (type) {
                case CTOKEN_LOGICAL_NOT: nodes[tmp_pos].type = CPARSE_EXPR_NODE_LOGICAL_NOT; break;
                case CTOKEN_BITWISE_NOT: nodes[tmp_pos].type = CPARSE_EXPR_NODE_BITWISE_NOT; break;
            }

            return merge_nodes(nodes, node_count, pos, MERGE_RIGHT);
        }
    } else if (nodes[tmp_pos].type == CPARSE_EXPR_NODE_RAW_CHILD) {
        if (tmp_pos == (*node_count)-1 ||
            is_value(nodes[tmp_pos+1]) == 0)
        return 0;

        nodes[tmp_pos].type = CPARSE_EXPR_NODE_CAST;
        return merge_nodes(nodes, node_count, pos, MERGE_RIGHT);
    } else if (nodes[tmp_pos].type == CPARSE_EXPR_NODE_UNLINKED_SIZEOF || nodes[tmp_pos].type == CPARSE_EXPR_NODE_UNLINKED_ALIGNOF) {
        if (tmp_pos == (*node_count)-1 ||
            is_value(nodes[tmp_pos+1]) == 0)
        return 0;

        switch (nodes[tmp_pos].type) {
            case CPARSE_EXPR_NODE_UNLINKED_SIZEOF:  nodes[tmp_pos].type = CPARSE_EXPR_NODE_SIZEOF; break;
            case CPARSE_EXPR_NODE_UNLINKED_ALIGNOF: nodes[tmp_pos].type = CPARSE_EXPR_NODE_ALIGNOF; break;
        }

        if (nodes[tmp_pos+1].type == CPARSE_EXPR_NODE_RAW_CHILD)
            nodes[tmp_pos+1].type = CPARSE_EXPR_NODE_UNINTERPRETED_CHILD;

        return merge_nodes(nodes, node_count, pos, MERGE_RIGHT);
    }

    return 0;
}

static int  parse_opt_level_n(cparse_expr_node_t *nodes, int *node_count, int *pos, int level) {
    int out_type = CPARSE_EXPR_NODE_END;
    int tmp_pos = *pos;

    if (
        tmp_pos < 0                                         ||
        tmp_pos >= *node_count                              ||
        nodes[tmp_pos].type != CPARSE_EXPR_NODE_RAW_TOKEN   ||
        is_value(nodes[tmp_pos+1]) == 0                     ||
        is_value(nodes[tmp_pos-1]) == 0
    ) return 0;

    // improvement: use table instead of ton of switch
    switch (level) {
        case 3: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_DIV:    out_type = CPARSE_EXPR_NODE_DIV;     break;
            case CTOKEN_STAR:   out_type = CPARSE_EXPR_NODE_MULT;    break;
            case CTOKEN_MOD:    out_type = CPARSE_EXPR_NODE_MOD;     break;
            default:break;
        } break;
        case 4: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_PLUS:   out_type = CPARSE_EXPR_NODE_PLUS;   break;
            case CTOKEN_MINUS:  out_type = CPARSE_EXPR_NODE_MINUS;  break;
            default:break;
        } break;
        case 5: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_BITWISE_LSHIFT: out_type = CPARSE_EXPR_NODE_BITWISE_LSHIFT;     break;
            case CTOKEN_BITWISE_RSHIFT: out_type = CPARSE_EXPR_NODE_BITWISE_RSHIFT;     break;
            default:break;
        } break;
        case 6: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_LESSER:         out_type = CPARSE_EXPR_NODE_LESSER;     break;
            case CTOKEN_LESSER_EQ:      out_type = CPARSE_EXPR_NODE_LESSER_EQ;  break;
            case CTOKEN_GREATER:        out_type = CPARSE_EXPR_NODE_GREATER;     break;
            case CTOKEN_GREATER_EQ:     out_type = CPARSE_EXPR_NODE_GREATER_EQ;  break;
            default:break;
        } break;
        case 7: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_EQUAL:      out_type = CPARSE_EXPR_NODE_EQUAL;     break;
            case CTOKEN_NOT_EQUAL:  out_type = CPARSE_EXPR_NODE_NOT_EQUAL;  break;
            default:break;
        } break;
        case 8: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_AND_SYM:    out_type = CPARSE_EXPR_NODE_BITWISE_AND;     break;
            default:break;
        } break;
        case 9: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_BITWISE_XOR: out_type = CPARSE_EXPR_NODE_BITWISE_XOR;     break;
            default:break;
        } break;
        case 10: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_BITWISE_OR: out_type = CPARSE_EXPR_NODE_BITWISE_OR;     break;
            default:break;
        } break;
        case 11: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_LOGICAL_AND: out_type = CPARSE_EXPR_NODE_LOGICAL_AND;     break;
            default:break;
        } break;
        case 12: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_LOGICAL_OR: out_type = CPARSE_EXPR_NODE_LOGICAL_OR;     break;
            default:break;
        } break;
        case 14: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_ASSIGNMENT:             out_type = CPARSE_EXPR_NODE_ASSIGNMENT;         break;
            case CTOKEN_DIF_ASSIGNMENT:         out_type = CPARSE_EXPR_NODE_DIF_ASSIGNMENT;     break;
            case CTOKEN_MOD_ASSIGNMENT:         out_type = CPARSE_EXPR_NODE_MOD_ASSIGNMENT;     break;
            case CTOKEN_BITOR_ASSIGNMENT:       out_type = CPARSE_EXPR_NODE_BITOR_ASSIGNMENT;   break;
            case CTOKEN_BITAND_ASSIGNMENT:      out_type = CPARSE_EXPR_NODE_BITAND_ASSIGNMENT;  break;
            case CTOKEN_BITXOR_ASSIGNMENT:      out_type = CPARSE_EXPR_NODE_BITXOR_ASSIGNMENT;  break;
            case CTOKEN_LSHIFT_ASSIGNMENT:      out_type = CPARSE_EXPR_NODE_LSHIFT_ASSIGNMENT;  break;
            case CTOKEN_RSHIFT_ASSIGNMENT:      out_type = CPARSE_EXPR_NODE_RSHIFT_ASSIGNMENT;  break;
            case CTOKEN_PRODUCT_ASSIGNMENT:     out_type = CPARSE_EXPR_NODE_PRODUCT_ASSIGNMENT; break;
            case CTOKEN_QUOTIENT_ASSIGNMENT:    out_type = CPARSE_EXPR_NODE_QUOTIENT_ASSIGNMENT;break;
            case CTOKEN_SUM_ASSIGNMENT:         out_type = CPARSE_EXPR_NODE_SUM_ASSIGNMENT;     break;
            default:break;
        } break;
        case 15: switch (nodes[tmp_pos].tokens[0].type) {
            case CTOKEN_COMMA: out_type = CPARSE_EXPR_NODE_COMMA; break;
            default: break;
        }
        default: break;
    }

    if (out_type != CPARSE_EXPR_NODE_END) {
        nodes[tmp_pos].type = out_type;
        return merge_nodes(nodes, node_count, pos, MERGE_BOTH);
    }

    return 0;
}

static int  parse_opt_level_13(cparse_expr_node_t *nodes, int *node_count, int *pos) {
    int tmp_pos = *pos;

    if (tmp_pos == 0 ||
        tmp_pos >= node_count[0]-3 ||
        // precedent node is not a token
        nodes[tmp_pos-1].type == CPARSE_EXPR_NODE_RAW_TOKEN         ||
        // this node is ?
        nodes[tmp_pos].type != CPARSE_EXPR_NODE_RAW_TOKEN           ||
        nodes[tmp_pos].tokens[0].type != CTOKEN_QUESTION_MARK       ||
        // next node is implicit parenthesis
        nodes[tmp_pos+1].type != CPARSE_EXPR_NODE_RAW_IMPLICIT_CHILD||
        // next node is :
        nodes[tmp_pos+2].type != CPARSE_EXPR_NODE_RAW_TOKEN           ||
        nodes[tmp_pos+2].tokens[0].type != CTOKEN_COLON               ||
        // next node is not a token
        nodes[tmp_pos+3].type == CPARSE_EXPR_NODE_RAW_TOKEN)
        return 0;

    nodes[tmp_pos].condition        = malloc(sizeof(*nodes));
    nodes[tmp_pos].right_operand    = malloc(sizeof(*nodes));
    nodes[tmp_pos].left_operand     = malloc(sizeof(*nodes));

    if (nodes[tmp_pos].condition        == NULL ||
        nodes[tmp_pos].right_operand    == NULL ||
        nodes[tmp_pos].left_operand     == NULL) {
        free(nodes[tmp_pos].condition);
        free(nodes[tmp_pos].right_operand);
        free(nodes[tmp_pos].left_operand);
        nodes[tmp_pos].condition        = NULL;
        nodes[tmp_pos].right_operand    = NULL;
        nodes[tmp_pos].left_operand     = NULL;
        return -__LINE__;
    }
    
    nodes[tmp_pos].type = CPARSE_EXPR_NODE_TERNARY;
    memcpy(nodes[tmp_pos].condition, &nodes[tmp_pos-1],     sizeof(*nodes));
    memcpy(nodes[tmp_pos].left_operand, &nodes[tmp_pos+1],  sizeof(*nodes));
    memcpy(nodes[tmp_pos].right_operand, &nodes[tmp_pos+3], sizeof(*nodes));
    memcpy(&nodes[tmp_pos-1], &nodes[tmp_pos], sizeof(*nodes));
    memmove(&nodes[tmp_pos], &nodes[tmp_pos+4], sizeof(*nodes)*((*node_count)-(tmp_pos+4)));
    *node_count -= 4;
    pos[0]--;

    return 0;
}

int parse_expr(cparse_token_t *tokens, size_t token_count, cparse_expr_node_t *opts) {
    int                 node_count  = 0;
    cparse_expr_node_t  *tmp_expr   = NULL;

    for (size_t i = 0; i < token_count; i++) {
        cparse_expr_node_t  node = {0};
        if (tokens[i].type == CTOKEN_RND_OPEN_BRKT || tokens[i].type == CTOKEN_SQR_OPEN_BRKT) {
            int c = count_until_closed(tokens+i, token_count-i);
            if (c < 0)  return free(tmp_expr), -__LINE__;

            if      (tokens[i].type == CTOKEN_RND_OPEN_BRKT)    node.type = CPARSE_EXPR_NODE_RAW_CHILD;
            else if (tokens[i].type == CTOKEN_SQR_OPEN_BRKT)    node.type = CPARSE_EXPR_NODE_RAW_INDEX;
            else return free(tmp_expr), -__LINE__;

            node.token_count = c;
            node.tokens = tokens+i;
            i += c - 1;
        } else if (tokens[i].type == CTOKEN_RND_CLOSE_BRKT || tokens[i].type == CTOKEN_SQR_CLOSE_BRKT) {
            return free(tmp_expr), -__LINE__;
        } else if (CTOKEN_IS_PRIMTYPE(tokens[i].type, CTOKEN_COMMENT)) {
            // comments gonna break parsing
        } else if (CTOKEN_IS_PRIMTYPE(tokens[i].type, CTOKEN_KEYWORD)) {
            switch (tokens[i].type) {
                case CTOKEN_KW_SIZEOF:      node.type = CPARSE_EXPR_NODE_UNLINKED_SIZEOF; break;
                case CTOKEN_KW__ALIGNOF:    node.type = CPARSE_EXPR_NODE_UNLINKED_ALIGNOF; break;
                case CTOKEN_KW_ALIGNOF:     node.type = CPARSE_EXPR_NODE_UNLINKED_ALIGNOF; break;
                default: return free(tmp_expr), -__LINE__;
            }
            node.token_count = 1;
            node.tokens = tokens+i;
        } else {
            if      (CTOKEN_IS_PRIMTYPE(tokens[i].type, CTOKEN_IDENTIFER))  node.type = CPARSE_EXPR_NODE_IDENTIFIER;
            else if (CTOKEN_IS_PRIMTYPE(tokens[i].type, CTOKEN_LITERAL))    node.type = CPARSE_EXPR_NODE_LITERAL;
            else                                                            node.type = CPARSE_EXPR_NODE_RAW_TOKEN;
            node.token_count = 1;
            node.tokens = tokens+i;
        }
        node_count += 1;
        assert(tmp_expr = realloc(tmp_expr, sizeof(node)*node_count+1));
        memcpy(tmp_expr+node_count-1, &node, sizeof(node));
    }

    // add implicit parenthesis nodes
    for (int i = node_count-1; i >= 0; i--) {
        assert(add_implicit_nodes(tmp_expr, &node_count, &i) == 0);
    }

    for (size_t i = 0; i < node_count; i++) {
        if (tmp_expr[i].type == CPARSE_EXPR_NODE_RAW_INDEX) {
            assert(tmp_expr[i].right_operand = calloc(1, sizeof(*tmp_expr)));
            int ret = parse_expr(tmp_expr[i].tokens+1, tmp_expr[i].token_count-2, tmp_expr[i].right_operand);
            assert(ret == 0);
        }
    }

    for (int i = 0; i < node_count; i++) {
        if (parse_opt_level_1(tmp_expr, &node_count, &i) != 0)
            return easy_fail(tmp_expr, node_count, -__LINE__);
    }
    
    for (int i = node_count-1; i >= 0; i--) {
        if (parse_opt_level_2(tmp_expr, &node_count, &i) != 0)
            return easy_fail(tmp_expr, node_count, -__LINE__);
    }

    for (size_t lvl = 3; lvl <= 12; lvl++) {
        // can't be operation at the begin or at the end
        for (int i = 1; i < node_count-1; i++) {
            if (parse_opt_level_n(tmp_expr, &node_count, &i, lvl) != 0)
                return easy_fail(tmp_expr, node_count, -__LINE__);
        }
    }

    for (int i = node_count-1; i >= 0; i--) {
        if (parse_opt_level_13(tmp_expr, &node_count, &i) != 0)
            return easy_fail(tmp_expr, node_count, -__LINE__);
    }

    for (int i = node_count-1; i >= 0; i--) {
        if (parse_opt_level_n(tmp_expr, &node_count, &i, 14) != 0)
            return easy_fail(tmp_expr, node_count, -__LINE__);
    }

    // can't be operation at the begin or at the end
    for (int i = 1; i < node_count-1; i++) {
        if (parse_opt_level_n(tmp_expr, &node_count, &i, 15) != 0)
            return easy_fail(tmp_expr, node_count, -__LINE__);
    }
    
    if (node_count != 1)
        return easy_fail(tmp_expr, node_count, -__LINE__);

    int ret = resolv_unresolved_nodes(tmp_expr);
    if (ret < 0)
        return easy_fail(tmp_expr, node_count, -__LINE__);

    memcpy(opts, tmp_expr, sizeof(*opts));
    return free(tmp_expr), 0;
}

int resolv_unresolved_nodes(cparse_expr_node_t *root) {
    while (root) {
        if (root->type == CPARSE_EXPR_NODE_RAW_CHILD) {
            root->right_operand = calloc(1, sizeof(*root->right_operand));
            if (root->right_operand == NULL)
                return -__LINE__;
            int ret = parse_expr(root->tokens+1, root->token_count-2, root->right_operand);
            if (ret < 0) return ret;
            root->type = CPARSE_EXPR_NODE_CHILD;
        } else if (root->type == CPARSE_EXPR_NODE_RAW_IMPLICIT_CHILD) {
            root->right_operand = calloc(1, sizeof(*root->right_operand));
            if (root->right_operand == NULL)
                return -__LINE__;
            int ret = parse_expr(root->tokens, root->token_count, root->right_operand);
            if (ret < 0) return ret;
            root->type = CPARSE_EXPR_NODE_IMPLICIT_CHILD;
        } else {
            if (root->condition) {
                int ret = resolv_unresolved_nodes(root->condition);
                if (ret < 0) return ret;
            }
            if (root->left_operand) {
                int ret = resolv_unresolved_nodes(root->left_operand);
                if (ret < 0) return ret;
            }
        }

        if (root->type < 0)
            return -__LINE__;
        
        root = root->right_operand;
    }
    return 0;
}