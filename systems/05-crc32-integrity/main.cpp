#include "crc32.hpp"
#include "../04-binary-message-codec/codec.hpp"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <variant>

int main() {
    const auto original = wire::encode(wire::Add{7, 10123, 250});
    if (!original) {
        std::cerr << "Could not encode sample event\n";
        return 1;
    }
    const auto protected_packet = integrity::append_crc(original->view());
    if (!protected_packet) {
        std::cerr << "Could not protect sample event\n";
        return 1;
    }

    std::cout << "Payload bytes: " << original->size << '\n';
    std::cout << "Transmitted bytes: " << protected_packet->size << '\n';
    std::cout << "CRC-32: 0x" << std::hex << std::uppercase
              << std::setw(8) << std::setfill('0')
              << integrity::crc32(original->view()) << std::dec << '\n';

    if (!integrity::verify_crc(protected_packet->view())) {
        std::cerr << "Checksum did not match\n";
        return 1;
    }
    // Verify the checksum first; decode only the original 20-byte message.
    const auto decoded = wire::decode(protected_packet->view().first(original->size));
    if (!decoded || !std::holds_alternative<wire::Add>(*decoded.message)) {
        std::cerr << "Inner message was invalid\n";
        return 1;
    }
    std::cout << "Verified original event: yes\n";

    auto corrupted = *protected_packet;
    corrupted.bytes[8] ^= std::byte{1};
    std::cout << "Verified altered event: "
              << (integrity::verify_crc(corrupted.view()) ? "yes" : "no") << '\n';
    return 0;
}
