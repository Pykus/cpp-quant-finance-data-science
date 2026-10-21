#include "online_covariance.hpp"
#include <iomanip>
#include <iostream>
#include <utility>
int main() {
    statistics::OnlineCovariance s;
    for (const auto& [visits,orders] :
         {std::pair{1.0,2.0},{2.0,4.0},{3.0,5.0},{4.0,4.0},{5.0,5.0}})
        s.add(visits,orders);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Weeks: " << s.count() << '\n';
    std::cout << "Sample covariance: " << *s.sample_covariance() << '\n';
    std::cout << "Pearson correlation: " << *s.correlation() << '\n';
}
