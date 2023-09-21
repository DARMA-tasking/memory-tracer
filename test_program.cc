
#include <vector>

extern "C" void start_memory_tracer();
extern "C"  void stop_memory_tracer();
extern  "C" void dump_memory_tracer();

int main() {
  start_memory_tracer();
  std::vector<int> a;
  for (int i = 0; i < 100000; i++) {
    a.push_back(i);
  }
  stop_memory_tracer();
  dump_memory_tracer();
  return 0;
}
