#pragma once
#include <SFML/Network.hpp>
#include "GameState.hpp"

/**
 * @brief Message types for network discovery and maintenance
 */
enum class DiscoveryMessageType : sf::Uint8 
{
    DISCOVERY = 255,  ///< Message for peer discovery
    PING = 254        ///< Message for connection keepalive
};

/**
 * @brief Message types for game state synchronization
 */
enum class GameMessageType : sf::Uint8 
{
    TRAFFIC_UPDATE = 0,   ///< Updates to traffic simulation
    ECONOMY_UPDATE = 1,   ///< Updates to economic simulation
    STATE_SYNC = 2        ///< Full game state synchronization
};


/**
 * @brief Handles network protocol serialization and deserialization
 */
class NetworkProtocol 
{
public:
    /**
     * @brief Creates a discovery packet sent by the host
     * @param[in] port Host's listening port
     * @return Packet ready to be sent
     */
    static sf::Packet createDiscoveryPacket(unsigned short port);
    
    /**
     * @brief Creates a ping packet sent by clients
     * @return Packet ready to be sent
     */
    static sf::Packet createPingPacket();
    
    /**
     * @brief Creates an economy calculation request packet
     * @param[in] startIdx Starting building index
     * @param[in] count Number of buildings to process
     * @return Packet ready to be sent
     */
    static sf::Packet createEconomyCalculationPacket(sf::Uint32 startIdx, sf::Uint32 count);
    
    /**
     * @brief Creates a state synchronization packet
     * @param[in] state Current game state to synchronize
     * @return Packet ready to be sent
     */
    static sf::Packet createStateSyncPacket(const GameState& state);
    
    /**
     * @brief Creates a packet for traffic calculation request
     * @param[in] zoneStartIdx Starting index of the traffic zone range
     * @param[in] zoneCount Number of zones to calculate
     * @return Packet containing the traffic calculation request
     */
    static sf::Packet createTrafficCalculationPacket(sf::Uint32 zoneStartIdx, sf::Uint32 zoneCount);
}; 