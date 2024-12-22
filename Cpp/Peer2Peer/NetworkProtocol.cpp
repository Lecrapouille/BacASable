#include "NetworkProtocol.hpp"
#include "Serialization.hpp"

#include <iostream>

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createDiscoveryPacket(unsigned short port)
{
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(DiscoveryMessageType::DISCOVERY);
    packet << port;
    return packet;
}

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createPingPacket(unsigned short port)
{
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(DiscoveryMessageType::PING);
    packet << port;
    return packet;
}

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createEconomyCalculationPacket(sf::Uint32 startIdx, sf::Uint32 count)
{
    std::cout << "Creating economy calculation packet startIdx: " << startIdx << " count: " << count << std::endl;
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(GameMessageType::ECONOMY_UPDATE);
    packet << startIdx;
    packet << count;
    return packet;
}

// ----------------------------------------------------------------------------
sf::Packet NetworkProtocol::createTrafficCalculationPacket(sf::Uint32 zoneStartIdx, sf::Uint32 zoneCount)
{
    std::cout << "Creating traffic calculation packet startIdx: " << zoneStartIdx << " count: " << zoneCount << std::endl;
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
    packet << state.traffic << state.economy;
    return packet;
}

// ----------------------------------------------------------------------------
void NetworkProtocol::processStateSync(sf::Packet& packet, GameState& state)
{
    packet >> state.traffic >> state.economy;
}

// ----------------------------------------------------------------------------
void NetworkProtocol::processTrafficUpdate(sf::Packet& packet, GameState& state)
{
    sf::Uint32 startIdx, count;
    packet >> startIdx >> count;

    std::cout << "Processing traffic update packet startIdx: " << startIdx << " count: " << count << std::endl;

    // Verify bounds
    if (startIdx + count > state.traffic.cars.size())
    {
        std::cerr << "Warning: Traffic update packet contains invalid range" << std::endl;
        return;
    }

    // Update car positions
    for (sf::Uint32 i = 0; i < count; ++i)
    {
        packet >> state.traffic.cars[startIdx + i];
        packet >> state.traffic.roads[startIdx + i];
    }
}

// ----------------------------------------------------------------------------
void NetworkProtocol::processEconomyUpdate(sf::Packet& packet, GameState& state)
{
    sf::Uint32 startIdx, count;
    packet >> startIdx >> count;

    std::cout << "Processing economy update packet startIdx: " << startIdx << " count: " << count << std::endl;

    // Verify bounds
    if (startIdx + count > state.economy.buildings.size())
    {
        std::cerr << "Warning: Economy update packet contains invalid range" << std::endl;
        return;
    }

    // Update building incomes
    for (sf::Uint32 i = 0; i < count; ++i)
    {
        packet >> state.economy.buildings[startIdx + i];
    }

    // Update total money
    packet >> state.economy.money >> state.economy.tax_rate;
}

// ----------------------------------------------------------------------------
void NetworkProtocol::processClientStateUpdate(sf::Packet& packet, GameState& state)
{
    // Extract client state from packet
    GameState clientState;
    packet >> clientState.traffic >> clientState.economy;

    // Merge client state with global state
    // Note: Here we could add a validation or conflict resolution logic
    state.traffic = clientState.traffic;
    state.economy = clientState.economy;
}