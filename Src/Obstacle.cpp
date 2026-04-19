#include "../include/Obstacle.hpp"
#include "../include/Utils.hpp"
#include <cmath>

// ============================================================
//  Obstacle.cpp — Les 2 obstacles du jeu
//
//  DIFFICULTE PROGRESSIVE :
//    Les obstacles recoivent la vitesse courante du jeu via
//    updateWithSpeed(deltaTime, speed).
//    Game.cpp passe m_scrollSpeed qui augmente chaque seconde.
//
//  POSITIONS GARANTISSANT LA JOUABILITE :
//    Sol (GROUND_Y = 490) :
//      MagneticContainer : Y=432, hauteur=58 => pose sur le sol
//      => Le joueur debout (Y=430, h=60) doit sauter
//
//    Aerien :
//      SecurityDrone : Y de base = 355, hitbox h=36
//      => Le bas du drone est a Y=391
//      => Le joueur debout : sommet a Y=430 => il se prend le drone
//      => Le joueur accroupi (h=30) : sommet a Y=460 => il passe !
// ============================================================

// ============================================================
//  Obstacle — classe mere
// ============================================================
Obstacle::Obstacle(float x, float y, float width, float height, ObstacleType type)
    : Entity(x, y, width, height),
      m_currentSpeed(Constants::SCROLL_SPEED_BASE),
      m_type(type)
{}

Obstacle::~Obstacle() {}

// update() de base (utilise la vitesse stockee)
void Obstacle::update(float deltaTime) {
    m_position.x -= m_currentSpeed * deltaTime;
}

void Obstacle::draw(sf::RenderWindow& window) {
    m_shape.setPosition(m_position.x, m_position.y);
    window.draw(m_shape);
}

// Mise a jour avec vitesse dynamique (appelee par Game.cpp)
void Obstacle::updateWithSpeed(float deltaTime, float speed) {
    m_currentSpeed = speed;
    m_position.x  -= speed * deltaTime;
}

ObstacleType Obstacle::getType() const { return m_type; }

bool Obstacle::isOffScreen() const {
    return (m_position.x + m_size.x) < 0.0f;
}

// ============================================================
//  MagneticContainer — Obstacle AU SOL
//  Action requise : SAUTER
// ============================================================
MagneticContainer::MagneticContainer(float x)
    : Obstacle(x, 432.0f, 45.0f, 58.0f, ObstacleType::GROUND),
      m_glowTimer(0.0f)
{
    m_shape.setSize(sf::Vector2f(45.0f, 58.0f));
    m_shape.setFillColor(sf::Color(50, 70, 110));
    m_shape.setOutlineColor(sf::Color(0, 180, 255));
    m_shape.setOutlineThickness(2.0f);

    m_topBar.setSize(sf::Vector2f(45.0f, 8.0f));
    m_topBar.setFillColor(sf::Color(0, 200, 255));
}

MagneticContainer::~MagneticContainer() {}

void MagneticContainer::updateWithSpeed(float deltaTime, float speed) {
    m_position.x -= speed * deltaTime;
    m_glowTimer  += deltaTime;
}

void MagneticContainer::draw(sf::RenderWindow& window) {
    float glow = (std::sin(m_glowTimer * 4.0f) + 1.0f) * 0.5f;
    sf::Color glowColor(
        0,
        static_cast<sf::Uint8>(150 + glow * 105),
        static_cast<sf::Uint8>(200 + glow * 55)
    );
    m_shape.setOutlineColor(glowColor);
    m_shape.setPosition(m_position.x, m_position.y);
    window.draw(m_shape);

    m_topBar.setPosition(m_position.x, m_position.y - 8.0f);
    m_topBar.setFillColor(glowColor);
    window.draw(m_topBar);
}

// ============================================================
//  SecurityDrone — Obstacle AERIEN
//  Action requise : SE BAISSER
// ============================================================
SecurityDrone::SecurityDrone(float x)
    : Obstacle(x, 355.0f, 52.0f, 36.0f, ObstacleType::AERIAL),
      m_hoverTimer(0.0f),
      m_baseY(355.0f)
{
    m_body.setRadius(18.0f);
    m_body.setFillColor(sf::Color(75, 75, 100));
    m_body.setOutlineColor(sf::Color(200, 50, 50));
    m_body.setOutlineThickness(2.0f);
    m_body.setOrigin(18.0f, 18.0f);

    m_blades[0].setSize(sf::Vector2f(42.0f, 6.0f));
    m_blades[0].setOrigin(21.0f, 3.0f);
    m_blades[0].setFillColor(sf::Color(140, 140, 170, 190));

    m_blades[1].setSize(sf::Vector2f(6.0f, 42.0f));
    m_blades[1].setOrigin(3.0f, 21.0f);
    m_blades[1].setFillColor(sf::Color(140, 140, 170, 190));
}

SecurityDrone::~SecurityDrone() {}

void SecurityDrone::updateWithSpeed(float deltaTime, float speed) {
    m_position.x  -= speed * deltaTime;
    m_hoverTimer  += deltaTime;
    // Oscillation verticale legere (independante de la vitesse)
    m_position.y = m_baseY + std::sin(m_hoverTimer * 3.0f) * 8.0f;
}

void SecurityDrone::draw(sf::RenderWindow& window) {
    float cx = m_position.x + 26.0f;
    float cy = m_position.y + 18.0f;

    float angle = m_hoverTimer * 300.0f;
    m_blades[0].setPosition(cx, cy); m_blades[0].setRotation(angle);
    m_blades[1].setPosition(cx, cy); m_blades[1].setRotation(angle + 45.0f);
    window.draw(m_blades[0]);
    window.draw(m_blades[1]);

    m_body.setPosition(cx, cy);
    window.draw(m_body);

    // Voyant rouge clignotant
    float blink = (std::sin(m_hoverTimer * 8.0f) + 1.0f) * 0.5f;
    sf::CircleShape light(4.0f);
    light.setOrigin(4.0f, 4.0f);
    light.setPosition(cx, cy + 8.0f);
    light.setFillColor(sf::Color(255, 0, 0, static_cast<sf::Uint8>(blink * 255)));
    window.draw(light);
}
