#pragma once
#include "Random.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include <chrono>
#include <cmath>
namespace gsh {
class ZeroTemp {
public:
  ZeroTemp() {}
  f64 operator()([[maybe_unused]] f64 progress) const { return 0.0; }
};
class LinearTemp {
  f64 init_temp, final_temp;
public:
  LinearTemp(f64 init_temp, f64 final_temp = 0.0) : init_temp(init_temp), final_temp(final_temp) {}
  f64 operator()(f64 progress) const { return init_temp + (final_temp - init_temp) * progress; }
};
class ExponentialTemp {
  f64 init_temp;
  f64 ratio;
public:
  ExponentialTemp(f64 init_temp, f64 final_temp = 0.001) : init_temp(init_temp), ratio(final_temp / init_temp) {}
  f64 operator()(f64 progress) const { return init_temp * std::pow(ratio, progress); }
};
class VariableExponentialTemp {
  f64 init_temp;
  f64 shape;
  f64 ratio;
public:
  VariableExponentialTemp(f64 init_temp, f64 final_temp = 0.001, f64 shape = 2.0) : init_temp(init_temp), shape(shape), ratio(final_temp / init_temp) {}
  f64 operator()(f64 progress) const { return init_temp * std::pow(ratio, std::pow(progress, shape)); }
};
class LogarithmicTemp {
  f64 init_temp;
  f64 shape;
public:
  LogarithmicTemp(f64 init_temp, f64 shape = 10000.0) : init_temp(init_temp), shape(shape) {}
  f64 operator()(f64 progress) { return init_temp / std::log(progress * shape + 2.71828182845904523536); }
};
class ReciprocalTemp {
  f64 init_temp;
  f64 shape;
public:
  ReciprocalTemp(f64 init_temp, f64 shape = 5.0) : init_temp(init_temp), shape(shape) {}
  f64 operator()(f64 progress) const { return init_temp / (progress * shape + 1.0); }
};
class IterationProgress {
  u32 max_iter;
  f64 current_progress = 0.0;
public:
  IterationProgress(u32 max_iter) : max_iter(max_iter) {}
  f64 progress() const { return current_progress; }
  bool update(u32 current_iter) {
    current_progress = static_cast<f64>(current_iter) / static_cast<f64>(max_iter);
    return current_iter < max_iter;
  }
};
class TimeProgress {
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::duration max_duration;
  f64 current_progress = 0.0;
public:
  TimeProgress(std::chrono::milliseconds max_duration) : start_time(std::chrono::steady_clock::now()), max_duration(std::chrono::duration_cast<std::chrono::steady_clock::duration>(max_duration)) {}
  f64 progress() const { return current_progress; }
  bool update([[maybe_unused]] u32 current_iter) {
    auto now = std::chrono::steady_clock::now();
    current_progress = static_cast<f64>((now - start_time).count()) / static_cast<f64>(max_duration.count());
    return now - start_time < max_duration;
  }
};
namespace annealing {
constexpr struct _Minimize {
} minimize;
constexpr struct _Maximize {
} maximize;
} // namespace annealing
template<class OptType, class TempFunction, class ProgressFunction> class Annealing {
  static_assert(std::is_same<OptType, annealing::_Minimize>::value || std::is_same<OptType, annealing::_Maximize>::value, "Type must be OptimizeType::Minimize or OptimizeType::Maximize");
  constexpr static bool is_minimize = std::is_same<OptType, annealing::_Minimize>::value;
  TempFunction temp_function;
  ProgressFunction progress_function;
  f64 current_score;
  f64 threshold_score;
  f64 best_score;
  f64 current_temp = 0.0;
  u32 ugap;
  u32 current_iter = 0;
  bool is_best_updated = false;
  [[no_unique_address]] std::allocator<f32> alloc;
  f32* rnd_buf = nullptr;
  u32 rnd_buf_size;
public:
  Annealing() = delete;
  Annealing(const OptType&, const TempFunction& tempf, const ProgressFunction& progressf, f64 init_score, u32 update_gap = 8, u32 seed = Rand32::default_seed, u32 buf_size = 128) : temp_function(tempf), progress_function(progressf), current_score(init_score), best_score(init_score), ugap(1u << update_gap), alloc(), rnd_buf_size(1u << buf_size) {
    if constexpr(!std::is_same<TempFunction, ZeroTemp>::value) {
      rnd_buf = alloc.allocate(rnd_buf_size);
      Rand32 rand_function(seed);
      for(u32 i = 0; i < rnd_buf_size; ++i) { rnd_buf[i] = std::log(1.0f - Canocicaled32(rand_function)); }
    }
  }
  ~Annealing() {
    if(rnd_buf) alloc.deallocate(rnd_buf, rnd_buf_size);
  }
  u32 iterations() const { return current_iter; }
  f64 temp() const { return current_temp; }
  f64 score() const { return current_score; }
  f64 threshold() const { return threshold_score; }
  f64 best() const { return best_score; }
  u32 gap() const { return ugap; }
  f64 progress() const { return progress_function.progress(); }
  TempFunction& tempf() { return temp_function; }
  ProgressFunction& progressf() { return progress_function; }
  bool update() {
    if(current_iter % ugap == 0) {
      if(!progress_function.update(current_iter)) return false;
      if constexpr(!std::is_same<TempFunction, ZeroTemp>::value) { current_temp = temp_function(progress_function.progress()); }
    }
    if constexpr(std::is_same<TempFunction, ZeroTemp>::value) {
      threshold_score = current_score;
    } else {
      f64 rnd_val = static_cast<f64>(rnd_buf[current_iter & (rnd_buf_size - 1)]);
      if constexpr(is_minimize) {
        threshold_score = current_score - current_temp * rnd_val;
      } else {
        threshold_score = current_score + current_temp * rnd_val;
      }
    }
    ++current_iter;
    return true;
  }
  bool acceptable(f64 new_score) {
    if constexpr(is_minimize) {
      return new_score < threshold_score;
    } else {
      return new_score > threshold_score;
    }
  }
  bool accept(f64 new_score) {
    is_best_updated = false;
    if constexpr(is_minimize) {
      if(new_score < threshold_score) {
        current_score = new_score;
        if(new_score < best_score) {
          best_score = new_score;
          is_best_updated = true;
        }
        return true;
      }
    } else {
      if(new_score > threshold_score) {
        current_score = new_score;
        if(new_score > best_score) {
          best_score = new_score;
          is_best_updated = true;
        }
        return true;
      }
    }
    return false;
  }
  bool is_best() { return is_best_updated; }
};
} // namespace gsh
