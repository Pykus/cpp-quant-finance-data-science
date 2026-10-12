#include "discounting.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace quant {

namespace {

void require_finite(double value, const char* name) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument(name);
    }
}

}  // namespace

double discount_factor(double annual_rate, double years) {
    require_finite(annual_rate, "annual_rate must be finite");
    require_finite(years, "years must be finite");
    if (years < 0.0) {
        throw std::invalid_argument("years cannot be negative");
    }

    const double exponent = -annual_rate * years;
    if (!std::isfinite(exponent)) {
        throw std::overflow_error("discount exponent is out of range");
    }

    const double factor = std::exp(exponent);
    if (!std::isfinite(factor)) {
        throw std::overflow_error("discount factor is out of range");
    }
    return factor;
}

double present_value(const CashFlow& cash_flow, double annual_rate) {
    require_finite(cash_flow.amount, "cash flow amount must be finite");
    const double factor = discount_factor(annual_rate, cash_flow.years);
    const double value = cash_flow.amount * factor;
    if (!std::isfinite(value)) {
        throw std::overflow_error("present value is out of range");
    }
    return value;
}

double net_present_value(const std::vector<CashFlow>& cash_flows,
                         double annual_rate) {
    require_finite(annual_rate, "annual_rate must be finite");
    long double total = 0.0L;
    for (const CashFlow& cash_flow : cash_flows) {
        total += static_cast<long double>(present_value(cash_flow, annual_rate));
    }
    if (!std::isfinite(total) ||
        std::abs(total) > static_cast<long double>(std::numeric_limits<double>::max())) {
        throw std::overflow_error("net present value is out of range");
    }
    return static_cast<double>(total);
}

}  // namespace quant
