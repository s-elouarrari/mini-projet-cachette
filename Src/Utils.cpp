#include "../include/Utils.hpp"
#include <random>
#include <sstream>
#include <iomanip>

// ============================================================
//  Utils.cpp — Fonctions utilitaires
//
//  Ces fonctions sont utilisées partout dans le jeu :
//    - randomFloat / randomInt : génération aléatoire pour
//      le placement des étoiles, panneaux, obstacles...
//    - lerpColor : interpolation de couleur pour les barres HUD
//    - formatTime : affichage du score en "SS.cc s"
//
//  L'utilisation d'un générateur statique (thread_local)
//  évite de recréer le générateur à chaque appel, ce qui
//  serait lent et produirait des séquences répétitives.
// ============================================================

namespace Utils {

    // ── Générateur de nombres aléatoires ─────────────────────
    // static = partagé entre tous les appels à randomFloat/randomInt.
    // Initialisé une seule fois avec std::random_device (vrai aléa).
    static std::mt19937& getRng() {
        static std::mt19937 rng(std::random_device{}());
        return rng;
    }

    // ------------------------------------------------------------
    //  randomFloat() — nombre flottant entre minVal et maxVal
    //  Exemple : randomFloat(0.0f, 900.0f) pour une position X
    // ------------------------------------------------------------
    float randomFloat(float minVal, float maxVal) {
        std::uniform_real_distribution<float> dist(minVal, maxVal);
        return dist(getRng());
    }

    // ------------------------------------------------------------
    //  randomInt() — entier entre minVal et maxVal (inclus)
    //  Exemple : randomInt(0, 1) pour choisir le type d'obstacle
    // ------------------------------------------------------------
    int randomInt(int minVal, int maxVal) {
        std::uniform_int_distribution<int> dist(minVal, maxVal);
        return dist(getRng());
    }

    // ------------------------------------------------------------
    //  lerpColor() — interpolation linéaire entre deux couleurs
    //
    //  t = 0.0 → retourne la couleur a
    //  t = 1.0 → retourne la couleur b
    //  t = 0.5 → retourne un mélange à 50%
    //
    //  Utilisé dans drawHUD() pour faire passer la barre
    //  d'explosion du vert (t=1) au rouge (t=0) progressivement.
    // ------------------------------------------------------------
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
        // Clamp t entre 0 et 1 pour éviter les débordements de couleur
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        return sf::Color(
            static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
            static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
            static_cast<sf::Uint8>(a.b + (b.b - a.b) * t),
            static_cast<sf::Uint8>(a.a + (b.a - a.a) * t)
        );
    }

    // ------------------------------------------------------------
    //  formatTime() — convertit des secondes en "SS.cc s"
    //  Exemple : 23.456f → "23.45s"
    //  Utilisé pour afficher le score de survie sur l'écran victoire.
    // ------------------------------------------------------------
    std::string formatTime(float seconds) {
        int intSec  = static_cast<int>(seconds);
        int centis  = static_cast<int>((seconds - intSec) * 100);

        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << intSec
            << "."
            << std::setfill('0') << std::setw(2) << centis
            << "s";
        return oss.str();
    }

} // namespace Utils
