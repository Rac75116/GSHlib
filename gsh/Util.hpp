#pragma once
#include <type_traits>  // std::decay_t
#include <utility>      // std::forward

namespace gsh {

class InPlaceTag {};
constexpr InPlaceTag InPlace;

namespace internal {
    template<class T> class StepResult1 {
        T ed;
    public:
        class iterator {
            T data;
        };
        template<class U> constexpr StepResult1(U&& e) : ed(std::forward<U>(e)) {}
    };
}  // namespace internal
template<class T> constexpr auto Step(T&& ed) {
    return internal::StepResult1<std::decay_t<T>>(std::forward<T>(ed));
}

}  // namespace gsh