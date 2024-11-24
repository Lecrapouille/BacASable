#include "GameManager.hpp"
#include <random>

// ----------------------------------------------------------------------------
void GameManager::createInitialState(GameState& state)
{
    // Générateur de nombres aléatoires
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist(50.0f, 750.0f); 
    std::uniform_real_distribution<float> income_dist(10.0f, 50.0f);
    std::uniform_real_distribution<float> speed_dist(0.0f, 20.0f);

    // Initialisation du trafic
    state.traffic.car_positions.clear();
    state.traffic.car_speeds.clear();

    // Ajout de quelques voitures de test
    for (int i = 0; i < 10; ++i)
    {
        state.traffic.car_positions.emplace_back(pos_dist(gen), pos_dist(gen));
        state.traffic.car_speeds.push_back(30.0f + speed_dist(gen));
    }
}

// ----------------------------------------------------------------------------
bool GameManager::validateState(const GameState& state)
{
    // Vérification de la cohérence des données de trafic
    if (state.traffic.car_positions.size() !=
        state.traffic.car_speeds.size())
    {
        return false;
    }

    // Vérification des positions des voitures
    for (const auto& pos : state.traffic.car_positions)
    {
        if (std::isnan(pos.x) || std::isnan(pos.y))
        {
            return false;
        }
    }

    // Vérification des données économiques
    if (std::isnan(state.economy.money))
    {
        return false;
    }

    return true;
}