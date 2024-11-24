#include "NetworkNode.hpp"
#include "NetworkProtocol.hpp"
#include "Serialization.hpp"
#include <SFML/Graphics/Color.hpp>
#include <cstdlib>

// ----------------------------------------------------------------------------
NetworkNode::NetworkNode(unsigned short port, bool hosting) : m_is_host(hosting)
{
    std::cout << (hosting ? "Starting host on port "
                          : "Starting client on port ")
              << port << " and discovery port " << DISCOVERY_PORT << std::endl;

    // Game communication socket.
    if (m_socket.bind(port) != sf::Socket::Done)
    {
        throw std::runtime_error("Failed to bind socket to port " +
                                 std::to_string(port));
    }
    m_socket.setBlocking(false);

    // Discovery socket for finding peers. Client starts with a random port.
    unsigned short discovery_port = (m_is_host)
        ? DISCOVERY_PORT : static_cast<unsigned short>(sf::Socket::AnyPort);
    if (m_discovery_socket.bind(discovery_port) != sf::Socket::Done)
    {
        throw std::runtime_error("Failed to bind socket to discovery port");
    }
    m_discovery_socket.setBlocking(false);
}

// ----------------------------------------------------------------------------
void NetworkNode::update(float deltaTime, GameState& state)
{
    // Discovery channel
    updatePeerDiscovery(deltaTime);
    receiveDiscoveryPackets();
    checkPeerTimeouts(deltaTime);

    // Distribute workload (host only)
    if (m_is_host && m_peers_updated)
    {
        if (hasAvailableNodes())
        {
            distributeTrafficLoad(state);
            distributeEconomyCalculations(state);
        }
        m_peers_updated = false;
    }

    // Game channel
    receiveGamePackets(state);

    // Client->Host: Clients send their game states to the host
    if (!m_is_host)
    {
        updateClientState(deltaTime, state);
    }

    // Host->Clients: synchronize client game states
    if (m_is_host && hasAvailableNodes())
    {
        synchronizeState(state);
    }
}

