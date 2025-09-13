#pragma once
#include <chrono>
#include <cmath>
#include "TypeDef.hpp"  // gsh::itype
#include "Random.hpp"
#include "Memory.hpp"
#include "Range.hpp"

namespace gsh {

class ZeroTemp {
public:
    ZeroTemp() {}
    ftype::f64 operator()([[maybe_unused]] ftype::f64 progress) const { return 0.0; }
};

class LinearTemp {
    ftype::f64 init_temp, final_temp;
public:
    LinearTemp(ftype::f64 init_temp, ftype::f64 final_temp = 0.0) : init_temp(init_temp), final_temp(final_temp) {}
    ftype::f64 operator()(ftype::f64 progress) const { return init_temp + (final_temp - init_temp) * progress; }
};

class ExponentialTemp {
    ftype::f64 init_temp;
    ftype::f64 ratio;
public:
    ExponentialTemp(ftype::f64 init_temp, ftype::f64 final_temp = 0.001) : init_temp(init_temp), ratio(final_temp / init_temp) {}
    ftype::f64 operator()(ftype::f64 progress) const { return init_temp * std::pow(ratio, progress); }
};

class VariableExponentialTemp {
    ftype::f64 init_temp;
    ftype::f64 shape;
    ftype::f64 ratio;
public:
    VariableExponentialTemp(ftype::f64 init_temp, ftype::f64 final_temp = 0.001, ftype::f64 shape = 2.0) : init_temp(init_temp), shape(shape), ratio(final_temp / init_temp) {}
    ftype::f64 operator()(ftype::f64 progress) const { return init_temp * std::pow(ratio, std::pow(progress, shape)); }
};

class SigmoidTemp {
    ftype::f64 init_temp, final_temp;
    ftype::f64 steepness;
public:
    SigmoidTemp(ftype::f64 init_temp, ftype::f64 final_temp = 0.0, ftype::f64 steepness = 10.0) : init_temp(init_temp), final_temp(final_temp), steepness(steepness) {}
    ftype::f64 operator()(ftype::f64 progress) const { return final_temp + (init_temp - final_temp) / (1.0 + std::exp(steepness * (progress - 0.5))); }
};

class LogarithmicTemp {
    ftype::f64 init_temp, final_temp;
    ftype::f64 steepness;
public:
    LogarithmicTemp(ftype::f64 init_temp, ftype::f64 final_temp = 0.0, ftype::f64 steepness = 10.0) : init_temp(init_temp), final_temp(final_temp), steepness(steepness) {}
    ftype::f64 operator()(ftype::f64 progress) const { return final_temp + (init_temp - final_temp) / (1.0 + std::log(steepness * (progress + 1e-6))); }
};

class QuadraticTemp {
    ftype::f64 init_temp, final_temp;
    ftype::f64 steepness;
public:
    QuadraticTemp(ftype::f64 init_temp, ftype::f64 final_temp = 0.0, ftype::f64 steepness = 10.0) : init_temp(init_temp), final_temp(final_temp), steepness(steepness) {}
    ftype::f64 operator()(ftype::f64 progress) const { return final_temp + (init_temp - final_temp) / (1.0 + steepness * (progress - 0.5) * (progress - 0.5)); }
};

class IterationProgress {
    itype::u32 max_iter;
    ftype::f64 current_progress = 0.0;
public:
    IterationProgress(itype::u32 max_iter) : max_iter(max_iter) {}
    ftype::f64 progress() const { return current_progress; }
    bool update(itype::u32 current_iter) {
        current_progress = static_cast<ftype::f64>(current_iter) / static_cast<ftype::f64>(max_iter);
        return current_iter < max_iter;
    }
};

class TimeProgress {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::duration max_duration;
    ftype::f64 current_progress = 0.0;
public:
    TimeProgress(std::chrono::milliseconds max_duration) : start_time(std::chrono::system_clock::now()), max_duration(std::chrono::duration_cast<std::chrono::system_clock::duration>(max_duration)) {}
    ftype::f64 progress() { return current_progress; }
    bool update([[maybe_unused]] itype::u32 current_iter) {
        auto now = std::chrono::system_clock::now();
        current_progress = static_cast<ftype::f64>((now - start_time).count()) / static_cast<ftype::f64>(max_duration.count());
        return now - start_time < max_duration;
    }
};

namespace annealing {
    constexpr struct _Minimize {
    } minimize;
    constexpr struct _Maximize {
    } maximize;
}  // namespace annealing

template<class OptType, class TempFunction, class ProgressFunction> class Annealing {
    static_assert(std::is_same<OptType, annealing::_Minimize>::value || std::is_same<OptType, annealing::_Maximize>::value, "Type must be OptimizeType::Minimize or OptimizeType::Maximize");
    constexpr static bool is_minimize = std::is_same<OptType, annealing::_Minimize>::value;
    TempFunction temp_function;
    ProgressFunction progress_function;
    ftype::f64 current_score;
    ftype::f64 threshold_score;
    ftype::f64 best_score;
    ftype::f64 current_temp = 0.0;
    itype::u32 ugap;
    itype::u32 current_iter = 0;
    bool is_best_updated = false;
    [[no_unique_address]] Allocator<ftype::f64> alloc;
    ftype::f64* rnd_buf = nullptr;
    itype::u32 rnd_buf_size;
public:
    Annealing() = delete;
    Annealing(const OptType&, const TempFunction& tempf, const ProgressFunction& progressf, ftype::f64 init_score, itype::u32 update_gap = 8, itype::u64 seed = Rand64::default_seed, itype::u32 buf_size = 12) : temp_function(tempf), progress_function(progressf), current_score(init_score), best_score(init_score), ugap(1u << update_gap), alloc(), rnd_buf_size(1u << buf_size) {
        if constexpr (!std::is_same<TempFunction, ZeroTemp>::value) {
            rnd_buf = alloc.allocate(rnd_buf_size);
            Rand64 rand_function(seed);
            for (itype::u32 i = 0; i < rnd_buf_size; ++i) {
                rnd_buf[i] = std::log(1.0 - Canocicaled32(rand_function));
            }
        }
    }
    ~Annealing() {
        if (rnd_buf) alloc.deallocate(rnd_buf, rnd_buf_size);
    }
    itype::u32 iterations() const { return current_iter; }
    ftype::f64 temp() const { return current_temp; }
    ftype::f64 score() const { return current_score; }
    ftype::f64 threshold() const { return threshold_score; }
    ftype::f64 best() const { return best_score; }
    itype::u32 gap() const { return ugap; }
    bool update() {
        if (current_iter % ugap == 0) {
            if (!progress_function.update(current_iter)) return false;
            if constexpr (!std::is_same<TempFunction, ZeroTemp>::value) {
                current_temp = temp_function(progress_function.progress());
            }
        }
        if constexpr (std::is_same<TempFunction, ZeroTemp>::value) {
            threshold_score = current_score;
        } else {
            if constexpr (is_minimize) {
                threshold_score = current_score - current_temp * rnd_buf[current_iter & (rnd_buf_size - 1)];
            } else {
                threshold_score = current_score + current_temp * rnd_buf[current_iter & (rnd_buf_size - 1)];
            }
        }
        ++current_iter;
        return true;
    }
    bool acceptable(ftype::f64 new_score) {
        if constexpr (is_minimize) {
            return new_score < threshold_score;
        } else {
            return new_score > threshold_score;
        }
    }
    bool accept(ftype::f64 new_score) {
        is_best_updated = false;
        if constexpr (is_minimize) {
            if (new_score < threshold_score) {
                current_score = new_score;
                if (new_score < best_score) {
                    best_score = new_score;
                    is_best_updated = true;
                }
                return true;
            }
        } else {
            if (new_score > threshold_score) {
                current_score = new_score;
                if (new_score > best_score) {
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

}  // namespace gsh
