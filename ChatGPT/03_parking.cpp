/*

 Ecris le code C++ d'un simulateur de voiture circulant dans un parking. Utilise
 la librairie SFML pour l’affichage et la librairie units pour les unités SI. Le
 parking est généré aléatoirement. Le parking a des places vides ou occupées. Le
 parking a des voies de circulation pour accéder aux places. Les voies ont une
 longueur, une largeur et un sens de circulation. Les places de parking ont une
 longueur et une largeur et sont faites pour se garer perpendiculairement. Les
 voitures sont régies par l’équation de la cinématique et par un angle de
 braquage. Une voiture autonome entre par l’entrée du parking et doit se garer.

- Bugs et correctifs humains:

   - ChatGPT utilise une vieille version de la librairie units https://github.com/nholthaus/units

   - ChatGPT n'a pas cree les voies de circulation. Donc la spec n'a pas ete
     respectee.

 */

#include <SFML/Graphics.hpp>
#include "units/units.hpp"
#include <random>

using namespace units::literals;
using namespace units::length;
using namespace units::velocity;
using namespace units::time;
using namespace units::angle;
using namespace units::angular_velocity;

// Définition de la classe Parking
class Parking {
public:
    // Constructeur du parking avec une largeur et une longueur données en entrée
    Parking(meter_t width, meter_t length) : width_(width), length_(length) {
        // Génération aléatoire du parking avec des places vides et occupées
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        for (meter_t x = 0_m; x < width_; x += kPlaceWidth) {
            for (meter_t y = 0_m; y < length_; y += kPlaceLength) {
                bool is_occupied = dis(gen);
                parking_spaces_.emplace_back(x, y, kPlaceWidth, kPlaceLength, is_occupied);
            }
        }
    }

    // Méthode qui dessine le parking avec la librairie SFML
    void Draw(sf::RenderWindow& window) const {
        // Dessin des places de parking
        for (const auto& space : parking_spaces_) {
            space.Draw(window);
        }
    }

private:
    // Définition de la classe Place de parking
    class ParkingSpace
    {
    public:
        // Constructeur d'une place de parking avec des coordonnées x et y, une largeur et une longueur, et un état occupé ou non
        ParkingSpace(meter_t x, meter_t y, meter_t width, meter_t length, bool is_occupied)
            : x_(x), y_(y), width_(width), length_(length), is_occupied_(is_occupied)
        {}

        // Méthode qui dessine une place de parking avec la librairie SFML
        void Draw(sf::RenderWindow& window) const
        {
            sf::RectangleShape rectangle;
            rectangle.setSize(sf::Vector2f(static_cast<float>(width_), static_cast<float>(length_)));
            rectangle.setPosition(static_cast<float>(x_), static_cast<float>(y_));
            if (is_occupied_) {
                rectangle.setFillColor(sf::Color::Red);
            } else {
                rectangle.setFillColor(sf::Color::Green);
            }
            window.draw(rectangle);
        }

    private:
        meter_t x_;
        meter_t y_;
        meter_t width_;
        meter_t length_;
        bool is_occupied_;
    };

    // Constantes pour la largeur et la longueur des places de parking
    static constexpr meter_t kPlaceWidth = 5_m;
    static constexpr meter_t kPlaceLength = 5_m;

    meter_t width_;
    meter_t length_;
    std::vector<ParkingSpace> parking_spaces_;
};

// Définition de la classe Voiture
class Car
{
public:
    // Constructeur de la voiture avec une position, une vitesse et un angle de braquage donnés en entrée
    Car(meter_t x, meter_t y, meters_per_second_t v, degree_t steering_angle)
        : x_(x), y_(y), v_(v), steering_angle_(steering_angle)
    {}

    // Méthode qui met à jour la position de la voiture en utilisant l'équation de la cinématique et l'angle de braquage
    void UpdatePosition(second_t dt) {
        x_ += v_ * dt * units::math::cos(steering_angle_);
        y_ += v_ * dt * units::math::sin(steering_angle_);
    }

    // Méthode qui dessine la voiture avec la librairie SFML
    void Draw(sf::RenderWindow& window) const {
        sf::CircleShape circle;
        circle.setRadius(1.0f);
        circle.setPosition(static_cast<float>(x_), static_cast<float>(y_));
        circle.setFillColor(sf::Color::Blue);
        window.draw(circle);
    }

private:
    meter_t x_;
    meter_t y_;
    meters_per_second_t v_;
    degree_t steering_angle_;
};

// g++ --std=c++17 -Wall -Wextra 03_parking.cpp -Iunits/include `pkg-config sfml-graphics --cflags --libs` -o prog
int main() {
    // Création de la fenêtre de simulation avec une largeur et une longueur données
    const meter_t kWindowWidth = 100_m;
    const meter_t kWindowLength = 100_m;
    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned>(kWindowWidth), static_cast<unsigned>(kWindowLength)), "Simulateur de voiture dans un parking");

    // Création du parking avec une largeur et une longueur données
    const meter_t kParkingWidth = 50_m;
    const meter_t kParkingLength = 50_m;

    Parking parking(kParkingWidth, kParkingLength);

    // Création de la voiture avec une position, une vitesse et un angle de braquage donnés en entrée
    const meter_t kCarX = 0_m;
    const meter_t kCarY = 0_m;
    const meters_per_second_t kCarV = 10_mps;
    const degree_t kCarSteeringAngle = 0_deg;
    Car car(kCarX, kCarY, kCarV, kCarSteeringAngle);

    // Boucle principale de la simulation
    while (window.isOpen()) {
        // Gestion des événements
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Mise à jour de la position de la voiture
        car.UpdatePosition(0.1_s);

        // Dessin du parking et de la voiture
        window.clear();
        parking.Draw(window);
        car.Draw(window);
        window.display();
    }

    return 0;
}
