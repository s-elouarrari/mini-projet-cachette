#pragma once
#include "Entity.hpp"

// ============================================================
//  Player.hpp — Le robot joueur
//
//  NOUVEAU : systeme de sante (3 coeurs)
//    - takeDamage() retire 1 coeur et active l'invincibilite
//    - Pendant l'invincibilite, le robot clignote (visible / invisible)
//    - A 0 coeurs : le robot est mort (Game Over)
// ============================================================

enum class PlayerAction {
    IDLE,      // Course normale
    JUMPING,   // En l'air
    CROUCHING  // Accroupi
};

class Player : public Entity {
public:
    Player(float startX, float startY);
    ~Player() override;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

    void jump();
    void crouch(bool isCrouching);

    // --- Sante ---
    void takeDamage();          // Retire 1 coeur (si pas invincible)
    bool isInvincible() const;  // Vrai pendant la periode de grace
    bool isDead()       const;  // Vrai quand health <= 0
    int  getHealth()    const;

    PlayerAction getAction() const;

private:
    // Constantes physiques
    static constexpr float GRAVITY       = 1200.0f;
    static constexpr float JUMP_FORCE    = -560.0f;
    static constexpr float GROUND_Y      = 430.0f;
    static constexpr float NORMAL_HEIGHT = 60.0f;
    static constexpr float CROUCH_HEIGHT = 30.0f;

    PlayerAction m_action;
    sf::Vector2f m_velocity;
    bool         m_isOnGround;

    // Sante
    int   m_health;              // Nombre de coeurs restants
    float m_invincibilityTimer;  // Compte a rebours d'invincibilite (secondes)

    // Visuels
    sf::RectangleShape m_bodyShape;
    sf::RectangleShape m_helmetShape;
    sf::CircleShape    m_visorShape;

    void applyGravity(float deltaTime);
    void clampToGround();
    void updateVisuals();
};
