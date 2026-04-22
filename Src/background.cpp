
#include "../include/Background.hpp"
#include "../include/Utils.hpp"
#include <cmath>

// ============================================================
//  Background.cpp — Implémentation du décor spatial
//
//  CONCEPT POO ILLUSTRÉ : Encapsulation
//  Toute la logique du fond visuel est ici, isolée de Game.
//  Game appelle juste background.update(dt) et background.draw(w).
// ============================================================

// ------------------------------------------------------------
//  Constructeur — initialise tout le décor
//  CONSTRUCTEUR : appelé automatiquement à la création de l'objet.
//  Initialise les membres dans l'ordre de déclaration du .hpp.
// ------------------------------------------------------------
Background::Background()
    : m_moonX(620.0f),
      m_moonY(80.0f)
{
    initStars();
    initMoon();
    initPanels();
}

// ------------------------------------------------------------
//  update() — défile tous les éléments vers la gauche
//  Paramètre par valeur (float) : type primitif, copie négligeable
// ------------------------------------------------------------
void Background::update(float deltaTime) {
    // Défilement des étoiles (deux couches à vitesses différentes)
    for (int layer = 0; layer < 2; ++layer) {
        for (auto& star : m_starLayers[layer].stars) {
            sf::Vector2f pos = star.getPosition();
            pos.x -= m_starLayers[layer].speed * deltaTime;
            // Recyclage : si l'étoile sort à gauche, elle réapparaît à droite
            if (pos.x < -4.0f) {
                pos.x = static_cast<float>(Constants::WINDOW_WIDTH) + 2.0f;
                pos.y = Utils::randomFloat(0.0f, static_cast<float>(Constants::GROUND_Y));
            }
            star.setPosition(pos);
        }
    }

    // Défilement des panneaux métalliques
    for (auto& panel : m_bgPanels) {
        sf::Vector2f pos = panel.rect.getPosition();
        pos.x -= panel.speed * deltaTime;
        if (pos.x < -100.0f) {
            pos.x = static_cast<float>(Constants::WINDOW_WIDTH) + 10.0f;
            pos.y = Utils::randomFloat(270.0f, Constants::GROUND_Y - 10.0f);
        }
        panel.rect.setPosition(pos);
    }

    // Défilement de la lune (très lent : effet de grande distance)
    m_moonX -= 4.0f * deltaTime;
    if (m_moonX < -80.0f)
        m_moonX = static_cast<float>(Constants::WINDOW_WIDTH) + 80.0f;
}

// ------------------------------------------------------------
//  draw() — dessine tous les éléments
//  Paramètre par référence (&) : évite de copier l'objet fenêtre
// ------------------------------------------------------------
void Background::draw(sf::RenderWindow& window) {
    // Couche 1 d'étoiles (lointaines, dessinées en premier = derrière)
    for (const auto& star : m_starLayers[0].stars)
        window.draw(star);

    // Lune (devant les étoiles lointaines)
    drawMoon(window);

    // Couche 2 d'étoiles (proches, devant la lune)
    for (const auto& star : m_starLayers[1].stars)
        window.draw(star);

    // Panneaux métalliques de la station (décor de sol)
    for (const auto& panel : m_bgPanels)
        window.draw(panel.rect);
}

// ------------------------------------------------------------
//  initStars() — génère les deux couches d'étoiles
// ------------------------------------------------------------
void Background::initStars() {
    // Couche 1 : étoiles lointaines, lentes, peu lumineuses
    m_starLayers[0].speed = 15.0f;
    for (int i = 0; i < 80; ++i) {
        sf::CircleShape s;
        s.setRadius(Utils::randomFloat(0.5f, 1.5f));
        s.setFillColor(sf::Color(200, 215, 255,
            static_cast<sf::Uint8>(Utils::randomInt(80, 160))));
        s.setPosition(
            Utils::randomFloat(0.0f, Constants::WINDOW_WIDTH),
            Utils::randomFloat(0.0f, Constants::GROUND_Y));
        m_starLayers[0].stars.push_back(s);
    }

    // Couche 2 : étoiles proches, rapides, plus lumineuses
    m_starLayers[1].speed = 50.0f;
    for (int i = 0; i < 40; ++i) {
        sf::CircleShape s;
        s.setRadius(Utils::randomFloat(1.0f, 2.5f));
        s.setFillColor(sf::Color(220, 230, 255,
            static_cast<sf::Uint8>(Utils::randomInt(140, 220))));
        s.setPosition(
            Utils::randomFloat(0.0f, Constants::WINDOW_WIDTH),
            Utils::randomFloat(0.0f, Constants::GROUND_Y));
        m_starLayers[1].stars.push_back(s);
    }
}

