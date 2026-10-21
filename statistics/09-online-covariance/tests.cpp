#include "online_covariance.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>
namespace {
std::size_t checks = 0, failures = 0;
void check(bool ok, std::string_view label) {
    ++checks; if (!ok) { ++failures; std::cerr << "FAIL " << label << '\n'; }
}
void near(double actual, double expected, double eps, std::string_view label) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= eps, label);
}
template<class E, class F> void rejects(F action, std::string_view label) {
    try { action(); check(false,label); }
    catch (const E&) {check(true,label);}
    catch (...) {check(false,label);}
}
void hand_calculation() {
    statistics::OnlineCovariance s;
    check(s.count() == 0, "empty count");
    check(!s.correlation(), "undefined r if empty");
    check(!s.sample_covariance(), "undefined sample if empty");
    check(!s.population_covariance(), "undefined population if empty");
    const std::array<double,5> x{1,2,3,4,5}, y{2,4,5,4,5};
    for (std::size_t i=0; i<x.size(); ++i) s.add(x[i],y[i]);
    near(s.mean_x(),3.0,1e-12,"mean x");
    near(s.mean_y(),4.0,1e-12,"mean y");
    near(*s.sample_covariance(),1.5,1e-12,"sample covariance");
    near(*s.population_covariance(),1.2,1e-12,"population covariance");
    near(*s.variance_x(),2.5,1e-12,"variance x");
    near(*s.variance_y(),1.5,1e-12,"variance y");
    near(*s.correlation(),1.5/std::sqrt(3.75),1e-12,"r");
}
void degeneracy() {
    statistics::OnlineCovariance s;
    s.add(3,4);
    check(!s.sample_covariance(),"one sample no sample covariance");
    check(!s.variance_x(),"one sample no variance");
    near(*s.population_covariance(),0.0,1e-12,"one population covariance");
    s.add(3,6); s.add(3,8);
    near(*s.variance_x(),0.0,1e-12,"constant X");
    check(!s.correlation(),"no r for constant X");
    statistics::OnlineCovariance negative, positive;
    for (int i=0;i<100;++i) {
        negative.add(i,99-i);
        positive.add(i,2*i+3);
    }
    near(*negative.correlation(),-1.0,1e-12,"negative r");
    near(*positive.correlation(),1.0,1e-12,"positive r");
}
void merges() {
    statistics::OnlineCovariance all, first, second;
    for (int i=0;i<1200;++i) {
        const double x=12+i*0.12, y=9+i*0.07+(i%7)*2.5;
        all.add(x,y);
        if (i<456) first.add(x,y); else second.add(x,y);
    }
    first.merge(second);
    check(first.count()==all.count(),"merged count");
    near(first.mean_x(),all.mean_x(),1e-10,"merged x mean");
    near(first.mean_y(),all.mean_y(),1e-10,"merged y mean");
    near(*first.sample_covariance(),*all.sample_covariance(),1e-7,"merged covariance");
    near(*first.variance_x(),*all.variance_x(),1e-7,"merged variance x");
    near(*first.variance_y(),*all.variance_y(),1e-7,"merged variance y");
    near(*first.correlation(),*all.correlation(),1e-12,"merged r");
    const auto count=first.count();
    first.merge({});
    check(first.count()==count,"empty merge");
    statistics::OnlineCovariance empty;
    empty.merge(second);
    check(empty.count()==second.count(),"copy into empty");
    near(*empty.sample_covariance(),*second.sample_covariance(),1e-12,"copy values");
}
void invalid_and_numeric() {
    statistics::OnlineCovariance s;
    const auto nan=std::numeric_limits<double>::quiet_NaN();
    const auto inf=std::numeric_limits<double>::infinity();
    rejects<std::invalid_argument>([&]{s.add(nan,1);},"reject nan x");
    rejects<std::invalid_argument>([&]{s.add(1,nan);},"reject nan y");
    rejects<std::invalid_argument>([&]{s.add(inf,1);},"reject inf x");
    rejects<std::invalid_argument>([&]{s.add(1,-inf);},"reject inf y");
    check(s.count()==0,"invalid inputs do not mutate");
    s.add(1,2);
    rejects<std::overflow_error>([&]{s.add(1e308,1e308);},"reject overflow");
    check(s.count()==1,"overflow does not mutate");
    statistics::OnlineCovariance offset;
    for (int i=0;i<10000;++i) offset.add(1e9+i,-1e9+3*i);
    near(offset.mean_x(),1e9+4999.5,.01,"large-offset mean");
    near(*offset.correlation(),1.0,1e-10,"large-offset r");
}
}
int main() {
    hand_calculation(); degeneracy(); merges(); invalid_and_numeric();
    std::cout << "Checks: " << checks << ", failures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
