#pragma once

#include <vector>

namespace quant {

// A known payment (positive) or expense (negative) at a given future time.
struct CashFlow {
    double years;
    double amount;
};

// Constant, continuously compounded annual rate expressed as a decimal.
// For example, 0.05 means 5% per year.
// Rejects non-finite inputs, negative times and unrepresentable outputs.
double discount_factor(double annual_rate, double years);

double present_value(const CashFlow& cash_flow, double annual_rate);

// Sum the present values of all cash flows. The list may be empty.
// Order of cash flows does not have economic meaning.
double net_present_value(const std::vector<CashFlow>& cash_flows,
                         double annual_rate);

}  // namespace quant
