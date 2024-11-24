#include "Client.hpp"
#include <iostream>

Client::Client(unsigned short port, bool hosting)
    : m_node(std::make_unique<NetworkNode>(port, hosting)),
      m_is_host(hosting)
{
    GameManager::createInitialState(m_game_state);
    initializeWindow();
}

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

void Client::initializeWindow()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    std::string title = m_is_host ? "SimCity P2P - Host" : "SimCity P2P - Client";
    m_window.create(sf::VideoMode(800, 600), title);
    m_window.setPosition(sf::Vector2i(m_is_host ? 0 : (desktop.width - 800),
                                    (desktop.height - 600) / 2));

    if (!m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
        std::cerr << "Erreur lors du chargement de la police" << std::endl;
    }

    m_status_text.setFont(m_font);
    m_status_text.setCharacterSize(14);
    m_status_text.setFillColor(sf::Color::White);
    m_status_text.setPosition(10, 10);
}

void Client::handleEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            m_window.close();

        if (event.type == sf::Event::KeyPressed)
        {
            handleKeyPress(event.key.code);
        }
    }
}

void Client::handleKeyPress(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Escape)
        m_window.close();
}

void Client::update(float deltaTime)
{
    m_node->update(deltaTime, m_game_state);
    updateStatusText();
}

void Client::updateStatusText()
{
    std::string status = m_is_host ? "Host - " : "Client - ";
    status += "Port: " + std::to_string(m_node->getPort()) + "\n";
    status += "Connected peers: " + std::to_string(m_node->getActivePeerCount()) + "\n";
    status += "Network status: " + m_node->getNetworkStatusString() + "\n";
    status += "Nodes available: " + std::string(m_node->hasAvailableNodes() ? "Yes" : "No");
    m_status_text.setString(status);
}

void Client::render()
{
    m_window.clear(sf::Color(50, 50, 50));
    renderGameElements();
    m_window.draw(m_status_text);
    m_window.display();
}

void Client::renderGameElements()
{
    // Affichage des voitures
    sf::CircleShape carShape(5.0f); // Rayon de 5 pixels
    carShape.setFillColor(sf::Color::Red);

    for (const auto& pos : m_game_state.traffic.car_positions)
    {
        carShape.setPosition(pos.x, pos.y);
        m_window.draw(carShape);
    }

    // Affichage des informations économiques
    sf::Text economyText;
    economyText.setFont(m_font);
    economyText.setCharacterSize(14);
    economyText.setFillColor(sf::Color::Green);
    economyText.setPosition(10, 550);

    std::string economyInfo = "Money: " + std::to_string(m_game_state.economy.money);
    if (!m_game_state.economy.building_incomes.empty())
    {
        economyInfo += "\nBuildings: " + std::to_string(m_game_state.economy.building_incomes.size());
        float totalIncome = 0.0f;
        for (float income : m_game_state.economy.building_incomes)
        {
            totalIncome += income;
        }
        economyInfo += "\nTotal Income: " + std::to_string(totalIncome);
    }
    
    economyText.setString(economyInfo);
    m_window.draw(economyText);
} 