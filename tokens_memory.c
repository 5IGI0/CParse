#include <stdlib.h>

#include "tokens.h"

void cparse_free_token(cparse_token_t *token, int free_ptr) {
    cparse_token_datatype_t dt = cparse_get_datatype_from_type(token->type);

    if (dt == CTOKEN_DATATYPE_ARRAY) {
        free(token->d.byte_array.data);
        token->d.byte_array.data = NULL;
    }
    
    if (free_ptr) {
        free(token);
    }
}

void cparse_free_token_array(cparse_token_t **tokens) {
    cparse_token_t *token = tokens[0];

    while (token->type != CTOKEN_END) {
        cparse_free_token(token, 0);
        token++;
    }
    
    free(tokens[0]);
    tokens[0] = NULL;
}