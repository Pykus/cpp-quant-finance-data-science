#include "codec.hpp"
#include <iostream>
#include <variant>

int main() {
    const auto packet = wire::encode(wire::Add{7, 10123, 250});
    if (!packet) return 1;
    const auto decoded = wire::decode(packet->view());
    if (!decoded || !std::holds_alternative<wire::Add>(*decoded.message)) return 2;
    const auto& add = std::get<wire::Add>(*decoded.message);
    std::cout << "frame bytes: " << packet->size << '\n'
              << "order ID: " << add.id << '\n'
              << "price ticks: " << add.price_ticks << '\n'
              << "quantity: " << add.quantity << '\n';
}
