#pragma once
#include <SFML/Network.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <queue>

#include "GameManager.hpp"
#include "GameState.hpp"

/**
 * @brief Handles peer-to-peer networking and game state synchronization
 */
class NetworkNode
{
public:

    static constexpr unsigned short DISCOVERY_PORT = 45678;
    static constexpr float PING_INTERVAL = 1.0f;
    static constexpr float PEER_TIMEOUT = 5.0f;

    /**
     * @brief Constructor for NetworkNode
     * @param port [in] Port number to bind to
     * @param hosting [in] Whether this node is a host (default: false)
     */
    NetworkNode(unsigned short port, bool hosting = false);

    /**
     * @brief Updates the network node
     * @param deltaTime [in] Time elapsed since last update in seconds
     * @param state [inout] Game state to update
     */
    void update(float deltaTime, GameState& state);

    /**
     * @brief Checks if there are available nodes for P2P communication
     * @return true if there are available nodes, false otherwise
     */
    bool hasAvailableNodes() const;

    /**
     * @brief Gets the count of active peers
     * @return Number of active peers
     */
    size_t getActivePeerCount() const;

    /**
     * @brief Gets the port number this node is bound to
     * @return Port number
     */
    unsigned short getPort() const;

    /**
     * @brief Gets a string describing the current network status
     * @return Status string
     */
    std::string getNetworkStatusString() const;

private:

    /**
     * @brief Updates peer discovery and broadcasts presence
     * @param deltaTime [in] Time elapsed since last update
     */
    void updatePeerDiscovery(float deltaTime);

    /**
     * @brief Adds a new peer to the network
     * @param id [in] Unique identifier for the peer
     * @param address [in] IP address of the peer
     * @param port [in] Port number of the peer
     */
    void addPeer(const std::string& id,
                 const sf::IpAddress& address,
                 unsigned short port);

    /**
     * @brief Checks for peer timeouts and marks inactive peers
     * @param deltaTime [in] Time elapsed since last update
     */
    void checkPeerTimeouts(float deltaTime);

    /**
     * @brief Handles network errors and logs them
     * @param operation [in] Description of the operation that failed
     * @param status [in] SFML Socket status code
     */
    void handleNetworkError(const std::string& operation,
                            sf::Socket::Status status);

    /**
     * @brief Receives and queues incoming network messages
     */
    void receiveMessages();

    /**
     * @brief Processes queued network messages
     * @param state [in] Current game state to process
     */
    void processMessages(GameState& state);

    /**
     * @brief Distributes economy calculations among peers
     * @param state [in] Current game state to process
     */
    void distributeEconomyCalculations(const GameState& state);

    /**
     * @brief Synchronizes game state with all peers
     * @param state [in] Current game state to synchronize
     */
    void synchronizeState(const GameState& state);

    /**
     * @brief Distributes traffic simulation load among peers
     * @param state [in] Current game state to process
     */
    void distributeTrafficLoad(const GameState& state);

    /**
     * @brief Process a traffic update message
     * @param[inout] state Game state to update
     * @param[in] packet Network packet containing the update data
     */
    void processTrafficUpdate(GameState& state, sf::Packet& packet);

    /**
     * @brief Process an economy update message
     * @param[inout] state Game state to update
     * @param[in] packet Network packet containing the update data
     */
    void processEconomyUpdate(GameState& state, sf::Packet& packet);

    /**
     * @brief Process a full state synchronization message
     * @param[inout] state Game state to update
     * @param[in] packet Network packet containing the state data
     */
    void processStateSync(GameState& state, sf::Packet& packet);

    /**
     * @brief Removes all inactive peers from the peer list
     */
    void removeInactivePeers();

private:

    //! @brief Information about a peer in the network
    struct PeerInfo
    {
        sf::IpAddress address; //!< IP address of the peer
        unsigned short port;   //!< Port number of the peer
        bool is_active;        //!< Whether the peer is currently active
        float last_ping;       //!< Time since last ping received
    };

    //! @brief Main communication socket
    sf::UdpSocket m_socket;                 
    //! @brief Discovery broadcast socket
    sf::UdpSocket m_discovery_socket;        
    //! @brief Connected peers
    std::map<std::string, PeerInfo> m_peers; 
    //! @brief Incoming message queue
    std::queue<sf::Packet> m_message_queue;  
    //! @brief Whether this is a host node
    bool m_is_host;                           
    //! @brief Time since last ping
    float m_last_ping_sent = 0.0f;           
};
