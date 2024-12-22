#include "Client.hpp"
#include "GameRenderer.hpp"
#include <iostream>
#include <random>

// ----------------------------------------------------------------------------
static sf::Color generateRandomColor()
{
    // Random color initialization: one color by client
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> color_dist(0, 255);
    return sf::Color(color_dist(gen), color_dist(gen), color_dist(gen), 255);
}

// ----------------------------------------------------------------------------
Client::Client(unsigned short port, bool hosting)
    : m_node(std::make_unique<NetworkNode>(port, hosting)),
      m_is_host(hosting)
{
    initializeWindow();
    m_renderer = std::make_unique<GameRenderer>(m_window);
    m_color = generateRandomColor();
}

// ----------------------------------------------------------------------------
void Client::run()
{
    sf::Clock clock;
    while (m_window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();
        handleEvents();
        update(deltaTime);
        render();
    }
}

// ----------------------------------------------------------------------------
void Client::initializeWindow()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    std::string title = m_is_host ? "SimCity P2P - Host" : "SimCity P2P - Client";
    m_window.create(sf::VideoMode(800, 600), title);
    m_window.setPosition(sf::Vector2i(m_is_host ? 0 : (desktop.width - 800),
                                    (desktop.height - 600) / 2));
}

// ----------------------------------------------------------------------------
    void Client::handleEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_window.close();
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            handleKeyPress(event.key.code);
        }
    }
}

// ----------------------------------------------------------------------------
void Client::handleKeyPress(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Escape)
    {
        m_window.close();
    }
}

// ----------------------------------------------------------------------------
void Client::render()
{
    m_window.clear(sf::Color(50, 50, 50));
    renderGameElements();
    m_window.display();
}

// ----------------------------------------------------------------------------
void Client::renderGameElements()
{
    m_renderer->render(m_game_state);
}

// ----------------------------------------------------------------------------
void Client::update(float deltaTime)
{
    m_node->update(deltaTime, m_game_state, m_color);
    updateStatusText();
}

// ----------------------------------------------------------------------------
void Client::updateStatusText()
{
    std::string status = m_is_host ? "Host - " : "Client - ";
    status += "Port: " + std::to_string(m_node->getPort()) + "\n";
    status += "Connected peers: " + std::to_string(m_node->getActivePeerCount()) + "\n";
    status += "Network status: " + m_node->getNetworkStatusString() + "\n";
    status += "Nodes available: " + std::string(m_node->hasAvailableNodes() ? "Yes" : "No");
    m_renderer->setConnectionStatus(status);
}