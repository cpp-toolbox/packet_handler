#include "packet_handler.hpp"
#include <iostream>

PacketHandler::PacketHandler(const std::unordered_map<PacketType, HandlerFunction> &handlers) : handlers_(handlers) {}

void PacketHandler::handle_packets(const std::vector<PacketWithSize> &packets) {
    for (const auto &packet : packets) {
        handle_packet(packet.data.data(), packet.size);
    }
}

void PacketHandler::handle_packet(const void *packet_data, size_t packet_size) {
    constexpr size_t SerializedPacketHeaderSize = sizeof(uint8_t) + sizeof(uint32_t);

    if (packet_size < SerializedPacketHeaderSize) {
        logger.error("packet didn't even have space for header");
        return;
    }

    // deserialize safely
    PacketHeader header;
    std::memcpy(&header.type, packet_data, sizeof(uint8_t));
    std::memcpy(&header.size_of_data_without_header, static_cast<const uint8_t *>(packet_data) + sizeof(uint8_t),
                sizeof(uint32_t));

    if (packet_size < SerializedPacketHeaderSize + header.size_of_data_without_header) {
        logger.error("had space for header but not enough for data");
        return;
    }

    // wrap raw data into vector<uint8_t>
    const uint8_t *raw_bytes = reinterpret_cast<const uint8_t *>(packet_data);
    std::vector<uint8_t> buffer(raw_bytes, raw_bytes + packet_size);

    auto it = handlers_.find(header.type);
    if (it != handlers_.end()) {
        it->second(buffer);
    } else {
        logger.error("Unknown packet type received: {}", static_cast<int>(header.type));
    }
}

void PacketHandler::register_handler(const PacketType &packet_type, const HandlerFunction &handler) {
    handlers_[packet_type] = handler;
}

void PacketHandler::register_handlers(const std::unordered_map<PacketType, HandlerFunction> &handlers) {
    for (const auto &[packet_type, handler] : handlers) {
        register_handler(packet_type, handler);
    }
}
