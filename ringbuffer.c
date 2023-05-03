/*
 * A reusable implementation of a ring buffer with timestamps for FIFO usage
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "config.h"
#include "ringbuffer.h"

rb_buffer_t *rb_create(int max_size) {
    rb_buffer_t *buffer = malloc(sizeof(rb_buffer_t));
    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->max_size = max_size;
    buffer->current_size = 0;
    return buffer;
}

void rb_destroy(rb_buffer_t *buffer) {
    rb_node_t *current = buffer->head;
    while (current != NULL) {
        rb_node_t *next = current->next;
        free(current);
        current = next;
    }
    free(buffer);
}

void rb_add(rb_buffer_t *buffer, void *data) {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    rb_node_t *node = malloc(sizeof(rb_node_t));
    node->data = data;
    node->timestamp = timestamp;
    node->next = NULL;

    if (buffer->current_size == 0) {
        buffer->head = node;
        buffer->tail = node;
        buffer->current_size++;
        return;
    }
    if (buffer->current_size == buffer->max_size) {
        rb_node_t *next_head = buffer->head->next;
        free(buffer->head);
        buffer->head = next_head;
        buffer->current_size--;
    }

    buffer->tail->next = node;
    buffer->tail = node;
    buffer->current_size++;
}

void *rb_get_most_recent(rb_buffer_t *buffer) {

    if (buffer->current_size == 0) {
        printf("Ring buffer is empty.\n");
        return NULL;
    }

    rb_node_t *current = buffer->head;
    rb_node_t *latest_node = current;

    while (current != NULL) {
        if (current->timestamp.tv_sec > latest_node->timestamp.tv_sec ||
            (current->timestamp.tv_sec == latest_node->timestamp.tv_sec &&
             current->timestamp.tv_nsec > latest_node->timestamp.tv_nsec)) {
            latest_node = current;
        }
        current = current->next;
    }
    return latest_node->data;
}

void *rb_get_range(rb_buffer_t *buffer, int start, int count) {
    if (buffer->current_size == 0) {
        printf("Ring buffer is empty.\n");
        return NULL;
    }
    if (start < 0 || start >= buffer->current_size) {
        printf("Invalid start index.\n");
        return NULL;
    }
    if (count < 1 || start + count > buffer->current_size) {
        printf("Invalid count.\n");
        return NULL;
    }
    void **array = malloc(count  *sizeof(void*));
    rb_node_t *current = buffer->head;

    int i = 0;

    while (i < start) {
        current = current->next;
        i++;
    }

    for (i = 0; i < count; i++) {
        array[i] = current->data;
        current = current->next;
    }

    return array;
}
