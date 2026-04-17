
#include "Utils.hpp"
#include <random>
#include <sstream>
#include <iomanip>

// ============================================================
//  Utils.cpp — Fonctions utilitaires
// ============================================================

namespace Utils {

    // Générateur de nombres aléatoires (local à cette TU)
    static std::mt19937& getRng() {
        static std::mt19937 rng(std::random_device{}());
        return rng;
    }

    float randomFloat(float minVal, float maxVal) {
        std::uniform_real_distribution<float> dist(minVal, maxVal);
        return dist(getRng());
    }

    int randomInt(int minVal, int maxVal) {
        std::uniform_int_distribution<int> dist(minVal, maxVal);
        return dist(getRng());
    }

    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
        t = std::max(0.0f, std::min(1.0f, t));
        return sf::Color(
            static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
            static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
            static_cast<sf::Uint8>(a.b + (b.b - a.b) * t),
            static_cast<sf::Uint8>(a.a + (b.a - a.a) * t)
        );
    }

    std::string formatTime(float seconds) {
        int intSec = static_cast<int>(seconds);
        int millis = static_cast<int>((seconds - intSec) * 100);
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << intSec
            << "." << std::setfill('0') << std::setw(2) << millis << "s";
        return oss.str();
    }
}
