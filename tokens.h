#ifndef SIG_LIBCPARSE_TOKENS_H_
#define SIG_LIBCPARSE_TOKENS_H_

#include <stddef.h>
#include <stdint.h>

// PRIMARY TYPES
#define CTOKEN_END                          0
#define CTOKEN_KEYWORD                      1
#define CTOKEN_LITERAL                      2
#define CTOKEN_IDENTIFER                    3
#define CTOKEN_OPERATOR                     4 // everything that has precedence
#define CTOKEN_COMMENT                      5
#define CTOKEN_CONTROL_FLOW                 6
#define CTOKEN_PREPROC_DIR                  7
#define CTOKEN_PRIMTYP_WIDTH                3
#define CTOKEN_PRIMTYP_MASK                 ((1u<<CTOKEN_PRIMTYP_WIDTH)-1)
#define CTOKEN_IS_PRIMTYPE(_type,_primtype) (((_type)&CTOKEN_PRIMTYP_MASK)==(_primtype))

// COMMENT TYPES
#define CTOKEN_COMMENT_ONELINE      ((1 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_COMMENT) // c comments
#define CTOKEN_COMMENT_MULTILINE    ((2 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_COMMENT) // cpp comments

// LITERAL TYPES
#define CTOKEN_LIT_INT              ((1 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_LITERAL)
#define CTOKEN_LIT_STR              ((2 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_LITERAL)
#define CTOKEN_LIT_FP               ((3 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_LITERAL)
#define CTOKEN_LIT_WIDTH            (CTOKEN_PRIMTYP_WIDTH+2)
#define CTOKEN_LIT_MASK             ((1u<<CTOKEN_LIT_WIDTH)-1)
#define CTOKEN_IS_LIT(_type,_lit)   (((_type)&CTOKEN_LIT_MASK)==(_lit))

// FLOATING POINT TYPES
#define CTOKEN_LIT_FLOAT    ((3 << CTOKEN_LIT_WIDTH) | CTOKEN_LIT_FP)
#define CTOKEN_LIT_DOUBLE   ((4 << CTOKEN_LIT_WIDTH) | CTOKEN_LIT_FP)

// INT LITERAL TYPES
#define CTOKEN_DEC_INT_LITERAL  ((1 << CTOKEN_LIT_WIDTH) | CTOKEN_LIT_INT)
#define CTOKEN_HEX_INT_LITERAL  ((2 << CTOKEN_LIT_WIDTH) | CTOKEN_LIT_INT)
#define CTOKEN_OCT_INT_LITERAL  ((3 << CTOKEN_LIT_WIDTH) | CTOKEN_LIT_INT)
#define CTOKEN_BIN_INT_LITERAL  ((4 << CTOKEN_LIT_WIDTH) | CTOKEN_LIT_INT) // not a part of the C standard
#define CTOKEN_CHR_INT_LITERAL  ((5 << CTOKEN_LIT_WIDTH) | CTOKEN_LIT_INT)

// CONTROL FLOW-REALTED TOKENS
#define CTOKEN_SEMICON      ((1 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_CONTROL_FLOW)
#define CTOKEN_BEGIN_SCOPE  ((2 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_CONTROL_FLOW)
#define CTOKEN_END_SCOPE    ((3 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_CONTROL_FLOW)

// OPERATOR TOKENS
#define CTOKEN_PLUS                 (( 1 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_MINUS                (( 2 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_RND_OPEN_BRKT        (( 3 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_RND_CLOSE_BRKT       (( 4 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_SQR_OPEN_BRKT        (( 5 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_SQR_CLOSE_BRKT       (( 6 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_DOT                  (( 7 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_ARROW                (( 8 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_LOGICAL_NOT          (( 9 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITWISE_NOT          ((10 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_AND_SYM              ((11 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_STAR                 ((12 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_DIV                  ((13 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_MOD                  ((14 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITWISE_RSHIFT       ((15 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITWISE_LSHIFT       ((16 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_GREATER              ((17 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_GREATER_EQ           ((18 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_LESSER               ((19 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_LESSER_EQ            ((20 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_EQUAL                ((21 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_NOT_EQUAL            ((22 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITWISE_XOR          ((23 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITWISE_OR           ((24 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_LOGICAL_AND          ((25 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_LOGICAL_OR           ((26 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_QUESTION_MARK        ((27 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_COLON                ((28 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_ASSIGNMENT           ((29 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_SUM_ASSIGNMENT       ((30 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_DIF_ASSIGNMENT       ((31 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_PRODUCT_ASSIGNMENT   ((32 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_QUOTIENT_ASSIGNMENT  ((33 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_MOD_ASSIGNMENT       ((34 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_LSHIFT_ASSIGNMENT    ((35 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_RSHIFT_ASSIGNMENT    ((36 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITAND_ASSIGNMENT    ((37 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITOR_ASSIGNMENT     ((38 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_BITXOR_ASSIGNMENT    ((39 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_COMMA                ((40 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_INCREMENT            ((41 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_DECREMENT            ((42 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_OPERATOR)
#define CTOKEN_OPERATOR_COUNT       (42)

