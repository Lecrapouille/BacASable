/*

  Ecris le code C++ d'un simulateur de voiture circulant dans un parking. Le
  parking est généré aléatoirement. Le parking a des places vides ou
  occupées. Le parking a des voies de circulation pour accéder aux places. Les
  voies ont une longueur, une largeur et un sens de circulation. Les places de
  parking ont une longueur et une largeur et sont faites pour se garer
  perpendiculairement. Les voitures sont regies par l’equation de la cinématique
  et par un angle de braquage. Une voiture autonome entre dans le parking et
  veut se garer.

- Bugs et correctifs humains:
   - chatGPT ecrit ParkingSpot[][] spots comme param aux fonctions. Ce n'est
     pas valide en C++. J'ai mis en dur la taille des tableaux.
   - Suppression des new qui ne compilait pas. De plus il y avait une fuite
     memoire.
   - Il y a un segfault en quittant le programme.

*/

#include <iostream>
#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>

const int PARKING_LENGTH = 100; // longueur du parking
const int PARKING_WIDTH = 50; // largeur du parking
const int SPOT_LENGTH = 6; // longueur d'une place de parking
const int SPOT_WIDTH = 4; // largeur d'une place de parking
const int LANE_LENGTH = 8; // longueur d'une voie de circulation
const int LANE_WIDTH = 2; // largeur d'une voie de circulation

// Structure de données pour représenter une voiture
struct Car
{
    double x; // position en x de la voiture (en mètres)
    double y; // position en y de la voiture (en mètres)
    double v; // vitesse de la voiture (en mètres/seconde)
    double theta; // angle de direction de la voiture (en radians)
    double steering_angle; // angle de braquage de la voiture (en radians)
};

// Structure de données pour représenter une place de parking
struct ParkingSpot
{
    double x; // position en x de la place de parking (en mètres)
    double y; // position en y de la place de parking (en mètres)
    bool occupied; // indique si la place de parking est occupée ou non
};

// Structure de données pour représenter une voie de circulation
struct Lane
{
    double x; // position en x de la voie de circulation (en mètres)
    double y; // position en y de la voie de circulation (en mètres)
    char direction; // direction de circulation de la voie ('N', 'S', 'E', 'W')
};

// Génère un parking aléatoire avec des places de stationnement et des voies de circulation
void generateParking(ParkingSpot spots[7][8], Lane lanes[7][8])
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    for (int i = 0; i < PARKING_LENGTH; i += LANE_LENGTH + SPOT_LENGTH)
    {
        for (int j = 0; j < PARKING_WIDTH; j += LANE_WIDTH + SPOT_WIDTH)
        {
            if (dis(gen) == 0)
            {
                // Ajoute une place de parking
                ParkingSpot spot;
                spot.x = i + SPOT_LENGTH / 2;
                spot.y = j + SPOT_WIDTH / 2;
                spot.occupied = false;
                spots[i / (LANE_LENGTH + SPOT_LENGTH)][j / (LANE_WIDTH + SPOT_WIDTH)] = spot;
            }
            else
            {
                // Ajoute une voie de circulation
                Lane lane;
                lane.x = i + LANE_LENGTH / 2;
                lane.y = j + LANE_WIDTH / 2;
                if (i == 0) {
                    lane.direction = 'S';
                } else if (i == PARKING_LENGTH - LANE_LENGTH) {
                    lane.direction = 'N';
                } else if (j == 0) {
                    lane.direction = 'E';
                } else {
                    lane.direction = 'W';
                }
                lanes[i / (LANE_LENGTH + SPOT_LENGTH)][j / (LANE_WIDTH + SPOT_WIDTH)] = lane;
            }
        }
    }
}

// Met à jour la position et la vitesse de la voiture en fonction de l'angle de braquage et du temps écoulé
void updateCar(Car &car, double dt)
{
    double vx = car.v * cos(car.theta);
    double vy = car.v * sin(car.theta);
    car.x += vx * dt;
    car.y += vy * dt;
    car.theta += car.steering_angle * dt;
}

