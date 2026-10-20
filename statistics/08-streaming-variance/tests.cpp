#include "running_moments.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

namespace {
int tests = 0, failures = 0;
void check(bool x, const char* name) {
    ++tests;
    if (!x) { ++failures; std::cerr << "FAIL " << name << "\n"; }
}
bool near(double a, double b, double tol=1e-8) { return std::abs(a-b) <= tol; }
stats::RunningMoments make(std::initializer_list<double> xs) {
    stats::RunningMoments s;
    for (double x : xs) s.add(x);
    return s;
}
}

int main() {
    {
        stats::RunningMoments s;
        check(s.count()==0, "empty count");
        check(!s.mean(), "empty mean");
        check(!s.population_variance(), "empty population variance");
        check(!s.sample_variance(), "empty sample variance");
        check(!s.sample_stddev(), "empty sample stddev");
        check(!s.minimum(), "empty min");
        check(!s.maximum(), "empty max");
        s.merge(stats::RunningMoments{});
        check(s.count()==0, "empty merge");
    }
    {
        auto s=make({10});
        check(s.count()==1, "singleton count");
        check(near(*s.mean(),10), "singleton mean");
        check(near(*s.population_variance(),0), "singleton population variance");
        check(!s.sample_variance(), "singleton sample variance undefined");
        check(!s.sample_stddev(), "singleton sample stddev undefined");
        check(near(*s.minimum(),10), "singleton min");
        check(near(*s.maximum(),10), "singleton max");
    }
    {
        auto s=make({10,12,14,16});
        check(s.count()==4, "worked count");
        check(near(*s.mean(),13), "worked mean");
        check(near(*s.population_variance(),5), "worked population variance");
        check(near(*s.sample_variance(),20.0/3), "worked sample variance");
        check(near(*s.sample_stddev(),std::sqrt(20.0/3)), "worked stddev");
        check(near(*s.minimum(),10), "worked min");
        check(near(*s.maximum(),16), "worked max");
    }
    {
        auto s=make({-7,-7,-7});
        check(near(*s.mean(),-7), "negative mean");
        check(near(*s.sample_variance(),0), "constant variance");
        check(near(*s.sample_stddev(),0), "constant stddev");
    }
    {
        auto s=make({1e12+1,1e12+2,1e12+3,1e12+4});
        check(near(*s.sample_variance(),5.0/3), "stable large offset variance");
        check(near(*s.mean(),1e12+2.5), "stable mean");
    }
    {
        auto all=make({10,12,14,16,18,20});
        auto a=make({10,12,14});
        auto b=make({16,18,20});
        a.merge(b);
        check(a.count()==6, "merge count");
        check(near(*a.mean(),*all.mean()), "merge mean");
        check(near(*a.sample_variance(),*all.sample_variance()), "merge variance");
        check(near(*a.minimum(),10), "merge min");
        check(near(*a.maximum(),20), "merge max");
        a.merge(stats::RunningMoments{});
        check(a.count()==6, "merge empty remains");
        stats::RunningMoments empty;
        empty.merge(a);
        check(empty.count()==6, "merge into empty");
        check(near(*empty.sample_variance(),*a.sample_variance()), "merge into empty variance");
    }
    {
        stats::RunningMoments s;
        for (double x : {std::numeric_limits<double>::quiet_NaN(),
                         std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity()}) {
            bool threw=false;
            try { s.add(x); } catch(const std::invalid_argument&) { threw=true; }
            check(threw,"reject non-finite");
        }
        check(s.count()==0,"failed add is atomic");
        s.add(0.0);
        check(s.count()==1,"zero accepted");
    }
    {
        stats::RunningMoments s;
        for (int i=1;i<=100000;++i) s.add(i);
        check(s.count()==100000,"large stream count");
        check(near(*s.mean(),50000.5),"large stream mean");
        check(near(*s.sample_variance(),100000.0*100001.0/12.0),"large stream variance");
    }
    std::cout << (tests-failures) << "/" << tests << " checks passed\n";
    return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
