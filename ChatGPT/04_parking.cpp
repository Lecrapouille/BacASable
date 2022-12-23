/*

Comme pour 03_parking.cpp la spec n'a pas ete respectee.

 */

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Classe représentant une voiture
class Car {
private:
    sf::RectangleShape shape; // Forme de la voiture
    float speed; // Vitesse de la voiture
    float steering; // Angle de braquage de la voiture

public:
    Car(float x, float y) {
        shape.setSize(sf::Vector2f(50, 30));
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Red);
        speed = 0.0f;
        steering = 0.0f;
    }

    // Met à jour la position de la voiture en utilisant l'équation de cinématique
    void update(float dt) {
        float x = shape.getPosition().x;
        float y = shape.getPosition().y;
        float newX = x + speed * dt * std::cos(steering);
        float newY = y + speed * dt * std::sin(steering);
        shape.setPosition(newX, newY);
    }

    // Dessine la voiture sur la fenêtre de rendu
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    // Modifie la vitesse de la voiture
    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }

    // Modifie l'angle de braquage de la voiture
    void setSteering(float newSteering) {
        steering = newSteering;
    }
};

int main() {
    srand(time(0)); // Initialisation du générateur de nombres aléatoires

    // Création de la fenêtre de rendu
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Parking Simulator");

    // Création d'un tableau de places de parking
    const int NUM_SPACES = 50;
    bool spaces[NUM_SPACES];
    for (int i = 0; i < NUM_SPACES; i++) {
        spaces[i] = (rand() % 2 == 0); // Génère aléatoirement si la place est occupée ou non
    }

    // Création de la voiture
    Car car(10, 10);

    // Boucle principale
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Mise à jour de la voiture
        car.update(1.0f / 60.0f); // 1/60

        // Dessin du parking
        for (int i = 0; i < NUM_SPACES; i++) {
            if (spaces[i]) {
                // Place occupée, dessin d'un rectangle noir
                sf::RectangleShape space(sf::Vector2f(50, 30));
                space.setPosition(100 + i * 60, 10);
                space.setFillColor(sf::Color::Black);
                window.draw(space);
            } else {
                // Place vide, dessin d'un rectangle blanc
                sf::RectangleShape space(sf::Vector2f(50, 30));
                space.setPosition(100 + i * 60, 10);
                space.setFillColor(sf::Color::White);
                window.draw(space);
            }
        }

        // Dessin de la voiture
        car.draw(window);

        // Affichage de la fenêtre de rendu
        window.display();
    }

    return 0;
}
