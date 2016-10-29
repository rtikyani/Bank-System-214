#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"

list_t *create_list(compare_function_t *compare_func, destroy_function_t *destroy_func) {
    list_t *list = malloc(sizeof(list_t));

    if (list == NULL) {
        return NULL;
    }
    list->compare_function = compare_func;
    list->destroy_function = destroy_func;
    list->head = NULL;
    return list;
}

void destroy_list(list_t *list) {
    list_element_t *index = list->head;
    while (index != NULL) {
        list_element_t *next = index->next;
        destroy_list_element(list, index);
        index = next;
    }
    free(list);
}

list_element_t *create_list_element(void *value, list_element_t *next) {
    list_element_t *element = malloc(sizeof(list_element_t));
    if (element == NULL) {
        return NULL;
    }

    element->value = value;
    element->next = next;
    return element;
}

void destroy_list_element(list_t *list, list_element_t *element) {
    list->destroy_function(element->value);
    free(element);
}

static void insert_after(list_element_t *index, list_element_t *element) {
    list_element_t *tmp = index->next;
    element->next = tmp;
    index->next = element;
}

bool insert_object(list_t *list, void *object) {
    if (list->head == NULL) {
        list->head = create_list_element(object, NULL);
    } else {
        list_element_t *index = list->head;
        while (true) {
            if (index->next == NULL) {
                insert_after(index, create_list_element(object, NULL));
                return true;
            }
            index = index->next;
        }
    }
    return false;
}

bool remove_object(list_t *list, void *object) {
    list_element_t *index = list->head;
    if (index->value != NULL) {
        if (list->compare_function(index->value, object)) {
            list->head = index->next;
            destroy_list_element(list, index);
            return true;
        }
    }

    while (index != NULL) {
        if (index->next != NULL && index->next->value != NULL) {
            list_element_t *current_element = index->next;
            if (list->compare_function(index->next->value, object)) {
                index->next = index->next->next;
                destroy_list_element(list, current_element);
                return true;
            }
        }
        index = index->next;
    }
    return false;
}

int get_size(list_t *list) {
    list_element_t *element = list->head;
    int size = 0;
    while (element != NULL) {
        element = element->next;
        size++;
    }
    return size;
}

list_iterator_t *create_iterator(list_t *list) {
    list_iterator_t *iterator = malloc(sizeof(list_iterator_t));

    if (list == NULL) {
        fprintf(stderr, "Error creating iterator!\n");
        return NULL;
    }
    iterator->current_index = list->head;
    return iterator;
}

void destroy_iterator(list_iterator_t *iterator) {
    free(iterator);
}

void *get_item(list_iterator_t *iterator) {
    if (iterator->current_index != NULL) {
        return iterator->current_index->value;
    }
    return NULL;
}

void *next_item(list_iterator_t *iterator) {
    if (iterator->current_index != NULL) {
        list_element_t *next_element = iterator->current_index->next;
        if (next_element != NULL) {
            iterator->current_index = next_element;
            return next_element->value;
        }
    }
    return NULL;
}
