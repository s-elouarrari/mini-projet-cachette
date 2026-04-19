#pragma once
#include "Entity.hpp"

// ============================================================
//  Obstacle.hpp — 2 types d'obstacles
//
//  HERITAGE : Entity -> Obstacle -> MagneticContainer
//                                -> SecurityDrone
//
//  NOUVEAU : update() recoit la vitesse actuelle en parametre
//  pour que la difficulte progressive fonctionne correctement.
// ============================================================

enum class ObstacleType {
    GROUND,  // Au sol : il faut SAUTER
    AERIAL   // En l'air : il faut SE BAISSER
};

// ---- Classe mere ----
class Obstacle : public Entity {
public:
    Obstacle(float x, float y, float width, float height, ObstacleType type);
    virtual ~Obstacle() override;

    // MODIFIE : on passe la vitesse en parametre pour la difficulte progressive
    virtual void update(float deltaTime) override;
    virtual void draw(sf::RenderWindow& window) override;

    // Methode supplementaire pour passer la vitesse dynamique
    virtual void updateWithSpeed(float deltaTime, float speed);

    ObstacleType getType()     const;
    bool         isOffScreen() const;

protected:
    float m_currentSpeed; // Vitesse actuelle (peut changer avec la difficulte)
    ObstacleType m_type;
};

// ---- Conteneur Magnetique : obstacle AU SOL ----
// Le joueur doit SAUTER par-dessus
class MagneticContainer : public Obstacle {
public:
    MagneticContainer(float x);
    ~MagneticContainer() override;

    void updateWithSpeed(float deltaTime, float speed) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::RectangleShape m_topBar;
    float              m_glowTimer;
};

// ---- Drone de securite : obstacle AERIEN ----
// Le joueur doit SE BAISSER pour passer dessous
class SecurityDrone : public Obstacle {
public:
    SecurityDrone(float x);
    ~SecurityDrone() override;

    void updateWithSpeed(float deltaTime, float speed) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::CircleShape    m_body;
    sf::RectangleShape m_blades[2];
    float              m_hoverTimer;
    float              m_baseY;
};
