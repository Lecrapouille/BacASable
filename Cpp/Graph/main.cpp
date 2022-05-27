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

float norm(sf::Vector2f const& v)
{
    return sqrtf(v.x * v.x + v.y * v.y) + 0.0001f;
}

# if 0
constexpr float POW2(float const x)  { return x * x; }

static sf::Vector2f repulsive_force(sf::Vector2f const& u, sf::Vector2f const& v)
{
    const float Crep = 2.0f;
    return Crep * (v - u) / (POW2(u.x - v.x) + POW2(u.y - v.y));
}

static sf::Vector2f spring_force(sf::Vector2f const& u, sf::Vector2f const& v)
{
    const float Cspring = 1.0f;
    const float l = 1.0f; // ideal spring length

    return Cspring * logf(sqrtf(POW2(u.x - v.x) + POW2(u.y - v.y)) / l) * (u - v);
}

static sf::Vector2f attractive_force(sf::Vector2f const& u, sf::Vector2f const& v)
{
    return spring_force(u, v) - repulsive_force(u, v);
}

static float cooler(float dt)
{
    return 1.0f;
}
#endif

const float K = 0.5f;
static float fa(float x) { return x * x / (2.0f * K); }
static float fr(float x) { return K * K / x; }

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

    std::vector<Node<sf::Vector2f>>& nodes = graph.nodes();
    std::vector<sf::Vector2f> forces;
    forces.resize(nodes.size());
    float temp = 1.0f;

    const float W = 800.0f;
    const float H = 600.0f;
    sf::RenderWindow window(sf::VideoMode(int(W), int(H)), "Force-directed Graph");
    Circle c(0.0f, 0.0f, 5.0f, sf::Color::Blue);
    sf::Vertex line[2];

    sf::Clock clock;
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

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

        // Repulsive forces
        std::cout << "Repulsive forces" << std::endl;
        for (size_t v = 0; v < nodes.size(); ++v)
        {
            forces[v] = { 0.0f, 0.0f };

            for (size_t u = 0; u < nodes.size(); ++u)
            {
               if (u == v)
                   continue;

               sf::Vector2f d = nodes[v].data - nodes[u].data;
               //std::cout << "d " << d.x << ", " << d.y << std::endl;
               float nd = norm(d);
               //std::cout << "nd " << nd << ": " << fr(nd) << std::endl;
               forces[v] += (d / nd) * fr(nd);
            }
        }

        for (auto const f: forces)
        {
            std::cout << "f: " << f.x << ", " << f.y << std::endl;
        }

        // Attractive forces
        std::cout << "Attractive forces" << std::endl;
        for (size_t u = 0; u < nodes.size(); ++u)
        {
            for (auto v: nodes[u].neighbors())
            {
               sf::Vector2f d = nodes[v].data - nodes[u].data;
               float nd = norm(d);
               sf::Vector2f f = (d / nd) * fa(nd);
               forces[v] -= f;
               forces[u] += f;
            }
        }

        for (auto const f: forces)
        {
            std::cout << "f: " << f.x << ", " << f.y << std::endl;
        }

        // Cooling and constrain
        std::cout << "Cooling:" << std::endl;
        for (size_t v = 0; v < nodes.size(); ++v)
        {
            const float n = norm(forces[v]);
//std::cout << "n " << n << std::endl;

            nodes[v].data.x += forces[v].x * std::min(forces[v].x, temp) / n;
            nodes[v].data.y += forces[v].y * std::min(forces[v].y, temp) / n;

std::cout << "P: " << nodes[v].data.x << ", " << nodes[v].data.y << std::endl;

            nodes[v].data.x = std::min(W, std::max(0.0f, nodes[v].data.x));
            nodes[v].data.y = std::min(H, std::max(0.0f, nodes[v].data.y));

std::cout << "Pc: " <<  nodes[v].data.x << ", " << nodes[v].data.y << std::endl;
        }


#if 0
        // Force-directed Graph: 
        // Force on node u = sum(repulsive forces on other nodes)
        //                 + sum(attractive forces on neighbors of u)
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            forces[i] = { 0.0f, 0.0f };
        }
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            for (size_t j = 0; j < nodes.size(); ++j)
            {
               if (i != j)
               {
                   forces[i] += repulsive_force(graph.node(i).data, graph.node(j).data);
               }
            }
        }
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            for (auto j: nodes[i].neighbors())
            {
               forces[i] += attractive_force(graph.node(i).data, graph.node(j).data);
            }
        }

        for (size_t i = 0; i < nodes.size(); ++i)
        {
std::cout << forces[i].x << ", " << forces[i].y << std::endl;
           graph.node(i).data += forces[i] * cooler(dt);
        }
#endif



        // Draw the graph
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
