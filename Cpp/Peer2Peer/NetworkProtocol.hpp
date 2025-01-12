#pragma once
#include <SFML/Network.hpp>
#include "GameState.hpp"
#include <iostream>

/**
 * @brief Message types for network discovery and maintenance
 */
enum class DiscoveryMessageType : sf::Uint8
{
    // @brief Broadcast by host to announce its presence to potential clients.
    DISCOVERY = 255,
    // @brief Sent by clients to host to maintain connection and by unconnected clients for discovery.
    PING = 254
};

/**
 * @brief Message types for game state synchronization
 */
enum class GameMessageType : sf::Uint8
{
    // Host->Client: Request traffic calculation for a subset of cars.
    // Client->Host: Response with updated car positions.
    TRAFFIC_DISTRIBUTION = 0,
    // Host->Client: Request economic calculation for a subset of buildings.
    // Client->Host: Response with updated building incomes.
    ECONOMY_DISTRIBUTION,
    // Host->All Clients: Full game state broadcast.
    // Sent periodically to ensure all clients are synchronized
    STATE_SYNC,
    // Client->Host: Response to TRAFFIC_DISTRIBUTION with updated player states
    TRAFFIC_UPDATED,
    // Client->Host: Response to ECONOMY_DISTRIBUTION with updated player states
    ECONOMY_UPDATED,
};

/**
 * @brief Handles network protocol serialization and deserialization.
 */
class NetworkProtocol
{
public:
    /**
     * @brief Creates a discovery broadcast packet.
     * @details Sent by host every PING_INTERVAL to broadcast its presence.
     *          Contains host's port number for direct communication.
     * @param[in] port Host's listening port.
     * @return Packet ready to be broadcast.
     */
    static sf::Packet createDiscoveryPacket(unsigned short port);

    /**
     * @brief Creates a ping packet
     * @details Used in two scenarios:
     *          1. By connected clients to maintain connection with host
     *          2. By unconnected clients broadcasting to discovery port
     * @param[in] port Client's listening port.
     * @return Ping packet
     */
    static sf::Packet createPingPacket(unsigned short port);

    /**
     * @brief Creates an economy calculation request.
     * @details Host divides economic calculations among connected clients.
     *          Each client processes a subset of buildings.
     * @param[in] startIdx First building index to process.
     * @param[in] count Number of buildings to process.
     * @return Packet containing calculation request.
     */
    static sf::Packet createEconomyCalculationPacket(sf::Uint32 startIdx, sf::Uint32 count);

    /**
     * @brief Creates a traffic calculation request.
     * @details Host divides traffic simulation among connected clients.
     *          Each client processes movement for a subset of cars.
     * @param[in] startIdx First car index to process.
     * @param[in] count Number of cars to process.
     * @return Packet containing calculation request.
     */
    static sf::Packet createTrafficCalculationPacket(sf::Uint32 startIdx, sf::Uint32 count);

    /**
     * @brief Creates a full state synchronization packet.
     * @details Host periodically broadcasts complete game state to all clients.
     *          Ensures all clients maintain a consistent view of the game.
     *          Contains:
     *          - All car positions and speeds.
     *          - Current money amount.
     * @param[in] state Current game state to broadcast.
     * @return Packet containing complete game state.
     */
    static sf::Packet createStateSyncPacket(const GameState& state);

    /**
     * @brief Processes a state synchronization packet.
     * @param[in] packet Packet containing the game state.
     * @param[in] state Game state to update.
     */
    static void processStateSync(sf::Packet& packet, GameState& state);

    /**
     * @brief Processes a traffic update packet.
     * @param[in] packet Packet containing the traffic update.
     * @param[in] state Game state to update.
     */
    static void processTrafficUpdate(sf::Packet& packet, GameState& state);

    /**
     * @brief Processes an economy update packet.
     * @param[in] packet Packet containing the economy update.
     * @param[in] state Game state to update.
     */
    static void processEconomyUpdate(sf::Packet& packet, GameState& state);

    /**
     * @brief Processes a client state update packet.
     * @param[in] packet Packet containing the client state update.
     * @param[in] state Game state to update.
     */
    static void processClientStateUpdate(sf::Packet& packet, GameState& state);

    /**
     * @brief Creates a player state update packet.
     * @details Host divides player state updates among connected clients.
     *          Each client processes a subset of players.
     * @param[in] state Current game state containing player information.
     * @param[in] startIdx First player index to process.
     * @param[in] count Number of players to process.
     * @return Packet containing player state update request.
     */
    static sf::Packet createPlayerStatePacket(const GameState& state, sf::Uint32 startIdx, sf::Uint32 count);

    /**
     * @brief Processes a player state update packet.
     * @param[in] packet Packet containing the player state update.
     * @param[in] state Game state to update.
     */
    static void processPlayerStateUpdate(sf::Packet& packet, GameState& state);
};
