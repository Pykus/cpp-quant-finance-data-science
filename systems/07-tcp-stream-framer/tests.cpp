#include "framing.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Bytes = std::vector<std::byte>;
using Decoder = stream_framing::StreamDecoder<64>;

std::size_t checks = 0;

void check(bool condition, int line) {
    ++checks;
    if (!condition) {
        throw std::runtime_error("Check failed at line " + std::to_string(line));
    }
}
#define CHECK(...) check(static_cast<bool>((__VA_ARGS__)), __LINE__)

[[nodiscard]] Bytes bytes(std::initializer_list<unsigned int> values) {
    Bytes result;
    for (const auto value : values) {
        result.push_back(std::byte{static_cast<unsigned char>(value)});
    }
    return result;
}

[[nodiscard]] std::span<const std::byte> view(const Bytes& b) noexcept {
    return {b.data(), b.size()};
}

void encode_validation() {
    CHECK(!stream_framing::encode_frame<64>(view(bytes({}))).has_value());
    CHECK(!stream_framing::encode_frame<64>(view(Bytes(65, std::byte{1}))).has_value());

    const auto payload = bytes({0x00, 0x7F, 0xFF});
    const auto encoded = stream_framing::encode_frame<64>(view(payload));
    CHECK(encoded.has_value());
    CHECK(encoded->used == 5);
    CHECK(Bytes(encoded->view().begin(), encoded->view().end())
          == bytes({0x00, 0x03, 0x00, 0x7F, 0xFF}));

    const auto max_encoded =
        stream_framing::encode_frame<64>(view(Bytes(64, std::byte{0xAC})));
    CHECK(max_encoded.has_value());
    CHECK(max_encoded->used == 66);
    CHECK(max_encoded->bytes[0] == std::byte{0x00});
    CHECK(max_encoded->bytes[1] == std::byte{0x40});
    CHECK(max_encoded->bytes[65] == std::byte{0xAC});
}

void one_byte_at_a_time() {
    const auto wire = bytes({0x00, 0x03, 0x2A, 0x00, 0xFF});
    Decoder decoder;
    std::vector<Bytes> received;
    for (std::size_t i = 0; i < wire.size(); ++i) {
        const auto result = decoder.feed(
            std::span<const std::byte>{wire.data() + i, 1},
            [&](const std::span<const std::byte> msg) {
                received.emplace_back(msg.begin(), msg.end());
            });
        CHECK(result.status == stream_framing::FeedStatus::ok);
        CHECK(result.bytes_consumed == 1);
        CHECK(result.frames_emitted == (i == wire.size() - 1 ? 1U : 0U));
        CHECK(decoder.buffered_bytes() == (i == wire.size() - 1 ? 0U : i + 1));
    }
    CHECK(received.size() == 1);
    CHECK(received[0] == bytes({0x2A, 0x00, 0xFF}));
}

void all_possible_two_chunk_splits() {
    const auto wire = bytes({0x00, 0x04, 1, 2, 3, 4});
    for (std::size_t cut = 0; cut <= wire.size(); ++cut) {
        Decoder decoder;
        std::vector<Bytes> received;
        auto handler = [&](const std::span<const std::byte> msg) {
            received.emplace_back(msg.begin(), msg.end());
        };
        const auto first = decoder.feed(
            std::span<const std::byte>{wire.data(), cut}, handler);
        const auto second = decoder.feed(
            std::span<const std::byte>{wire.data() + cut, wire.size() - cut},
            handler);
        CHECK(first.status == stream_framing::FeedStatus::ok);
        CHECK(first.bytes_consumed == cut);
        CHECK(second.bytes_consumed == wire.size() - cut);
        CHECK(first.frames_emitted + second.frames_emitted == 1);
        CHECK(received == std::vector<Bytes>{bytes({1, 2, 3, 4})});
        CHECK(decoder.buffered_bytes() == 0);
    }
}

void merged_frames_and_partial_tail() {
    const auto wire = bytes({0, 1, 7, 0, 2, 8, 9, 0, 3, 10});
    Decoder decoder;
    std::vector<Bytes> received;
    auto handler = [&](const std::span<const std::byte> msg) {
        received.emplace_back(msg.begin(), msg.end());
    };
    const auto first = decoder.feed(view(wire), handler);
    CHECK(first.status == stream_framing::FeedStatus::ok);
    CHECK(first.frames_emitted == 2);
    CHECK(first.bytes_consumed == wire.size());
    CHECK(decoder.buffered_bytes() == 3); // header + first payload byte
    CHECK(received == std::vector<Bytes>{bytes({7}), bytes({8, 9})});

    const auto second = decoder.feed(view(bytes({11, 12})), handler);
    CHECK(second.frames_emitted == 1);
    CHECK(received.back() == bytes({10, 11, 12}));
    CHECK(decoder.buffered_bytes() == 0);
}

void malformed_length_requires_explicit_reset() {
    Decoder decoder;
    std::size_t calls = 0;
    auto handler = [&](std::span<const std::byte>) { ++calls; };

    const auto zero = bytes({0x00, 0x00, 0x00, 0x01, 0x11});
    const auto bad = decoder.feed(view(zero), handler);
    CHECK(bad.status == stream_framing::FeedStatus::invalid_length);
    CHECK(bad.bytes_consumed == 2); // does not consume trailing bytes
    CHECK(bad.frames_emitted == 0);
    CHECK(decoder.failed());
    CHECK(calls == 0);

    const auto valid = bytes({0, 1, 0x22});
    const auto blocked = decoder.feed(view(valid), handler);
    CHECK(blocked.status == stream_framing::FeedStatus::invalid_length);
    CHECK(blocked.bytes_consumed == 0);
    CHECK(calls == 0);

    decoder.reset();
    CHECK(!decoder.failed());
    CHECK(decoder.buffered_bytes() == 0);
    const auto ok = decoder.feed(view(valid), handler);
    CHECK(ok.status == stream_framing::FeedStatus::ok);
    CHECK(ok.frames_emitted == 1);
    CHECK(calls == 1);

    const auto excessive = decoder.feed(view(bytes({0, 65, 0x01})), handler);
    CHECK(excessive.status == stream_framing::FeedStatus::invalid_length);
    CHECK(excessive.bytes_consumed == 2);
    CHECK(decoder.failed());
}

