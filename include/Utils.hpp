#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
//  Utils.hpp — Constantes de configuration et fonctions utilitaires
//
//  ORGANISATION :
//    namespace Constants → toutes les valeurs "magiques" du jeu.
//      Modifier ici pour régler le gameplay sans chercher dans
//      tout le code source.
//
//    namespace Utils → fonctions helpers utilisées partout.
//
//  HITBOX DU JOUEUR (résumé pratique) :
//    GROUND_Y     = 490
//    NORMAL_HEIGHT= 80   → debout   : Y 410..490
//    CROUCH_HEIGHT= 35   → accroupi : Y 455..490
//    JUMP_FORCE   = -560 → apex saut: Y ~268..348
//
//    Bloc sol    Y 390..490 → debout TOUCHE, saut PASSE
//    Drone aérien Y 350..445 → debout TOUCHE, accroupi PASSE
// ============================================================

namespace Constants {

    // ── Fenêtre ───────────────────────────────────────────────
    constexpr unsigned int WINDOW_WIDTH  = 900;   // largeur en pixels
    constexpr unsigned int WINDOW_HEIGHT = 600;   // hauteur en pixels
    constexpr unsigned int TARGET_FPS    = 60;    // images par seconde cible
    constexpr const char*  WINDOW_TITLE  = "Station Evac: Hull Breach";

    // ── Positions du monde ────────────────────────────────────
    // GROUND_Y : Y du bord supérieur du sol (le rectangle du sol
    // commence ici et s'étend vers le bas).
    constexpr float GROUND_Y       = 490.0f;

    // Position de départ du robot :
    //   X = fixe (côté gauche de l'écran)
    //   Y = GROUND_Y - NORMAL_HEIGHT = 490 - 80 = 410
    //     (sommet de la hitbox quand le robot est debout)
    constexpr float PLAYER_START_X = 120.0f;
    constexpr float PLAYER_START_Y = 410.0f;

    // ── Santé ─────────────────────────────────────────────────
    constexpr int   MAX_HEALTH             = 3;     // cœurs au départ
    constexpr float INVINCIBILITY_AFTER_HIT = 1.8f; // secondes d'invincibilité

    // ── Durées du jeu ─────────────────────────────────────────
    // EXPLOSION_TIME doit être > SURVIVAL_TIME_FOR_CAPSULE
    // pour que le joueur ait le temps d'atteindre la capsule.
    constexpr float EXPLOSION_TIME            = 90.0f;  // sec avant explosion
    constexpr float SURVIVAL_TIME_FOR_CAPSULE = 60.0f;  // sec pour la capsule

    // ── Obstacles : vitesse et fréquence d'apparition ────────
    constexpr float SCROLL_SPEED_BASE    = 200.0f; // vitesse initiale (px/s)
    constexpr float SCROLL_SPEED_MAX     = 520.0f; // vitesse maximale (px/s)
    constexpr float SPEED_INCREMENT      = 6.0f;   // accélération (px/s par sec)

    // Intervalle entre deux obstacles (secondes) :
    //   Commence à START, décroît jusqu'à MIN au fil du temps.
    constexpr float SPAWN_INTERVAL_START = 3.2f;
    constexpr float SPAWN_INTERVAL_MIN   = 1.0f;
    constexpr float SPAWN_REDUCTION      = 0.025f; // réduction par seconde

    // ── Couleurs du thème spatial ─────────────────────────────
    const sf::Color COLOR_BACKGROUND    = sf::Color(8,   12,  28);  // bleu nuit très sombre
    const sf::Color COLOR_ACCENT_CYAN   = sf::Color(0,  220, 255);  // cyan lumineux
    const sf::Color COLOR_ACCENT_ORANGE = sf::Color(255, 140,   0); // orange chaud
    const sf::Color COLOR_ACCENT_RED    = sf::Color(220,  40,  40); // rouge danger
    const sf::Color COLOR_ACCENT_GREEN  = sf::Color(40,  220, 100); // vert victoire
    const sf::Color COLOR_HUD_TEXT      = sf::Color(200, 220, 255); // blanc bleuté

} // namespace Constants


namespace Utils {

    // Nombre flottant aléatoire dans [minVal, maxVal]
    float randomFloat(float minVal, float maxVal);

    // Entier aléatoire dans [minVal, maxVal] (bornes incluses)
    int randomInt(int minVal, int maxVal);

    // Interpolation linéaire entre deux couleurs SFML
    // t = 0.0 → couleur a, t = 1.0 → couleur b
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t);

    // Formate des secondes en chaîne "SS.ccs" (ex: "23.45s")
    std::string formatTime(float seconds);

} // namespace Utils
