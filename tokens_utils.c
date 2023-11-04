#include <string.h>

#include "tokens.h"
#include "tokens_utils.h"

const cparse_keyword_meta_t *cparse_get_keyword_by_name(char const *name, size_t l) {
    for (size_t i = 0; i < CTOKEN_KEYWORD_COUNT; i++) {
        if (l == cparse_keyword_tbl[i].keyword_len && memcmp(name, cparse_keyword_tbl[i].keyword, l) == 0)
            return cparse_keyword_tbl+i;
    }
    return NULL;
}

cparse_token_datatype_t cparse_get_datatype_from_type(int type) {
    if (
        CTOKEN_IS_PRIMTYPE(type, CTOKEN_IDENTIFER)        ||
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