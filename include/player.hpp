#pragma once
#include "Entity.hpp"

// ============================================================
//  Player.hpp — Le robot joueur
//  Encapsulation : tous les attributs sont prives
// ============================================================

// Action actuelle du joueur (utile pour les animations)
enum class PlayerAction {
    IDLE,      // Marche normale
    JUMPING,   // En l'air
    CROUCHING  // Accroupi
};

class Player : public Entity {
public:
    Player(float startX, float startY);
    ~Player() override;

    // Methodes principales (surchargees depuis Entity)
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

    // Actions du joueur
    void jump();
    void crouch(bool isCrouching);

    // Accesseur
    PlayerAction getAction() const;

private:
    // Constantes physiques du joueur
    static constexpr float GRAVITY       = 1200.0f; // Force de gravite
    static constexpr float JUMP_FORCE    = -560.0f; // Force du saut (negative = vers le haut)
    static constexpr float GROUND_Y      = 430.0f;  // Niveau du sol
    static constexpr float NORMAL_HEIGHT = 60.0f;   // Hauteur normale
    static constexpr float CROUCH_HEIGHT = 30.0f;   // Hauteur accroupi

    PlayerAction m_action;      // Action courante
    sf::Vector2f m_velocity;    // Vitesse (x, y)
    bool         m_isOnGround;  // Vrai si le joueur est au sol

    // Formes visuelles du robot
    sf::RectangleShape m_bodyShape;   // Corps
    sf::RectangleShape m_helmetShape; // Casque
    sf::CircleShape    m_visorShape;  // Visiere

    // Methodes internes
    void applyGravity(float deltaTime);
    void clampToGround();
    void updateVisuals();
};
