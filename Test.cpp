#define NDEBUG
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/TypeDef.hpp>
#include <gsh/Algorithm.hpp>
#include <gsh/Vec.hpp>
#include <gsh/Arr.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Numeric.hpp>
#include <gsh/Macro.hpp>
#include <gsh/Util.hpp>
#include <gsh/DisjointSet.hpp>
#include <iostream>
#include <cassert>

int main() {
    try {
        gsh::ClockTimer t;
        [[maybe_unused]] gsh::Rand64 engine;
        {
            using namespace std;
            using namespace gsh;
            using namespace itype;
            using namespace ftype;
            Vec<int> r{ 2, 3, 4, 5 };
            for (auto& x : r.slice(1, 3)) {
                x = 100;
            }
            for (auto& x : r) {
                cout << x << ' ';
            }
            cout << endl;
        }
        //t.print();
    } catch (gsh::Exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
