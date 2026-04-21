#pragma once
#include "Entity.hpp"

// ============================================================
//  Player.hpp — Le robot joueur
//
//  HITBOX DE REFERENCE (calculees, pas au hasard)
//  GROUND_Y     = 490
//  NORMAL_HEIGHT= 80    → debout   : Y 410..490
//  CROUCH_HEIGHT= 35    → accroupi : Y 455..490
//  JUMP_FORCE   = -560  → apex     : Y ~268..348
//
//  Drone aerien : Y 350..445  → debout touche, accroupi passe
//  Bloc sol     : Y 390..490  → debout ET accroupi touchent → SAUTER
// ============================================================

enum class PlayerAction {
    IDLE,
    JUMPING,
    CROUCHING
};

class Player : public Entity {
public:
    Player(float startX, float startY);
    ~Player() override;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

    void jump();
    void crouch(bool isCrouching);

    void takeDamage();
    bool isInvincible() const;
    bool isDead()       const;
    int  getHealth()    const;

    PlayerAction getAction() const;

private:
    static constexpr float GRAVITY       = 1100.0f;
    static constexpr float JUMP_FORCE    = -560.0f;  // plus fort pour sauter le bloc sol
    static constexpr float GROUND_Y      = 490.0f;
    static constexpr float NORMAL_HEIGHT = 80.0f;    // hauteur totale debout
    static constexpr float CROUCH_HEIGHT = 35.0f;    // hauteur accroupi (moins de moitie)

    PlayerAction m_action;
    sf::Vector2f m_velocity;
    bool         m_isOnGround;

    int   m_health;
    float m_invincibilityTimer;

    sf::RectangleShape m_bodyShape;
    sf::RectangleShape m_helmetShape;
    sf::CircleShape    m_visorShape;

    void applyGravity(float deltaTime);
    void clampToGround();
    void updateVisuals();
};
