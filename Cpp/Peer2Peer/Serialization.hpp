#pragma once

#include "GameState.hpp"
#include <SFML/Network.hpp>

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const size_t& value)
{
    return packet << static_cast<sf::Uint32>(value);
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, size_t& value)
{
    sf::Uint32 v;
    packet >> v;
    value = static_cast<size_t>(v);
    return packet;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const sf::Color& color)
{
    return packet << color.r << color.g << color.b << color.a;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, sf::Color& color)
{
    packet >> color.r >> color.g >> color.b >> color.a;
    return packet;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const GameState::Car& car)
{
    return packet << car.position.x << car.position.y 
           << car.speed
           << car.source_building_idx 
           << car.destination_building_idx
           << car.is_returning;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, GameState::Car& car)
{
    return packet >> car.position.x >> car.position.y 
           >> car.speed
           >> car.source_building_idx 
           >> car.destination_building_idx
           >> car.is_returning;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const GameState::Building& building)
{
    return packet << building.position.x << building.position.y 
           << building.income;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, GameState::Building& building)
{
    return packet >> building.position.x >> building.position.y 
           >> building.income;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const GameState::Road& road)
{
    return packet << road.building1_idx << road.building2_idx;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, GameState::Road& road)
{
    return packet >> road.building1_idx >> road.building2_idx;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const GameState::Traffic& traffic)
{
    packet << traffic.cars.size();
    for (const auto& car : traffic.cars)
    {
        packet << car;
    }
    
    packet << traffic.roads.size();
    for (const auto& road : traffic.roads)
    {
        packet << road;
    }
    return packet;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, GameState::Traffic& traffic)
{
    size_t count;
    packet >> count;
    traffic.cars.resize(count);
    for (auto& car : traffic.cars)
    {
        packet >> car;
    }
    
    packet >> count;
    traffic.roads.resize(count);
    for (auto& road : traffic.roads)
    {
        packet >> road;
    }
    return packet;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const GameState::Economy& economy)
{
    packet << economy.buildings.size();
    for (const auto& building : economy.buildings)
    {
        packet << building;
    }
    packet << economy.money << economy.tax_rate;
    return packet;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, GameState::Economy& economy)
{
    size_t count;
    packet >> count;
    economy.buildings.resize(count);
    for (auto& building : economy.buildings)
    {
        packet >> building;
    }
    packet >> economy.money >> economy.tax_rate;
    return packet;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator<<(sf::Packet& packet, const GameState& state)
{
    packet << state.color << state.traffic << state.economy;
    return packet;
}

// ----------------------------------------------------------------------------
inline sf::Packet& operator>>(sf::Packet& packet, GameState& state)
{
    packet >> state.color >> state.traffic >> state.economy;
    return packet;
}