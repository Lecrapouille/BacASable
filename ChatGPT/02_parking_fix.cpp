/*

Suite de 00_parking.cpp demande de re-ecriture car l'ancien code fait un
segfaut.

- Iteration 1:

  Ecris le code C++ d'un simulateur de voiture circulant dans un parking. Le
  parking est généré aléatoirement. Le parking a des places vides ou
  occupées. Le parking a des voies de circulation pour accéder aux places. Les
  voies ont une longueur, une largeur et un sens de circulation. Les places de
  parking ont une longueur et une largeur et sont faites pour se garer
  perpendiculairement. Les voitures sont regies par l’equation de la cinématique
  et par un angle de braquage. Une voiture autonome entre dans le parking et
  veut se garer.


- Iteration 2:

  Ecris une fonction qui permet de dessiner le parking avec la librairie SFML.

- Iteration 3:

  Reecris tout le code en renplacant les tableaux [][] par std::vector et new
  par des std::make_unique

- Bugs et correctifs humains:
  - drawCar() remplacement de CircleShape par une forme rectangulaire. Ajout de
    la rotation du rectangle manquqnt.
  - main() la condition du if (car.v > 0) etait avec le while (window.isOpen())
    ce qui faisait quitter tout de suite le programme.
  - Remplace int par size_t concernant container::size() sinon ca genere des warnings.
  - 02_parking_fix.cpp:140:67: warning: unused parameter 'lanes' [-Wunused-parameter]
    non corrige.
  - La voiture va on ne sait ou. et les voies de circulation sont bizzarement placees.
 */

#include <iostream>
#include <cmath>
#include <limits>
#include <SFML/Graphics.hpp>

const int PARKING_LENGTH = 800;
const int PARKING_WIDTH = 600;
const int LANE_LENGTH = 100;
const int LANE_WIDTH = 20;
const int SPOT_LENGTH = 50;
const int SPOT_WIDTH = 30;

struct Car {
    double x;
    double y;
    double v;
    double theta;
    double steering_angle;
};

struct ParkingSpot {
    double x;
    double y;
    bool occupied;
};

struct Lane {
    double x;
    double y;
    char direction;
};

// Génère aléatoirement le parking avec des places de stationnement et des voies
// de circulation
void generateParking(std::vector<std::vector<std::unique_ptr<ParkingSpot>>> &spots,
                     std::vector<std::vector<std::unique_ptr<Lane>>> &lanes) {
    for (int i = 0; i < PARKING_LENGTH / (LANE_LENGTH + SPOT_LENGTH); i++) {
        std::vector<std::unique_ptr<ParkingSpot>> spotRow;
        std::vector<std::unique_ptr<Lane>> laneRow;
        for (int j = 0; j < PARKING_WIDTH / (LANE_WIDTH + SPOT_WIDTH); j++) {
            double x = i * (LANE_LENGTH + SPOT_LENGTH);
            double y = j * (LANE_WIDTH + SPOT_WIDTH);
            std::unique_ptr<ParkingSpot> spot = std::make_unique<ParkingSpot>();
            spot->x = x;
            spot->y = y;
            spot->occupied = (rand() % 2 == 0);
            std::unique_ptr<Lane> lane = std::make_unique<Lane>();
            lane->x = x;
            lane->y = y;
            if (i % 2 == 0) {
                lane->direction = 'E';
            } else {
                lane->direction = 'W';
            }
            if (j % 2 == 0) {
                lane->direction = 'N';
            } else {
                lane->direction = 'S';
            }
            spotRow.push_back(std::move(spot));
            laneRow.push_back(std::move(lane));
        }
        spots.push_back(std::move(spotRow));
        lanes.push_back(std::move(laneRow));
    }
}

// Met à jour la position et la vitesse de la voiture en fonction de l'angle de braquage et du temps écoulé
void updateCar(Car &car, double dt) {
    double vx = car.v * std::cos(car.theta);
    double vy = car.v * std::sin(car.theta);
    double omega = car.v / LANE_LENGTH * std::tan(car.steering_angle);
    car.x += vx * dt;
    car.y += vy * dt;
    car.theta += omega * dt;
}

