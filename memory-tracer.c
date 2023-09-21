#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include "histogram_wrapper.h"

#define TEMP_MALLOC_LEN 4096
static void* temp_malloc_list[TEMP_MALLOC_LEN];
static size_t temp_malloc_count = 0;

static void* (*sys_malloc)(size_t) = 0;
static void (*sys_free)(void*) = 0;

static int in_initialize = 0;
static int in_histogram = 0;

void* histogram = 0;

static void initialize_memory_tracer()  {
  fputs("Initializing memory tracer\n", stdout);

  sys_malloc = dlsym(RTLD_NEXT, "malloc");
  if (!sys_malloc) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }

  sys_free = dlsym(RTLD_NEXT, "free");
  if (!sys_free) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }

  fputs("Finished initializing memory tracer\n", stdout);
}

static void* temp_alloc(size_t size) {
  fputs("temp_alloc called", stderr);
  if (temp_malloc_count >= TEMP_MALLOC_LEN - 1) {
    fputs("out of temporary allocation\n", stderr);
    return 0;
  }

  void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
  if (MAP_FAILED == ptr) {
    perror("mmap failed");
    return 0;
  }

  temp_malloc_list[temp_malloc_count++] = ptr;
  return ptr;
}

static int should_trace_count = 1;

void start_memory_tracer() {
  should_trace_count++;
}

void stop_memory_tracer() {
  should_trace_count--;
}

void* malloc(size_t size) {
  if (in_histogram) {
    return sys_malloc(size);
  }

  if (in_initialize) {
    // We need some allocation while we are initializing
    return temp_alloc(size);
  }

  // init on demand
  if (sys_malloc == 0) {
    in_initialize = 1;
    initialize_memory_tracer();
    in_initialize = 0;
    if (histogram == 0) {
      in_histogram = 1;
      histogram = makeHistogram(8);
      in_histogram = 0;
    }
  }

  void* ptr = sys_malloc(size);
  if (should_trace_count > 0) {
    fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);
    if (histogram) {
      in_histogram = 1;
      addValue(histogram, size, 1);
      in_histogram = 0;
    }
  }
  return ptr;
}

void free(void* ptr) {
  if (in_initialize) {
    fputs("in init free\n", stdout);
    // leak the memory
    return;
  }

  if (sys_free == 0 || ptr == 0) {
    // leak during startup
    return;
  }

  if (in_histogram) {
    sys_free(ptr);
    return;
  }

  sys_free(ptr);

  if (should_trace_count > 0) {
    fprintf(stderr, "free = %p\n", ptr);
  }
}
