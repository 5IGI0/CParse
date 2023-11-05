#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "tokens.h"
#include "tokens_utils.h"

#define IS_IDENTIFIER_FIRST_CHAR(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define IS_IDENTIFIER_CHAR(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9'))
#define IS_CONTROLFLOW_CHAR(c) (c == ';' || c == '{' || c == '}')
#define IS_HEX(c) ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define HEX2NUM(c) ((c <= '9' && c >= '0') ? c-'0' : ((c >= 'a') ? (c-'a'+10) : (c-'A'+10)))

static inline
int parse_oneline_comment(char const *data, cparse_token_t *token) {
    size_t l = strchrnul(data+2, '\n')-(data+2);

    token->type = CTOKEN_COMMENT_ONELINE;
    token->len = l+2+(data[l] != 0);
    token->d.byte_array.len = l;
    assert(token->d.byte_array.data = malloc(l+1));
    memcpy(token->d.byte_array.data, data+2, l);
    token->d.byte_array.data[l] = 0;

    return token->len;
}

static inline
int parse_multiline_comment(char const *data, cparse_token_t *token) {
    size_t l;

    {
        char const *tmp = strstr(data+2, "*/");
        if (tmp == NULL) tmp = data+strlen(data);
        l = tmp-(data+2);
    }

    token->type = CTOKEN_COMMENT_MULTILINE;
    token->len = l + 2 + (2 * (data[l] != 0));
    token->d.byte_array.len = l;
    assert(token->d.byte_array.data = malloc(l+1));
    memcpy(token->d.byte_array.data, data+2, l);
    token->d.byte_array.data[l] = 0;

    return token->len;
}

static inline
int parse_identifier(char const *data, cparse_token_t *token) {
    size_t l = 0;

    while (IS_IDENTIFIER_CHAR(data[l]))l++;
    
    cparse_keyword_meta_t const *meta = cparse_get_keyword_by_name(data, l);
    if (meta) {
        token->type = meta->type;
        token->len = l;
        return l;
    }

    token->type = CTOKEN_IDENTIFER;
    token->len = l;
    assert(token->d.byte_array.data = malloc(l+1));
    memcpy(token->d.byte_array.data, data, l);
    token->d.byte_array.data[l] = 0;

    return l;
}

static inline
int parse_operator(char const *data, cparse_token_t *token) {
    token->len=3;
    if (data[0] == '<' && data[1] == '<' && data[2] == '=')     {token->type=CTOKEN_LSHIFT_ASSIGNMENT;return 2;} // <<=
    else if (data[0] == '>' && data[1] == '>' && data[2] == '='){token->type=CTOKEN_RSHIFT_ASSIGNMENT;return 2;} // >>=

    token->len=2;
    if (data[0] == '-' && data[1] == '>')       {token->type=CTOKEN_ARROW;              return 2;} // ->
    else if (data[0] == '<' && data[1] == '<')  {token->type=CTOKEN_BITWISE_LSHIFT;     return 2;} // <<
    else if (data[0] == '>' && data[1] == '>')  {token->type=CTOKEN_BITWISE_RSHIFT;     return 2;} // >>
    else if (data[0] == '>' && data[1] == '=')  {token->type=CTOKEN_GREATER_EQ;         return 2;} // >=
    else if (data[0] == '<' && data[1] == '=')  {token->type=CTOKEN_LESSER_EQ;          return 2;} // <=
    else if (data[0] == '=' && data[1] == '=')  {token->type=CTOKEN_EQUAL;              return 2;} // ==
    else if (data[0] == '!' && data[1] == '=')  {token->type=CTOKEN_NOT_EQUAL;          return 2;} // !=
    else if (data[0] == '&' && data[1] == '&')  {token->type=CTOKEN_LOGICAL_AND;        return 2;} // &&
    else if (data[0] == '|' && data[1] == '|')  {token->type=CTOKEN_LOGICAL_OR;         return 2;} // ||
    else if (data[0] == '+' && data[1] == '=')  {token->type=CTOKEN_SUM_ASSIGNMENT;     return 2;} // +=
    else if (data[0] == '-' && data[1] == '=')  {token->type=CTOKEN_DIF_ASSIGNMENT;     return 2;} // -=
    else if (data[0] == '*' && data[1] == '=')  {token->type=CTOKEN_PRODUCT_ASSIGNMENT; return 2;} // *=
    else if (data[0] == '/' && data[1] == '=')  {token->type=CTOKEN_QUOTIENT_ASSIGNMENT;return 2;} // /=
    else if (data[0] == '%' && data[1] == '=')  {token->type=CTOKEN_MOD_ASSIGNMENT;     return 2;} // %=
    else if (data[0] == '&' && data[1] == '=')  {token->type=CTOKEN_BITAND_ASSIGNMENT;  return 2;} // &=
    else if (data[0] == '^' && data[1] == '=')  {token->type=CTOKEN_BITXOR_ASSIGNMENT;  return 2;} // ^=
    else if (data[0] == '|' && data[1] == '=')  {token->type=CTOKEN_BITOR_ASSIGNMENT;   return 2;} // |=

    token->len = 1;
    switch (data[0]) {
        case '+': token->type=CTOKEN_PLUS;          return 1;
        case '-': token->type=CTOKEN_MINUS;         return 1;
        case '/': token->type=CTOKEN_DIV;           return 1;
        case '%': token->type=CTOKEN_MOD;           return 1;
        case '*': token->type=CTOKEN_STAR;          return 1;
        case '!': token->type=CTOKEN_LOGICAL_NOT;   return 1;
        case '~': token->type=CTOKEN_BITWISE_NOT;   return 1;
        case '&': token->type=CTOKEN_AND_SYM;       return 1;
        case '.': token->type=CTOKEN_DOT;           return 1;
        case '<': token->type=CTOKEN_LESSER;        return 1;
        case '>': token->type=CTOKEN_GREATER;       return 1;
        case '^': token->type=CTOKEN_BITWISE_XOR;   return 1;
        case '|': token->type=CTOKEN_BITWISE_OR;    return 1;
        case '?': token->type=CTOKEN_QUESTION_MARK; return 1;
        case ':': token->type=CTOKEN_COLON;         return 1;
        case '=': token->type=CTOKEN_ASSIGNMENT;    return 1;
        case ',': token->type=CTOKEN_COMMA;         return 1;
        case '[': token->type=CTOKEN_SQR_OPEN_BRKT; return 1;
        case ']': token->type=CTOKEN_SQR_CLOSE_BRKT;return 1;
        case '(': token->type=CTOKEN_RND_OPEN_BRKT; return 1;
        case ')': token->type=CTOKEN_RND_CLOSE_BRKT;return 1;
        default: break;
    }
    assert(0 && "unknown operator");
}

