#include "Client.hpp"
#include <iostream>
#include <random>

// ----------------------------------------------------------------------------
Client::Client(unsigned short port, bool hosting)
    : m_node(std::make_unique<NetworkNode>(port, hosting)),
      m_is_host(hosting)
{
    initializeWindow();

    // Random color initialization: one color by client
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> color_dist(0, 255);
    m_game_state.color = sf::Color(color_dist(gen), color_dist(gen), color_dist(gen), 255);
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

    if (!m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
        std::cerr << "Error loading font" << std::endl;
    }

    m_status_text.setFont(m_font);
    m_status_text.setCharacterSize(14);
    m_status_text.setFillColor(sf::Color::White);
    m_status_text.setPosition(10, 10);
}

// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
void Client::handleKeyPress(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Escape)
        m_window.close();
}

// ----------------------------------------------------------------------------
void Client::update(float deltaTime)
{
    m_node->update(deltaTime, m_game_state);
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
    m_status_text.setString(status);
}

// ----------------------------------------------------------------------------
void Client::render()
{
    m_window.clear(sf::Color(50, 50, 50));
    renderGameElements();
    m_window.draw(m_status_text);
    m_window.display();
}

// ----------------------------------------------------------------------------
void Client::renderGameElements()
{
    // Render roads
    sf::RectangleShape roadLine;
    roadLine.setFillColor(m_game_state.color);

    for (const auto& road : m_game_state.traffic.roads)
    {
        const auto& pos1 = m_game_state.economy.buildings[road.building1_idx].position;
        const auto& pos2 = m_game_state.economy.buildings[road.building2_idx].position;

        // Calculer la longueur et l'angle de la route
        float dx = pos2.x - pos1.x;
        float dy = pos2.y - pos1.y;
        float length = std::sqrt(dx * dx + dy * dy);
        float angle = std::atan2(dy, dx) * 180.0f / 3.14159f;

        roadLine.setSize(sf::Vector2f(length, 2.0f));
        roadLine.setPosition(pos1);
        roadLine.setRotation(angle);
        m_window.draw(roadLine);
    }

    // Render buildings
    sf::RectangleShape buildingShape;
    buildingShape.setSize(sf::Vector2f(20.0f, 20.0f));
    buildingShape.setOrigin(10.0f, 10.0f);
    buildingShape.setFillColor(m_game_state.color);
    
    // Render cars
    sf::CircleShape carShape(5.0f);
    carShape.setOrigin(2.5f, 2.5f);
    carShape.setFillColor(m_game_state.color);
    carShape.setOutlineColor(sf::Color::Black);
    carShape.setOutlineThickness(1.0f);

    for (const auto& car : m_game_state.traffic.cars)
    {
        carShape.setPosition(car.position);
        m_window.draw(carShape);
    }

    for (size_t i = 0; i < m_game_state.economy.buildings.size(); ++i)
    {
        const auto& building = m_game_state.economy.buildings[i];
        buildingShape.setPosition(building.position);
        m_window.draw(buildingShape);

        sf::Text buildingText;
        buildingText.setFont(m_font);
        buildingText.setCharacterSize(12);
        buildingText.setFillColor(sf::Color::Black);
        buildingText.setString(std::to_string(i));
        buildingText.setPosition(building.position.x - 5, building.position.y - 5);
        m_window.draw(buildingText);
    }

    // Render economic information
    sf::Text economyText;
    economyText.setFont(m_font);
    economyText.setCharacterSize(14);
    economyText.setFillColor(sf::Color::Green);
    economyText.setPosition(10, 550);

    std::string economyInfo = "Money: " + std::to_string(m_game_state.economy.money);
    economyInfo += "\nBuildings: " + std::to_string(m_game_state.economy.buildings.size());
    
    float totalIncome = 0.0f;
    for (const auto& building : m_game_state.economy.buildings)
    {
        totalIncome += building.income;
    }
    economyInfo += "\nTotal Income: " + std::to_string(totalIncome);
    economyInfo += "\nTax Rate: " + std::to_string(m_game_state.economy.tax_rate * 100.0f) + "%";

    economyText.setString(economyInfo);
    m_window.draw(economyText);
}