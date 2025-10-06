class Button
{
public:

    Button(std::string const& on, std::string const& off, sf::Vector2f const& dim, sf::Vector2f const& pos)
    {
        m_textures[0].loadFromFile(on);
        m_textures[1].loadFromFile(off);

        m_rectangle.setSize(dim);
        m_rectangle.setPosition(pos);
        m_rectangle.setTexture(&m_textures[0]);
    }

    bool click(sf::Vector2f const& mouse)
    {
        if (m_rectangle.getGlobalBounds().contains(mouse.x, mouse.y))
        {
            if (m_rectangle.getTexture() == &m_textures[0])
            {
                m_rectangle.setTexture(&m_textures[1]);
            }
            else
            {
                m_rectangle.setTexture(&m_textures[0]);
            }
            return true;
        }
        return false;
    }

    void draw(sf::RenderWindow& renderer)
    {
        renderer.draw(m_rectangle);
    }

private:

    sf::Texture m_textures[2];
    sf::RectangleShape m_rectangle;
};

class BtMarchArret: public Button
{
public:

    BtMarchArret(sf::Vector2f const& pos)
        : Button("data/arret.png",
                 "data/marche.png",
                 sf::Vector2f(78.f, 142.f), pos)
    {}
};

class BtToggle: public Button
{
public:

    BtToggle(sf::Vector2f const& pos)
        : Button("data/toggleup.png",
                 "data/toggledown.png",
                 sf::Vector2f(64.f, 70.f), pos)
    {}
};

class LED: public Button
{
public:

    LED(sf::Vector2f const& pos)
        : Button("data/bt_vert_up.png",
                 "data/bt_vert_dn.png",
                 sf::Vector2f(64.f, 64.f), pos)
    {}
};
