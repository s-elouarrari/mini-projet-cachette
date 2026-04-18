#include "player.hpp"
#include "Utils.hpp"

// ============================================================
//  Player.cpp — Logique du robot joueur
//
//  Physique :
//    - Gravite appliquee en continu quand le joueur est en l'air
//    - Saut : on applique une force negative sur l'axe Y
//    - Accroupissement : on reduit la hauteur de la hitbox
// ============================================================

Player::Player(float startX, float startY)
    : Entity(startX, startY, 30.0f, NORMAL_HEIGHT),
      m_action(PlayerAction::IDLE),
      m_velocity(0.0f, 0.0f),
      m_isOnGround(true)
{
    // --- Corps du robot (rectangle gris metallique) ---
    m_bodyShape.setSize(sf::Vector2f(30.0f, 38.0f));
    m_bodyShape.setFillColor(sf::Color(180, 190, 210));
    m_bodyShape.setOutlineColor(sf::Color(100, 120, 160));
    m_bodyShape.setOutlineThickness(1.5f);

    // --- Casque (rectangle plus clair) ---
    m_helmetShape.setSize(sf::Vector2f(28.0f, 24.0f));
    m_helmetShape.setFillColor(sf::Color(200, 210, 230));
    m_helmetShape.setOutlineColor(sf::Color(100, 120, 160));
    m_helmetShape.setOutlineThickness(1.5f);

    // --- Visiere (cercle cyan translucide) ---
    m_visorShape.setRadius(9.0f);
    m_visorShape.setFillColor(sf::Color(0, 200, 255, 180));
    m_visorShape.setOutlineColor(sf::Color(0, 220, 255));
    m_visorShape.setOutlineThickness(1.0f);

    updateVisuals();
}

Player::~Player() {
    // Pas de ressources dynamiques a liberer
}

// ============================================================
//  update() — Physique frame par frame
// ============================================================

void Player::update(float deltaTime) {
    // Applique la gravite si le joueur est en l'air
    applyGravity(deltaTime);

    // Deplacement vertical
    m_position.y += m_velocity.y * deltaTime;

    // On s'assure que le joueur ne passe pas sous le sol
    clampToGround();

    // Mise a jour des positions des formes visuelles
    updateVisuals();
}

// ============================================================
//  draw() — Affichage du robot
// ============================================================

void Player::draw(sf::RenderWindow& window) {
    window.draw(m_bodyShape);
    window.draw(m_helmetShape);
    window.draw(m_visorShape);
}

// ============================================================
//  jump() — Saut lunaire (faible gravite)
// ============================================================

void Player::jump() {
    // On ne peut sauter que si on est sur le sol
    if (m_isOnGround) {
        m_velocity.y = JUMP_FORCE; // Force vers le haut (valeur negative)
        m_isOnGround = false;
        m_action     = PlayerAction::JUMPING;
    }
}

// ============================================================
//  crouch() — Accroupissement
// ============================================================

void Player::crouch(bool isCrouching) {
    if (isCrouching && m_isOnGround) {
        // On reduit la hauteur de la hitbox
        m_size.y = CROUCH_HEIGHT;
        m_bodyShape.setSize(sf::Vector2f(30.0f, CROUCH_HEIGHT));
        m_action = PlayerAction::CROUCHING;
    } else {
        // On remet la hauteur normale
        m_size.y = NORMAL_HEIGHT;
        m_bodyShape.setSize(sf::Vector2f(30.0f, NORMAL_HEIGHT));
        if (m_isOnGround) m_action = PlayerAction::IDLE;
    }
    updateVisuals();
}

PlayerAction Player::getAction() const {
    return m_action;
}

// ============================================================
//  Methodes privees : physique et visuels
// ============================================================

void Player::applyGravity(float deltaTime) {
    // On applique la gravite seulement quand le joueur est en l'air
    if (!m_isOnGround) {
        m_velocity.y += GRAVITY * deltaTime;
    }
}

void Player::clampToGround() {
    // Niveau du sol selon la hauteur actuelle du joueur
    float groundLevel = GROUND_Y - m_size.y;

    if (m_position.y >= groundLevel) {
        m_position.y = groundLevel;  // On pose le joueur sur le sol
        m_velocity.y = 0.0f;         // On annule la vitesse verticale
        m_isOnGround = true;

        // Si on atterrit, on revient a l'etat IDLE
        if (m_action == PlayerAction::JUMPING) {
            m_action = PlayerAction::IDLE;
        }
    }
}

void Player::updateVisuals() {
    // Corps : en bas de la zone du joueur
    m_bodyShape.setPosition(
        m_position.x,
        m_position.y + (m_size.y - m_bodyShape.getSize().y)
    );
    // Casque : au-dessus du corps
    m_helmetShape.setPosition(m_position.x + 1.0f, m_position.y - 22.0f);
    // Visiere : sur le casque
    m_visorShape.setPosition(m_position.x + 5.0f, m_position.y - 20.0f);
}
