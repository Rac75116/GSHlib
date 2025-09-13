#pragma once
#include <chrono>
#include <cmath>
#include "TypeDef.hpp"  // gsh::itype
#include "Random.hpp"

namespace gsh {

class ZeroTemp {
public:
    ZeroTemp() {}
    ftype::f32 operator()([[maybe_unused]] ftype::f32 progress) const { return 0.0f; }
};

class LinearTemp {
    ftype::f32 init_temp, final_temp;
public:
    LinearTemp(ftype::f32 init_temp, ftype::f32 final_temp = 0.0f) : init_temp(init_temp), final_temp(final_temp) {}
    ftype::f32 operator()(ftype::f32 progress) const { return init_temp + (final_temp - init_temp) * progress; }
};

class ExponentialTemp {
    ftype::f32 init_temp;
    ftype::f32 ratio;
public:
    ExponentialTemp(ftype::f32 init_temp, ftype::f32 final_temp = 0.001f) : init_temp(init_temp), ratio(final_temp / init_temp) {}
    ftype::f32 operator()(ftype::f32 progress) const { return init_temp * std::pow(ratio, progress); }
};

class VariableExponentialTemp {
    ftype::f32 init_temp;
    ftype::f32 shape;
    ftype::f32 ratio;
public:
    VariableExponentialTemp(ftype::f32 init_temp, ftype::f32 final_temp = 0.001f, ftype::f32 shape = 2.0f) : init_temp(init_temp), shape(shape), ratio(final_temp / init_temp) {}
    ftype::f32 operator()(ftype::f32 progress) const { return init_temp * std::pow(ratio, std::pow(progress, shape)); }
};

class SigmoidTemp {
    ftype::f32 init_temp, final_temp;
    ftype::f32 steepness;
public:
    SigmoidTemp(ftype::f32 init_temp, ftype::f32 final_temp = 0.0f, ftype::f32 steepness = 10.0f) : init_temp(init_temp), final_temp(final_temp), steepness(steepness) {}
    ftype::f32 operator()(ftype::f32 progress) const { return final_temp + (init_temp - final_temp) / (1.0f + std::exp(steepness * (progress - 0.5f))); }
};

class LogarithmicTemp {
    ftype::f32 init_temp, final_temp;
    ftype::f32 steepness;
public:
    LogarithmicTemp(ftype::f32 init_temp, ftype::f32 final_temp = 0.0f, ftype::f32 steepness = 10.0f) : init_temp(init_temp), final_temp(final_temp), steepness(steepness) {}
    ftype::f32 operator()(ftype::f32 progress) const { return final_temp + (init_temp - final_temp) / (1.0f + std::log(steepness * (progress + 1e-6f))); }
};

class QuadraticTemp {
    ftype::f32 init_temp, final_temp;
    ftype::f32 steepness;
public:
    QuadraticTemp(ftype::f32 init_temp, ftype::f32 final_temp = 0.0f, ftype::f32 steepness = 10.0f) : init_temp(init_temp), final_temp(final_temp), steepness(steepness) {}
    ftype::f32 operator()(ftype::f32 progress) const { return final_temp + (init_temp - final_temp) / (1.0f + steepness * (progress - 0.5f) * (progress - 0.5f)); }
};

class IterationProgress {
    itype::u32 max_iter;
    ftype::f32 current_progress = 0.0f;
public:
    IterationProgress(itype::u32 max_iter) : max_iter(max_iter) {}
    ftype::f32 progress() const { return current_progress; }
    bool update(itype::u32 current_iter) {
        current_progress = static_cast<ftype::f32>(current_iter) / static_cast<ftype::f32>(max_iter);
        return current_iter < max_iter;
    }
};

class TimeProgress {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::duration max_duration;
    ftype::f32 current_progress = 0.0f;
public:
    TimeProgress(std::chrono::milliseconds max_duration) : start_time(std::chrono::system_clock::now()), max_duration(std::chrono::duration_cast<std::chrono::system_clock::duration>(max_duration)) {}
    ftype::f32 progress() { return current_progress; }
    bool update([[maybe_unused]] itype::u32 current_iter) {
        auto now = std::chrono::system_clock::now();
        current_progress = static_cast<ftype::f32>((now - start_time).count()) / static_cast<ftype::f32>(max_duration.count());
        return now - start_time < max_duration;
    }
};

enum class OptimizeType { Minimize, Maximize };
template<OptimizeType Type, class TempFunction, class ProgressFunction> class Annealing {
    TempFunction temp_function;
    ProgressFunction progress_function;
    Rand32 rand_function;
    ftype::f32 current_score;
    ftype::f32 threshold_score;
    ftype::f32 best_score;
    ftype::f32 current_temp = 0.0f;
    itype::u32 ugap, current_iter = 0;
    bool is_best_updated = false;
public:
    Annealing() = delete;
    Annealing(const TempFunction& tempf, const ProgressFunction& progressf, ftype::f32 init_score, itype::u32 update_gap = 16, itype::u32 seed = Rand32::default_seed) : temp_function(tempf), progress_function(progressf), rand_function(seed), current_score(init_score), best_score(init_score), ugap(update_gap) {}
    itype::u32 iter() const { return current_iter; }
    ftype::f32 temp() const { return current_temp; }
    ftype::f32 score() const { return current_score; }
    ftype::f32 threshold() const { return threshold_score; }
    ftype::f32 best() const { return best_score; }
    itype::u32 gap() const { return ugap; }
    bool update() {
        if (current_iter % ugap == 0) {
            if (!progress_function.update(current_iter)) return false;
            current_temp = temp_function(progress_function.progress());
        }
        if constexpr (Type == OptimizeType::Minimize) {
            threshold_score = current_score - current_temp * std::log(1.0f - Canocicaled32(rand_function));
        } else {
            threshold_score = current_score + current_temp * std::log(1.0f - Canocicaled32(rand_function));
        }
        ++current_iter;
        return true;
    }
    bool acceptable(ftype::f32 new_score) {
        if constexpr (Type == OptimizeType::Minimize) {
            return new_score < threshold_score;
        } else {
            return new_score > threshold_score;
        }
    }
    bool accept(ftype::f32 new_score) {
        is_best_updated = false;
        if constexpr (Type == OptimizeType::Minimize) {
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
