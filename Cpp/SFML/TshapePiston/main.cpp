#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <cmath>
#include <random>

// *************************************************************************************
//! \brief Conversion factor between Box2D meters and screen pixels
const float SCALE = 30.0f; // Pixels per meter for Box2D
// *************************************************************************************

// *************************************************************************************
//! \brief T-shaped piston that can move horizontally and shoot vertically a box.
//! This class represents a T-shaped piston that can be controlled horizontally
//! by the player or AI, and can shoot vertically to hit the box.
//! The piston can be either the bottom or the top piston.
// *************************************************************************************
class TPiston
{
public:
    //! \brief Horizontal movement speed of the piston
    static constexpr float PISTON_SPEED = 5.0f;
    //! \brief Force applied by the piston impact
    static constexpr float PISTON_FORCE = 50.0f;
    //! \brief Vertical speed of the piston when shooting
    static constexpr float PISTON_SHOOT_SPEED = 15.0f;
    //! \brief Density of the piston
    static constexpr float PISTON_DENSITY = 1.0f;

    // ---------------------------------------------------------------------------------
    //! \brief Construct a new TPiston object.
    //! \param p_world Reference to the Box2D physics world
    //! \param p_bottom Whether this is the bottom piston (true) or top piston (false)
    // ---------------------------------------------------------------------------------
    TPiston(b2World& p_world, bool p_bottom) : m_is_bottom(p_bottom), m_position(0), m_y_offset(0)
    {
        // Rigid body definition
        b2BodyDef body_def;
        body_def.type = b2_kinematicBody;
        m_y_default = p_bottom ? 550.0f / SCALE : 50.0f / SCALE;
        body_def.position.Set(400.0f / SCALE, m_y_default);
        m_body = p_world.CreateBody(&body_def);

        // Vertical shaft
        b2PolygonShape shaft_shape;
        shaft_shape.SetAsBox(0.3f, 1.5f);
        b2FixtureDef shaft_fixture;
        shaft_fixture.shape = &shaft_shape;
        shaft_fixture.density = PISTON_DENSITY;
        m_body->CreateFixture(&shaft_fixture);

        // Head horizontal (part of the T)
        b2PolygonShape head_shape;
        if (p_bottom)
        {
            head_shape.SetAsBox(2.0f, 0.3f, b2Vec2(0, -1.5f), 0);
        }
        else
        {
            head_shape.SetAsBox(2.0f, 0.3f, b2Vec2(0, 1.5f), 0);
        }
        b2FixtureDef head_fixture;
        head_fixture.shape = &head_shape;
        head_fixture.density = PISTON_DENSITY;
        m_body->CreateFixture(&head_fixture);

        // Visual configuration
        m_shaft.setSize(sf::Vector2f(20, 90));
        m_shaft.setFillColor(sf::Color(100, 100, 200));
        m_shaft.setOrigin(10, 45);
        m_head.setSize(sf::Vector2f(120, 20));
        m_head.setFillColor(sf::Color(100, 100, 200));
        m_head.setOrigin(60, 10);
    }

    // ---------------------------------------------------------------------------------
    //! \brief Move the piston horizontally.
    //! \param p_dx Change in x position (positive = right, negative = left)
    // ---------------------------------------------------------------------------------
    void move(float p_dx)
    {
        m_position += p_dx;
        // Limit to window edges (800px width, head is 120px wide -> 60px radius)
        m_position = std::max(-340.0f, std::min(340.0f, m_position));
        updatePosition();
    }

