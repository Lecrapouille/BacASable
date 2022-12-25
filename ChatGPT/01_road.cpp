#include <iostream>
#include <vector>

const int NUM_LANES = 2;

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

struct Lane : public Rectangle {
  LaneDirection direction;
};

struct Road {
  std::vector<Lane> lanes;
};

struct Car {
  int x;
  int y;
};

void initializeCar(Car& car, const Lane& lane, int xPercent, int yPercent) {
  car.x = lane.x + (lane.width * xPercent / 100);
  car.y = lane.y + (lane.height * yPercent / 100);
}

int main() {
  Road road;

  // Initialise les voies de la route
  int laneX = 0;
  for (int i = 0; i < NUM_LANES; i++) {
    Lane lane;
    lane.x = laneX;
    lane.y = 0;
    lane.width = 3;
    lane.height = 100;
    lane.direction = (i % 2 == 0) ? LaneDirection::FORWARD : LaneDirection::BACKWARD;
    road.lanes.push_back(lane);
    laneX += lane.width;
  }

  // Initialise les voitures sur la route
  std::vector<Car> cars;
  for (int i = 0; i < NUM_LANES; i++) {
    Car car;
    initializeCar(car, road.lanes[i], 25, 75);
    cars.push_back(car);
  }

  return 0;
}