// ----------------------------------------------------------------------------
// FIXME les voitures ne bougent pas. Le host ne reçoit pas les états des clients.
// Et de plus le startidx+count n'est pas envoyé (l'host pourrait le stocker dans PeerInfo)
// Les clients doivent envoyer leurs couleurs.
void NetworkNode::updateClientState(float deltaTime, GameState& state)
{
    // Update local state
    GameManager::update(state, deltaTime);

    // Send updated state to the host
    for (const auto& [name, peer] : m_peers)
    {
        if (peer.is_active && name == "host")
        {
            sf::Packet packet = NetworkProtocol::createStateSyncPacket(state);
            if (m_socket.send(packet, peer.address, peer.port) != sf::Socket::Done)
            {
                std::cerr << "Failed to send state update to host" << std::endl;
            }
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::updatePeerDiscovery(float deltaTime)
{
    m_last_ping_sent += deltaTime;

    if (m_last_ping_sent >= PING_INTERVAL)
    {
        m_last_ping_sent = 0.0f;

        if (m_is_host)
        {
            // Host broadcasts its presence.
            sf::Packet packet = NetworkProtocol::createDiscoveryPacket(m_socket.getLocalPort());
            m_discovery_socket.send(packet, sf::IpAddress::Broadcast, DISCOVERY_PORT);
        }
        else if (m_peers.empty()) // Client without connection
        {
            // The client sends a ping to the discovery port of the host for its subscription.
            sf::Packet packet = NetworkProtocol::createPingPacket(m_socket.getLocalPort());
            m_discovery_socket.send(packet, sf::IpAddress::LocalHost, DISCOVERY_PORT);
        }
        else // Client connected
        {
            // Send regular pings to the host to keep it aware of the client's presence,
            // else the host will timeout the client.
            sf::Packet packet = NetworkProtocol::createPingPacket(m_socket.getLocalPort()   );
            for (auto& [name, peer] : m_peers)
            {
                if (m_socket.send(packet, peer.address, peer.port) != sf::Socket::Done)
                {
                    std::cerr << "Failed to send ping to " << name << std::endl;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::receiveDiscoveryPackets()
{
    sf::Packet discoveryPacket;
    sf::IpAddress senderAddress;
    unsigned short senderPort;
    sf::Uint8 messageType;

    while (true)
    {
        sf::Socket::Status status = m_discovery_socket.receive(
               discoveryPacket, senderAddress, senderPort);

        if (status == sf::Socket::Done)
        {
            // Ignore packets from ourselves
            if ((senderAddress == sf::IpAddress::LocalHost ||
                senderAddress == sf::IpAddress::getLocalAddress()) &&
                (m_is_host && senderPort == DISCOVERY_PORT))
            {
                continue;
            }

            // Process the discovery packet
            discoveryPacket >> messageType;
            switch (DiscoveryMessageType(messageType))
            {
                case DiscoveryMessageType::DISCOVERY:
                    if (!m_is_host)
                    {
                        unsigned short hostPort;
                        discoveryPacket >> hostPort;
                        std::cout << "Client discovered host at port " << hostPort << std::endl;
                        addPeer("host", senderAddress, hostPort);
                    }
                    break;
                case DiscoveryMessageType::PING:
                    if (m_is_host)
                    {
                        // Remember that clientPort != senderPort since the client is using a
                        // temporary port on the discovery socket
                        unsigned short clientPort;
                        discoveryPacket >> clientPort;
                        std::cout << "Host received ping from client " << senderAddress
                                << " port: " << clientPort << std::endl;

                        std::string clientId = "client_" + std::to_string(clientPort);
                        addPeer(clientId, senderAddress, clientPort);
                    }
                    break;
            }

        }
        else if (status == sf::Socket::NotReady)
        {
            break; // No more messages to receive
        }
        else
        {
            handleNetworkError("receive", status);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::checkPeerTimeouts(float deltaTime)
{
    bool found_inactive_peers = false;

    for (auto& [name, peer] : m_peers)
    {
        if (peer.is_active)
        {
            peer.last_ping += deltaTime;
            if (peer.last_ping > PEER_TIMEOUT)
            {
                std::cout << "Peer " << name << " timed out" << std::endl;
                peer.is_active = false;
                found_inactive_peers = true;
            }
        }
    }

    if (found_inactive_peers)
    {
        removeInactivePeers();
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::receiveGamePackets(GameState& state)
{
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short senderPort;
    sf::Uint8 messageType;

    while (true)
    {
        sf::Socket::Status status =
            m_socket.receive(packet, sender, senderPort);

        if (status == sf::Socket::Done)
        {
            // Update last ping time for the sending peer
            for (auto& [name, peer] : m_peers)
            {
                if ((peer.address == sender) && (peer.port == senderPort))
                {
                    peer.last_ping = 0.0f;
                    peer.is_active = true;
                    break;
                }
            }

            // Process the game message
            packet >> messageType;
            switch (GameMessageType(messageType))
            {
                case GameMessageType::TRAFFIC_UPDATE:
                    NetworkProtocol::processTrafficUpdate(packet, state);
                    break;
                case GameMessageType::ECONOMY_UPDATE:
                    NetworkProtocol::processEconomyUpdate(packet, state);
                    break;
                case GameMessageType::STATE_SYNC:
                    NetworkProtocol::processStateSync(packet, state);
                    break;
            }
        }
        else if (status == sf::Socket::NotReady)
        {
            break; // No more messages to receive
        }
        else
        {
            handleNetworkError("receive", status);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
// TODO: to be cached + inactive peers have already been removed
bool NetworkNode::hasAvailableNodes() const
{
    if (m_is_host)
    {
        // For the host, check if there are active clients
        return getActivePeerCount() > 0;
    }
    else
    {
        // For a client, check if we are connected to a host
        for (const auto& [name, peer] : m_peers)
        {
            if ((peer.is_active) && (name == "host"))
            {
                return true;
            }
        }
        return false;
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::distributeTrafficLoad(const GameState& state)
{
    // Check active peer count
    size_t active_peer_count = getActivePeerCount();
    if (active_peer_count == 0)
    {
        std::cerr << "Warning: No active peers to distribute traffic calculations" << std::endl;
        return;
    }

    // Verify car positions
    if (state.traffic.cars.empty())
    {
        std::cerr << "Warning: No cars to process" << std::endl;
        return;
    }

    // Calculate cars per peer and distribute workload
    size_t cars_per_peer = std::max(
        size_t(1), state.traffic.cars.size() / active_peer_count);

    size_t start_idx = 0;
    for (auto const& [name, peer_info] : m_peers)
    {
        if (peer_info.is_active)
        {
            // Check bounds
            size_t end_idx = std::min(start_idx + cars_per_peer,
                                    state.traffic.cars.size());

            if (start_idx >= end_idx)
            {
                break;
            }

            // Send calculation request
            sf::Packet packet = NetworkProtocol::createTrafficCalculationPacket(
                static_cast<sf::Uint32>(start_idx),
                static_cast<sf::Uint32>(end_idx - start_idx));
            m_socket.send(packet, peer_info.address, peer_info.port);
            start_idx = end_idx;
        }
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::distributeEconomyCalculations(const GameState& state)
{
    // Check active peer count
    size_t active_peer_count = 0;
    for (const auto& [name, peer_info] : m_peers)
    {
        if (peer_info.is_active)
        {
            active_peer_count++;
        }
    }

    if (active_peer_count == 0)
    {
        std::cerr << "Warning: No active peers to distribute calculations"
                  << std::endl;
        return;
    }

    // Verify vector size
    if (state.economy.buildings.empty())
    {
        std::cerr << "Warning: No buildings to process" << std::endl;
        return;
    }

    // Calculate buildings per peer and distribute workload
    size_t buildings_per_peer = std::max(
        size_t(1), state.economy.buildings.size() / active_peer_count);

    size_t start_idx = 0;
    for (auto const& [name, peer_info] : m_peers)
    {
        if (peer_info.is_active)
        {
            // Check bounds
            size_t end_idx = std::min(start_idx + buildings_per_peer,
                                      state.economy.buildings.size());

            if (start_idx >= end_idx)
            {
                break;
            }

            // Send calculation request
            sf::Packet packet = NetworkProtocol::createEconomyCalculationPacket(start_idx, end_idx - start_idx);
            m_socket.send(packet, peer_info.address, peer_info.port);
            start_idx = end_idx;
        }
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::synchronizeState(const GameState& state)
{
    // Validate game state data
    if (!GameManager::validateState(state))
    {
        std::cerr << "Warning: Invalid game state detected" << std::endl;
        return;
    }

    // Prepare synchronization packet
    sf::Packet packet = NetworkProtocol::createStateSyncPacket(state);

    // Broadcast to active peers
    for (auto const& [name, peer_info] : m_peers)
    {
        if (peer_info.is_active)
        {
            if (m_socket.send(packet, peer_info.address, peer_info.port) !=
                sf::Socket::Done)
            {
                std::cerr << "Warning: Failed to send state to peer " << name
                          << std::endl;
            }
        }
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::addPeer(const std::string& id,
                          const sf::IpAddress& address,
                          unsigned short port)
{
    std::cout << "addPeer " << id << ", address " << address << ", port "
              << port << std::endl;
    m_peers[id] = {address, port, true, 0.0f};
    m_peers_updated = true;
}

// ----------------------------------------------------------------------------
void NetworkNode::removeInactivePeers()
{
    auto it = m_peers.begin();
    while (it != m_peers.end())
    {
        if (!it->second.is_active)
        {
            it = m_peers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// ----------------------------------------------------------------------------
size_t NetworkNode::getActivePeerCount() const
{
    size_t count = 0;
    for (const auto& [name, peer] : m_peers)
    {
        if (peer.is_active)
            count++;
    }
    return count;
}

// ----------------------------------------------------------------------------
unsigned short NetworkNode::getPort() const
{
    return m_socket.getLocalPort();
}

// ----------------------------------------------------------------------------
std::string NetworkNode::getNetworkStatusString() const
{
    if (m_is_host)
      return "Hosting";

    if (getActivePeerCount() > 0u)
        return "Connected";

    return "Searching for host...";
}

// ----------------------------------------------------------------------------
void NetworkNode::handleNetworkError(const std::string& operation,
                                     sf::Socket::Status status)
{
    switch (status)
    {
        case sf::Socket::NotReady:
            std::cerr << "Network operation not ready: " << operation
                      << std::endl;
            break;
        case sf::Socket::Disconnected:
            std::cerr << "Network disconnection during: " << operation
                      << std::endl;
            break;
        case sf::Socket::Error:
            std::cerr << "Network error during: " << operation << std::endl;
            break;
        default:
            break;
    }
}