    // ---------------------------------------------------------------------------------
    //! \brief Activate vertical shooting motion of the piston
    //! \param p_active True to shoot (move toward the box), false to return to rest position
    // ---------------------------------------------------------------------------------
    void shoot(bool p_active)
    {
        // Bottom piston - moves up when active
        // Top piston - moves down when active
        if (m_is_bottom)
        {
            m_y_offset = p_active ? -40.0f / SCALE : 0;
        }
        else
        {
            m_y_offset = p_active ? 40.0f / SCALE : 0;
        }

        // Apply movement using linear velocity
        b2Vec2 velocity;
        if (p_active)
        {
            // Fast movement in the appropriate direction
            velocity = m_is_bottom ? b2Vec2(0, -PISTON_SHOOT_SPEED)
                                   : b2Vec2(0, PISTON_SHOOT_SPEED);
        }
        else
        {
            // Slower return to original position
            velocity = m_is_bottom ? b2Vec2(0, PISTON_SHOOT_SPEED / 2.0f)
                                   : b2Vec2(0, -PISTON_SHOOT_SPEED / 2.0f);
        }

        m_body->SetLinearVelocity(velocity);
    }

    // ---------------------------------------------------------------------------------
    //! \brief Update the piston's position in the physics world.
    // ---------------------------------------------------------------------------------
    void updatePosition()
    {
        b2Vec2 pos = m_body->GetPosition();
        pos.x = (400.0f + m_position) / SCALE;
        pos.y = m_y_default + m_y_offset;
        m_body->SetTransform(pos, 0);
    }

    // ---------------------------------------------------------------------------------
    //! \brief Update piston position and enforce movement constraints.
    //! This method is called every frame to update the piston's physics and
    //! visual representation, limiting its movement within the allowed range.
    // ---------------------------------------------------------------------------------
    void update()
    {
        b2Vec2 pos = m_body->GetPosition();

        // Limit the vertical position of the piston
        float limit_y;
        if (m_is_bottom)
        {

            float min_y = m_y_default - 40.0f / SCALE;
            float max_y = m_y_default;
            limit_y = std::max(min_y, std::min(max_y, pos.y));
        }
        else
        {
            float min_y = m_y_default;
            float max_y = m_y_default + 40.0f / SCALE;
            limit_y = std::max(min_y, std::min(max_y, pos.y));
        }

        // If position was limited, update the body position
        if (limit_y != pos.y)
        {
            pos.y = limit_y;
            m_body->SetTransform(pos, 0);

            // If we reach a limit, stop vertical velocity
            if ((limit_y == m_y_default) || (limit_y == m_y_default + m_y_offset))
            {
                b2Vec2 vel = m_body->GetLinearVelocity();
                vel.y = 0;
                m_body->SetLinearVelocity(vel);
            }
        }

        // Update visual elements
        m_shaft.setPosition(pos.x * SCALE, pos.y * SCALE);
        if (m_is_bottom)
        {
            m_head.setPosition(pos.x * SCALE, (pos.y - 1.5f) * SCALE);
        }
        else
        {
            m_head.setPosition(pos.x * SCALE, (pos.y + 1.5f) * SCALE);
        }
    }

    // ---------------------------------------------------------------------------------
    //! \brief Get the position of the piston.
    //! \return The position of the piston.
    // ---------------------------------------------------------------------------------
    b2Vec2 getPosition() const { return m_body->GetPosition(); }

    // ---------------------------------------------------------------------------------
    //! \brief Draw the piston on the window.
    //! \param p_window The SFML render window to draw on .
    // ---------------------------------------------------------------------------------
    void draw(sf::RenderWindow& p_window)
    {
        p_window.draw(m_shaft);
        p_window.draw(m_head);
    }

private:

    //! \brief Box2D physics body
    b2Body* m_body;
    //! \brief Vertical shaft visual element
    sf::RectangleShape m_shaft;
    //! \brief Horizontal head visual element
    sf::RectangleShape m_head;
    //! \brief Whether this is the bottom piston
    bool m_is_bottom;
    //! \brief Horizontal offset from center
    float m_position;
    //! \brief Default vertical position
    float m_y_default;
    //! \brief Vertical offset for shooting movement
    float m_y_offset;
};

// *************************************************************************************
//! \brief Represents the box object that gets hit by the pistons.
//! This class creates a box physics object that can move freely in the game area,
//! interacting with pistons and walls.
// *************************************************************************************
class Box
{
public:
    //! \brief Density of the box
    static constexpr float BOX_DENSITY = 0.8f;
    //! \brief Friction of the box
    static constexpr float BOX_FRICTION = 0.3f;
    //! \brief Restitution of the box
    static constexpr float BOX_RESTITUTION = 0.6f;

