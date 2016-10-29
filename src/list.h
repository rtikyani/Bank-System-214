#ifndef _SORTED_LIST_H_
#define _SORTED_LIST_H_

#include <stdbool.h>

typedef bool compare_function_t(void *, void *);

typedef void destroy_function_t(void *);

typedef struct sorted_list_element {
    struct sorted_list_element *next;
    void *value;
} list_element_t;

typedef struct sorted_list {
    compare_function_t *compare_function;
    destroy_function_t *destroy_function;
    list_element_t *head;
} list_t;

list_t *create_list(compare_function_t *, destroy_function_t *);

void destroy_list(list_t *);

list_element_t *create_list_element(void *, list_element_t *);

void destroy_list_element(list_t *, list_element_t *);

bool insert_object(list_t *, void *);

bool remove_object(list_t *, void *);

int get_size(list_t *);

typedef struct sorted_list_iterator {
    list_element_t *current_index;
} list_iterator_t;

list_iterator_t *create_iterator(list_t *);

void destroy_iterator(list_iterator_t *);

void *get_item(list_iterator_t *);

void *next_item(list_iterator_t *);

#endif