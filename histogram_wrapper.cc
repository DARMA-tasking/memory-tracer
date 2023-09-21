
#include "histogram_wrapper.h"
#include "histogram_approx.hpp"

using Histogram = vt::adt::HistogramApprox<double, int64_t>;

extern "C" void* makeHistogram(int64_t num_centroids) {
  auto h = new Histogram(num_centroids);
  return static_cast<void*>(h);
}

extern "C" void addValue(void* histogram, double value, int64_t count) {
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