// ------------------------------------------------------------
//  initMoon() — prépare la lune et ses cratères
// ------------------------------------------------------------
void Background::initMoon() {
    m_moon.setRadius(55.0f);
    m_moon.setFillColor(sf::Color(190, 195, 210));
    m_moon.setOutlineColor(sf::Color(150, 155, 175));
    m_moon.setOutlineThickness(2.0f);
    m_moon.setOrigin(55.0f, 55.0f);

    // Définition des cratères (position et taille relative au centre)
    struct CraterDef { float dx, dy, r; };
    CraterDef defs[] = {
        {-20.0f, -15.0f, 9.0f},
        { 18.0f,  10.0f, 12.0f},
        { -5.0f,  22.0f, 6.0f},
        { 28.0f, -20.0f, 5.0f},
        {-30.0f,  18.0f, 7.0f},
        { 10.0f,  -8.0f, 4.0f},
    };
    for (auto& d : defs) {
        Crater c;
        c.offset = sf::Vector2f(d.dx, d.dy);
        c.radius = d.r;
        m_moonCraters.push_back(c);
    }
}

// ------------------------------------------------------------
//  initPanels() — génère les panneaux métalliques aléatoires
// ------------------------------------------------------------
void Background::initPanels() {
    for (int i = 0; i < 14; ++i) {
        PanelStrip p;
        p.rect.setSize(sf::Vector2f(
            Utils::randomFloat(30.0f, 85.0f),
            Utils::randomFloat(7.0f, 18.0f)));
        p.rect.setPosition(
            Utils::randomFloat(0.0f, Constants::WINDOW_WIDTH),
            Utils::randomFloat(270.0f, Constants::GROUND_Y - 10.0f));
        bool cyan = (Utils::randomInt(0, 1) == 0);
        p.color = cyan ? sf::Color(28, 42, 78, 110) : sf::Color(0, 75, 115, 90);
        p.rect.setFillColor(p.color);
        p.speed = Utils::randomFloat(70.0f, 140.0f);
        m_bgPanels.push_back(p);
    }
}

// ------------------------------------------------------------
//  drawMoon() — dessine la lune avec ses cratères et reflet
//  Méthode PRIVÉE : n'est appelée que depuis draw()
// ------------------------------------------------------------
void Background::drawMoon(sf::RenderWindow& window) {
    m_moon.setPosition(m_moonX, m_moonY);
    window.draw(m_moon);

    // Dessine chaque cratère (plus foncé que la lune)
    for (const auto& c : m_moonCraters) {
        sf::CircleShape crater(c.radius);
        crater.setFillColor(sf::Color(155, 158, 175));
        crater.setOutlineColor(sf::Color(130, 134, 155));
        crater.setOutlineThickness(1.0f);
        crater.setOrigin(c.radius, c.radius);
        crater.setPosition(m_moonX + c.offset.x, m_moonY + c.offset.y);
        window.draw(crater);
    }

    // Reflet lumineux (coin supérieur gauche de la lune)
    sf::CircleShape shine(14.0f);
    shine.setFillColor(sf::Color(230, 235, 245, 60));
    shine.setOrigin(14.0f, 14.0f);
    shine.setPosition(m_moonX - 28.0f, m_moonY - 28.0f);
    window.draw(shine);
}
