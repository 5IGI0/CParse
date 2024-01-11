#include <string.h>

#include "tokens.h"

const cparse_keyword_meta_t *cparse_get_keyword_by_name(char const *name, size_t l) {
    for (size_t i = 0; i < CTOKEN_KEYWORD_COUNT; i++) {
        if (l == cparse_keyword_tbl[i].keyword_len && memcmp(name, cparse_keyword_tbl[i].keyword, l) == 0)
            return cparse_keyword_tbl+i;
    }
    return NULL;
}

const cparse_keyword_meta_t *cparse_get_keyword_by_type(int type) {
    for (size_t i = 0; i < CTOKEN_KEYWORD_COUNT; i++) {
        if (cparse_keyword_tbl[i].type == type)
            return cparse_keyword_tbl+i;
    }
    return NULL;
}

cparse_token_datatype_t cparse_get_datatype_from_type(int type) {
    if (
        CTOKEN_IS_PRIMTYPE(type, CTOKEN_IDENTIFER)      ||
        CTOKEN_IS_PRIMTYPE(type, CTOKEN_COMMENT)        ||
        CTOKEN_IS_PRIMTYPE(type, CTOKEN_PREPROC_DIR)    ||
        CTOKEN_IS_LIT(type, CTOKEN_LIT_STR)    
    )   return CTOKEN_DATATYPE_ARRAY;

    if (
        CTOKEN_IS_LIT(type,CTOKEN_LIT_INT)
    )   return CTOKEN_DATATYPE_INT;

    if (
        CTOKEN_IS_LIT(type,CTOKEN_LIT_FP)
    )   return CTOKEN_DATATYPE_FP;

    return CTOKEN_DATATYPE_NO_DATA;
}

char cparse_get_needed_space(cparse_token_t *before, cparse_token_t *after) {
    if (before == NULL || after == NULL)
        return 0;

    int btype = before->type;
    int atype = after->type;

    switch (btype) {
        case CTOKEN_PREPROC_DIR:        return '\n';
        case CTOKEN_COMMENT_ONELINE:    return '\n';
        case CTOKEN_COMMENT_MULTILINE:  return '\0';
        case CTOKEN_CHR_INT_LITERAL:    return '\0';
        case CTOKEN_LIT_STR:            return '\0'; // TODO: check for not prefixed string (L"foo", ...)
        default:break;
    }

    switch (atype) {
        case CTOKEN_PREPROC_DIR:        return '\n';
        case CTOKEN_CHR_INT_LITERAL:    return '\0';
        case CTOKEN_LIT_STR:            return '\0'; // TODO: check for not prefixed string (L"foo", ...)
        case CTOKEN_COMMENT_ONELINE:    return '\0';
        case CTOKEN_COMMENT_MULTILINE:  return '\0';
        default: break;
    }

    if (
        CTOKEN_IS_PRIMTYPE(btype,CTOKEN_OPERATOR)       ||
        CTOKEN_IS_PRIMTYPE(btype,CTOKEN_CONTROL_FLOW)   ||
        CTOKEN_IS_PRIMTYPE(atype,CTOKEN_OPERATOR)       ||
        CTOKEN_IS_PRIMTYPE(atype,CTOKEN_CONTROL_FLOW)   ||
        CTOKEN_IS_PRIMTYPE(atype,CTOKEN_COMMENT)
    )   return 0;

    return ' ';
}