// KEYWORD TYPES
#define CTOKEN_KW_ALIGNAS           ((1 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // alignas
#define CTOKEN_KW_ALIGNOF           ((2 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // alignof
#define CTOKEN_KW_AUTO              ((3 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // auto
#define CTOKEN_KW_BOOL              ((4 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // bool
#define CTOKEN_KW_BREAK             ((5 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // break
#define CTOKEN_KW_CASE              ((6 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // case
#define CTOKEN_KW_CHAR              ((7 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // char
#define CTOKEN_KW_CONST             ((8 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // const
#define CTOKEN_KW_CONSTEXPR         ((9 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // constexpr
#define CTOKEN_KW_CONTINUE          ((10 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // continue
#define CTOKEN_KW_DEFAULT           ((11 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // default
#define CTOKEN_KW_DO                ((12 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // do
#define CTOKEN_KW_DOUBLE            ((13 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // double
#define CTOKEN_KW_ELSE              ((14 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // else
#define CTOKEN_KW_ENUM              ((15 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // enum
#define CTOKEN_KW_EXTERN            ((16 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // extern
#define CTOKEN_KW_FALSE             ((17 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // false
#define CTOKEN_KW_FLOAT             ((18 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // float
#define CTOKEN_KW_FOR               ((19 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // for
#define CTOKEN_KW_GOTO              ((20 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // goto
#define CTOKEN_KW_IF                ((21 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // if
#define CTOKEN_KW_INLINE            ((22 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // inline
#define CTOKEN_KW_INT               ((23 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // int
#define CTOKEN_KW_LONG              ((24 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // long
#define CTOKEN_KW_NULLPTR           ((25 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // nullptr
#define CTOKEN_KW_REGISTER          ((26 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // register
#define CTOKEN_KW_RESTRICT          ((27 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // restrict
#define CTOKEN_KW_RETURN            ((28 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // return
#define CTOKEN_KW_SHORT             ((29 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // short
#define CTOKEN_KW_SIGNED            ((30 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // signed
#define CTOKEN_KW_SIZEOF            ((31 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // sizeof
#define CTOKEN_KW_STATIC            ((32 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // static
#define CTOKEN_KW_STATIC_ASSERT     ((33 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // static_assert
#define CTOKEN_KW_STRUCT            ((34 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // struct
#define CTOKEN_KW_SWITCH            ((35 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // switch
#define CTOKEN_KW_THREAD_LOCAL      ((36 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // thread_local
#define CTOKEN_KW_TRUE              ((37 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // true
#define CTOKEN_KW_TYPEDEF           ((38 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // typedef
#define CTOKEN_KW_TYPEOF            ((39 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // typeof
#define CTOKEN_KW_TYPEOF_UNQUAL     ((40 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // typeof_unqual
#define CTOKEN_KW_UNION             ((41 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // union
#define CTOKEN_KW_UNSIGNED          ((42 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // unsigned
#define CTOKEN_KW_VOID              ((43 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // void
#define CTOKEN_KW_VOLATILE          ((44 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // volatile
#define CTOKEN_KW_WHILE             ((45 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // while
#define CTOKEN_KW__ALIGNAS          ((46 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Alignas
#define CTOKEN_KW__ALIGNOF          ((47 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Alignof
#define CTOKEN_KW__ATOMIC           ((48 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Atomic
#define CTOKEN_KW__BITINT           ((49 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _BitInt
#define CTOKEN_KW__BOOL             ((50 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Bool
#define CTOKEN_KW__COMPLEX          ((51 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Complex
#define CTOKEN_KW__DECIMAL128       ((52 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Decimal128
#define CTOKEN_KW__DECIMAL32        ((53 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Decimal32
#define CTOKEN_KW__DECIMAL64        ((54 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Decimal64
#define CTOKEN_KW__GENERIC          ((55 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Generic
#define CTOKEN_KW__IMAGINARY        ((56 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Imaginary
#define CTOKEN_KW__NORETURN         ((57 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Noreturn
#define CTOKEN_KW__STATIC_ASSERT    ((58 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Static_assert
#define CTOKEN_KW__THREAD_LOCAL     ((59 << CTOKEN_PRIMTYP_WIDTH) | CTOKEN_KEYWORD) // _Thread_local
#define CTOKEN_KEYWORD_COUNT        59

typedef struct {
    int         type;
    char const  *keyword;
    size_t      keyword_len;
} cparse_keyword_meta_t;

extern const cparse_keyword_meta_t cparse_keyword_tbl[];
extern const char *cparse_op2str[];

typedef struct {
    int     type;
    size_t  pos;
    size_t  len;
    union {
        uint64_t    integer;
        double      fp;
        struct {
            size_t  len;
            uint8_t *data;
        } byte_array;
    } d;
} cparse_token_t;

typedef enum {
    CTOKEN_DATATYPE_NO_DATA     = 0,
    CTOKEN_DATATYPE_FP          = 1,
    CTOKEN_DATATYPE_INT         = 2,
    CTOKEN_DATATYPE_ARRAY       = 3
} cparse_token_datatype_t;

int                         cparse_tokenize(char const *data, cparse_token_t **tokens);
void                        cparse_free_token(cparse_token_t *token, int free_ptr);
const cparse_keyword_meta_t *cparse_get_keyword_by_name(char const *name, size_t l);
const cparse_keyword_meta_t *cparse_get_keyword_by_type(int type);
cparse_token_datatype_t     cparse_get_datatype_from_type(int type);
char                        cparse_get_needed_space(cparse_token_t *before, cparse_token_t *after);
char                        *cparse_stringify_token(cparse_token_t token);
int                         cparse_stringify_token_r(cparse_token_t token, char **str, size_t *alloclen);
void                        cparse_free_token(cparse_token_t *token, int free_ptr);
void                        cparse_free_token_array(cparse_token_t **tokens);

#endif // SIG_LIBCPARSE_TOKENS_H_