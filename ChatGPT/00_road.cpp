/*

 Ecris le code en C++ d’un simulateur de voitures. Les voitures circulent sur
 des routes. Les routes ont plusieurs voies. Les voies ont un sens de
 circulation. Les voitures sont regies par l’equation de la cinématique. Les
 voitures connaissent la voiture de devant et de derriere. La position initiale
 des voitures est relative a la position de la voie.

- Correctifs humains:
  - Road::GetSurroundingCars() code douteux utilisation de pointeurs sur
    variables locales. Code non modifie.

  - Prevent segfault accessing to dummy container.

  - Code retourne toujours:
    Voiture devant: 0 m, vitesse: 0 m/s
    Voiture derrière: 0 m, vitesse: 0 m/s
 */

#include <iostream>
#include <vector>

// Classe représentant une voiture
class Car {
public:
    // Constructeur prenant en entrée la position initiale de la voiture et la vitesse
    Car(double x, double v) : position_(x), velocity_(v) {}

    // Accesseurs pour la position et la vitesse de la voiture
    double position() const { return position_; }
    double velocity() const { return velocity_; }

    // Méthode pour mettre à jour la position de la voiture en fonction du temps
    void UpdatePosition(double time) {
        position_ = position_ + velocity_ * time;
    }

private:
    double position_; // position de la voiture (en mètres)
    double velocity_; // vitesse de la voiture (en mètres par seconde)
};

// Classe représentant une voie de circulation
class Lane {
public:
    // Constructeur prenant en entrée le nombre de voitures sur la voie et la longueur de la voie
    Lane(int num_cars, double length) : length_(length) {
        cars_.reserve(num_cars);
    }

    // Accesseur pour la longueur de la voie
    double length() const { return length_; }

    // Méthode pour ajouter une voiture à la fin de la voie
    void AddCar(const Car& car) {
        cars_.push_back(car);
    }

    // Méthode pour mettre à jour la position de toutes les voitures sur la voie
    void UpdateCars(double time) {
        for (auto& car : cars_) {
            car.UpdatePosition(time);
        }
    }

    // Méthode pour obtenir la voiture devant et derrière une voiture donnée sur cette voie
    void GetSurroundingCars(const Car& car, Car* front, Car* back) const {
        if (cars_.empty())
            return ;

        for (size_t i = 0; i < cars_.size(); ++i) {
            if (cars_[i].position() > car.position()) {
                *front = cars_[i];
                if (i > 0) {
                    *back = cars_[i - 1];
                }
                return;
            }
        }
        *back = cars_.back();
    }

private:
    double length_; // longueur de la voie (en mètres)
    std::vector<Car> cars_; // vecteur contenant les voitures sur cette voie
};

// Classe représentant une route avec plusieurs voies
class Road {
public:
    // Constructeur prenant en entrée le nombre de voies sur la route
    Road(int num_lanes) {
        lanes_.reserve(num_lanes);
    }

    // Méthode pour ajouter une voie à la route
    void AddLane(const Lane& lane) {
        lanes_.push_back(lane);
    }

    // Méthode pour mettre à jour la position de toutes les voitures sur la route
    void UpdateCars(double time) {
        for (auto& lane : lanes_) {
            lane.UpdateCars(time);
        }
    }

    // Méthode pour obtenir la voiture devant et derrière une voiture donnée sur la route
    void GetSurroundingCars(const Car& car, Car* front, Car* back) const {
        Car front_candidate(0, 0);
        Car back_candidate(0, 0);
        *front = Car(0, 0);
        *back = Car(0, 0);
        for (const auto& lane : lanes_) {
            lane.GetSurroundingCars(car, &front_candidate, &back_candidate);
            if (front_candidate.position() > front->position()) {
                *front = front_candidate;
            }
            if (back_candidate.position() < back->position()) {
                *back = back_candidate;
            }
        }
    }

private:
    std::vector<Lane> lanes_; // vecteur contenant les voies sur cette route
};

//  g++ --std=c++17 -Wall -Wextra 00_road.cpp `pkg-config sfml-graphics --cflags --libs` -o road
int main() {
    // Création d'une route avec deux voies
    Road road(2);

    // Création de deux voies pour la route
    Lane lane1(5, 100); // voie de longueur 100 mètres avec 5 voitures
    Lane lane2(3, 100); // voie de longueur 100 mètres avec 3 voitures

    // Ajout des voies à la route
    road.AddLane(lane1);
    road.AddLane(lane2);

    // Ajout de voitures aux voies
    lane1.AddCar(Car(0, 50));   // position initiale 0 m, vitesse 50 m/s
    lane1.AddCar(Car(10, 60));  // position initiale 10 m, vitesse 60 m/s
    lane1.AddCar(Car(20, 70));  // position initiale 20 m, vitesse 70 m/s
    lane1.AddCar(Car(30, 80));  // position initiale 30 m, vitesse 80 m/s
    lane1.AddCar(Car(40, 90));  // position initiale 40 m, vitesse 90 m/s
    lane2.AddCar(Car(0, 40));   // position initiale 0 m, vitesse 40 m/s
    lane2.AddCar(Car(15, 50));  // position initiale 15 m, vitesse 50 m/s
    lane2.AddCar(Car(30, 60));  // position initiale 30 m, vitesse 60 m/s

    // Mise à jour de la position des voitures sur la route
    road.UpdateCars(10); //

    // Exemple d'utilisation de la méthode GetSurroundingCars
    Car front(0,0); Car back(0,0);
    road.GetSurroundingCars(Car(5, 0), &front, &back);
    std::cout << "Voiture devant: " << front.position() << " m, vitesse: " << front.velocity() << " m/s" << std::endl;
    std::cout << "Voiture derrière: " << back.position() << " m, vitesse: " << back.velocity() << " m/s" << std::endl;

    return 0;
}
