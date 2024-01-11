#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "tokens.h"

inline static
int auto_alloc(char **str, size_t *alloclen, size_t needed) {
    if (needed < *alloclen) return 0;

    char *tmp = (char *)realloc(*str, needed);
    if (tmp == NULL) return -__LINE__;
    *alloclen = needed;
    *str = tmp;
    return 0;
}

inline static
size_t get_integer_len(uint64_t i, int base) {
    size_t ret = 1;
    i /= base;

    while (i) {
        i /= base;
        ret++;
    }
    return ret;
}

#define CHR2ESC_BUFFLEN 5 // \xXX + <nullbyte>
inline static
int chr2escape(char const *in, char *out, int *readlen) {
    const char *esc_tbl[127] = {
           [0] = "\\0", ['\n'] = "\\n", ['\r'] = "\\r",
        ['\f'] = "\\f", ['\v'] = "\\v", ['\t'] = "\\t",
        ['\b'] = "\\b", ['\a'] = "\\a", ['\\'] = "\\\\",
        ['\''] = "\\'", ['\"'] = "\\\""};

    *readlen = 1;

    if (esc_tbl[in[0]]) {
        int ret = strlen(esc_tbl[in[0]]);
        memcpy(out, esc_tbl[in[0]], ret+1);
        return ret;
    } else if (isprint(in[0])) {
        out[0] = in[0];
        out[1] = 1;
        return 1;
    } else {
        return sprintf(out, "\\x%02X", out[0]);
    }
}

inline static
int escape_str(char const *in, size_t len, char **str, size_t *alloclen) {
    size_t needed = 0;
    char escchr[CHR2ESC_BUFFLEN];
    int readlen;

    if (auto_alloc(str, alloclen, 3) < 0)
            return -__LINE__;

    while (len) {
        int ret = chr2escape(in, escchr, &readlen);
        assert(readlen > 0 && ret > 0 && readlen <= len);
        if (auto_alloc(str, alloclen, needed+ret+3) < 0)
            return -__LINE__;
        memcpy(str[0]+needed+1, escchr, ret+1);
        needed+=ret;
        in+=readlen;
        len-=readlen;
    }
    str[0][0] = '"';
    str[0][needed+1] = '"';
    str[0][needed+2] = 0;
    return needed+2;
}

inline static
int int2str(int type, uint64_t num, char **str, size_t *alloclen) {
    int         base;
    char const  *prefix;
    int         prefix_len;

    if (type == CTOKEN_CHR_INT_LITERAL) {
        char tmp[2] = {num,0};
        int  tmp2;
        assert(num < 256);
        if (auto_alloc(str, alloclen, CHR2ESC_BUFFLEN+2) < 0)
            return -__LINE__;
        str[0][0] = '\'';
        tmp2 = chr2escape(tmp, str[0]+1, &tmp2);
        str[0][tmp2+1] = '\'';
        str[0][tmp2+2] = 0;
        return tmp2+2;
    }

    const struct {
        char const *prefix;
        int         prefix_len;
        int         base;
    } type2base[] = {
        {"",    0,    10},
        {"0x",  2,  0x10},
        {"0",   1,   010},
        {"0b",  2,  0b10}
    };

    {
        int idx = (type >> CTOKEN_LIT_WIDTH)-1;
        if (idx < 0 || idx >= (sizeof(type2base)/sizeof(type2base[0])))
            return -__LINE__;
        base        = type2base[idx].base;
        prefix      = type2base[idx].prefix;
        prefix_len  = type2base[idx].prefix_len;
    }

    size_t s = get_integer_len(num, base);
    if (auto_alloc(str, alloclen, prefix_len+1+s) < 0)
        return -__LINE__;

    memcpy(*str, prefix, prefix_len);
    for (size_t i = 0; i < s; i++) {
        str[0][prefix_len+s-1-i] = "0123456789ABCDEF"[num%base];
        num /= base; 
    }
    str[0][prefix_len+s] = 0;
    return prefix_len+s;
}

