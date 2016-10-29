#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct tokenizer {
    char *stream;
    size_t index;
} tokenizer_t;

tokenizer_t *create_tokenizer(char *string);

void destroy_tokenizer(tokenizer_t *);

char *next_token(tokenizer_t *);
