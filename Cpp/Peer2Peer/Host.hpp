#pragma once

#include "Client.hpp"
#include <SFML/Graphics.hpp>

class Host : public Client
{
public:
    /// \brief Constructor
    /// \param[in] port Port number to use for network communication
    explicit Host(unsigned short port);

private:
    void handleKeyPress(sf::Keyboard::Key key) override;
    void renderGameElements() override;

private:
    sf::Text m_control_text;
};