static inline
int parse_numeric(char const *data, cparse_token_t *token) {
    char *endptr;

    endptr = (char *)data;
    if (data[0] != '.') {
        int base = 10;
        if (data[0] == '0') {
            if (isdigit(data[1]))
                base=8;
            else {
                switch (data[1]) {
                    case 'X':
                    case 'x': base=16; break;
                    case 'B':
                    case 'b': base=2; break; // NOTE: not C standard
                    default: assert(IS_IDENTIFIER_CHAR(data[1]) == 0 && "unknown base");
                }
            }
        }

        switch (base) {
            case 10: token->type=CTOKEN_DEC_INT_LITERAL; break;
            case  8: token->type=CTOKEN_OCT_INT_LITERAL; break;
            case 16: token->type=CTOKEN_HEX_INT_LITERAL; break;
            case  2: token->type=CTOKEN_BIN_INT_LITERAL; break;
            default: assert(0 && "unexcepted base");
        }
        token->d.integer = strtoull(data, &endptr, base);
        assert(IS_IDENTIFIER_CHAR(endptr[0]) == 0); // handle number subfixes
    }

    if (endptr[0] == '.') {
        token->d.fp = strtod(data, &endptr);
        assert(IS_IDENTIFIER_CHAR(endptr[0]) == 0|| endptr[0] == 'f' || endptr[0] == 'F');
        if (endptr[0] == 'f' || endptr[0] == 'F') {
            endptr++;
            token->type = CTOKEN_LIT_FLOAT;
        } else
            token->type = CTOKEN_LIT_DOUBLE;
    }

    return endptr-data;
}

#define CHARACTER_BUFFLEN 1
static inline
size_t parse_character(char const *data, char *ret, size_t *retlen) {

    // TODO:
    // [ ] \unnnn
    // [ ] \Unnnnnnnn
    // [X] \x (hex)
    // [X] \nnn (octal)
    // [X] \n\f\v\t
    
    if (data[0] != '\\') {
        ret[0] = data[0];
        retlen[0] = 1;
        return 1;
    } else if (data[1] == 'x') {
        assert(IS_HEX(data[2]));

        retlen[0] = 1;
        ret[0] = HEX2NUM(data[2]);

        if (IS_HEX(data[3])) {
            ret[0] = (ret[0]<<4)|HEX2NUM(data[3]);
            return 4;
        }        

        return 3;
    } else if (data[1] >= '0' && data[1] <= '7') {
        int i = 1;
        ret[0] = 0;
        retlen[0] = 1;

        while (i <= 3 && data[i] >= '0' && data[i] <= '7') {
            ret[0] = (data[i]-'0')+(ret[0]*8);
            i++;
        }

        return i+1;
    } else {
        retlen[0] = 1;
        switch (data[1]) {
            case '\'':  ret[0] = '\'';  return 2;
            case  '"':  ret[0] = '"';   return 2;
            case  '?':  ret[0] = '?';   return 2;
            case '\\':  ret[0] = '\\';  return 2;
            case  'a':  ret[0] = '\a';  return 2;
            case  'b':  ret[0] = '\b';  return 2;
            case  'f':  ret[0] = '\f';  return 2;
            case  'n':  ret[0] = '\n';  return 2;
            case  'r':  ret[0] = '\r';  return 2;
            case  't':  ret[0] = '\t';  return 2;
            case  'v':  ret[0] = '\v';  return 2;
            case '\n':  ret[0] = '\n';  return 2;
        }

        ret[0] = data[1];
        return 2;
    }
}

