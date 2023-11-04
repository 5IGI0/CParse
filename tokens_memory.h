#ifndef SIG_LIBCPARSE_TOKENS_MEMORY_H_
#define SIG_LIBCPARSE_TOKENS_MEMORY_H_

#include "tokens.h"

void cparse_free_token(cparse_token_t *token);
void cparse_free_token_array(cparse_token_t **tokens);

#endif // SIG_LIBCPARSE_TOKENS_MEMORY_H_