// Trouve la place de parking la plus proche de la voiture et la direction à prendre pour s'y rendre
void findNearestParkingSpot(Car const& car, ParkingSpot spots[7][8], double &distance, char &direction)
{
    distance = std::numeric_limits<double>::infinity();
    for (int i = 0; i < PARKING_LENGTH / (LANE_LENGTH + SPOT_LENGTH); i++)
    {
        for (int j = 0; j < PARKING_WIDTH / (LANE_WIDTH + SPOT_WIDTH); j++)
        {
            if (!spots[i][j].occupied)
            {
                // Calcule la distance entre la voiture et la place de parking
                double d = std::sqrt((car.x - spots[i][j].x) * (car.x - spots[i][j].x) +
                                     (car.y - spots[i][j].y) * (car.y - spots[i][j].y));
                if (d < distance)
                {
                    distance = d;
                    // Détermine la direction à prendre pour s'y rendre
                    if (std::abs(car.x - spots[i][j].x) > std::abs(car.y - spots[i][j].y))
                    {
                        if (car.x < spots[i][j].x) {
                            direction = 'E';
                        } else {
                            direction = 'W';
                        }
                    } else {
                        if (car.y < spots[i][j].y) {
                            direction = 'N';
                        } else {
                            direction = 'S';
                        }
                    }
                }
            }
        }
    }
}

// Dessine la voiture sur la fenêtre de dessin
void drawCar(sf::RenderWindow &window, Car const& car)
{
    sf::CircleShape shape(2.0);
    shape.setPosition(car.x - 2, car.y - 2);
    window.draw(shape);
}

// Dessine le parking avec des places de stationnement et des voies de circulation
void drawParking(sf::RenderWindow &window, ParkingSpot spots[7][8], Lane lanes[7][8])
{
    sf::RectangleShape spotShape(sf::Vector2f(SPOT_LENGTH, SPOT_WIDTH));
    sf::RectangleShape laneShape(sf::Vector2f(LANE_LENGTH, LANE_WIDTH));
    for (int i = 0; i < PARKING_LENGTH / (LANE_LENGTH + SPOT_LENGTH); i++)
    {
        for (int j = 0; j < PARKING_WIDTH / (LANE_WIDTH + SPOT_WIDTH); j++)
        {
            double x = i * (LANE_LENGTH + SPOT_LENGTH);
            double y = j * (LANE_WIDTH + SPOT_WIDTH);
            if (spots[i][j].occupied) {
                spotShape.setFillColor(sf::Color::Red);
            } else {
                spotShape.setFillColor(sf::Color::Green);
            }
            spotShape.setPosition(x, y);
            window.draw(spotShape);
            if (lanes[i][j].direction == 'N') {
                laneShape.setRotation(0);
            } else if (lanes[i][j].direction == 'S') {
                laneShape.setRotation(180);
            } else if (lanes[i][j].direction == 'E') {
                laneShape.setRotation(90);
            } else {
                laneShape.setRotation(270);
            }
            laneShape.setPosition(x, y);
            window.draw(laneShape);
        }
    }
}

// g++ --std=c++17 -Wall -Wextra parking.cpp `pkg-config sfml-graphics --cflags --libs` -o prog
int main()
{
    // Crée une fenêtre de dessin et génère le parking
    sf::RenderWindow window(sf::VideoMode(PARKING_LENGTH, PARKING_WIDTH), "Parking Simulator");

    // Génère le parking et place la voiture au milieu de la voie de circulation en bas
    ParkingSpot spots[PARKING_LENGTH / (LANE_LENGTH + SPOT_LENGTH)][PARKING_WIDTH / (LANE_WIDTH + SPOT_WIDTH)];
    Lane lanes[PARKING_LENGTH / (LANE_LENGTH + SPOT_LENGTH)][PARKING_WIDTH / (LANE_WIDTH + SPOT_WIDTH)];
    generateParking(spots, lanes);

    Car car;
    car.x = LANE_LENGTH / 2;
    car.y = PARKING_WIDTH - LANE_WIDTH / 2;
    car.v = 1; // La voiture roule à 1 m/s
    car.theta = M_PI; // La voiture regarde vers le sud
    car.steering_angle = 0; // La voiture ne braque pas au départ

    // Simule la voiture en mouvement jusqu'à ce qu'elle trouve une place de parking vide
    double dt = 0.1; // intervalle de temps entre chaque itération

    // Dessine le parking en boucle jusqu'à ce que la fenêtre soit fermée
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Trouve la place de parking la plus proche et la direction à prendre pour s'y rendre
        double distance;
        char direction;

        findNearestParkingSpot(car, spots, distance, direction);
        // Si la voiture est déjà sur une place de parking vide, arrête la simulation
        if (distance != 0)
        {
            // Modifie l'angle de braquage de la voiture pour s'orienter dans la direction de la place de parking
            if (direction == 'N') {
                car.steering_angle = M_PI / 2;
            } else if (direction == 'S') {
                car.steering_angle = -M_PI / 2;
            } else if (direction == 'E') {
                car.steering_angle = 0;
            } else {
                car.steering_angle = M_PI;
            }

            // Met à jour la position et la vitesse de la voiture
            updateCar(car, dt);
        }

        window.clear();
        drawParking(window, spots, lanes);
        drawCar(window, car);
        window.display();
    }

    return 0;
}
