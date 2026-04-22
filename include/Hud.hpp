#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
//  HUD.hpp — Interface utilisateur en jeu (Heads-Up Display)
//
//  ENCAPSULATION : regroupe tout l'affichage des informations
//  de jeu (barres, cœurs, hints, instructions).
//
//  Game lui passe les données nécessaires via draw() plutôt
//  que de gérer le rendu directement.
// ============================================================
class HUD {
public:
    // Constructeur : prépare les formes des barres
    explicit HUD(const sf::Font& font);

    // Dessine le HUD complet
    // Paramètres passés par valeur (types primitifs) ou const& (objets)
    void draw(sf::RenderWindow& window,
              float explosionTimer,    // temps restant avant explosion
              float survivalTime,      // temps survie accumulé
              bool  capsuleVisible,    // true = capsule apparue
              int   playerHealth,      // 0..3
              const std::vector<std::pair<float,bool>>& obstacleHints);
              // liste des obstacles visibles : (posX, isGround)

private:
    const sf::Font& m_font; // référence à la police de Game (pas de copie)

    // Barres de progression (formes pré-créées pour performance)
    sf::RectangleShape m_timerBarBg;    // fond barre rouge (explosion)
    sf::RectangleShape m_timerBarFill;  // remplissage barre rouge
    sf::RectangleShape m_progressBarBg;  // fond barre verte (capsule)
    sf::RectangleShape m_progressBarFill;// remplissage barre verte

    // Méthodes privées de dessin (décomposition)
    void drawExplosionBar(sf::RenderWindow& window, float ratio);
    void drawProgressBar(sf::RenderWindow& window, float ratio, bool capsuleVisible);
    void drawHearts(sf::RenderWindow& window, int health);
    void drawHints(sf::RenderWindow& window,
                   const std::vector<std::pair<float,bool>>& hints);

    // Crée un sf::Text prêt à l'emploi
    sf::Text makeText(const std::string& str, unsigned int size, sf::Color color) const;
};
