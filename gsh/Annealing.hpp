#pragma once
#include <chrono>
#include "TypeDef.hpp"  // gsh::itype
#include "Random.hpp"

namespace gsh {

enum class OptimizeType { Minimize, Maximize };
template<OptimizeType Type = OptimizeType::Minimize> class Annealing {
    ftype::f32 itemp, iscore, ugap;
public:
    Annealing(ftype::f32 init_temp, ftype::f32 init_score, itype::u32 update_gap = 16) : itemp(init_temp), iscore(init_score), ugap(update_gap) {}
};

}  // namespace gsh
