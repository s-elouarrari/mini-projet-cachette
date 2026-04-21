#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
//  Utils.hpp — Toutes les constantes du jeu
//  Modifier ici pour ajuster le gameplay sans chercher dans
//  tout le code.
// ============================================================

namespace Constants {
    // --- Fenetre ---
    constexpr unsigned int WINDOW_WIDTH  = 900;
    constexpr unsigned int WINDOW_HEIGHT = 600;
    constexpr unsigned int TARGET_FPS    = 60;
    constexpr const char*  WINDOW_TITLE  = "Station Evac: Hull Breach";

    // ─────────────────────────────────────────────────────────
    //  POSITIONS DU MONDE
    // ─────────────────────────────────────────────────────────
    // GROUND_Y = Y du sol visible (bord superieur du sol)
    constexpr float GROUND_Y        = 490.0f;

    // Position de depart du robot = GROUND_Y - NORMAL_HEIGHT
    //   490 - 60 = 430  (robot pose sur le sol)
    constexpr float PLAYER_START_X  = 120.0f;
    constexpr float PLAYER_START_Y  = 410.0f;  // = GROUND_Y - NORMAL_HEIGHT = 490-80

    // ─────────────────────────────────────────────────────────
    //  SANTE
    // ─────────────────────────────────────────────────────────
    constexpr int   MAX_HEALTH             = 3;
    constexpr float INVINCIBILITY_AFTER_HIT = 1.8f; // secondes

    // ─────────────────────────────────────────────────────────
    //  DUREE DU JEU
    //  EXPLOSION_TIME     : temps avant la destruction de la station
    //  SURVIVAL_TIME_FOR_CAPSULE : temps de survie pour faire
    //                       apparaitre la capsule (doit etre
    //                       inferieur a EXPLOSION_TIME !)
    // ─────────────────────────────────────────────────────────
    constexpr float EXPLOSION_TIME             = 90.0f;  // 90 secondes
    constexpr float SURVIVAL_TIME_FOR_CAPSULE  = 60.0f;  // 60 secondes

    // ─────────────────────────────────────────────────────────
    //  DIFFICULTE : VITESSE ET SPAWN
    // ─────────────────────────────────────────────────────────
    // Vitesse initiale douce pour laisser le joueur decouvrir
    constexpr float SCROLL_SPEED_BASE    = 200.0f;
    constexpr float SCROLL_SPEED_MAX     = 520.0f;
    constexpr float SPEED_INCREMENT      = 6.0f;   // px/s par seconde

    // Spawn : large au debut, serre a la fin
    constexpr float SPAWN_INTERVAL_START = 3.2f;
    constexpr float SPAWN_INTERVAL_MIN   = 1.0f;
    constexpr float SPAWN_REDUCTION      = 0.025f; // reduction/seconde

    // ─────────────────────────────────────────────────────────
    //  COULEURS
    // ─────────────────────────────────────────────────────────
    const sf::Color COLOR_BACKGROUND    = sf::Color(8,   12,  28);
    const sf::Color COLOR_ACCENT_CYAN   = sf::Color(0,  220, 255);
    const sf::Color COLOR_ACCENT_ORANGE = sf::Color(255, 140,   0);
    const sf::Color COLOR_ACCENT_RED    = sf::Color(220,  40,  40);
    const sf::Color COLOR_ACCENT_GREEN  = sf::Color(40,  220, 100);
    const sf::Color COLOR_HUD_TEXT      = sf::Color(200, 220, 255);
}

namespace Utils {
    float       randomFloat(float minVal, float maxVal);
    int         randomInt(int minVal, int maxVal);
    sf::Color   lerpColor(const sf::Color& a, const sf::Color& b, float t);
    std::string formatTime(float seconds);
}
