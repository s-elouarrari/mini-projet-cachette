#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
//  Utils.hpp — Constantes et fonctions utilitaires
//  Toutes les valeurs "magiques" sont ici, pas dans le code
// ============================================================

namespace Constants {
    // --- Fenetre ---
    constexpr unsigned int WINDOW_WIDTH  = 900;
    constexpr unsigned int WINDOW_HEIGHT = 600;
    constexpr unsigned int TARGET_FPS    = 60;
    constexpr const char*  WINDOW_TITLE  = "Station Evac: Hull Breach";

    // --- Positions du monde ---
    constexpr float GROUND_Y        = 490.0f;  // Y du sol
    constexpr float PLAYER_START_X  = 120.0f;
    constexpr float PLAYER_START_Y  = 430.0f;

    // --- Sante du joueur ---
    constexpr int   MAX_HEALTH           = 3;     // Coeurs au depart
    constexpr float INVINCIBILITY_AFTER_HIT = 1.5f; // Secondes d'invincibilite apres un choc

    // --- Temps avant explosion ---
    constexpr float EXPLOSION_TIME       = 35.0f; // Secondes avant la fin (barre rouge en haut)

    // --- Progression vers la capsule ---
    constexpr float SURVIVAL_TIME_FOR_CAPSULE = 20.0f; // Secondes pour remplir la barre verte

    // --- Obstacles : vitesse et spawn ---
    constexpr float SCROLL_SPEED_BASE   = 280.0f; // Vitesse initiale des obstacles
    constexpr float SCROLL_SPEED_MAX    = 600.0f; // Vitesse maximale
    constexpr float SPEED_INCREMENT     = 12.0f;  // Acceleration par seconde

    constexpr float SPAWN_INTERVAL_START = 2.5f;  // Intervalle initial entre obstacles (s)
    constexpr float SPAWN_INTERVAL_MIN   = 0.8f;  // Intervalle minimum (difficulte max)
    constexpr float SPAWN_REDUCTION      = 0.04f; // Reduction de l'intervalle par seconde

    // --- Couleurs du theme spatial ---
    const sf::Color COLOR_BACKGROUND  = sf::Color(8,   12,  28);
    const sf::Color COLOR_ACCENT_CYAN = sf::Color(0,  220, 255);
    const sf::Color COLOR_ACCENT_ORANGE= sf::Color(255, 140,  0);
    const sf::Color COLOR_ACCENT_RED  = sf::Color(220,  40,  40);
    const sf::Color COLOR_ACCENT_GREEN= sf::Color(40,  220, 100);
    const sf::Color COLOR_HUD_TEXT    = sf::Color(200, 220, 255);
}

namespace Utils {
    float     randomFloat(float minVal, float maxVal);
    int       randomInt(int minVal, int maxVal);
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t);
    std::string formatTime(float seconds);
}
