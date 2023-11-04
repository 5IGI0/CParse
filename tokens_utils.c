#include <string.h>

#include "tokens.h"

const cparse_keyword_meta_t *cparse_get_keyword_by_name(char const *name, size_t l) {
    for (size_t i = 0; i < CTOKEN_KEYWORD_COUNT; i++) {
        if (l == cparse_keyword_tbl[i].keyword_len && memcmp(name, cparse_keyword_tbl[i].keyword, l) == 0)
            return cparse_keyword_tbl+i;
    }
    return NULL;
}