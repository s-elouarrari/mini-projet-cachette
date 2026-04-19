#include "../include/Player.hpp"
#include "../include/Utils.hpp"

// ============================================================
//  Player.cpp — Physique + sante du robot
//
//  SYSTEME DE SANTE :
//    1. Le robot demarre avec MAX_HEALTH coeurs (3).
//    2. Quand il touche un obstacle, takeDamage() est appele :
//       - Si invincible : on ignore le choc
//       - Sinon : on retire 1 coeur + on active l'invincibilite
//    3. Pendant l'invincibilite, le robot clignote (alternance
//       visible / invisible toutes les 0.1 secondes).
//    4. Quand health == 0 : isDead() retourne true => Game Over.
// ============================================================

Player::Player(float startX, float startY)
    : Entity(startX, startY, 30.0f, NORMAL_HEIGHT),
      m_action(PlayerAction::IDLE),
      m_velocity(0.0f, 0.0f),
      m_isOnGround(true),
      m_health(Constants::MAX_HEALTH),
      m_invincibilityTimer(0.0f)
{
    // Corps gris metallique
    m_bodyShape.setSize(sf::Vector2f(30.0f, 38.0f));
    m_bodyShape.setFillColor(sf::Color(180, 190, 210));
    m_bodyShape.setOutlineColor(sf::Color(100, 120, 160));
    m_bodyShape.setOutlineThickness(1.5f);

    // Casque
    m_helmetShape.setSize(sf::Vector2f(28.0f, 24.0f));
    m_helmetShape.setFillColor(sf::Color(200, 210, 230));
    m_helmetShape.setOutlineColor(sf::Color(100, 120, 160));
    m_helmetShape.setOutlineThickness(1.5f);

    // Visiere cyan
    m_visorShape.setRadius(9.0f);
    m_visorShape.setFillColor(sf::Color(0, 200, 255, 180));
    m_visorShape.setOutlineColor(sf::Color(0, 220, 255));
    m_visorShape.setOutlineThickness(1.0f);

    updateVisuals();
}

Player::~Player() {}

// ============================================================
//  update() — Physique + decompte invincibilite
// ============================================================
void Player::update(float deltaTime) {
    applyGravity(deltaTime);
    m_position.y += m_velocity.y * deltaTime;
    clampToGround();

    // Decompte de l'invincibilite
    if (m_invincibilityTimer > 0.0f) {
        m_invincibilityTimer -= deltaTime;
        if (m_invincibilityTimer < 0.0f)
            m_invincibilityTimer = 0.0f;
    }

    updateVisuals();
}

// ============================================================
//  draw() — Clignotement si invincible, rouge si blesse
// ============================================================
void Player::draw(sf::RenderWindow& window) {
    // Clignotement : visible pendant les frames paires, cache pendant les impaires
    if (m_invincibilityTimer > 0.0f) {
        // On utilise le timer pour alterner toutes les 0.1s
        int frame = static_cast<int>(m_invincibilityTimer * 10.0f);
        if (frame % 2 == 0) return; // Frame cachee => on ne dessine rien
    }

    // Couleur selon la sante : orange si blesse (2 coeurs), rouge si critique (1 coeur)
    if (m_health == 2) {
        m_bodyShape.setFillColor(sf::Color(255, 180, 80));  // Orange : attention
        m_helmetShape.setFillColor(sf::Color(255, 200, 100));
    } else if (m_health == 1) {
        m_bodyShape.setFillColor(sf::Color(220, 80, 80));   // Rouge : danger
        m_helmetShape.setFillColor(sf::Color(240, 100, 100));
    } else {
        m_bodyShape.setFillColor(sf::Color(180, 190, 210)); // Gris : normal
        m_helmetShape.setFillColor(sf::Color(200, 210, 230));
    }

    window.draw(m_bodyShape);
    window.draw(m_helmetShape);
    window.draw(m_visorShape);
}

void Player::jump() {
    if (m_isOnGround) {
        m_velocity.y = JUMP_FORCE;
        m_isOnGround = false;
        m_action     = PlayerAction::JUMPING;
    }
}

void Player::crouch(bool isCrouching) {
    if (isCrouching && m_isOnGround) {
        m_size.y = CROUCH_HEIGHT;
        m_bodyShape.setSize(sf::Vector2f(30.0f, CROUCH_HEIGHT));
        m_action = PlayerAction::CROUCHING;
    } else {
        m_size.y = NORMAL_HEIGHT;
        m_bodyShape.setSize(sf::Vector2f(30.0f, NORMAL_HEIGHT));
        if (m_isOnGround) m_action = PlayerAction::IDLE;
    }
    updateVisuals();
}

// ============================================================
//  takeDamage() — Retire 1 coeur si le robot n'est pas invincible
// ============================================================
void Player::takeDamage() {
    if (m_invincibilityTimer > 0.0f) return; // Grace : on ignore

    m_health -= 1;

    // Active l'invincibilite temporaire pour eviter les chocs en rafale
    m_invincibilityTimer = Constants::INVINCIBILITY_AFTER_HIT;
}

bool Player::isInvincible() const {
    return m_invincibilityTimer > 0.0f;
}

bool Player::isDead() const {
    return m_health <= 0;
}

int Player::getHealth() const {
    return m_health;
}

PlayerAction Player::getAction() const {
    return m_action;
}

void Player::applyGravity(float deltaTime) {
    if (!m_isOnGround)
        m_velocity.y += GRAVITY * deltaTime;
}

void Player::clampToGround() {
    float groundLevel = GROUND_Y - m_size.y;
    if (m_position.y >= groundLevel) {
        m_position.y = groundLevel;
        m_velocity.y = 0.0f;
        m_isOnGround = true;
        if (m_action == PlayerAction::JUMPING)
            m_action = PlayerAction::IDLE;
    }
}

void Player::updateVisuals() {
    m_bodyShape.setPosition(
        m_position.x,
        m_position.y + (m_size.y - m_bodyShape.getSize().y)
    );
    m_helmetShape.setPosition(m_position.x + 1.0f, m_position.y - 22.0f);
    m_visorShape.setPosition(m_position.x + 5.0f,  m_position.y - 20.0f);
}
