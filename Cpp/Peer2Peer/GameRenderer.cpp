#include "GameRenderer.hpp"
#include <cmath>
#include <iostream>

// ----------------------------------------------------------------------------
GameRenderer::GameRenderer(sf::RenderWindow& window)
    : m_window(window)
{
    if (!m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
        std::cerr << "Error loading font" << std::endl;
    }
}

// ----------------------------------------------------------------------------
void GameRenderer::render(const GameState& game_state)
{
    renderRoads(game_state);
    renderBuildings(game_state);
    renderCars(game_state);
    renderEconomyInfo(game_state);
    renderConnectionStatus();
    renderMenuText();
}

// ----------------------------------------------------------------------------
void GameRenderer::renderRoads(const GameState& game_state)
{
    sf::RectangleShape roadLine;

    for (const auto& road : game_state.traffic.roads)
    {
        const auto& pos1 = game_state.economy.buildings[road.building1_idx].position;
        const auto& pos2 = game_state.economy.buildings[road.building2_idx].position;

        // Calculate road length and angle
        float dx = pos2.x - pos1.x;
        float dy = pos2.y - pos1.y;
        float length = std::sqrt(dx * dx + dy * dy);
        float angle = std::atan2(dy, dx) * 180.0f / 3.14159f;

        roadLine.setSize(sf::Vector2f(length, 2.0f));
        roadLine.setPosition(pos1);
        roadLine.setRotation(angle);
        roadLine.setFillColor(road.color);
        m_window.draw(roadLine);
    }
}

// ----------------------------------------------------------------------------
void GameRenderer::renderBuildings(const GameState& game_state)
{
    sf::RectangleShape buildingShape;
    buildingShape.setSize(sf::Vector2f(20.0f, 20.0f));
    buildingShape.setOrigin(10.0f, 10.0f);

    for (size_t i = 0; i < game_state.economy.buildings.size(); ++i)
    {
        const auto& building = game_state.economy.buildings[i];
        buildingShape.setPosition(building.position);
        buildingShape.setFillColor(building.color);
        m_window.draw(buildingShape);

        sf::Text buildingText;
        buildingText.setFont(m_font);
        buildingText.setCharacterSize(12);
        buildingText.setFillColor(sf::Color::Black);
        buildingText.setString(std::to_string(i));
        buildingText.setPosition(building.position.x - 5, building.position.y - 5);
        m_window.draw(buildingText);
    }
}

// ----------------------------------------------------------------------------
void GameRenderer::renderCars(const GameState& game_state)
{
    sf::CircleShape carShape(5.0f);
    carShape.setOrigin(2.5f, 2.5f);
    carShape.setOutlineColor(sf::Color::Black);
    carShape.setOutlineThickness(1.0f);

    for (const auto& car : game_state.traffic.cars)
    {
        carShape.setFillColor(car.color);
        carShape.setPosition(car.position);
        m_window.draw(carShape);
    }
}

// ----------------------------------------------------------------------------
void GameRenderer::renderEconomyInfo(const GameState& game_state)
{
    sf::Text economyText;
    economyText.setFont(m_font);
    economyText.setCharacterSize(14);
    economyText.setFillColor(sf::Color::Green);
    economyText.setPosition(10, 550);

    std::string economyInfo = "Money: " + std::to_string(game_state.economy.money);
    economyInfo += "\nBuildings: " + std::to_string(game_state.economy.buildings.size());
    
    float totalIncome = 0.0f;
    for (const auto& building : game_state.economy.buildings)
    {
        totalIncome += building.income;
    }
    economyInfo += "\nTotal Income: " + std::to_string(totalIncome);
    economyInfo += "\nTax Rate: " + std::to_string(game_state.economy.tax_rate * 100.0f) + "%";

    economyText.setString(economyInfo);
    m_window.draw(economyText);
} 

// ----------------------------------------------------------------------------
void GameRenderer::setConnectionStatus(std::string const& text)
{
    m_connection_status.setString(text);
}

// ----------------------------------------------------------------------------
void GameRenderer::renderConnectionStatus()
{
    m_connection_status.setFont(m_font);
    m_connection_status.setCharacterSize(14);
    m_connection_status.setFillColor(sf::Color::White);
    m_connection_status.setPosition(10, 10);
    m_window.draw(m_connection_status);
}

// ----------------------------------------------------------------------------
void GameRenderer::setMenuText(std::string const& text)
{
    m_menu_text.setString(text);
}

// ----------------------------------------------------------------------------
void GameRenderer::renderMenuText()
{
    m_menu_text.setFont(m_font);
    m_menu_text.setCharacterSize(14);
    m_menu_text.setFillColor(sf::Color::Yellow);
    m_menu_text.setPosition(10, 100);
    m_window.draw(m_menu_text);
}