void reject_after_delivering_earlier_frames() {
    const auto wire = bytes({0, 1, 42, 0, 65, 77});
    Decoder decoder;
    std::vector<Bytes> received;
    const auto result = decoder.feed(view(wire),
        [&](const std::span<const std::byte> msg) {
            received.emplace_back(msg.begin(), msg.end());
        });
    CHECK(result.status == stream_framing::FeedStatus::invalid_length);
    CHECK(result.bytes_consumed == 5);
    CHECK(result.frames_emitted == 1);
    CHECK(received.size() == 1);
    CHECK(received[0] == bytes({42}));
}

void reset_discards_partial_frame() {
    Decoder decoder;
    std::vector<Bytes> messages;
    auto handler = [&](const std::span<const std::byte> msg) {
        messages.emplace_back(msg.begin(), msg.end());
    };
    const auto partial = decoder.feed(view(bytes({0, 3, 9})), handler);
    CHECK(partial.frames_emitted == 0);
    CHECK(decoder.buffered_bytes() == 3);
    decoder.reset();
    CHECK(decoder.buffered_bytes() == 0);
    const auto done = decoder.feed(view(bytes({0, 1, 8})), handler);
    CHECK(done.frames_emitted == 1);
    CHECK(messages == std::vector<Bytes>{bytes({8})});
}

void high_byte_of_length_is_big_endian() {
    Bytes payload(300, std::byte{0xE0});
    const auto encoded = stream_framing::encode_frame<512>(view(payload));
    CHECK(encoded.has_value());
    CHECK(encoded->bytes[0] == std::byte{0x01});
    CHECK(encoded->bytes[1] == std::byte{0x2C});
    stream_framing::StreamDecoder<512> decoder;
    std::size_t received = 0;
    const auto result = decoder.feed(encoded->view(),
        [&](const std::span<const std::byte> msg) {
            CHECK(msg.size() == 300);
            CHECK(msg.front() == std::byte{0xE0});
            CHECK(msg.back() == std::byte{0xE0});
            ++received;
        });
    CHECK(result.status == stream_framing::FeedStatus::ok);
    CHECK(result.frames_emitted == 1);
    CHECK(received == 1);
}

void callback_failure_does_not_retain_old_frame() {
    Decoder decoder;
    bool thrown = false;
    try {
        const auto ignored = decoder.feed(view(bytes({0, 1, 7})),
            [](std::span<const std::byte>) { throw std::runtime_error("handler"); });
        (void)ignored;
    } catch (const std::runtime_error&) {
        thrown = true;
    }
    CHECK(thrown);
    CHECK(!decoder.failed());
    CHECK(decoder.buffered_bytes() == 0);
    std::size_t calls = 0;
    const auto result = decoder.feed(view(bytes({0, 1, 8})),
        [&](std::span<const std::byte>) { ++calls; });
    CHECK(result.frames_emitted == 1);
    CHECK(calls == 1);
}

void deterministic_stress_with_fragmentation() {
    Bytes stream;
    std::vector<Bytes> expected;
    for (std::size_t i = 0; i < 512; ++i) {
        Bytes payload;
        const auto length = 1U + (i * 37U) % 64U;
        for (std::size_t j = 0; j < length; ++j) {
            payload.push_back(std::byte{
                static_cast<unsigned char>((i * 17U + j * 29U) & 0xFFU)});
        }
        const auto frame = stream_framing::encode_frame<64>(view(payload));
        CHECK(frame.has_value());
        stream.insert(stream.end(), frame->view().begin(), frame->view().end());
        expected.push_back(std::move(payload));
    }

    Decoder decoder;
    std::vector<Bytes> received;
    std::size_t offset = 0;
    std::size_t frames_count = 0;
    std::uint32_t seed = 0x1234ABCDU;
    while (offset < stream.size()) {
        seed = seed * 1664525U + 1013904223U;
        const auto chunk = std::min<std::size_t>(1U + seed % 23U,
                                                  stream.size() - offset);
        const auto result = decoder.feed(
            std::span<const std::byte>{stream.data() + offset, chunk},
            [&](const std::span<const std::byte> msg) {
                received.emplace_back(msg.begin(), msg.end());
            });
        CHECK(result.status == stream_framing::FeedStatus::ok);
        CHECK(result.bytes_consumed == chunk);
        frames_count += result.frames_emitted;
        offset += chunk;
    }
    CHECK(frames_count == expected.size());
    CHECK(received.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        CHECK(received[i] == expected[i]);
    }
    CHECK(!decoder.failed());
    CHECK(decoder.buffered_bytes() == 0);
}
} // namespace

int main() {
    try {
        encode_validation();
        one_byte_at_a_time();
        all_possible_two_chunk_splits();
        merged_frames_and_partial_tail();
        malformed_length_requires_explicit_reset();
        reject_after_delivering_earlier_frames();
        reset_discards_partial_frame();
        high_byte_of_length_is_big_endian();
        callback_failure_does_not_retain_old_frame();
        deterministic_stress_with_fragmentation();
        std::cout << "Passed " << checks << "/" << checks << " checks\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << " (" << checks << " checks attempted)\n";
        return 1;
    }
}
