#pragma once
#include "Entity.hpp"

// ============================================================
//  Player.hpp — Classe Joueur (Encapsulation stricte)
// ============================================================

enum class PlayerState {
    HEALTHY,
    HURT,
    DEAD
};

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

    PlayerState  getState()  const;
    PlayerAction getAction() const;
    bool         canTakeDamage() const;

private:
    static constexpr float GRAVITY          = 1200.0f;
    static constexpr float JUMP_FORCE       = -560.0f;
    static constexpr float GROUND_Y         = 430.0f;
    static constexpr float NORMAL_HEIGHT    = 60.0f;
    static constexpr float CROUCH_HEIGHT    = 30.0f;
    static constexpr float INVINCIBILITY_TIME = 1.5f;

    PlayerState  m_state;
    PlayerAction m_action;

    sf::Vector2f m_velocity;
    bool         m_isOnGround;
    float        m_invincibilityTimer;

    // Visuels
    sf::RectangleShape m_bodyShape;
    sf::RectangleShape m_helmetShape;
    sf::CircleShape    m_visorShape;
    sf::Color          m_hurtFlashColor;
    float              m_hurtFlashTimer;

    void applyGravity(float deltaTime);
    void updateVisuals();
    void clampToGround();
};