    // ---------------------------------------------------------------------------------
     //! \brief Construct a new Box object
     //! \param world Reference to the Box2D physics world
     // ---------------------------------------------------------------------------------
    Box(b2World& p_world)
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(400.0f / SCALE, 300.0f / SCALE);
        m_body = p_world.CreateBody(&bodyDef);

        b2PolygonShape boxShape;
        boxShape.SetAsBox(1.2f, 0.8f); // Bigger box (72x48 pixels)

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = BOX_DENSITY;     // More mass
        fixtureDef.friction = BOX_FRICTION;
        fixtureDef.restitution = BOX_RESTITUTION;
        m_body->CreateFixture(&fixtureDef);

        // Increase angular damping for more stable rotation
        m_body->SetAngularDamping(0.2f);

        m_shape.setSize(sf::Vector2f(72, 48)); // Visual size matches physics size
        m_shape.setFillColor(sf::Color(180, 100, 50));
        m_shape.setOrigin(36, 24);
    }

    // ---------------------------------------------------------------------------------
    //! \brief Update the box's visual representation from its physics state
    // ---------------------------------------------------------------------------------
    void update()
    {
        b2Vec2 pos = m_body->GetPosition();
        float angle = m_body->GetAngle();
        m_shape.setPosition(pos.x * SCALE, pos.y * SCALE);
        m_shape.setRotation(angle * 180.0f / b2_pi);
    }

    // ---------------------------------------------------------------------------------
    //! \brief Draw the box on the window.
    //! \param window The SFML render window to draw on.
    // ---------------------------------------------------------------------------------
    void draw(sf::RenderWindow& p_window)
    {
        p_window.draw(m_shape);
    }

    // ---------------------------------------------------------------------------------
    //! \brief Reset the box to its initial position.
    //! \param randomAngle Optional angle to set the box to (in radians).
    // ---------------------------------------------------------------------------------
    void reset(float p_random_angle)
    {
        m_body->SetTransform(b2Vec2(400.0f / SCALE, 300.0f / SCALE), p_random_angle);
        m_body->SetLinearVelocity(b2Vec2(0, 0));
        m_body->SetAngularVelocity(0);
    }

    // ---------------------------------------------------------------------------------
    //! \brief Check if the box is outside the visible area.
    //! \return true if the box is out of bounds.
    //! \return false if the box is in the visible area.
    // ---------------------------------------------------------------------------------
    bool isOutOfBounds()
    {
        b2Vec2 pos = m_body->GetPosition();
        // Check if box is outside the visible area (with some margin)
        return ((pos.y < -2.0f) || (pos.y > 22.0f));
    }

public:
    //! \brief Box2D physics body
    b2Body* m_body;
    //! \brief Visual representation of the box
    sf::RectangleShape m_shape;
};

