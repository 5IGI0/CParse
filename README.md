# CParse: Experimental C Code Parsing Library

CParse is an experimental C code parsing library designed for internal use within your programs.

_This library is not intended for external use and should be copied directly into your project's source code.\
Please note that this library is still in development, and error management is not completely done; therefore, numerous assert statements are used.\
Do not use this library for any production purposes._

## Prototypes and Structures

CParse provides the following prototypes and structures:
```c

typedef struct {
    int     type; // Check tokens.h for more details
    size_t  pos;
    size_t  len;
    union {
        uint64_t    integer;
        double      fp;
        struct {
            size_t      len;
            uint8_t     *data;
        } byte_array;
    } d;
} cparse_token_t;

int cparse_tokenize(char const *data, cparse_token_t **tokens);
```

Features
- `cparse_token_t` structure, which contains information about parsed tokens, such as type, position, length, and data.
- `cparse_tokenize` function, which tokenizes C code and populates an array of cparse_token_t objects.

## Usage

To use CParse, copy the relevant code into your program's source code and include the necessary headers.\
_The library is not meant for standalone usage and should be integrated into your project as needed._

For additional details and examples, please refer to the library's source code.

## TODO

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
  - [ ] Develop token stringification

- [ ] **Code Tree Design:**
  - [ ] Plan and implement code tree structure


Feel free to make a pull request to help improve and expand this library. Your contributions are welcome!