#pragma once

#include "GameManager.hpp"
#include "NetworkNode.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

/**
 * @brief Client class. Manages the client side of the game.
 */
class Client
{
public:
    /**
     * @brief Constructor
     * @param[in] port Port number to use for network communication
     * @param[in] hosting Whether this client is a host
     */
    explicit Client(unsigned short port, bool hosting = false);
    virtual ~Client() = default;
    virtual void run();

protected:

    void initializeWindow();
    void handleEvents();
    virtual void handleKeyPress(sf::Keyboard::Key key);
    void update(float deltaTime);
    void updateStatusText();
    void render();
    virtual void renderGameElements();

protected:
    std::unique_ptr<NetworkNode> m_node;
    sf::RenderWindow m_window;
    bool m_is_host;
    sf::Font m_font;
    sf::Text m_status_text;
    GameState m_game_state;
};