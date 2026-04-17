#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
//  Utils.hpp — Constantes globales + fonctions utilitaires
// ============================================================

namespace Constants {
    // Fenêtre
    constexpr unsigned int WINDOW_WIDTH    = 900;
    constexpr unsigned int WINDOW_HEIGHT   = 600;
    constexpr unsigned int TARGET_FPS      = 60;
    constexpr const char*  WINDOW_TITLE    = "Station Evac: Hull Breach";

    // Gameplay
    constexpr float PRESSURE_MAX          = 30.0f;   // secondes d'oxygène
    constexpr float DISTANCE_TO_CAPSULE   = 1000.0f; // unités de progression
    constexpr float OBSTACLE_SPAWN_INTERVAL_MIN = 1.2f;
    constexpr float OBSTACLE_SPAWN_INTERVAL_MAX = 2.8f;
    constexpr float SCROLL_SPEED_BASE     = 300.0f;
    constexpr float SCROLL_SPEED_MAX      = 550.0f;
    constexpr float SPEED_INCREMENT       = 10.0f;

    // Positions sol
    constexpr float GROUND_Y              = 490.0f;
    constexpr float PLAYER_START_X        = 120.0f;
    constexpr float PLAYER_START_Y        = 430.0f;

    // Obstacles aériens
    constexpr float AERIAL_OBSTACLE_Y     = 310.0f;

    // Couleurs thème spatial
    const sf::Color COLOR_BACKGROUND      = sf::Color(8,  12, 28);
    const sf::Color COLOR_FLOOR           = sf::Color(30, 40, 70);
    const sf::Color COLOR_ACCENT_CYAN     = sf::Color(0,  220, 255);
    const sf::Color COLOR_ACCENT_ORANGE   = sf::Color(255, 140, 0);
    const sf::Color COLOR_ACCENT_RED      = sf::Color(220, 40,  40);
    const sf::Color COLOR_ACCENT_GREEN    = sf::Color(40,  220, 100);
    const sf::Color COLOR_HUD_TEXT        = sf::Color(200, 220, 255);
    const sf::Color COLOR_PRESSURE_LOW    = sf::Color(255, 60,  60);
    const sf::Color COLOR_PRESSURE_HIGH   = sf::Color(0,   200, 255);
}

// Fonctions utilitaires (friend-like helpers)
namespace Utils {
    float randomFloat(float minVal, float maxVal);
    int   randomInt(int minVal, int maxVal);
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t);
    std::string formatTime(float seconds);
}
