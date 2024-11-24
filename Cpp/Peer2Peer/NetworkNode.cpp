#include "NetworkNode.hpp"
#include "NetworkProtocol.hpp"
#include <cstdlib>

// ----------------------------------------------------------------------------
NetworkNode::NetworkNode(unsigned short port, bool hosting) : m_is_host(hosting)
{
    std::cout << (hosting ? "Starting host on port "
                          : "Starting client on port ")
              << port << " and discovery port " << DISCOVERY_PORT << std::endl;

    // Main P2P communication socket.
    if (m_socket.bind(port) != sf::Socket::Done)
    {
        throw std::runtime_error("Failed to bind socket to port " +
                                 std::to_string(port));
    }
    m_socket.setBlocking(false);

    // Discovery socket for finding peers.
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
    updatePeerDiscovery(deltaTime);
    checkPeerTimeouts(deltaTime);
    receiveMessages();
    processMessages(state);

    if (m_is_host && hasAvailableNodes())
    {
        distributeTrafficLoad(state);
        distributeEconomyCalculations(state);
    }

    if (hasAvailableNodes())
    {
        synchronizeState(state);
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
    if (state.traffic.car_positions.empty())
    {
        std::cerr << "Warning: No cars to process" << std::endl;
        return;
    }

    // Calculate cars per peer and distribute workload
    size_t cars_per_peer = std::max(
        size_t(1), state.traffic.car_positions.size() / active_peer_count);
    
    size_t start_idx = 0;
    for (auto const& [name, peer_info] : m_peers)
    {
        if (peer_info.is_active)
        {
            // Check bounds
            size_t end_idx = std::min(start_idx + cars_per_peer,
                                    state.traffic.car_positions.size());
            
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
    if (state.economy.building_incomes.empty())
    {
        std::cerr << "Warning: No buildings to process" << std::endl;
        return;
    }

    // Calculate buildings per peer and distribute workload
    size_t buildings_per_peer = std::max(
        size_t(1), state.economy.building_incomes.size() / active_peer_count);

    size_t start_idx = 0;
    for (auto const& [name, peer_info] : m_peers)
    {
        if (peer_info.is_active)
        {
            // Check bounds
            size_t end_idx = std::min(start_idx + buildings_per_peer,
                                      state.economy.building_incomes.size());

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
void NetworkNode::updatePeerDiscovery(float deltaTime)
{
    m_last_ping_sent += deltaTime;

    if (m_last_ping_sent >= PING_INTERVAL)
    {
        m_last_ping_sent = 0.0f;

        if (m_is_host)
        {
            // Host broadcasts its presence
            sf::Packet packet = NetworkProtocol::createDiscoveryPacket(m_socket.getLocalPort());
            std::cout << "Host broadcasting on discovery port " << DISCOVERY_PORT << std::endl;
            m_discovery_socket.send(packet, sf::IpAddress::Broadcast, DISCOVERY_PORT);
        }
        else if (m_peers.empty()) // Client sans connexion
        {
            // Le client envoie un ping au port de découverte
            sf::Packet packet = NetworkProtocol::createPingPacket();
            std::cout << "Client sending discovery ping to port " << DISCOVERY_PORT << std::endl;
            m_discovery_socket.send(packet, sf::IpAddress::LocalHost, DISCOVERY_PORT);
        }
        else // Client connecté
        {
            // Envoie des pings réguliers au host
            sf::Packet packet = NetworkProtocol::createPingPacket();
            for (auto& [name, peer] : m_peers)
            {
                if (m_socket.send(packet, peer.address, peer.port) != sf::Socket::Done)
                {
                    std::cerr << "Failed to send ping to " << name << std::endl;
                }
            }
        }
    }

    // Handle discovery packets
    sf::Packet discoveryPacket;
    sf::IpAddress senderAddress;
    unsigned short senderPort;

    while (m_discovery_socket.receive(
               discoveryPacket, senderAddress, senderPort) == sf::Socket::Done)
    {
        // Ignore les paquets provenant de nous-mêmes
        if ((senderAddress == sf::IpAddress::LocalHost || 
             senderAddress == sf::IpAddress::getLocalAddress()) && 
            (m_is_host && senderPort == DISCOVERY_PORT))
        {
            continue;
        }

        sf::Uint8 messageType;
        discoveryPacket >> messageType;

        std::cout << "Received discovery packet type " << static_cast<int>(messageType) 
                  << " from " << senderAddress << ":" << senderPort << std::endl;

        // Host discovery packet
        if (DiscoveryMessageType(messageType) == DiscoveryMessageType::DISCOVERY && !m_is_host)
        {
            unsigned short hostPort;
            discoveryPacket >> hostPort;
            std::cout << "Client discovered host at port " << hostPort << std::endl;
            addPeer("host", senderAddress, hostPort);
        }
        // Ping packet
        else if (DiscoveryMessageType(messageType) == DiscoveryMessageType::PING && m_is_host)
        {
            std::cout << "Host received ping from " << senderAddress << ":" << senderPort << std::endl;
            // Le host ajoute le client qui ping
            std::string clientId = "client_" + std::to_string(senderPort);
            addPeer(clientId, senderAddress, senderPort);
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
void NetworkNode::receiveMessages()
{
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short senderPort;

    while (true)
    {
        sf::Socket::Status status =
            m_socket.receive(packet, sender, senderPort);

        if (status == sf::Socket::Done)
        {
            m_message_queue.push(packet);

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
void NetworkNode::processMessages(GameState& state)
{
    while (!m_message_queue.empty())
    {
        sf::Packet packet = m_message_queue.front();
        m_message_queue.pop();

        sf::Uint8 messageType;
        packet >> messageType;

        switch (GameMessageType(messageType))
        {
            case GameMessageType::TRAFFIC_UPDATE:
                processTrafficUpdate(state, packet);
                break;
            case GameMessageType::ECONOMY_UPDATE:
                processEconomyUpdate(state, packet);
                break;
            case GameMessageType::STATE_SYNC:
                processStateSync(state, packet);
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

// ----------------------------------------------------------------------------
void NetworkNode::processTrafficUpdate(GameState& state, sf::Packet& packet)
{
    sf::Uint32 startIdx, count;
    packet >> startIdx >> count;

    // Vérification des limites
    if (startIdx + count > state.traffic.car_positions.size())
    {
        std::cerr << "Warning: Traffic update packet contains invalid range" << std::endl;
        return;
    }

    // Mise à jour des positions des voitures
    for (sf::Uint32 i = 0; i < count; ++i)
    {
        float x, y;
        packet >> x >> y;
        state.traffic.car_positions[startIdx + i] = {x, y};
    }
}

// ----------------------------------------------------------------------------
void NetworkNode::processEconomyUpdate(GameState& state, sf::Packet& packet)
{
    sf::Uint32 startIdx, count;
    packet >> startIdx >> count;

    // Vérification des limites
    if (startIdx + count > state.economy.building_incomes.size())
    {
        std::cerr << "Warning: Economy update packet contains invalid range" << std::endl;
        return;
    }

    // Mise à jour des revenus des bâtiments
    for (sf::Uint32 i = 0; i < count; ++i)
    {
        float income;
        packet >> income;
        state.economy.building_incomes[startIdx + i] = income;
    }

    // Mise à jour de l'argent total
    float money;
    packet >> money;
    state.economy.money = money;
}

// ----------------------------------------------------------------------------
void NetworkNode::processStateSync(GameState& state, sf::Packet& packet)
{
    // Lecture du nombre de voitures
    sf::Uint32 car_count;
    packet >> car_count;

    // Mise à jour des positions des voitures
    state.traffic.car_positions.resize(car_count);
    for (sf::Uint32 i = 0; i < car_count; ++i)
    {
        float x, y;
        packet >> x >> y;
        state.traffic.car_positions[i] = {x, y};
    }

    // Mise à jour de l'argent
    packet >> state.economy.money;
}