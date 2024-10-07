# packet_handler
to use the packet handler you have to create a subproject called packet_types that defines a struct as follows:

`packet_type.hpp`
```cpp
enum class PacketType : uint8_t {
    UNIQUE_CLIENT_ID,
    GAME_UPDATE_POSITIONS,
    GAME_UPDATE_ONE_TIME_EVENTS,
    USER_MESSAGES,
};
```
when you run `sbpt` it will correctly include it.
