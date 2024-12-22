#include "Host.hpp"

Host::Host(unsigned short port)
    : Client(port, true)
{
   m_renderer->setMenuText("Host Controls:\n"
                            "F5: Start new simulation\n"
                            "F6: Pause/Resume\n"
                            "F7: Save state\n"
                            "F8: Load state");
}

void Host::handleKeyPress(sf::Keyboard::Key key)
{
    Client::handleKeyPress(key);

    switch (key)
    {
        case sf::Keyboard::F5:
            GameManager::createInitialState(m_game_state);
            break;
        case sf::Keyboard::F6:
            // Pause/Reprise
            break;
        case sf::Keyboard::F7:
            // Sauvegarde
            break;
        case sf::Keyboard::F8:
            // Chargement
            break;
        default:
            break;
    }
}

void Host::renderGameElements()
{
    Client::renderGameElements();
    
    m_window.draw(m_control_text);
}