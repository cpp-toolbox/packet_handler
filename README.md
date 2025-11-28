# packet_handler
In the following example we will show how to use the packet handler in the context of a client server game. First you have to create a subproject called packet_types that defines a struct as follows:

`packet_type.hpp` (client and server):
```cpp
enum class PacketType : uint8_t {
    UNIQUE_CLIENT_ID,
    GAME_UPDATE_POSITIONS,
    GAME_UPDATE_ONE_TIME_EVENTS,
    USER_MESSAGES,
};
```
**NOTE: its very important that you inherit from unint8_t or else the logic that deserialized the header will go wrong**
when you run `sbpt` it will correctly include it. Additionally both the client and server must have the same packet types available.

Next when constructing a `PacketHandler` you must create a map like this (client):
```cpp
std::unordered_map<PacketType, PacketHandler::HandlerFunction> initialize_packet_handlers() {
    std::unordered_map<PacketType, PacketHandler::HandlerFunction> handlers;

    handlers[PacketType::UNIQUE_CLIENT_ID] = [](const void* data) {
        const UniqueClientIDPacket* packet = reinterpret_cast<const UniqueClientIDPacket*>(data);
        handle_unique_client_id_packet(*packet);
    };

    handlers[PacketType::GAME_UPDATE_POSITIONS] = [](const void* data) {
        const PacketHeader* header = reinterpret_cast<const PacketHeader*>(data);
        size_t num_positions = header->size_of_data_without_header / sizeof(PositionArray);

        GameUpdatePositionsPacket packet;
        packet.header = *header;
        packet.positions.resize(num_positions);
        std::memcpy(packet.positions.data(), reinterpret_cast<const char*>(data) + sizeof(PacketHeader), num_positions * sizeof(PositionArray));

        handle_game_update_positions_packet(packet);
    };

    return handlers;
}

```

and pass it in when you construct the packet handler, this will allow it to automatically run the correct callback function whenever a packet of that type is received, to make this happen you usually want to do something like this: 
```cpp
        std::cout << "tick" << "\n";
        std::vector<PacketWithSize> packets_since_last_tick = network.get_network_events_received_since_last_tick();
        for (const auto& packet : packets_since_last_tick) {
            std::cout << "Received packet of size: " << packet.size << "\n";
        }
        packet_handler.handle_packets(packets_since_last_tick);
```

To send over packets you should do something similar to this: 
```cpp
        server.get_network_events_since_last_tick();

        GameUpdatePositionsPacket packet = toggle ? create_mock_game_update_positions_packet_2() : create_mock_game_update_positions_packet_1();

        size_t packet_size = sizeof(PacketHeader) + packet.positions.size() * sizeof(PositionArray);
        std::vector<char> buffer(packet_size);
        std::memcpy(buffer.data(), &packet.header, sizeof(PacketHeader));
        std::memcpy(buffer.data() + sizeof(PacketHeader), packet.positions.data(), packet.positions.size() * sizeof(PositionArray));

        server.reliable_broadcast(buffer.data(), buffer.size());
```

## creating packets
Note that in the above code we assumed that at the start of the packet, there was a packet header this is an assumption that is made by the packet_handler system and you must do this to properly integrate with it, therefore when creating packets, they must have the following form: 
```cpp
class XPacket {
public:
    PacketHeader header;
    ...
}
```

