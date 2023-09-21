
#if !defined(MEMORY_TRACER_HISTOGRAM_WRAPPER)
#define MEMORY_TRACER_HISTOGRAM_WRAPPER

#include <cstdint>

extern "C" void* makeHistogram(int64_t num_centroids);

extern "C" void addValue(void* histogram, double value, int64_t count);

extern "C" void dump(void* histogram);

extern "C" void freeHistogram(void* histogram);

#endif /*MEMORY_TRACER_HISTOGRAM_WRAPPER*/
