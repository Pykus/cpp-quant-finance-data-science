#include "framing.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
[[nodiscard]] std::span<const std::byte> as_bytes(std::string_view text) noexcept {
    return std::as_bytes(std::span<const char>{text.data(), text.size()});
}

void append_frame(std::vector<std::byte>& wire, std::string_view text) {
    const auto frame = stream_framing::encode_frame<64>(as_bytes(text));
    if (!frame) {
        throw std::runtime_error("Demo message exceeds the frame limit");
    }
    wire.insert(wire.end(), frame->view().begin(), frame->view().end());
}
} // namespace

int main() {
    // Fictional market data events, NOT an exchange protocol.
    std::vector<std::byte> tcp_stream;
    append_frame(tcp_stream, "ADD|7|10123|250");
    append_frame(tcp_stream, "CANCEL|7");

    stream_framing::StreamDecoder<64> decoder;
    std::size_t offset = 0;
    std::size_t event_count = 0;

    // Simulate recv() returning different amounts of data each time.
    while (offset < tcp_stream.size()) {
        const auto chunk_size = std::min<std::size_t>(
            1U + (offset % 5U), tcp_stream.size() - offset);
        const auto report = decoder.feed(
            std::span<const std::byte>{tcp_stream.data() + offset, chunk_size},
            [&](const std::span<const std::byte> message) {
                std::string text;
                for (const auto byte : message) {
                    text.push_back(static_cast<char>(
                        std::to_integer<unsigned char>(byte)));
                }
                ++event_count;
                std::cout << "Event " << event_count << ": " << text << '\n';
            });
        if (report.status != stream_framing::FeedStatus::ok) {
            std::cerr << "Invalid frame length\n";
            return 1;
        }
        offset += report.bytes_consumed;
    }
    if (event_count != 2 || decoder.buffered_bytes() != 0) {
        std::cerr << "Incomplete input stream\n";
        return 1;
    }
    std::cout << "Decoded " << event_count << " events\n";
}
