#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

struct GameState
{
    struct Traffic
    {
        std::vector<sf::Vector2f> car_positions;
        std::vector<float> car_speeds;
    } traffic;

    struct Economy
    {
        float money;
        std::vector<float> building_incomes;
        float tax_rate;
    } economy;
};