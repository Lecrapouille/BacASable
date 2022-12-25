#include <SFML/Graphics.hpp>
#include <random>
#include <cmath>

// Constantes de la simulation
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PARKING_WIDTH = 600;
const int PARKING_HEIGHT = 400;
const int ROAD_WIDTH = 50;
const int CAR_LENGTH = 30;
const int CAR_WIDTH = 20;
const int PARKING_SPOT_LENGTH = 50;
const int PARKING_SPOT_WIDTH = 30;
const float FOLLOW_THRESHOLD = 50.0f;

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
    bool direction; // True si la voie est horizontale, false si elle est verticale
};

// Structure de données pour représenter une voiture
struct Car {
    sf::Vector2f position;
    float velocity;
    float angle;
    float steer_angle;
};

int main() {
    // Initialisation de la fenêtre et du générateur de nombres aléatoires
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Parking Simulator");
    std::mt19937 rng;
    std::uniform_int_distribution<int> uni(0, 1);

    // Génération des places de parking
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
        road.direction = true; // True si la voie est horizontale
        roads.push_back(road);
    }
    for (int x = 0; x < PARKING_WIDTH; x += PARKING_SPOT_LENGTH + ROAD_WIDTH) {
        Road road;
        road.start = sf::Vector2f(x, 0);
        road.end = sf::Vector2f(x, PARKING_HEIGHT);
        road.width = ROAD_WIDTH;
        road.length = PARKING_HEIGHT;
        road.direction = false; // False si la voie est verticale
        roads.push_back(road);
    }

    // Initialisation de la voiture
    Car car;
    car.position = sf::Vector2f(ROAD_WIDTH / 2, ROAD_WIDTH / 2);
    car.velocity = 0.0f;
    car.angle = 0.0f;
    car.steer_angle = 0.0f;

    // Boucle principale de la simulation
    while (window.isOpen()) {
        // Gestion des évènements
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Mise à jour de la simulation
        float delta_time = 1.0f / 60.0f;
        for (const auto& road : roads) {
            float distance = 0.0f;
            if (road.direction) {
                // Voie de circulation horizontale
                distance = std::abs(car.position.y - road.start.y);
                if (distance < FOLLOW_THRESHOLD) {
                    // La voiture doit suivre cette voie
                    if (std::abs(car.position.x - road.start.x) > ROAD_WIDTH / 2) {
                        // La voiture n'est pas sur la voie, elle doit donc y retourner
                        float target_angle = std::atan2(road.start.y - car.position.y, road.start.x - car.position.x);
                        car.steer_angle = target_angle - car.angle;
                        car.steer_angle = std::fmod(car.steer_angle + M_PI, 2 * M_PI) - M_PI;
                        car.angle += car.steer_angle * delta_time;
                        car.velocity = std::min(car.velocity + 1.0f * delta_time, 10.0f);
                    } else {
                        // La voiture est sur la voie, elle peut avancer tout droit
                        car.steer_angle = 0.0f;
                        car.velocity = std::min(car.velocity + 1.0f * delta_time, 50.0f);
                    }
                }
            } else {
                // Voie de circulation verticale
                distance = std::abs(car.position.x - road.start.x);
                if (distance < FOLLOW_THRESHOLD) {
                    // La voiture doit suivre cette voie
                    if (std::abs(car.position.y - road.start.y) > ROAD_WIDTH / 2) {
                        // La voiture n'est pas sur la voie, elle doit donc y retourner
                        float target_angle = std::atan2(road.start.y - car.position.y, road.start.x - car.position.x);
                        car.steer_angle  =target_angle - car.angle;
                        car.steer_angle = std::fmod(car.steer_angle + M_PI, 2 * M_PI) - M_PI;
                        car.angle += car.steer_angle * delta_time;
                        car.velocity = std::min(car.velocity + 1.0f * delta_time, 10.0f);
                    } else {
                        // La voiture est sur la voie, elle peut avancer tout droit
                        car.steer_angle = 0.0f;
                        car.velocity = std::min(car.velocity + 1.0f * delta_time, 50.0f);
                    }
                }
            }
        }

        // Mise à jour de la position de la voiture en fonction de sa vitesse et de son angle
        car.position.x += car.velocity * std::cos(car.angle) * delta_time;
        car.position.y += car.velocity * std::sin(car.angle) * delta_time;

        // Vérification de l'arrivée de la voiture dans une place de parking
        bool parked = false;
        for (auto& spot : parking_spots) {
            if (!spot.occupied && car.position.x >= spot.position.x && car.position.x <= spot.position.x + PARKING_SPOT_LENGTH && car.position.y >= spot.position.y && car.position.y <= spot.position.y + PARKING_SPOT_WIDTH) {
                // La voiture est arrivée dans une place de parking libre, elle s'y gare
                car.velocity = 0.0f;
                car.angle = 0.0f;
                car.steer_angle = 0.0f;
                spot.occupied = true;
                parked = true;
                break;
            }
        }
        if (parked) {
            // La voiture s'est garée, on arrête la simulation
            break;
        }

        // Dessin de la simulation
        window.clear();
        // Dessin des places de parking
        for (const auto& spot : parking_spots) {
            sf::RectangleShape shape(sf::Vector2f(PARKING_SPOT_LENGTH, PARKING_SPOT_WIDTH));
            shape.setPosition(spot.position);
            shape.setFillColor(spot.occupied ? sf::Color::Red : sf::Color::Green);
            window.draw(shape);
        }
        // Dessin des voies de circulation
        for (const auto& road : roads) {
            sf::RectangleShape shape(sf::Vector2f(road.length, road.width));
            shape.setPosition(road.start);
            shape.setFillColor(sf::Color::White);
            window.draw(shape);
        }
        // Dessin de la voiture
        sf::RectangleShape car_shape(sf::Vector2f(CAR_LENGTH, CAR_WIDTH));
        car_shape.setOrigin(CAR_LENGTH / 2, CAR_WIDTH / 2);
        car_shape.setPosition(car.position);
        car_shape.setRotation(car.angle * 180.0f / M_PI);
        car_shape.setFillColor(sf::Color::Blue);
        window.draw(car_shape);
        window.display();
    }

    return 0;
}
