/*
 * A reusable implementation of a ring buffer with timestamps for FIFO usage
 */
#include "config.h"
#include "ringbuffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

RingBuffer* rb_create(int max_size) {
    RingBuffer* buffer = malloc(sizeof(RingBuffer));
    buffer->buffer = malloc(max_size * sizeof(void*));
    buffer->head = 0;
    buffer->tail = 0;
    buffer->max_size = max_size;
    buffer->current_size = 0;
    return buffer;
}

void rb_destroy(RingBuffer* buffer) {
    free(buffer->buffer);
    free(buffer);
}

int rb_is_buffer_full(RingBuffer* buffer) {
    return buffer->current_size == buffer->max_size;
}

int rb_is_buffer_empty(RingBuffer* buffer) {
    return buffer->current_size == 0;
}

void rb_push(RingBuffer* buffer, void* data) {
    if (rb_is_buffer_full(buffer)) {
        printf("Ring buffer is full, dropping oldest item.\n");
        free(buffer->buffer[buffer->tail]);
        buffer->tail = (buffer->tail + 1) % buffer->max_size;
        buffer->current_size--;
    }
    buffer->buffer[buffer->head] = data;
    buffer->head = (buffer->head + 1) % buffer->max_size;
    buffer->current_size++;
}

void* rb_pop(RingBuffer* buffer) {
    if (rb_is_buffer_empty(buffer)) {
        printf("Ring buffer is empty.\n");
        return NULL;
    }
    void* data = buffer->buffer[buffer->tail];
    buffer->tail = (buffer->tail + 1) % buffer->max_size;
    buffer->current_size--;
    return data;
}

#if	defined(__TEST__)
int main(int argc, char **argv) {
    RingBuffer* buffer = rb_create(5);
    for (int i = 0; i < 10; i++) {
        time_t timestamp = time(NULL);
        char* data = malloc(sizeof(char) * 20);
        sprintf(data, "Item %d at %ld", i, timestamp);
        rb_push(buffer, data);
    }
    while (!rb_is_buffer_empty(buffer)) {
        char* data = (char*) rb_pop(buffer);
        printf("%s\n", data);
        free(data);
    }
    rb_destroy(buffer);
    return 0;
}
#endif	// __TEST__
