#pragma once
#include "Formatter.hpp"
#include "TypeDef.hpp"
#include <chrono>
namespace gsh {
template<class Clock = std::chrono::system_clock> class Timer {
  Clock::time_point start_time;
public:
  Timer() { start_time = Clock::now(); }
  void restart() { start_time = Clock::now(); }
  u64 elapsed() const { return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time).count(); }
};
template<class T> class Formatter<Timer<T>> {
public:
  template<class Stream> void operator()(Stream&& stream, const Timer<T>& t) {
    Formatter<u64>{}(stream, t.elapsed());
    Formatter<c8>{}(stream, 'm');
    Formatter<c8>{}(stream, 's');
  }
};
}
