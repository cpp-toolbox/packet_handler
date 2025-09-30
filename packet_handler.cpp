#include "packet_handler.hpp"
#include <iostream>

PacketHandler::PacketHandler(const std::unordered_map<PacketType, HandlerFunction> &handlers) : handlers_(handlers) {}

void PacketHandler::handle_packets(const std::vector<PacketWithSize> &packets) {
    for (const auto &packet : packets) {
        handle_packet(packet.data.data(), packet.size);
    }
}

void PacketHandler::handle_packet(const void *packet_data, size_t packet_size) {
    LogSection _(global_logger, "handle_packet");
    constexpr size_t SerializedPacketHeaderSize = sizeof(uint8_t) + sizeof(uint32_t);

    if (packet_size < SerializedPacketHeaderSize) {
        global_logger.error("packet didn't even have space for header");
        return;
    }

    // deserialize safely
    PacketHeader header;
    std::memcpy(&header.type, packet_data, sizeof(uint8_t));
    std::memcpy(&header.size_of_data_without_header, static_cast<const uint8_t *>(packet_data) + sizeof(uint8_t),
                sizeof(uint32_t));

    if (packet_size < SerializedPacketHeaderSize + header.size_of_data_without_header) {
        global_logger.error("Had space for header but not enough for data.\n"
                            "  packet_size = {}\n"
                            "  SerializedPacketHeaderSize = {}\n"
                            "  header.size_of_data_without_header = {}\n"
                            "  required_size = {}",
                            packet_size, SerializedPacketHeaderSize, header.size_of_data_without_header,
                            SerializedPacketHeaderSize + header.size_of_data_without_header);
        return;
    }

    // wrap raw data into vector<uint8_t>
    const uint8_t *raw_bytes = reinterpret_cast<const uint8_t *>(packet_data);
    std::vector<uint8_t> buffer(raw_bytes, raw_bytes + packet_size);

    auto it = handlers_.find(header.type);
    if (it != handlers_.end()) {
        it->second(buffer);
    } else {
        global_logger.error("Unknown packet type received: {}", static_cast<int>(header.type));
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
