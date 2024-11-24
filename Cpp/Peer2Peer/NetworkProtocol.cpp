#include "NetworkProtocol.hpp"

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createDiscoveryPacket(unsigned short port)
{
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(DiscoveryMessageType::DISCOVERY);
    packet << port;
    return packet;
}

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createPingPacket()
{
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(DiscoveryMessageType::PING);
    return packet;
}

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createEconomyCalculationPacket(sf::Uint32 startIdx, sf::Uint32 count)
{
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(GameMessageType::ECONOMY_UPDATE);
    packet << startIdx;
    packet << count;
    return packet;
}

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createTrafficCalculationPacket(sf::Uint32 zoneStartIdx, sf::Uint32 zoneCount)
{
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(GameMessageType::TRAFFIC_UPDATE);
    packet << zoneStartIdx;
    packet << zoneCount;
    return packet;
}

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createStateSyncPacket(const GameState& state)
{
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(GameMessageType::STATE_SYNC);

    // Serialize car positions
    const size_t car_count = state.traffic.car_positions.size();
    packet << sf::Uint32(car_count);

    for (size_t i = 0; i < car_count; ++i)
    {
        const auto& pos = state.traffic.car_positions[i];
        packet << pos.x << pos.y;
    }

    packet << state.economy.money;
    return packet;
}