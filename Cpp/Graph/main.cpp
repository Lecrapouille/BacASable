#include <SFML/Graphics.hpp>
#include "Graph.hpp"
#include <cassert>
#include <cmath>

class Circle: public sf::Drawable
{
public:

    Circle(float x, float y, float r, sf::Color color = sf::Color::Red)
        : m_shape(r, 16)
    {
        m_shape.setOrigin(m_shape.getRadius(), m_shape.getRadius());
        m_shape.setPosition(x, y);
        m_shape.setFillColor(color);
        m_shape.setOutlineThickness(1.0f);
        m_shape.setOutlineColor(color);
    }

    void position(sf::Vector2f const& p)
    {
        m_shape.setPosition(p);
    }

private:

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override final
    {
        target.draw(m_shape, states);
    }

private:

    sf::CircleShape m_shape;
};

// g++ -Wall -Wextra --std=c++11 main.cpp `pkg-config --cflags --libs sfml-graphics`
int main()
{
    std::vector<Edge> edges =
    {
        {0, 1}, {0, 4}, {0, 5}, {1, 3},
        {1, 4}, {2, 1}, {3, 2}, {3, 4}
    };

    std::vector<sf::Vector2f> data =
    {
       {100.0f, 100.0f}, {200.0f, 200.0f}, {300.0f, 300.0f},
       {300.0f, 100.0f}, {200.0f, 100.0f}, {100.0f, 200.0f}
    };

    Graph<sf::Vector2f> graph(edges, data, true);
    //graph.print();
    std::vector<Node<sf::Vector2f>> const& nodes = graph.nodes();

    sf::RenderWindow window(sf::VideoMode(800, 600), "Force-directed Graph");
    Circle c(0.0f, 0.0f, 5.0f, sf::Color::Blue);
    sf::Vertex line[2];

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        window.clear(sf::Color::White);

        for (size_t j = 0; j < nodes.size(); ++j)
        {
            c.position(graph.node(j).data);
            window.draw(c);

            for (auto idx: nodes[j].neighbors())
            {
                line[0] = sf::Vertex(graph.node(j).data);
                line[1] = sf::Vertex(graph.node(idx).data);
                line[0].color = sf::Color::Blue;
                line[1].color = sf::Color::Blue;
                window.draw(line, 2, sf::Lines);
            }
        }

        window.display();
    }

    return EXIT_SUCCESS;
}
