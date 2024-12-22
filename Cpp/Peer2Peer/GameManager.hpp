#pragma once

#include "GameState.hpp"

class GameManager
{
public:

    /**
     * @brief Initialize the game state with default values
     * @param state [inout] The game state to initialize
     */
    static void createInitialState(GameState& state);

    /**
     * @brief Validate the game state data
     * @param state [in] The game state to validate
     * @return true if the state is valid, false otherwise
     */
    static bool validateState(const GameState& state);

    /**
     * @brief Updates the game state
     * @param[inout] state The game state to update
     * @param[in] dt Time elapsed since last update in seconds
     * @param[in] color The color specified for each peer.
     */
    static void update(GameState& state, float dt, sf::Color color);
};
