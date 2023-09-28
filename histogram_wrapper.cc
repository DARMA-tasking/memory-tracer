
#include "histogram_wrapper.h"
#include "histogram_approx.hpp"

#include <mutex>

using Histogram = vt::adt::HistogramApprox<double, int64_t>;

// Try to use a single mutex for all histograms
static std::mutex histogram_mutex;

extern "C" void* makeHistogram(int64_t num_centroids) {
  auto h = new Histogram(num_centroids);
  return static_cast<void*>(h);
}

extern "C" void addValue(void* histogram, double value, int64_t count) {
  // Obtain mutex for add operation
  std::lock_guard<std::mutex> const lock(histogram_mutex);
  Histogram* h = static_cast<Histogram*>(histogram);
  h->add(value, count);
}

extern "C" void dump(void* histogram) {
  Histogram* h = static_cast<Histogram*>(histogram);
  h->dump();
}

extern "C" void freeHistogram(void* histogram) {
  Histogram* h = static_cast<Histogram*>(histogram);
  delete h;
}
