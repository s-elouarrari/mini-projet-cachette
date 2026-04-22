#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ============================================================
//  Background.hpp — Décor spatial animé (parallaxe)
//
//  ENCAPSULATION : Cette classe regroupe TOUT ce qui concerne
//  le fond visuel du jeu. Game n'a plus besoin de connaître
//  les détails (étoiles, lune, panneaux). C'est le principe
//  de "séparation des responsabilités".
//
//  Contient :
//    - Deux couches d'étoiles (vitesses différentes = parallaxe)
//    - Une lune avec cratères (défilement très lent)
//    - Des panneaux métalliques de station
// ============================================================
class Background {
public:
    // Constructeur : génère aléatoirement tout le décor
    Background();

    // Mise à jour : fait défiler les éléments de droite à gauche
    // Passage par valeur (float) car type primitif, pas de copie coûteuse
    void update(float deltaTime);

    // Dessin de tous les éléments dans la fenêtre
    // Passage par référence (&) : on ne copie pas la fenêtre SFML
    void draw(sf::RenderWindow& window);

    // Accesseurs (getters) — respecte l'encapsulation :
    // les attributs privés ne sont pas accessibles directement
    float getMoonX() const { return m_moonX; }
    float getMoonY() const { return m_moonY; }

private:
    // ── Données privées (encapsulation) ──────────────────────
    // Aucun code extérieur ne peut modifier ces variables directement.

    // Étoiles : deux couches pour l'effet de profondeur (parallaxe)
    struct StarLayer {
        std::vector<sf::CircleShape> stars; // std::vector de la STL
        float speed;                         // vitesse de défilement
    };
    StarLayer m_starLayers[2]; // tableau de 2 couches

    // Lune : cercle gris avec une liste de cratères
    struct Crater {
        sf::Vector2f offset; // position relative au centre de la lune
        float        radius; // rayon du cratère
    };
    sf::CircleShape     m_moon;
    std::vector<Crater> m_moonCraters;
    float               m_moonX;
    float               m_moonY;

    // Panneaux métalliques de la station
    struct PanelStrip {
        sf::RectangleShape rect;
        sf::Color          color;
        float              speed;
    };
    std::vector<PanelStrip> m_bgPanels;

    // ── Méthodes privées ─────────────────────────────────────
    void initStars();   // initialise les étoiles
    void initMoon();    // initialise la lune et ses cratères
    void initPanels();  // initialise les panneaux métalliques
    void drawMoon(sf::RenderWindow& window); // dessin spécifique lune
};