// *************************************************************************************
//! \brief Main entry point for the T-Piston game
//! \return int Exit status code
// *************************************************************************************
int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "T-Piston Game");
    window.setFramerateLimit(60);

    // Create the Box2D physics world
    b2Vec2 gravity(0.0f, 9.8f);
    b2World world(gravity);

    // Setup random number generator for box orientation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * b2_pi);

    // Create the pistons
    TPiston bottom_piston(world, true);
    TPiston top_piston(world, false);

    // Create the box with random initial orientation
    Box box(world);
    float initial_angle = angle_dist(gen);
    box.reset(initial_angle);

    // Create left wall physics object
    b2BodyDef left_wall_def;
    left_wall_def.position.Set(0.0f, 300.0f / SCALE);
    b2Body* left_wall_body = world.CreateBody(&left_wall_def);

    b2EdgeShape left_wall_shape;
    left_wall_shape.SetTwoSided(b2Vec2(0, -300.0f / SCALE), b2Vec2(0, 300.0f / SCALE));

    b2FixtureDef left_wall_fixture;
    left_wall_fixture.shape = &left_wall_shape;
    left_wall_fixture.restitution = 0.8f;  // Bounce factor
    left_wall_fixture.friction = 0.2f;     // Low friction for better bouncing
    left_wall_body->CreateFixture(&left_wall_fixture);

    // Create right wall physics object
    b2BodyDef right_wall_def;
    right_wall_def.position.Set(800.0f / SCALE, 300.0f / SCALE);
    b2Body* right_wall_body = world.CreateBody(&right_wall_def);

    b2EdgeShape right_wall_shape;
    right_wall_shape.SetTwoSided(b2Vec2(0, -300.0f / SCALE), b2Vec2(0, 300.0f / SCALE));

    b2FixtureDef right_wall_fixture;
    right_wall_fixture.shape = &right_wall_shape;
    right_wall_fixture.restitution = 0.8f;  // Bounce factor
    right_wall_fixture.friction = 0.2f;     // Low friction for better bouncing
    right_wall_body->CreateFixture(&right_wall_fixture);

    // Create visual representation of walls
    sf::RectangleShape left_wall_visual(sf::Vector2f(5, 600));
    left_wall_visual.setPosition(0, 0);
    left_wall_visual.setFillColor(sf::Color(150, 150, 150)); // Gray

    sf::RectangleShape right_wall_visual(sf::Vector2f(5, 600));
    right_wall_visual.setPosition(795, 0);
    right_wall_visual.setFillColor(sf::Color(150, 150, 150)); // Gray

    // Setup game instructions text
    sf::Font font;
    sf::Text instructions;
    instructions.setCharacterSize(20);
    instructions.setFillColor(sf::Color::White);
    instructions.setPosition(10, 10);
    instructions.setString("Left/Right arrows: Move bottom piston\nSpace: Shoot\nR: Reset box");

    sf::Clock clock;
    bool spacePressed = false;

    // Main game loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // Reset box with random orientation
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        {
            float random_angle = angle_dist(gen);
            box.reset(random_angle);
        }

        // Handle bottom piston horizontal movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            bottom_piston.move(-TPiston::PISTON_SPEED);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            bottom_piston.move(TPiston::PISTON_SPEED);
        }

        // Handle shooting with the space bar
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            if (!spacePressed)
            {
                bottom_piston.shoot(true);  // Move piston up to hit the box
                spacePressed = true;
            }
        }
        else
        {
            if (spacePressed)
            {
                bottom_piston.shoot(false); // Return piston to normal position
                spacePressed = false;
            }
        }

        // Simple AI for the top piston - follows the box horizontally
        b2Vec2 box_pos = box.m_body->GetPosition();
        b2Vec2 top_pos = top_piston.getPosition();
        float diff = (box_pos.x - top_pos.x) * SCALE;
        if (diff > 10)  // Box is to the right
        {
            top_piston.move(TPiston::PISTON_SPEED * 0.8f);  // Move right (slower than player)
        }
        else if (diff < -10)  // Box is to the left
        {
            top_piston.move(-TPiston::PISTON_SPEED * 0.8f);  // Move left (slower than player)
        }

        // Top piston hits the box when it's close enough
        if (std::abs(box_pos.y - top_pos.y) < 0.5f && std::abs(diff) < 80)
        {
            b2Vec2 impulse(0, TPiston::PISTON_FORCE * 0.8f);
            box.m_body->ApplyLinearImpulseToCenter(impulse, true);
        }

        // Run physics simulation step
        world.Step(1.0f / 60.0f, 8, 3);

        // Reset box with random orientation if it goes out of bounds
        if (box.isOutOfBounds())
        {
            float random_angle = angle_dist(gen);
            box.reset(random_angle);
        }

        // Update all game objects
        bottom_piston.update();
        top_piston.update();
        box.update();

        // Render the scene
        window.clear(sf::Color(30, 30, 40));  // Dark blue-gray background
        window.draw(left_wall_visual);
        window.draw(right_wall_visual);
        bottom_piston.draw(window);
        top_piston.draw(window);
        box.draw(window);
        window.draw(instructions);
        window.display();
    }

    return EXIT_SUCCESS;
}