#pragma once
#include "Entity.hpp"

// ============================================================
//  Obstacle.hpp — Classe mère + sous-classes (Polymorphisme)
// ============================================================

enum class ObstacleType {
    GROUND,   // Jump requis
    AERIAL    // Crouch requis
};

// ---- Classe mère Obstacle ----
class Obstacle : public Entity {
public:
    Obstacle(float x, float y, float width, float height, ObstacleType type);
    virtual ~Obstacle() override;

    virtual void update(float deltaTime) override;
    virtual void draw(sf::RenderWindow& window) override;

    ObstacleType getType()     const;
    bool         isOffScreen() const;

protected:
    static constexpr float SCROLL_SPEED = 300.0f;
    ObstacleType m_type;
    sf::Color    m_primaryColor;
    sf::Color    m_accentColor;
};

// ---- Sous-classe : Conteneur Magnétique (sol) ----
class MagneticContainer : public Obstacle {
public:
    MagneticContainer(float x);
    ~MagneticContainer() override;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::RectangleShape m_topBar;
    float              m_glowTimer;
};

// ---- Sous-classe : Fuite de Plasma (sol) ----
class PlasmaLeak : public Obstacle {
public:
    PlasmaLeak(float x);
    ~PlasmaLeak() override;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::CircleShape m_particles[4];
    float           m_animTimer;
};

// ---- Sous-classe : Drone de Sécurité (aérien) ----
class SecurityDrone : public Obstacle {
public:
    SecurityDrone(float x);
    ~SecurityDrone() override;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::CircleShape m_body;
    sf::RectangleShape m_blades[2];
    float           m_hoverTimer;
    float           m_baseY;
};

// ---- Sous-classe : Conduit Arraché (aérien) ----
class TornDuct : public Obstacle {
public:
    TornDuct(float x);
    ~TornDuct() override;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::RectangleShape m_duct;
    sf::RectangleShape m_sparks[3];
    float              m_sparkTimer;
};
