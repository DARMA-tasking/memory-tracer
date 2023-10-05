#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdatomic.h>
#include <string.h>

#include "histogram_wrapper.h"

#define TEMP_MALLOC_LEN 4096

#define SHOW_ALLOC_PRINTS 0

#define HISTOGRAM_BINS 1023

static void* temp_malloc_list[TEMP_MALLOC_LEN];
static size_t temp_malloc_count = 0;

static void* (*sys_malloc)(size_t) = 0;
static void (*sys_free)(void*) = 0;

static int in_initialize = 0;

void* size_histogram = 0;
void* malloc_time_histogram = 0;
void* touch_time_histogram = 0;
void* touch_ratio_histogram = 0;

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

static int should_trace_malloc = 0;
static int should_touch = 0;

void start_memory_tracer() {
  should_trace_malloc++;
  fputs("start_memory_tracer\n", stdout);
}

void start_memory_tracer_with_touch() {
  should_trace_malloc++;
  should_touch = 1;
  fputs("start_memory_tracer_with_touch\n", stdout);
}

void stop_memory_tracer() {
  should_trace_malloc--;
  fputs("stop_memory_tracer\n", stdout);
}

void dump_memory_tracer() {
  if (size_histogram) {
    printf("malloc size (b):\n");
    dump(size_histogram);
  }
  if (malloc_time_histogram) {
    printf("malloc time (ms):\n");
    dump(malloc_time_histogram);
  }
  if (should_touch) {
    if (touch_time_histogram) {
      printf("touch time (ms):\n");
      dump(touch_time_histogram);
    }
    if (touch_ratio_histogram) {
      printf("touch ratio (b/ms):\n");
      dump(touch_ratio_histogram);
    }
  }
}

void* malloc(size_t size) {
  if (in_initialize) {
    // We need some allocation while we are initializing
    return temp_alloc(size);
  }

  // init on demand
  if (sys_malloc == 0) {
    in_initialize = 1;
    initialize_memory_tracer();
    in_initialize = 0;
    if (size_histogram == 0) {
      size_histogram = makeHistogram(HISTOGRAM_BINS);
    }
    if (malloc_time_histogram == 0) {
      malloc_time_histogram = makeHistogram(HISTOGRAM_BINS);
    }
    if (touch_time_histogram == 0) {
      touch_time_histogram = makeHistogram(HISTOGRAM_BINS);
    }
    if (touch_ratio_histogram == 0) {
      touch_ratio_histogram = makeHistogram(HISTOGRAM_BINS);
    }
  }

  struct timeval t_start, t_end;
  gettimeofday(&t_start, NULL);
  void* ptr = sys_malloc(size);
  gettimeofday(&t_end, NULL);

  double malloc_elapsed_time =
    (t_end.tv_sec  - t_start.tv_sec ) * 1000.0 +  //  s to ms
    (t_end.tv_usec - t_start.tv_usec) / 1000.0;   // us to ms

  if (should_trace_malloc > 0) {
#if SHOW_ALLOC_PRINTS
    fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);
#endif
    if (size_histogram) {
      addValue(size_histogram, size, 1);
    }
    if (malloc_time_histogram) {
      addValue(malloc_time_histogram, malloc_elapsed_time, 1);
    }

    if (should_touch) {
      gettimeofday(&t_start, NULL);
      memset(ptr, 0, size);
      gettimeofday(&t_end, NULL);

      double touch_elapsed_time =
        (t_end.tv_sec  - t_start.tv_sec ) * 1000.0 +  //  s to ms
        (t_end.tv_usec - t_start.tv_usec) / 1000.0;   // us to ms
      double touch_ratio = (double)(size) / touch_elapsed_time;

      if (touch_time_histogram) {
        addValue(touch_time_histogram, touch_elapsed_time, 1);
      }
      if (touch_ratio_histogram) {
        addValue(touch_ratio_histogram, touch_ratio, 1);
      }
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

  sys_free(ptr);

  if (should_trace_malloc > 0) {
#if SHOW_ALLOC_PRINTS
    fprintf(stderr, "free = %p\n", ptr);
#endif
  }
}
