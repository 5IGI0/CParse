#ifndef SIG_LIBCPARSE_TOKENS_STRINGIFY_H_
#define SIG_LIBCPARSE_TOKENS_STRINGIFY_H_

#include <stddef.h>

#include "tokens.h"

int cparse_stringify_token(cparse_token_t token, char **str, size_t *alloclen);

#endif