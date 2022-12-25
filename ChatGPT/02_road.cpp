

#include <iostream>
#include <vector>
#include <random>

const int NUM_LANES = 2;
const int MAX_CARS_PER_LANE = 10;

enum class LaneDirection {
    FORWARD,
    BACKWARD
};

struct Rectangle {
    int x;
    int y;
    int width;
    int height;
};

struct Lane : Rectangle {
    LaneDirection direction;
};

struct Road {
    std::vector<Lane> lanes;
};

struct Car : Rectangle {
  const Lane* lane;
};

std::mt19937 rng;

int generateRandomPercent(int minPercent, int maxPercent) {
    std::uniform_int_distribution<int> dist(minPercent, maxPercent);
    return dist(rng);
}

bool isOverlapping(const Car& car1, const Car& car2) {
    // Si les rectangles se chevauchent, alors les voitures se chevauchent également
    return car1.x < car2.x + car2.width && car1.x + car1.width > car2.x &&
            car1.y < car2.y + car2.height && car1.y + car1.height > car2.y;
}

void initializeCar(Car& car, const Lane& lane, int xPercent, int yPercent) {
    car.x = lane.x + (lane.width * xPercent / 100);
    car.y = lane.y + (lane.height * yPercent / 100);
    car.width = 2;
    car.height = 5;
    car.lane = &lane;
}

void printCarPositions(const std::vector<Car>& cars, const std::vector<Lane>& lanes)
{
  for (int i = 0; i < cars.size(); i++) {
    const Car& car = cars[i];
    const Lane& lane = lanes[i / MAX_CARS_PER_LANE];

    std::cout << "Voiture " << i << ": voie " << i / MAX_CARS_PER_LANE
              << ", position (" << car.x << ", " << car.y << ")"
              << std::endl;
  }
}

int main() {
    rng.seed(std::random_device{}());

    Road road;

    // Initialise les voies de la route
    road.lanes.resize(NUM_LANES);
    int laneX = 0;
    for (int i = 0; i < NUM_LANES; i++) {
        Lane& lane = road.lanes[i];
        lane.x = laneX;
        lane.y = 0;
        lane.width = 3;
        lane.height = 100;
        lane.direction = (i % 2 == 0) ? LaneDirection::FORWARD : LaneDirection::BACKWARD;
        laneX += lane.width;
    }

    // Initialise les voitures sur les voies
    std::vector<Car> cars;
    for (const Lane& lane : road.lanes) {
        int numCars = generateRandomPercent(0, MAX_CARS_PER_LANE);
        for (int i = 0; i < numCars; i++) {
            Car car;
            while (true) {
                // Génère des positions aléatoires jusqu'à ce qu'une position valide soit trouvée
                int xPercent = generateRandomPercent(0, 100);
                int yPercent = generateRandomPercent(0, 100);
                initializeCar(car, lane, xPercent, yPercent);

                bool isValidPosition = true;
                for (const Car& otherCar : cars) {
                    if (isOverlapping(car, otherCar)) {
                        isValidPosition = false;
                        break;
                    }
                }

                if (isValidPosition) {
                    break;
                }
            }

            cars.push_back(car);
        }
    }

    // Affiche l'état de chaque voiture
    for (int i = 0; i < cars.size(); i++) {
        const Car& car = cars[i];
        std::cout << "Voiture " << i << ": voie "
                  << car.lane - road.lanes.data()
                  << ", position (" << car.x << ", " << car.y << ")"
                  << std::endl;
    }


    return 0;
}
