#pragma once

#include <SFML/Graphics.hpp>
#include "GameState.hpp"

// ----------------------------------------------------------------------------
/// \class GameRenderer
/// \brief Class handling all rendering logic for the game
class GameRenderer
{
public:
    /// \brief Constructor
    /// \param[in] window Reference to the SFML window
    GameRenderer(sf::RenderWindow& window);

    /// \brief Render all game elements
    /// \param[in] game_state Current state of the game to render
    void render(const GameState& game_state);

    /// \brief Set the connection status text
    /// \param[in] text Text to set
    void setConnectionStatus(std::string const& text);

    /// \brief Set the menu text
    /// \param[in] text Text to set
    void setMenuText(std::string const& text);

private:
    /// \brief Render all roads in the game
    /// \param[in] game_state Current state of the game
    void renderRoads(const GameState& game_state);

    /// \brief Render all buildings in the game
    /// \param[in] game_state Current state of the game
    void renderBuildings(const GameState& game_state);

    /// \brief Render all cars in the game
    /// \param[in] game_state Current state of the game
    void renderCars(const GameState& game_state);

    /// \brief Render economic information
    /// \param[in] game_state Current state of the game
    void renderEconomyInfo(const GameState& game_state);

    /// \brief Render connection status
    void renderConnectionStatus();
    
    /// \brief Render menu text
    void renderMenuText();

    sf::RenderWindow& m_window;  ///< SFML window
    sf::Font m_font;  ///< Font for text rendering
    sf::Text m_connection_status;  ///< Connection status text display
    sf::Text m_menu_text;  ///< Menu text display
};