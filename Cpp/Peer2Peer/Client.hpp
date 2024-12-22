#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "NetworkNode.hpp"
#include "GameState.hpp"
#include "GameRenderer.hpp"

class GameRenderer;  // Forward declaration

// ----------------------------------------------------------------------------
/// \class Client
/// \brief Main client class handling the game window and network communication
class Client
{
public:
    /// \brief Constructor
    /// \param[in] port Port number for network communication
    /// \param[in] hosting Whether this client is the host
    Client(unsigned short port, bool hosting);

    /// \brief Main game loop
    void run();

protected:

    virtual void handleKeyPress(sf::Keyboard::Key key);
    virtual void renderGameElements();

private:

    void initializeWindow();
    void handleEvents();
    void update(float deltaTime);
    void updateStatusText();
    void render();

protected:

    sf::RenderWindow m_window;  ///< SFML window
    std::unique_ptr<NetworkNode> m_node;  ///< Network communication node
    bool m_is_host;  ///< Whether this client is the host
    std::unique_ptr<GameRenderer> m_renderer;  ///< Game renderer
    GameState m_game_state;  ///< Current game state
    sf::Color m_color;  ///< Client specific color
};
