#include "discounting.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    try {
        if (argc > 2) {
            throw std::invalid_argument("usage: dcf_demo [annual_rate_as_decimal]");
        }

        double annual_rate = 0.05;
        if (argc == 2) {
            const std::string text = argv[1];
            std::size_t parsed = 0;
            try {
                annual_rate = std::stod(text, &parsed);
            } catch (const std::exception&) {
                throw std::invalid_argument("rate must be a decimal such as 0.05");
            }
            if (parsed != text.size()) {
                throw std::invalid_argument("rate must be a decimal such as 0.05");
            }
        }

        // Synthetic project: pay 1000 today; receive 400 after 1, 2 and 3 years.
        const std::vector<quant::CashFlow> cash_flows = {
            {0.0, -1000.0}, {1.0, 400.0}, {2.0, 400.0}, {3.0, 400.0}
        };

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "annual_rate=" << annual_rate << '\n';
        for (const auto& flow : cash_flows) {
            std::cout << "year=" << flow.years << " amount=" << flow.amount
                      << " present_value=" << quant::present_value(flow, annual_rate)
                      << '\n';
        }

        const double npv = quant::net_present_value(cash_flows, annual_rate);
        std::cout << "NPV=" << npv << '\n';
        std::cout << "NPV_meaning="
                  << (npv > 0.0 ? "positive" : (npv < 0.0 ? "negative" : "zero"))
                  << "_under_assumed_rate\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
