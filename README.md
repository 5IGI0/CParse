# CParse: Experimental C Code Parsing Library

CParse is an experimental C code parsing library designed for internal use within your programs.

_This library is not intended for external use and should be copied directly into your project's source code.\
Please note that this library is still in development, and error management is not completely done; therefore, numerous assert statements are used.\
Do not use this library for any production purposes._

## Prototypes and Structures

CParse provides the following prototypes and structures:
```c
// cf. tokens.h
cparse_token_t;
cparse_keyword_meta_t;
cparse_token_datatype_t;

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

// cf. expr.h
cparse_expr_node_t;
int   parse_expr(cparse_token_t *tokens, size_t token_count, cparse_expr_node_t *opts);
void  free_expr(cparse_expr_node_t *expr, int flags);
```

## Usage

To use CParse, copy the relevant code into your program's source code and include the necessary headers.\
_The library is not meant for standalone usage and should be integrated into your project as needed._

For additional details and examples, please refer to the library's source code.

## TODO

- [ ] **Documentation**
  - [ ] Add doxygen comments to all structures and functions

- [x] **Expressions:**
  - [x] Expressions tree
  - [x] Parse expressions (in theory)
  - [ ] Implement expression memory management
    - [x] free
    - [ ] copy
    - [ ] copy input's tokens

- [ ] **Tokenization:**
  - [ ] U, u, L prefix support
  - [ ] Integer suffix support
  - [ ] "\u" escape sequence support
  - [ ] "\U" escape sequence support
  - [ ] Preprocessor directive support

- [ ] **Token Management:**
  - [ ] Implement token memory management
    - [x] free
    - [ ] copy
  - [x] Develop token stringification

- [ ] **Code Tree Design:**
  - [ ] Plan and implement code tree structure


Feel free to make a pull request to help improve and expand this library. Your contributions are welcome!