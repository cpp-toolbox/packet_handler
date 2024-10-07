#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include "../client_networking/network.hpp"
#include "../packet_types.hpp"

#include <unordered_map>
#include <functional>

struct PacketHeader {
    PacketType type; 
    uint32_t size_of_data_without_header;
};

class PacketHandler {
public:
    using HandlerFunction = std::function<void(const void*)>;
    PacketHandler(const std::unordered_map<PacketType, HandlerFunction>& handlers);
    void handle_packets(const std::vector<PacketWithSize>& packets);
private:
    std::unordered_map<PacketType, HandlerFunction> handlers_;
    void handle_packet(const void* packet_data, size_t packet_size);
};

#endif // PACKET_HANDLER_HPP
