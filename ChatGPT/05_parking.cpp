#include <SFML/Graphics.hpp>
#include <random>
#include <cmath>

// Constantes de la simulation
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PARKING_WIDTH = 600;
const int PARKING_HEIGHT = 400;
const int ROAD_WIDTH = 50;
const int CAR_LENGTH = 50;
const int CAR_WIDTH = 30;
const int PARKING_SPOT_LENGTH = 60;
const int PARKING_SPOT_WIDTH = 80;

// Structure de données pour représenter une place de parking
struct ParkingSpot {
    sf::Vector2f position;
    bool occupied;
};

// Structure de données pour représenter une voie de circulation
struct Road {
    sf::Vector2f start;
    sf::Vector2f end;
    int width;
    int length;
    bool direction; // True si la voie est horizontale, False si la voie est verticale
};

// Structure de données pour représenter une voiture
struct Car {
    sf::Vector2f position;
    float velocity;
    float angle;
    float steer_angle;
};

int main() {
    // Initialisation de la fenêtre de rendu et de l'horloge de SFML
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Parking Simulator");
    sf::Clock clock;

    // Génération aléatoire des places de parking
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0,1);
    std::vector<ParkingSpot> parking_spots;
    for (int y = ROAD_WIDTH; y < PARKING_HEIGHT; y += PARKING_SPOT_WIDTH + ROAD_WIDTH) {
        for (int x = ROAD_WIDTH; x < PARKING_WIDTH; x += PARKING_SPOT_LENGTH + ROAD_WIDTH) {
            ParkingSpot spot;
            spot.position = sf::Vector2f(x, y);
            spot.occupied = (uni(rng) == 1);
            parking_spots.push_back(spot);
        }
    }

    // Génération des voies de circulation
    std::vector<Road> roads;
    for (int y = 0; y < PARKING_HEIGHT; y += PARKING_SPOT_WIDTH + ROAD_WIDTH) {
        Road road;
        road.start = sf::Vector2f(0, y);
        road.end = sf::Vector2f(PARKING_WIDTH, y);
        road.width = ROAD_WIDTH;
        road.length = PARKING_WIDTH;
        road.direction = true;
        roads.push_back(road);
    }
    for (int x = 0; x < PARKING_WIDTH; x += PARKING_SPOT_LENGTH + ROAD_WIDTH) {
        Road road;
        road.start = sf::Vector2f(x, 0);
        road.end = sf::Vector2f(x, PARKING_HEIGHT);
        road.width = ROAD_WIDTH;
        road.length = PARKING_HEIGHT;
        road.direction = false;
        roads.push_back(road);
    }

    // Création de la voiture et initialisation de sa position et de sa vitesse
                                                                                      Car car;
    car.position = sf::Vector2f(ROAD_WIDTH, ROAD_WIDTH);
    car.velocity = 0.0f;
    car.angle = 0.0f;
    car.steer_angle = 0.0f;

    // Boucle principale de la simulation
    while (window.isOpen()) {
        // Gestion des événements de la fenêtre
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Mise à jour de la simulation
        float delta_time = clock.restart().asSeconds();
        car.velocity += delta_time;
        car.angle += car.steer_angle * delta_time;
        car.position.x += std::cos(car.angle) * car.velocity * delta_time;
        car.position.y += std::sin(car.angle) * car.velocity * delta_time;

        // Dessin de la simulation
        window.clear(sf::Color::White);
        for (const auto& road : roads) {
            sf::RectangleShape shape;
            shape.setSize(sf::Vector2f(road.width, road.length));
            shape.setPosition(road.start);
            shape.setFillColor(sf::Color::Black);
            window.draw(shape);
        }
        for (const auto& spot : parking_spots) {
            sf::RectangleShape shape;
            shape.setSize(sf::Vector2f(PARKING_SPOT_LENGTH, PARKING_SPOT_WIDTH));
            shape.setPosition(spot.position);
            if (spot.occupied) {
                shape.setFillColor(sf::Color::Red);
            } else {
                shape.setFillColor(sf::Color::Green);
            }
            window.draw(shape);
        }
        sf::RectangleShape car_shape;
        car_shape.setSize(sf::Vector2f(CAR_LENGTH, CAR_WIDTH));
        car_shape.setPosition(car.position);
        car_shape.setFillColor(sf::Color::Blue);
        car_shape.setOrigin(CAR_LENGTH / 2, CAR_WIDTH / 2);
        car_shape.setRotation(car.angle * 180.0f / 3.14159f);
        window.draw(car_shape);
        window.display();
    }

    return 0;
}