int cparse_stringify_token_r(cparse_token_t token, char **str, size_t *alloclen) {
    if (CTOKEN_IS_PRIMTYPE(token.type,CTOKEN_KEYWORD)) {
        cparse_keyword_meta_t const *meta = cparse_get_keyword_by_type(token.type);

        if (
            meta==NULL ||
            auto_alloc(str,alloclen,meta->keyword_len+1) < 0
        ) return -__LINE__;
        memcpy(*str, meta->keyword, meta->keyword_len+1);
        str[0][meta->keyword_len] = 0;
        return meta->keyword_len;
    }

    if (CTOKEN_IS_PRIMTYPE(token.type,CTOKEN_OPERATOR)) {
        int op_id = token.type>>CTOKEN_PRIMTYP_WIDTH;
        if (op_id <= 0 || op_id > CTOKEN_OPERATOR_COUNT) return -__LINE__;
        const char *opstr = cparse_op2str[op_id];
        size_t strl = strlen(opstr);
        if (auto_alloc(str, alloclen, strl+1) < 0) return -__LINE__;
        memcpy(*str, opstr, strl+1);
        return strl;
    }

    if (CTOKEN_IS_PRIMTYPE(token.type,CTOKEN_IDENTIFER)) {
        if (auto_alloc(str, alloclen, token.d.byte_array.len+1) < 0) return -__LINE__;
        memcpy(*str, token.d.byte_array.data, token.d.byte_array.len+1);
        return token.d.byte_array.len;
    }

    if (CTOKEN_IS_PRIMTYPE(token.type,CTOKEN_CONTROL_FLOW)) {
        if (auto_alloc(str, alloclen, 2) < 0) return -__LINE__;
        switch (token.type) {
            case CTOKEN_BEGIN_SCOPE:    str[0][0] = '{';    break;
            case CTOKEN_END_SCOPE:      str[0][0] = '}';    break;
            case CTOKEN_SEMICON:        str[0][0] = ';';    break;
            default:                                        return -__LINE__;
        }
        str[0][1] = 0;
        return 1;
    }

    if (CTOKEN_IS_PRIMTYPE(token.type,CTOKEN_PREPROC_DIR)) {
        if (auto_alloc(str, alloclen, token.d.byte_array.len+2) < 0) return -__LINE__;
        str[0][0] = '#';
        memcpy(str[0]+1, token.d.byte_array.data, token.d.byte_array.len);
        str[0][token.d.byte_array.len+1] = 0;
        return token.d.byte_array.len+1;
    }

    if (CTOKEN_IS_LIT(token.type,CTOKEN_LIT_INT))
        return int2str(token.type, token.d.integer, str, alloclen);

    if (CTOKEN_IS_LIT(token.type,CTOKEN_LIT_STR))
        return escape_str(
            token.d.byte_array.data,
            token.d.byte_array.len,
            str, alloclen);
    
    if (CTOKEN_IS_LIT(token.type,CTOKEN_LIT_FP)) {
        int n = snprintf(NULL, 0, "%f", token.d.fp);
        if (auto_alloc(str, alloclen, n+1+(token.type==CTOKEN_LIT_FLOAT)) < 0) return -__LINE__;
        snprintf(*str, *alloclen, "%f", token.d.fp);
        if (token.type==CTOKEN_LIT_FLOAT) {
            str[0][n] = 'f';
            str[0][n+1] = 0;
            n++;
        }
        return n;
    }
    
    if (token.type == CTOKEN_COMMENT_ONELINE) { // TODO: remove newlines or add more comments
        if (auto_alloc(str, alloclen, token.d.byte_array.len+3) < 0) return -__LINE__;
        memcpy(*str, "//", 2);
        memcpy(str[0]+2, token.d.byte_array.data, token.d.byte_array.len+1);
        return token.d.byte_array.len+2;
    }
    
    if (token.type == CTOKEN_COMMENT_MULTILINE) { // TODO: remove */ in comments
        if (auto_alloc(str, alloclen, token.d.byte_array.len+5) < 0) return -__LINE__;
        memcpy(*str, "/*", 2);
        memcpy(str[0]+2, token.d.byte_array.data, token.d.byte_array.len);
        memcpy(str[0]+2+token.d.byte_array.len, "*/", 3);
        return token.d.byte_array.len+4;
    }
    
    return -__LINE__;
}


char    *cparse_stringify_token(cparse_token_t token) {
    static size_t alloclen = 0;
    static char *str = NULL;
    if (cparse_stringify_token_r(token, &str, &alloclen) < 0)
        return NULL;
    return str;
}