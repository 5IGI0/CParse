#ifndef SIG_LIBCPARSE_TOKENS_UTILS_H_
#define SIG_LIBCPARSE_TOKENS_UTILS_H_

#include "tokens.h"

const cparse_keyword_meta_t *cparse_get_keyword_by_name(char const *name, size_t l);
const cparse_keyword_meta_t *cparse_get_keyword_by_type(int type);

typedef enum {
    CTOKEN_DATATYPE_NO_DATA     = 0,
    CTOKEN_DATATYPE_FP          = 1,
    CTOKEN_DATATYPE_INT         = 2,
    CTOKEN_DATATYPE_ARRAY       = 3
} cparse_token_datatype_t;
cparse_token_datatype_t cparse_get_datatype_from_type(int type);

char cparse_get_needed_space(cparse_token_t *before, cparse_token_t *after);

#endif // SIG_LIBCPARSE_TOKENS_UTILS_H_