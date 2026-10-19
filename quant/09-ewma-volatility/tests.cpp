#include "ewma.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace {
int passed=0, failed=0;
void check(bool ok, std::string_view label) {
    if (ok) ++passed;
    else { ++failed; std::cerr << "FAIL: " << label << '\n'; }
}
bool near(double actual, double expected, double tol=1e-12) {
    return std::isfinite(actual) && std::fabs(actual-expected)<=tol;
}
template<class Error,class Func>
void raises(Func fn,std::string_view label) {
    try { fn(); check(false,label); }
    catch(const Error&) { check(true,label); }
    catch(...) { check(false,label); }
}
}
int main() {
    const auto nan=std::numeric_limits<double>::quiet_NaN();
    const auto inf=std::numeric_limits<double>::infinity();
    const auto huge=std::numeric_limits<double>::max();
    quant::EwmaVolatility m{0.94,0.0001};
    check(near(m.decay(),0.94),"decay");
    check(near(m.forecast_variance(),0.0001),"initial variance");
    check(near(m.forecast_volatility(),0.01),"initial sigma");
    check(m.observations()==0,"initial count");
    check(near(m.half_life_observations(),std::log(0.5)/std::log(0.94)),"half-life");
    m.observe(0.02);
    check(near(m.forecast_variance(),0.000118),"positive update");
    check(near(m.forecast_volatility(),std::sqrt(0.000118)),"sigma after update");
    check(m.observations()==1,"first count");
    m.observe(-0.02);
    check(near(m.forecast_variance(),0.00013492),"negative update");
    check(m.observations()==2,"second count");
    m.observe(0);
    check(near(m.forecast_variance(),0.0001268248),"zero return");
    check(m.observations()==3,"third count");
    check(near(m.annualized_volatility(252),std::sqrt(0.0001268248*252)),"annualization");
    check(near(m.annualized_volatility(1),m.forecast_volatility()),"one-period scaling");
    check(near(m.annualized_volatility(4),2*m.forecast_volatility()),"square root of time");

    quant::EwmaVolatility fast{0.5,0.0001},slow{0.99,0.0001};
    fast.observe(0.1);slow.observe(0.1);
    check(fast.forecast_variance()>slow.forecast_variance(),"faster response");
    check(slow.half_life_observations()>fast.half_life_observations(),"longer memory");
    quant::EwmaVolatility same{0.5,0.04};
    same.observe(0.2);
    check(near(same.forecast_variance(),0.04),"stationary state");
    quant::EwmaVolatility zero{0.94,0};
    zero.observe(0);
    check(near(zero.forecast_variance(),0),"zero variance");
    check(near(zero.forecast_volatility(),0),"zero volatility");

    for(double x:{-0.1,0.0,1.0,1.1,nan,inf,-inf})
        raises<std::invalid_argument>([&]{quant::EwmaVolatility bad{x,0};(void)bad;},"bad decay");
    for(double x:{-1.0,nan,inf,-inf})
        raises<std::invalid_argument>([&]{quant::EwmaVolatility bad{0.94,x};(void)bad;},"bad seed");

    const auto previous=m.forecast_variance();
    const auto count=m.observations();
    for(double x:{nan,inf,-inf}) {
        raises<std::invalid_argument>([&]{m.observe(x);},"bad observed return");
        check(near(m.forecast_variance(),previous),"invalid observation preserves variance");
        check(m.observations()==count,"invalid observation preserves count");
    }
    raises<std::overflow_error>([&]{m.observe(huge);},"squared return overflow");
    check(near(m.forecast_variance(),previous),"overflow preserves variance");
    check(m.observations()==count,"overflow preserves count");
    for(double x:{-1.0,0.0,nan,inf,-inf})
        raises<std::invalid_argument>([&]{(void)m.annualized_volatility(x);},"bad scale");
    quant::EwmaVolatility giant{0.94,huge};
    raises<std::overflow_error>([&]{(void)giant.annualized_volatility(huge);},"annual overflow");

    const auto r=quant::normal_tail_risk_95(0.02,10000);
    check(near(r.value_at_risk_fraction,0.032897072539029444),"normal VaR fraction");
    check(near(r.expected_shortfall_fraction,0.04125425615014851),"normal ES fraction");
    check(near(r.value_at_risk_amount,328.97072539029444,1e-9),"normal VaR amount");
    check(near(r.expected_shortfall_amount,412.5425615014851,1e-9),"normal ES amount");
    check(r.expected_shortfall_amount>r.value_at_risk_amount,"ES greater than VaR");
    const auto r0=quant::normal_tail_risk_95(0,100);
    check(near(r0.value_at_risk_amount,0),"zero VaR");
    check(near(r0.expected_shortfall_amount,0),"zero ES");
    for(double x:{-0.1,nan,inf,-inf})
        raises<std::invalid_argument>([&]{(void)quant::normal_tail_risk_95(x,1);},"bad sigma");
    for(double x:{-1.0,0.0,nan,inf,-inf})
        raises<std::invalid_argument>([&]{(void)quant::normal_tail_risk_95(0.01,x);},"bad value");
    raises<std::overflow_error>([&]{(void)quant::normal_tail_risk_95(huge,huge);},"tail risk overflow");

    quant::EwmaVolatility timing{0.5,0.0004};
    const auto d1=timing.forecast_variance();timing.observe(0.03);
    const auto d2=timing.forecast_variance();timing.observe(0.01);
    const auto d3=timing.forecast_variance();
    check(near(d1,0.0004),"first day uses seed");
    check(near(d2,0.00065),"second day uses first return");
    check(near(d3,0.000375),"third day uses second return");
    check(d1!=d2 && d2!=d3,"forecast evolves");
    std::cout << "Passed: " << passed << ", failed: " << failed << '\n';
    return failed==0?0:1;
}
