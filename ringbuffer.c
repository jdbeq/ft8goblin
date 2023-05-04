/*
 * A reusable implementation of a ring buffer with timestamps for FIFO usage
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "config.h"
#include "ringbuffer.h"
#include "logger.h"

rb_buffer_t *rb_create(int max_size) {
    rb_buffer_t *buffer = malloc(sizeof(rb_buffer_t));

    if (buffer == NULL) {
       fprintf(stderr, "rb_create: out of memory!\n");
       exit(ENOMEM);
    }

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

        if (current->needs_freed && current->data != NULL)
           free(current->data);

        free(current);
        current = next;
    }
    free(buffer);
}

int rb_add(rb_buffer_t *buffer, void *data, int needs_freed) {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);

    rb_node_t *node = malloc(sizeof(rb_node_t));
    
    if (node == NULL) {
       fprintf(stderr, "rb_add: out of memory!\n");
       exit(ENOMEM);
    }
    node->data = data;
    node->timestamp = timestamp;
    node->next = NULL;
    node->needs_freed = needs_freed;

    log_send(mainlog, LOG_DEBUG, "Adding entry %p to rb:%p, needs_freed: %d", data, buffer, needs_freed);

    if (buffer->current_size == 0) {
        buffer->head = node;
        buffer->tail = node;
        buffer->current_size++;
        return 0;
    }
    if (buffer->current_size == buffer->max_size) {
        rb_node_t *next_head = buffer->head->next;
        free(buffer->head);
        buffer->head = next_head;
        buffer->current_size--;
    }

    // set ourselves onto the end of the list
    buffer->tail->next = node;

    // we are now the tail of the list
    buffer->tail = node;
    buffer->current_size++;
    return 0;
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

    if ((void *)array == NULL) {
       fprintf(stderr, "rb_get_range: out of memory!\n");
       exit(ENOMEM);
    }

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
