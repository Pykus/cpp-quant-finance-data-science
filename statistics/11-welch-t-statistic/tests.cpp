#include "welch.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
int checks = 0;
void expect(bool ok, const char* msg) {
    ++checks;
    if (!ok) { std::cerr << "FAIL: " << msg << '\n'; std::exit(EXIT_FAILURE); }
}
void near(double actual, double expected, double tolerance, const char* msg) {
    expect(std::abs(actual-expected) <= tolerance, msg);
}
template<typename E,typename F>
void throws(F&& f) {
    bool yes=false;
    try { f(); } catch(const E&) { yes=true; }
    expect(yes, "exception expected");
}
}
int main() {
    using statistics11::welch;
    using statistics11::summarize;
    constexpr std::array<double,3> a{1,2,3}, b{2,3,4};
    const auto result=welch(a,b);
    near(result.mean_difference,-1.0,1e-12,"difference");
    near(result.standard_error,std::sqrt(2.0/3.0),1e-12,"SE");
    near(result.t_statistic,-std::sqrt(3.0/2.0),1e-12,"t");
    near(result.approximate_degrees_of_freedom,4.0,1e-12,"df");
    const auto reversed=welch(b,a);
    near(reversed.t_statistic,-result.t_statistic,1e-12,"symmetry");
    near(summarize(a).sample_variance,1.0,1e-12,"sample variance");

    constexpr std::array<double,4> unequal{2,4,8,16};
    const auto mixed=welch(a,unequal);
    expect(mixed.standard_error>0,"positive uncertainty");
    expect(mixed.approximate_degrees_of_freedom>0,"valid df");

    constexpr std::array<double,1> single{1};
    throws<std::invalid_argument>([&]{(void)welch(single,b);});
    constexpr std::array<double,2> constants{3,3};
    throws<std::domain_error>([&]{(void)welch(constants,constants);});
    const std::array<double,2> invalid{1,std::numeric_limits<double>::infinity()};
    throws<std::invalid_argument>([&]{(void)welch(invalid,b);});

    std::vector<double> long_a, long_b;
    for (int i=0;i<10000;++i) {
        long_a.push_back(static_cast<double>(i%5));
        long_b.push_back(static_cast<double>((i%5)+1));
        expect(long_a.size()==static_cast<std::size_t>(i+1),"growing sample");
    }
    const auto long_result=welch(long_a,long_b);
    near(long_result.mean_difference,-1,1e-10,"long sample difference");
    near(long_result.approximate_degrees_of_freedom,19998,0.01,"large equal variances");
    std::cout<<"PASS "<<checks<<" checks\n";
}
