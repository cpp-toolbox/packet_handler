#include "packet_handler.hpp"
#include <iostream>

PacketHandler::PacketHandler(const std::unordered_map<PacketType, HandlerFunction>& handlers)
    : handlers_(handlers) {}

void PacketHandler::handle_packets(const std::vector<PacketWithSize>& packets) {
    for (const auto& packet : packets) {
        handle_packet(packet.data.data(), packet.size);
    }
}

void PacketHandler::handle_packet(const void* packet_data, size_t packet_size) {
    if (packet_size < sizeof(PacketHeader)) {
        return;
    }

    const PacketHeader* header = reinterpret_cast<const PacketHeader*>(packet_data);

    if (packet_size < sizeof(PacketHeader) + header->size_of_data_without_header) {
        return;
    }

    auto it = handlers_.find(header->type);
    if (it != handlers_.end()) {
        it->second(packet_data);
    } else {
        std::cout << "Unknown packet type received: " << static_cast<int>(header->type) << "\n";
    }
}
