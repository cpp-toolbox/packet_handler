#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include "sbpt_generated_includes.hpp"

#include <unordered_map>
#include <functional>

class PacketHandler {
  public:
    using HandlerFunction = std::function<void(const void *)>;
    PacketHandler() {};
    PacketHandler(const std::unordered_map<PacketType, HandlerFunction> &handlers);

    ConsoleLogger logger{"packet_handler"};

    void handle_packets(const std::vector<PacketWithSize> &packets);
    void register_handler(const PacketType &packet_type, const HandlerFunction &handler);
    void register_handlers(const std::unordered_map<PacketType, HandlerFunction> &handlers);

  private:
    std::unordered_map<PacketType, HandlerFunction> handlers_;
    void handle_packet(const void *packet_data, size_t packet_size);
};

#endif // PACKET_HANDLER_HPP
