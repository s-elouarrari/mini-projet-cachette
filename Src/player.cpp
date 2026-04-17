#include "Player.hpp"
#include "Utils.hpp"

// ============================================================
//  Player.cpp — Logique joueur : physique + états + visuels
// ============================================================

Player::Player(float startX, float startY)
    : Entity(startX, startY, 30.0f, NORMAL_HEIGHT),
      m_state(PlayerState::HEALTHY),
      m_action(PlayerAction::IDLE),
      m_velocity(0.0f, 0.0f),
      m_isOnGround(true),
      m_invincibilityTimer(0.0f),
      m_hurtFlashColor(sf::Color::White),
      m_hurtFlashTimer(0.0f)
{
    // Corps de l'astronaute
    m_bodyShape.setSize(sf::Vector2f(30.0f, 38.0f));
    m_bodyShape.setFillColor(sf::Color(180, 190, 210));
    m_bodyShape.setOutlineColor(sf::Color(100, 120, 160));
    m_bodyShape.setOutlineThickness(1.5f);

    // Casque
    m_helmetShape.setSize(sf::Vector2f(28.0f, 24.0f));
    m_helmetShape.setFillColor(sf::Color(200, 210, 230));
    m_helmetShape.setOutlineColor(sf::Color(100, 120, 160));
    m_helmetShape.setOutlineThickness(1.5f);

    // Visière
    m_visorShape.setRadius(9.0f);
    m_visorShape.setFillColor(sf::Color(0, 200, 255, 180));
    m_visorShape.setOutlineColor(sf::Color(0, 220, 255));
    m_visorShape.setOutlineThickness(1.0f);

    updateVisuals();
}

Player::~Player() {
    // Pas de ressources dynamiques
}

void Player::update(float deltaTime) {
    if (m_state == PlayerState::DEAD) return;

    applyGravity(deltaTime);
    m_position.y += m_velocity.y * deltaTime;
    clampToGround();

    // Timer d'invincibilité
    if (m_invincibilityTimer > 0.0f) {
        m_invincibilityTimer -= deltaTime;
        if (m_invincibilityTimer <= 0.0f) {
            m_invincibilityTimer = 0.0f;
            if (m_state == PlayerState::HURT) {
                m_state = PlayerState::HEALTHY;
            }
        }
    }

    // Flash rouge après choc
    if (m_hurtFlashTimer > 0.0f) {
        m_hurtFlashTimer -= deltaTime;
    }

    updateVisuals();
}

void Player::draw(sf::RenderWindow& window) {
    if (m_state == PlayerState::DEAD) return;

    // Clignotement si invincible
    bool visible = true;
    if (m_invincibilityTimer > 0.0f) {
        int blinkFrame = static_cast<int>(m_invincibilityTimer * 10) % 2;
        visible = (blinkFrame == 0);
    }
    if (!visible) return;

    window.draw(m_bodyShape);
    window.draw(m_helmetShape);
    window.draw(m_visorShape);
}

void Player::jump() {
    if (m_isOnGround && m_state != PlayerState::DEAD) {
        m_velocity.y   = JUMP_FORCE;
        m_isOnGround   = false;
        m_action       = PlayerAction::JUMPING;
    }
}

void Player::crouch(bool isCrouching) {
    if (m_state == PlayerState::DEAD) return;
    if (isCrouching && m_isOnGround) {
        m_size.y = CROUCH_HEIGHT;
        m_action = PlayerAction::CROUCHING;
    } else {
        m_size.y = NORMAL_HEIGHT;
        if (m_isOnGround) m_action = PlayerAction::IDLE;
    }
    m_bodyShape.setSize(sf::Vector2f(30.0f, m_size.y));
    updateVisuals();
}

void Player::takeDamage() {
    if (!canTakeDamage()) return;

    if (m_state == PlayerState::HEALTHY) {
        m_state = PlayerState::HURT;
    } else if (m_state == PlayerState::HURT) {
        m_state = PlayerState::DEAD;
        m_isAlive = false;
        return;
    }
    m_invincibilityTimer = INVINCIBILITY_TIME;
    m_hurtFlashTimer     = 0.3f;
}

PlayerState Player::getState() const {
    return m_state;
}

PlayerAction Player::getAction() const {
    return m_action;
}

bool Player::canTakeDamage() const {
    return m_invincibilityTimer <= 0.0f && m_state != PlayerState::DEAD;
}

// ---- Méthodes privées ----

void Player::applyGravity(float deltaTime) {
    if (!m_isOnGround) {
        m_velocity.y += GRAVITY * deltaTime;
    }
}

void Player::clampToGround() {
    float groundLevel = GROUND_Y - m_size.y;
    if (m_position.y >= groundLevel) {
        m_position.y = groundLevel;
        m_velocity.y = 0.0f;
        m_isOnGround = true;
        if (m_action == PlayerAction::JUMPING) {
            m_action = PlayerAction::IDLE;
        }
    }
}

void Player::updateVisuals() {
    sf::Color bodyColor = (m_hurtFlashTimer > 0.0f)
        ? sf::Color(255, 80, 80)
        : sf::Color(180, 190, 210);

    m_bodyShape.setFillColor(bodyColor);
    m_bodyShape.setPosition(m_position.x, m_position.y + (m_size.y - m_bodyShape.getSize().y));

    m_helmetShape.setPosition(m_position.x + 1.0f, m_position.y - 22.0f);

    m_visorShape.setPosition(
        m_position.x + 5.0f,
        m_position.y - 20.0f
    );
}
