#if	!defined(_ringbuffer_h)
#define	_ringbuffer_h
#include <time.h>
#ifdef __cplusplus

extern "C" {
#endif
   typedef struct {
      void** buffer;
      int head;
      int tail;
      int max_size;
      int current_size;
      struct timespec timestamp;
   } RingBuffer;

   extern RingBuffer* rb_create(int max_size);
   extern void rb_destroy(RingBuffer* buffer);
   extern int rb_is_buffer_full(RingBuffer* buffer);
   extern int rb_is_buffer_empty(RingBuffer* buffer);
   extern void rb_push(RingBuffer* buffer, void* data);
   extern int rb_is_buffer_full(RingBuffer* buffer);
#ifdef __cplusplus
};
#endif
#endif	// !defined(_ringbuffer_h)