// Trouve la place de parking la plus proche et la direction à prendre pour s'y rendre
void findNearestParkingSpot(Car car, std::vector<std::vector<std::unique_ptr<ParkingSpot>>> &spots, double &distance, char &direction) {
    distance = std::numeric_limits<double>::max();
    for (size_t i = 0; i < spots.size(); i++) {
        for (size_t j = 0; j < spots[i].size(); j++) {
            if (!spots[i][j]->occupied) {
                double d = std::sqrt((car.x - spots[i][j]->x) * (car.x - spots[i][j]->x) + (car.y - spots[i][j]->y) * (car.y - spots[i][j]->y));
                if (d < distance) {
                    distance = d;
                    if (car.x < spots[i][j]->x) {
                        direction = 'E';
                    } else {
                        direction = 'W';
                    }
                    if (car.y < spots[i][j]->y) {
                        direction = 'N';
                    } else {
                        direction = 'S';
                    }
                }
            }
        }
    }
}

// Dessine le parking avec des places de stationnement et des voies de circulation
void drawParking(sf::RenderWindow &window,
                 std::vector<std::vector<std::unique_ptr<ParkingSpot>>> &spots,
                 std::vector<std::vector<std::unique_ptr<Lane>>> &lanes) {
    sf::RectangleShape spotShape(sf::Vector2f(SPOT_LENGTH, SPOT_WIDTH));
    sf::RectangleShape laneShape(sf::Vector2f(LANE_LENGTH, LANE_WIDTH));
    for (int i = 0; i < PARKING_LENGTH / (LANE_LENGTH + SPOT_LENGTH); i++) {
        for (int j = 0; j < PARKING_WIDTH / (LANE_WIDTH + SPOT_WIDTH); j++) {
            double x = i * (LANE_LENGTH + SPOT_LENGTH);
            double y = j * (LANE_WIDTH + SPOT_WIDTH);
            if (spots[i][j]->occupied) {
                spotShape.setFillColor(sf::Color::Red);
            } else {
                spotShape.setFillColor(sf::Color::Green);
            }
            spotShape.setPosition(x, y);
            window.draw(spotShape);
            laneShape.setPosition(x, y);
            window.draw(laneShape);
        }
    }
}

// Dessine la voiture sur la fenêtre de dessin
void drawCar(sf::RenderWindow &window, Car car) {
    sf::RectangleShape shape(sf::Vector2f(50.0, 20.0));
    shape.setPosition(car.x - 2, car.y - 2);
    shape.setRotation(car.theta * 57.295779513f);
    shape.setFillColor(sf::Color::Yellow);
    window.draw(shape);
}

//  g++ --std=c++17 -Wall -Wextra 02_parking_fix.cpp `pkg-config sfml-graphics --cflags --libs` -o parking
int main()
{
    // Crée une fenêtre de dessin et génère le parking
    sf::RenderWindow window(sf::VideoMode(PARKING_LENGTH, PARKING_WIDTH), "Parking Simulator");
    std::vector<std::vector<std::unique_ptr<ParkingSpot>>> spots;
    std::vector<std::vector<std::unique_ptr<Lane>>> lanes;
    generateParking(spots, lanes);

    // Crée une voiture et la place au début du parking
    Car car;
    car.x = 0;
    car.y = 0;
    car.v = 1; // la voiture avance à une vitesse constante
    car.theta = 0;
    car.steering_angle = 0;

    // Boucle jusqu'à ce que la fenêtre soit fermée ou que la voiture ait trouvé une place de parking
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (car.v > 0)
        {
            // Trouve la place de parking la plus proche et la direction à prendre pour s'y rendre
            double distance;
            char direction;
            findNearestParkingSpot(car, spots, distance, direction);

            std::cout << direction << std::endl;
            // Met à jour l'angle de braquage de la voiture en fonction de la direction à prendre
            if (direction == 'E') {
                car.steering_angle = std::atan2(car.y, distance);
                //std::cout << "E" << std::endl;
            } else if (direction == 'W') {
                car.steering_angle = std::atan2(car.y, -distance);
                //std::cout << "W" << std::endl;
            } else if (direction == 'N') {
                car.steering_angle = std::atan2(distance, car.x);
                //std::cout << "N" << std::endl;
            } else {
                car.steering_angle = std::atan2(-distance, car.x);
                //std::cout << "S" << std::endl;
            }

            // met à jour la position et la vitesse de la voiture en fonction de l'angle de braquage et du temps écoulé
            updateCar(car, 0.1);
        }


        // Dessine le parking et la voiture
        window.clear();
        drawParking(window, spots, lanes);
        drawCar(window, car);
        window.display();
    }

    return 0;
}
