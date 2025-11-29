#include "packet_handler.hpp"
#include <iostream>

void PacketHandler::handle_packets(const std::vector<PacketWithSize> &packets) {

    bool no_custom_handling_methods = packet_type_to_handling_method.empty();
    if (no_custom_handling_methods) {
        // fallback: process all packets normally.
        for (const auto &packet : packets) {
            handle_packet(packet.data.data(), packet.size);
        }
        return;
    }

    // otherwise there is at least one custom packet handling method for a packet type

    using RefPacket = std::reference_wrapper<const PacketWithSize>;
    std::vector<std::pair<PacketType, RefPacket>> packet_with_type_in_arrival_order;
    packet_with_type_in_arrival_order.reserve(packets.size());

    // NOTE: this also filters out any "bad" packets, but you should never be getting bad packets with enet.
    for (const auto &p : packets) {
        auto packet_header = extract_packet_header_from_raw_packet(p.data.data(), p.size);
        if (packet_header.has_value()) {
            packet_with_type_in_arrival_order.emplace_back(packet_header->type, std::cref(p));
        }
    }

    for (const auto &[type, method] : packet_type_to_handling_method) {
        if (method == PacketTypeHandlingMethod::LAST_RECEIVED_PACKET_ONLY) {

            // find last occurrence
            const PacketWithSize *last_packet = nullptr;
            for (const auto &entry : packet_with_type_in_arrival_order) {
                if (entry.first == type) {
                    last_packet = &entry.second.get();
                }
            }

            // remove everything except the last one
            if (last_packet) {
                packet_with_type_in_arrival_order.erase(
                    std::remove_if(
                        packet_with_type_in_arrival_order.begin(), packet_with_type_in_arrival_order.end(),
                        [&](const auto &entry) { return entry.first == type && &entry.second.get() != last_packet; }),
                    packet_with_type_in_arrival_order.end());
            }
        }
    }

    // processing occurs in arrival order with packets filtered out from above.
    for (const auto &[type, packet_ref] : packet_with_type_in_arrival_order) {
        const PacketWithSize &packet = packet_ref.get();
        handle_packet(packet.data.data(), packet.size);
    }
}

std::optional<PacketHeader> PacketHandler::extract_packet_header_from_raw_packet(const void *packet_data,
                                                                                 size_t packet_size) {

    // NOTE: we acutally used mp to serialize but it just happens to match this.
    constexpr size_t serialized_packet_header_size = sizeof(uint8_t) + sizeof(uint32_t);

    if (packet_size < serialized_packet_header_size) {
        global_logger->error("packet didn't even have space for header");
        return std::nullopt;
    }

    // deserialize safely
    PacketHeader header;
    std::memcpy(&header.type, packet_data, sizeof(uint8_t));
    std::memcpy(&header.size_of_data_without_header, static_cast<const uint8_t *>(packet_data) + sizeof(uint8_t),
                sizeof(uint32_t));

    return header;
}

void PacketHandler::handle_packet(const void *packet_data, size_t packet_size) {
    LogSection _(*global_logger, "handle_packet");
    // NOTE: we acutally used mp to serialize but it just happens to match this.
    constexpr size_t serialized_packet_header_size = sizeof(uint8_t) + sizeof(uint32_t);

    if (packet_size < serialized_packet_header_size) {
        global_logger->error("packet didn't even have space for header");
        return;
    }

    // NOTE: if ever the values obtained here are incorrect be sure that you PacketType enum inherits from uint8_t

    // deserialize safely
    PacketHeader header;
    std::memcpy(&header.type, packet_data, sizeof(uint8_t));
    std::memcpy(&header.size_of_data_without_header, static_cast<const uint8_t *>(packet_data) + sizeof(uint8_t),
                sizeof(uint32_t));

    if (packet_size < serialized_packet_header_size + header.size_of_data_without_header) {
        global_logger->error("Had space for header but not enough for data.\n"
                             "  packet_size = {}\n"
                             "  SerializedPacketHeaderSize = {}\n"
                             "  header.size_of_data_without_header = {}\n"
                             "  required_size = {}",
                             packet_size, serialized_packet_header_size, header.size_of_data_without_header,
                             serialized_packet_header_size + header.size_of_data_without_header);
        return;
    }

    // wrap raw data into vector<uint8_t>
    const uint8_t *raw_bytes = reinterpret_cast<const uint8_t *>(packet_data);
    std::vector<uint8_t> buffer(raw_bytes, raw_bytes + packet_size);

    auto it = handlers_.find(header.type);
    if (it != handlers_.end()) {
        it->second(buffer);
    } else {
        global_logger->error("Unknown packet type received: {}", static_cast<int>(header.type));
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
