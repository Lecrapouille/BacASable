#include "GameManager.hpp"
#include <SFML/Graphics/Color.hpp>
#include <random>
#include <iostream>

// ----------------------------------------------------------------------------
void GameManager::createInitialState(GameState& state)
{
    const int NUM_CARS = 4;
    const int NUM_BUILDINGS = 8;

    // Random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist_x(100.0f, 790.0f);
    std::uniform_real_distribution<float> pos_dist_y(10.0f, 590.0f);
    std::uniform_real_distribution<float> speed_dist(20.0f, 40.0f);
    std::uniform_int_distribution<int> color_dist(0, 255);
    std::uniform_real_distribution<float> income_dist(100.0f, 500.0f);
    std::uniform_int_distribution<int> building_dist(0, NUM_BUILDINGS - 1);

    // Clear existing data
    state.traffic.cars.clear();
    state.traffic.roads.clear();

    // Buildings initialization
    std::cout << "Creating " << NUM_BUILDINGS << " buildings" << std::endl;
    state.economy.buildings.clear();
    for (int i = 0; i < NUM_BUILDINGS; ++i)
    {
        GameState::Building building;
        building.position = {pos_dist_x(gen), pos_dist_y(gen)};
        building.income = income_dist(gen);
        building.color = sf::Color(100, 100, 100, 255);
        state.economy.buildings.push_back(building);
        std::cout << "  Building " << i << " created at " << building.position.x << ", " << building.position.y << " with income " << building.income << std::endl;
    }

    // Roads initialization
    // Create a simple road network connecting consecutive buildings
    std::cout << "Creating " << NUM_BUILDINGS - 1 << " roads" << std::endl;
    for (int i = 0; i < NUM_BUILDINGS - 1; ++i)
    {
        GameState::Road road;
        road.building1_idx = i;
        road.building2_idx = i + 1;
        road.color = sf::Color(100, 100, 100, 255);
        state.traffic.roads.push_back(road);
        std::cout << "  Road " << i << " created between building " << i << " and building " << i + 1 << std::endl;
    }

    // Connect last building to first to create a loop
    GameState::Road road;
    road.building1_idx = NUM_BUILDINGS - 1;
    road.building2_idx = 0;
    road.color = sf::Color(100, 100, 100, 255);
    state.traffic.roads.push_back(road);

    // Traffic initialization
    std::cout << "Creating " << NUM_CARS << " cars" << std::endl;
    for (int i = 0; i < NUM_CARS; ++i)
    {
        GameState::Car car;
        // Assign random source and destination buildings
        // Choose a random road
        std::uniform_int_distribution<int> road_dist(0, state.traffic.roads.size() - 1);
        int road_idx = road_dist(gen);
        const auto& chosen_road = state.traffic.roads[road_idx];
        
        // Assign source and destination buildings according to the chosen road
        car.source_building_idx = chosen_road.building1_idx;
        car.destination_building_idx = chosen_road.building2_idx;

        // Start at source building
        car.position = state.economy.buildings[car.source_building_idx].position;
        car.speed = speed_dist(gen);
        car.is_returning = false;
        state.traffic.cars.push_back(car);
        std::cout << "  Car " << i << " created at " << car.position.x << ", " << car.position.y << " with speed " << car.speed << " and destination building " << car.destination_building_idx << std::endl;
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
void GameManager::update(GameState& state, float dt, sf::Color color)
{
    for (sf::Uint32 i = state.traffic.startIdx; i < state.traffic.startIdx + state.traffic.count; ++i)
    {
        auto& car = state.traffic.cars[i];
    
        // Update car color
        car.color = color;

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
    for (sf::Uint32 i = state.economy.startIdx; i < state.economy.startIdx + state.economy.count; ++i)
    {
        auto& building = state.economy.buildings[i];
        building.color = color;
        building.income = 0.0f;
    }
}