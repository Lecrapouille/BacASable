#include "GameManager.hpp"
#include <SFML/Graphics/Color.hpp>
#include <random>
#include <iostream>

// ----------------------------------------------------------------------------
void GameManager::createInitialState(GameState& state)
{
    const int NUM_CARS = 11;
    const int NUM_BUILDINGS = 10;

    std::cout << "Creating " << NUM_BUILDINGS << " buildings" << std::endl;
    std::cout << "Creating " << NUM_CARS << " cars" << std::endl;

    // Random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist_x(100.0f, 790.0f);
    std::uniform_real_distribution<float> pos_dist_y(10.0f, 590.0f);
    std::uniform_real_distribution<float> speed_dist(20.0f, 40.0f);
    std::uniform_int_distribution<int> color_dist(0, 255);
    std::uniform_real_distribution<float> income_dist(100.0f, 500.0f);
    std::uniform_int_distribution<int> building_dist(0, NUM_BUILDINGS - 1);

    // Color initialization
    state.color = sf::Color(color_dist(gen), color_dist(gen), color_dist(gen), 255);

    // Clear existing data
    state.traffic.cars.clear();
    state.traffic.roads.clear();

    // Buildings initialization
    state.economy.buildings.clear();
    for (int i = 0; i < NUM_BUILDINGS; ++i)
    {
        GameState::Building building;
        building.position = {pos_dist_x(gen), pos_dist_y(gen)};
        building.income = income_dist(gen);
        state.economy.buildings.push_back(building);
    }

    // Roads initialization
    // Create a simple road network connecting consecutive buildings
    for (int i = 0; i < NUM_BUILDINGS - 1; ++i)
    {
        GameState::Road road;
        road.building1_idx = i;
        road.building2_idx = i + 1;
        state.traffic.roads.push_back(road);
    }

    // Connect last building to first to create a loop
    GameState::Road road;
    road.building1_idx = NUM_BUILDINGS - 1;
    road.building2_idx = 0;
    state.traffic.roads.push_back(road);

    // Traffic initialization
    for (int i = 0; i < NUM_CARS; ++i)
    {
        GameState::Car car;
        // Assign random source and destination buildings
        car.source_building_idx = building_dist(gen);
        do {
            car.destination_building_idx = building_dist(gen);
        } while (car.destination_building_idx == car.source_building_idx);

        // Start at source building
        car.position = state.economy.buildings[car.source_building_idx].position;
        car.speed = speed_dist(gen);
        car.is_returning = false;
        state.traffic.cars.push_back(car);
    }

    state.economy.money = 10000.0f;
    state.economy.tax_rate = 0.1f;
}

// ----------------------------------------------------------------------------
bool GameManager::validateState(const GameState& state)
{
    // Verify car data
    for (const auto& car : state.traffic.cars)
    {
        if (std::isnan(car.position.x) || std::isnan(car.position.y) ||
            std::isnan(car.speed))
        {
            return false;
        }
    }

    // Verify economic data
    if (std::isnan(state.economy.money))
    {
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
void GameManager::update(GameState& state, float dt)
{
    for (auto& car : state.traffic.cars)
    {
        // Get current target position
        sf::Vector2f target_pos;
        if (!car.is_returning)
        {
            target_pos = state.economy.buildings[car.destination_building_idx].position;
        }
        else
        {
            target_pos = state.economy.buildings[car.source_building_idx].position;
        }

        // Calculate direction to target
        sf::Vector2f direction = target_pos - car.position;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        // If we reached the target
        if (distance < 5.0f)
        {
            // Switch direction if we reached destination
            car.is_returning = !car.is_returning;
        }
        else
        {
            // Normalize direction and move car
            direction /= distance;
            car.position += direction * car.speed * dt;
        }
    }

    // Update building incomes
    for (auto& building : state.economy.buildings)
    {
        building.income = 0.0f;
    }
}