
#if !defined(MEMORY_TRACER_HISTOGRAM_WRAPPER)
#define MEMORY_TRACER_HISTOGRAM_WRAPPER

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* makeHistogram(int64_t num_centroids);

void addValue(void* histogram, double value, int64_t count);

void dump(void* histogram);

void freeHistogram(void* histogram);

#ifdef __cplusplus
}
#endif

#endif /*MEMORY_TRACER_HISTOGRAM_WRAPPER*/
