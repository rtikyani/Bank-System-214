#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

tokenizer_t *create_tokenizer(char *string) {
    tokenizer_t *tokenizer = malloc(sizeof(*tokenizer));
    tokenizer->index = 0;
    tokenizer->stream = strdup(string);
    return tokenizer;
}

void destroy_tokenizer(tokenizer_t *t) {
    free(t->stream);
    free(t);
}

static void skip_whitespace(tokenizer_t *tokenizer) {
    size_t index = tokenizer->index;

    while (tokenizer->stream[index] && !isalnum(tokenizer->stream[index])) {
        index++;
    }

    tokenizer->index = index;
}

static void reallocate(void **pointer, size_t size) {
    void *temp = realloc(*pointer, 2 * size);
    *pointer = temp;
}

static char *next_word(tokenizer_t *tokenizer) {

    size_t index = tokenizer->index;
    size_t length = 2;
    size_t dot_count = 0;

    char *token = malloc(3 * sizeof(char));

    if (index >= strlen(tokenizer->stream)) {
        return NULL;
    }

    while ((isalnum((int) tokenizer->stream[index]) || (tokenizer->stream[index] == '.')) && (dot_count <= 1)) {

        if (tokenizer->stream[index] == '.') {
            dot_count++;
        }

        if (index - tokenizer->index + 1 == length) {
            reallocate((void **) &token, 2 * length);
            length *= 2;
        }
        token[index - tokenizer->index] = tokenizer->stream[index];
        index++;
    }

    token[index - tokenizer->index] = '\0';
    tokenizer->index = index;

    return token;
}

char *next_token(tokenizer_t *tokenizer) {

    char *string = NULL;

    if (strlen(tokenizer->stream) > tokenizer->index) {
        skip_whitespace(tokenizer);

        if (tokenizer->stream[tokenizer->index]) {
            string = next_word(tokenizer);
        }
    }

    return string;
}
