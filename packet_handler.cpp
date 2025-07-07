#include "packet_handler.hpp"
#include <iostream>

PacketHandler::PacketHandler(const std::unordered_map<PacketType, HandlerFunction> &handlers) : handlers_(handlers) {}

void PacketHandler::handle_packets(const std::vector<PacketWithSize> &packets) {
    for (const auto &packet : packets) {
        handle_packet(packet.data.data(), packet.size);
    }
}

void PacketHandler::handle_packet(const void *packet_data, size_t packet_size) {
    if (packet_size < sizeof(PacketHeader)) {
        logger.error("packet didn't even have space for header");
        return;
    }

    const PacketHeader *header = reinterpret_cast<const PacketHeader *>(packet_data);

    if (packet_size < sizeof(PacketHeader) + header->size_of_data_without_header) {
        logger.error("had space for header but not enough for data");
        return;
    }

    auto it = handlers_.find(header->type);
    if (it != handlers_.end()) {
        it->second(packet_data);
    } else {
        logger.error("Unknown packet type received: {}", static_cast<int>(header->type));
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
