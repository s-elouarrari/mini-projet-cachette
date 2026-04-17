#include "Obstacle.hpp"
#include "Utils.hpp"
#include <cmath>

// ============================================================
//  Obstacle.cpp — Classe mère + 4 sous-classes d'obstacles
// ============================================================

// ========== Obstacle (classe mère) ==========

Obstacle::Obstacle(float x, float y, float width, float height, ObstacleType type)
    : Entity(x, y, width, height),
      m_type(type),
      m_primaryColor(sf::Color::White),
      m_accentColor(sf::Color::Cyan)
{
}

Obstacle::~Obstacle() {}

void Obstacle::update(float deltaTime) {
    m_position.x -= SCROLL_SPEED * deltaTime;
}

void Obstacle::draw(sf::RenderWindow& window) {
    m_shape.setPosition(m_position.x, m_position.y);
    window.draw(m_shape);
}

ObstacleType Obstacle::getType() const {
    return m_type;
}

bool Obstacle::isOffScreen() const {
    return (m_position.x + m_size.x) < 0.0f;
}

// ========== MagneticContainer (sol) ==========

MagneticContainer::MagneticContainer(float x)
    : Obstacle(x, 395.0f, 45.0f, 58.0f, ObstacleType::GROUND),
      m_glowTimer(0.0f)
{
    m_shape.setFillColor(sf::Color(60, 80, 120));
    m_shape.setOutlineColor(sf::Color(0, 180, 255));
    m_shape.setOutlineThickness(2.0f);
    m_shape.setSize(sf::Vector2f(45.0f, 58.0f));

    m_topBar.setSize(sf::Vector2f(45.0f, 8.0f));
    m_topBar.setFillColor(sf::Color(0, 200, 255));
}

MagneticContainer::~MagneticContainer() {}

void MagneticContainer::update(float deltaTime) {
    m_position.x -= SCROLL_SPEED * deltaTime;
    m_glowTimer  += deltaTime;
}

