#include "varint.hpp"
#include <array>
#include <iomanip>
#include <iostream>
int main() {
    // A synthetic event feed contains numeric IDs, not textual labels.
    for (const std::uint64_t id : {127ULL, 300ULL, 15000ULL}) {
        std::array<std::uint8_t, 10> buffer{};
        const auto n = systems::encode(id, buffer);
        const auto result = systems::decode(
            std::span<const std::uint8_t>(buffer.data(), n));
        std::cout << "Event ID " << id << " ->";
        for (std::size_t i = 0; i < n; ++i) {
            std::cout << ' ' << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<unsigned>(buffer[i]);
        }
        std::cout << std::dec << " -> " << result.value << '\n';
    }
}
