#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>

static void* (*sys_malloc)(size_t) = 0;
static void (*sys_free)(void*) = 0;

static void initialize_memory_tracer() {
  sys_malloc = dlsym(RTLD_NEXT, "malloc");
  if (!sys_malloc) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }

  sys_free = dlsym(RTLD_NEXT, "free");
  if (!sys_free) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }

}

void* malloc(size_t size) {
  // init on demand
  if (!sys_malloc) {
    initialize_memory_tracer();
  }

  void* ptr = sys_malloc(size);
  fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);
  return ptr;
}

void free(void* ptr) {
  // init on demand
  if (!sys_free) {
    initialize_memory_tracer();
  }

  sys_free(ptr);
  fprintf(stderr, "free = %p\n", ptr);
}
