#include <cstdint>
#include <iostream>
#include <limits>

constexpr bool fits_in_byte(std::uint32_t value) {
    return value <= std::numeric_limits<std::uint8_t>::max();
}

int main() {
    constexpr std::uint32_t sensor_reading = 300;
    constexpr std::uint32_t safe_reading = 200;
    static_assert(fits_in_byte(safe_reading));
    static_assert(!fits_in_byte(sensor_reading));

    std::cout << "sensor_reading=" << sensor_reading << '\n';
    std::cout << "fits_uint8=" << std::boolalpha << fits_in_byte(sensor_reading) << '\n';
    std::cout << "safe_reading=" << safe_reading << '\n';
    std::cout << "safe_as_uint8=" << static_cast<unsigned>(static_cast<std::uint8_t>(safe_reading)) << '\n';
}