static inline
int parse_string(char const *data, cparse_token_t *token) {
    size_t  l = 0;
    size_t  retlen = 0;
    char    chr_tmp[CHARACTER_BUFFLEN];

    if (data[0] == '\'') {
        assert(data[1] != '\'' && data[l] != '\n' && data[l] != 0);
        l = parse_character(data+1, chr_tmp, &retlen);
        token->type = CTOKEN_CHR_INT_LITERAL;
        token->d.integer = chr_tmp[0];
        assert(data[1+l] == '\'');
        return l+2;
    } else if (data[0] == '"') {
        l++;
        token->type = CTOKEN_LIT_STR;
        token->d.byte_array.data = calloc(1,1);
        while (data[l] != '"') {
            assert(data[l] != '\n' && data[l] != 0);
            l += parse_character(data+l, chr_tmp, &retlen);
            assert(token->d.byte_array.data = realloc(token->d.byte_array.data, token->d.byte_array.len+retlen+1));
            memcpy(
                token->d.byte_array.data+token->d.byte_array.len,
                chr_tmp,
                retlen
            );
            token->d.byte_array.len+=retlen;
            token->d.byte_array.data[token->d.byte_array.len] = 0;
        }
        return l+1;
    } else {
        assert(0 && "unknown string type");
    }

    return l;
}

static inline
int check_for_empty_line(char const *data, size_t index) {
    while (index > 0) {
        if (data[index] == '\n')
            return 1;
        if (isspace(data[index]) == 0)
            return 0;
        index--;
    }

    return isspace(data[index]);
}

static inline
int parse_preproc_directive(char const *data, cparse_token_t *token) {
    size_t l = 1;

    assert(data[0] == '#');

    token->type = CTOKEN_PREPROC_DIR;

    while (1) {
        size_t ll;

        {
            char const *tmp = strstr(data+l, "\n");
            if (tmp == NULL)
                ll = strlen(data+l);
            else
                ll = tmp-(data+l);
        }

        assert(token->d.byte_array.data = realloc(token->d.byte_array.data, token->d.byte_array.len+ll+1));
        memcpy(
            token->d.byte_array.data+token->d.byte_array.len,
            data+l, ll);
        l+=ll+(data[l+ll]!= 0);
        token->d.byte_array.len += ll;
        token->d.byte_array.data[token->d.byte_array.len] = 0;
        if (token->d.byte_array.data[token->d.byte_array.len-1] == '\\') {
            token->d.byte_array.data[token->d.byte_array.len-1] = ' ';
        } else {
            return l;
        }
    }

    return l;
}

int cparse_tokenize(char const *data, cparse_token_t **tokens) {
    size_t index = 0;
    int token_count = 0;
    cparse_token_t token;

    tokens[0] = NULL;

    while (data[index]) {
        int ret = 0;
        index += strspn(data+index, " \t\r\n\v\f");
        if (data[index] == 0) break;

        memset(&token, 0, sizeof(token));

        if (data[index] == '#' && (index == 0 || check_for_empty_line(data, index-1))) {
            ret = parse_preproc_directive(data+index, &token);
        } else if (data[index] == '/' && data[index+1] == '/')
            ret = parse_oneline_comment(data+index, &token);
        else if (data[index] == '/' && data[index+1] == '*')
            ret = parse_multiline_comment(data+index, &token);
        else if (isdigit(data[index]) || (data[index] == '.' && isdigit(data[index+1]))) {
            ret = parse_numeric(data+index, &token);
        } else if (IS_IDENTIFIER_FIRST_CHAR(data[index])) {
            ret = parse_identifier(data+index, &token);
        } else if (IS_CONTROLFLOW_CHAR(data[index])){
            ret = 1;
            token.len = 1;
            switch (data[index]) {
                case ';': token.type = CTOKEN_SEMICON; break;
                case '{': token.type = CTOKEN_BEGIN_SCOPE; break;
                case '}': token.type = CTOKEN_END_SCOPE; break;
                default: assert(0 && "unknown control flow character"); break;
            }
        } else if (data[index] == '"' || data[index] == '\'') {
            ret = parse_string(data+index, &token);
        } else {
            ret = parse_operator(data+index, &token);
        }

        assert(ret);
        token.pos = index;
        index += ret;

        assert(tokens[0] = realloc(tokens[0], sizeof(token)*(token_count+2)));
        tokens[0][token_count] = token;
        tokens[0][token_count+1].type = CTOKEN_END;
        token_count++;
    }

    return token_count;
}