void MagneticContainer::draw(sf::RenderWindow& window) {
    float glow = (std::sin(m_glowTimer * 4.0f) + 1.0f) * 0.5f;
    sf::Color glowColor(
        static_cast<sf::Uint8>(0),
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

// ========== PlasmaLeak (sol) ==========

PlasmaLeak::PlasmaLeak(float x)
    : Obstacle(x, 430.0f, 30.0f, 30.0f, ObstacleType::GROUND),
      m_animTimer(0.0f)
{
    for (int i = 0; i < 4; ++i) {
        m_particles[i].setRadius(5.0f + i * 2.0f);
        m_particles[i].setFillColor(sf::Color(255, 100 + i * 30, 0, 200 - i * 40));
        m_particles[i].setOrigin(5.0f + i * 2.0f, 5.0f + i * 2.0f);
    }
}

PlasmaLeak::~PlasmaLeak() {}

void PlasmaLeak::update(float deltaTime) {
    m_position.x -= SCROLL_SPEED * deltaTime;
    m_animTimer  += deltaTime;
}

void PlasmaLeak::draw(sf::RenderWindow& window) {
    for (int i = 0; i < 4; ++i) {
        float wave = std::sin(m_animTimer * 6.0f + i * 1.2f) * 4.0f;
        m_particles[i].setPosition(
            m_position.x + 15.0f,
            m_position.y + 15.0f - i * 9.0f + wave
        );
        sf::Uint8 alpha = static_cast<sf::Uint8>(200 - i * 40);
        sf::Color c = m_particles[i].getFillColor();
        c.a = alpha;
        m_particles[i].setFillColor(c);
        window.draw(m_particles[i]);
    }
}

// ========== SecurityDrone (aérien) ==========

SecurityDrone::SecurityDrone(float x)
    : Obstacle(x, Constants::AERIAL_OBSTACLE_Y, 50.0f, 30.0f, ObstacleType::AERIAL),
      m_hoverTimer(0.0f),
      m_baseY(Constants::AERIAL_OBSTACLE_Y)
{
    m_body.setRadius(18.0f);
    m_body.setFillColor(sf::Color(80, 80, 100));
    m_body.setOutlineColor(sf::Color(200, 50, 50));
    m_body.setOutlineThickness(2.0f);
    m_body.setOrigin(18.0f, 18.0f);

    m_blades[0].setSize(sf::Vector2f(40.0f, 5.0f));
    m_blades[0].setOrigin(20.0f, 2.5f);
    m_blades[0].setFillColor(sf::Color(150, 150, 180, 180));

    m_blades[1].setSize(sf::Vector2f(5.0f, 40.0f));
    m_blades[1].setOrigin(2.5f, 20.0f);
    m_blades[1].setFillColor(sf::Color(150, 150, 180, 180));
}

SecurityDrone::~SecurityDrone() {}

void SecurityDrone::update(float deltaTime) {
    m_position.x -= SCROLL_SPEED * deltaTime;
    m_hoverTimer += deltaTime;
    m_position.y = m_baseY + std::sin(m_hoverTimer * 3.0f) * 8.0f;
}

void SecurityDrone::draw(sf::RenderWindow& window) {
    float cx = m_position.x + 25.0f;
    float cy = m_position.y + 15.0f;

    float bladeAngle = m_hoverTimer * 300.0f;
    m_blades[0].setPosition(cx, cy);
    m_blades[0].setRotation(bladeAngle);
    m_blades[1].setPosition(cx, cy);
    m_blades[1].setRotation(bladeAngle + 45.0f);

    window.draw(m_blades[0]);
    window.draw(m_blades[1]);

    m_body.setPosition(cx, cy);
    window.draw(m_body);

    // Lumière rouge clignotante
    float blink = (std::sin(m_hoverTimer * 8.0f) + 1.0f) * 0.5f;
    sf::CircleShape light(4.0f);
    light.setOrigin(4.0f, 4.0f);
    light.setPosition(cx, cy + 5.0f);
    light.setFillColor(sf::Color(255, 0, 0, static_cast<sf::Uint8>(blink * 255)));
    window.draw(light);
}

// ========== TornDuct (aérien) ==========

TornDuct::TornDuct(float x)
    : Obstacle(x, Constants::AERIAL_OBSTACLE_Y - 10.0f, 35.0f, 35.0f, ObstacleType::AERIAL),
      m_sparkTimer(0.0f)
{
    m_duct.setSize(sf::Vector2f(35.0f, 35.0f));
    m_duct.setFillColor(sf::Color(90, 90, 110));
    m_duct.setOutlineColor(sf::Color(150, 150, 180));
    m_duct.setOutlineThickness(2.0f);

    for (int i = 0; i < 3; ++i) {
        m_sparks[i].setSize(sf::Vector2f(3.0f, 8.0f));
        m_sparks[i].setFillColor(sf::Color(255, 220, 0));
    }
}

TornDuct::~TornDuct() {}

void TornDuct::update(float deltaTime) {
    m_position.x -= SCROLL_SPEED * deltaTime;
    m_sparkTimer += deltaTime;
}

void TornDuct::draw(sf::RenderWindow& window) {
    m_duct.setPosition(m_position.x, m_position.y);
    window.draw(m_duct);

    // Étincelles animées
    for (int i = 0; i < 3; ++i) {
        float sparkX = m_position.x + 5.0f + i * 10.0f;
        float sparkY = m_position.y + m_size.y;
        float flicker = std::sin(m_sparkTimer * 15.0f + i * 2.1f) * 6.0f;
        m_sparks[i].setPosition(sparkX, sparkY + flicker);

        sf::Uint8 alpha = static_cast<sf::Uint8>(std::abs(std::sin(m_sparkTimer * 12.0f + i)) * 255.0f);
        m_sparks[i].setFillColor(sf::Color(255, 220, 0, alpha));
        window.draw(m_sparks[i]);
    }
}
