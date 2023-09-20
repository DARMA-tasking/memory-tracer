#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>

static struct temp_memory {
  size_t pos;
  size_t allocs;
  char buf[16384];
} temp = { 0, 0 };

static void* (*sys_malloc)(size_t) = 0;
static void (*sys_free)(void*) = 0;

static int in_initialize = 0;

static void initialize_memory_tracer() {
  in_initialize = 1;

  sys_malloc = dlsym(RTLD_NEXT, "malloc");
  if (!sys_malloc) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }

  sys_free = dlsym(RTLD_NEXT, "free");
  if (!sys_free) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }

  in_initialize = 0;
}

void* malloc(size_t size) {
  // init on demand
  if (!sys_malloc) {
    if (in_initialize) {

      // We need some allocation while we are initializing
      if (temp.pos + size < sizeof(temp.buf)) {
        void* ptr = temp.buf + temp.pos;
        temp.pos += size;
        temp.allocs++;
        return ptr;
      } else {
        // Not enough memory
        fprintf(
          stderr,
          "Ran out of temp memory: pos=%zu, allocs=%zu\n", temp.pos, temp.allocs
        );
      }
    } else {
      initialize_memory_tracer();
    }
  }

  void* ptr = sys_malloc(size);
  fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);
  return ptr;
}

void free(void* ptr) {
  // Handle a temp free
  if (ptr >= (void*)temp.buf && ptr <= (void*)(temp.buf + temp.pos)) {
    // We are freeing memory, but won't actually change anything
  }

  sys_free(ptr);
  fprintf(stderr, "free = %p\n", ptr);
}
