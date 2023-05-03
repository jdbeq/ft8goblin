#if	!defined(_ringbuffer_h)
#define	_ringbuffer_h
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
   typedef struct rb_node {
      void *data;
      struct timespec timestamp;
      struct rb_node *next;
   } rb_node_t;

   typedef struct rb_buffer {
      rb_node_t *head;
      rb_node_t *tail;
      int max_size;
      int current_size;
   } rb_buffer_t;

   extern rb_buffer_t* rb_create(int max_size);
   extern void rb_destroy(rb_buffer_t* buffer);
   extern void rb_add(rb_buffer_t* buffer, void* data);
   extern void* rb_get_most_recent(rb_buffer_t* buffer);
   extern void* rb_get_range(rb_buffer_t* buffer, int start, int count);
#ifdef __cplusplus
}
#endif

#endif	// !defined(_ringbuffer_h)
