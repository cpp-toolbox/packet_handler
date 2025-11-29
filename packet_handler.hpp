#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include "sbpt_generated_includes.hpp"

#include <unordered_map>
#include <functional>

class PacketHandler {
  public:
    /**
     * Let us denote packets as (pi, ti) where pi means packet i, and ti means type i. Also note we assume that the
     * types are defined in some enum of the form :
     *
     * enum class PacketTypes { t1, t2, t3, t4, ... }
     *
     * For all further documentation lets suppose that we've received packets in the following order:
     *
     * (p1, t1), (p2, t1), (p3, t1), (p4, t2), (p5, t1), (p6, t3), (p7, t3), (p8, t3), (p9, t2)
     *
     * @note The original arrival order is maintained through this ordering.
     *
     */
    enum class PacketTypeHandlingMethod {
        /**
         * If we're talking about packet type t2 then the order in which packets of type t2 will be processed is:
         *
         * ..., (p4, t2), ...,  (p9, t2), ...
         */
        // ALL_PACKETS_IN_THE_ORDER_THEY_WERE_RECEIVED, (this is default behavior)

        /**
         *
         * @brief Only calls the packet handler on the last received packet of a certain type during handle_packet
         *
         * we're talking about packet type t1 then the order in which packets of type t1 will be ordered is
         *
         * ..., (p5, t1), ...
         *
         * @note that in the first batch of ellipses no packet of type t1 will be iterated over.
         * @note This only modifies the packets if there is at least two packets of this type
         *
         */
        LAST_RECEIVED_PACKET_ONLY,
    };

    std::unordered_map<PacketType, PacketTypeHandlingMethod> packet_type_to_handling_method = {};
    std::unordered_map<PacketType, std::function<std::string(std::vector<uint8_t>)>> packet_type_to_to_string = {};

    bool logging_enabled = false;

    // NOTE: the argument is the raw packet in a buffer
    using HandlerFunction = std::function<void(std::vector<uint8_t>)>;
    PacketHandler() {};
    PacketHandler(const std::unordered_map<PacketType, HandlerFunction> &handlers) : handlers_(handlers) {}

    Logger logger{"packet_handler"};

    /**
     * @brief takes in packets, and then calls the associated handler function
     * @pre assumes that packets have a particular form, namely they start with a packet header which states that type
     * of packet it is.
     */
    void handle_packets(const std::vector<PacketWithSize> &packets);
    void register_handler(const PacketType &packet_type, const HandlerFunction &handler);
    void register_handlers(const std::unordered_map<PacketType, HandlerFunction> &handlers);

    std::optional<PacketHeader> extract_packet_header_from_raw_packet(const void *packet_data, size_t packet_size);

  private:
    std::unordered_map<PacketType, HandlerFunction> handlers_;
    void handle_packet(const void *packet_data, size_t packet_size);
};

#endif // PACKET_HANDLER_HPP
