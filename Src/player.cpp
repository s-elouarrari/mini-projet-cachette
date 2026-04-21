#include "../include/Player.hpp"
#include "../include/Utils.hpp"

// ============================================================
//  Player.cpp
//
//  HITBOX DU ROBOT — valeurs de reference
//  ═══════════════════════════════════════
//  GROUND_Y     = 490  (Y du sol, bord superieur)
//  NORMAL_HEIGHT= 80   (hauteur totale debout : corps+tete)
//  CROUCH_HEIGHT= 35   (hauteur accroupi : juste le corps aplati)
//
//  Etat DEBOUT    getBounds() : top=410, bottom=490  (h=80)
//  Etat ACCROUPI  getBounds() : top=455, bottom=490  (h=35)
//  Etat EN SAUT   getBounds() : top descend jusqu'a ~270 (apex)
//
//  DRONE (obstacle aerien) : top=350, bottom=445  (h=95)
//    → debout   410..490 vs 350..445 → overlap 410..445=35px  TOUCHE ✓
//    → accroupi 455..490 vs 350..445 → 455>445 → PAS de touche ✓
//    → saut apex ~270..350 vs 350..445 → bord exact, on descend
//      JUMP_FORCE=-560, GRAVITY=1100 → apex height=142px
//      top_apex = 410-142=268, bottom_apex=268+80=348 < 350 → PASSE ✓
//      (Le saut est juste assez haut pour passer par-dessus)
//
//  BOX (obstacle sol) : top=390, bottom=490  (h=100)
//    → debout   410..490 vs 390..490 → overlap 80px  TOUCHE ✓
//    → accroupi 455..490 vs 390..490 → overlap 35px  TOUCHE ✓
//      (accroupissement inutile — il faut SAUTER)
//    → saut apex bottom=348 < 390 → PASSE ✓
// ============================================================

Player::Player(float startX, float startY)
    : Entity(startX, startY, 36.0f, NORMAL_HEIGHT),
      m_action(PlayerAction::IDLE),
      m_velocity(0.0f, 0.0f),
      m_isOnGround(true),
      m_health(Constants::MAX_HEALTH),
      m_invincibilityTimer(0.0f)
{
    // Corps du robot (partie basse, plus large)
    // En mode debout : occcupe la moitie inferieure de la hitbox
    m_bodyShape.setSize(sf::Vector2f(36.0f, 42.0f));
    m_bodyShape.setFillColor(sf::Color(160, 175, 205));
    m_bodyShape.setOutlineColor(sf::Color(90, 115, 160));
    m_bodyShape.setOutlineThickness(2.0f);

    // Tete / casque (partie haute)
    m_helmetShape.setSize(sf::Vector2f(30.0f, 28.0f));
    m_helmetShape.setFillColor(sf::Color(195, 210, 230));
    m_helmetShape.setOutlineColor(sf::Color(90, 115, 160));
    m_helmetShape.setOutlineThickness(2.0f);

    // Visiere cyan lumineuse
    m_visorShape.setRadius(10.0f);
    m_visorShape.setFillColor(sf::Color(0, 210, 255, 200));
    m_visorShape.setOutlineColor(sf::Color(0, 235, 255));
    m_visorShape.setOutlineThickness(1.5f);

    updateVisuals();
}

Player::~Player() {}

// ============================================================
//  update() — physique + decompte invincibilite
// ============================================================
void Player::update(float deltaTime) {
    applyGravity(deltaTime);
    m_position.y += m_velocity.y * deltaTime;
    clampToGround();

    if (m_invincibilityTimer > 0.0f) {
        m_invincibilityTimer -= deltaTime;
        if (m_invincibilityTimer < 0.0f)
            m_invincibilityTimer = 0.0f;
    }

    updateVisuals();
}

// ============================================================
//  draw() — clignotement + couleur selon sante
// ============================================================
void Player::draw(sf::RenderWindow& window) {
    // Clignotement pendant l'invincibilite (alterne toutes les 0.1s)
    if (m_invincibilityTimer > 0.0f) {
        int frame = static_cast<int>(m_invincibilityTimer * 10.0f);
        if (frame % 2 == 0) return;
    }

    // Couleur selon la sante restante
    if (m_health == 2) {
        m_bodyShape.setFillColor(sf::Color(255, 155, 20));
        m_helmetShape.setFillColor(sf::Color(255, 180, 50));
        m_visorShape.setFillColor(sf::Color(255, 210, 0, 220));
    } else if (m_health == 1) {
        m_bodyShape.setFillColor(sf::Color(215, 45, 45));
        m_helmetShape.setFillColor(sf::Color(235, 70, 70));
        m_visorShape.setFillColor(sf::Color(255, 40, 40, 220));
    } else {
        m_bodyShape.setFillColor(sf::Color(160, 175, 205));
        m_helmetShape.setFillColor(sf::Color(195, 210, 230));
        m_visorShape.setFillColor(sf::Color(0, 210, 255, 200));
    }

    window.draw(m_bodyShape);

    // La tete n'est affichee qu'en mode debout ou en saut
    // En mode accroupi : le robot est "ecrase", pas de tete visible
    if (m_action != PlayerAction::CROUCHING) {
        window.draw(m_helmetShape);
        window.draw(m_visorShape);
    } else {
        // En accroupi : on dessine une petite antenne aplatie
        // pour montrer que le robot est la mais s'est baisse
        sf::RectangleShape flat(sf::Vector2f(36.0f, 6.0f));
        flat.setFillColor(sf::Color(195, 210, 230));
        flat.setOutlineColor(sf::Color(90, 115, 160));
        flat.setOutlineThickness(1.0f);
        flat.setPosition(m_position.x, m_position.y);
        window.draw(flat);
    }
}

// ============================================================
//  jump() — saut depuis le sol uniquement
// ============================================================
void Player::jump() {
    if (m_isOnGround) {
        m_velocity.y = JUMP_FORCE;
        m_isOnGround = false;
        m_action     = PlayerAction::JUMPING;
    }
}

// ============================================================
//  crouch() — accroupissement REEL
//
//  Accroupi  : height=CROUCH_HEIGHT, position.y = GROUND_Y - CROUCH_HEIGHT
//              getBounds() → top=455, bottom=490
//  Debout    : height=NORMAL_HEIGHT, position.y = GROUND_Y - NORMAL_HEIGHT
//              getBounds() → top=410, bottom=490
//
//  Le bas (bottom=490) reste TOUJOURS colle au sol.
//  Seul le sommet (top) monte : 410 → 455 en s'accroupissant.
//  => Le robot passe sous le drone (qui finit a bottom=445).
// ============================================================
void Player::crouch(bool isCrouching) {
    if (isCrouching && m_isOnGround) {
        // S'ACCROUPIR : hitbox reduite, collee au sol
        m_size.y     = CROUCH_HEIGHT;
        m_position.y = GROUND_Y - CROUCH_HEIGHT;  // 490 - 35 = 455
        m_bodyShape.setSize(sf::Vector2f(36.0f, CROUCH_HEIGHT));
        m_action = PlayerAction::CROUCHING;
    } else if (!isCrouching && m_isOnGround) {
        // SE RELEVER : retour a la taille normale
        m_size.y     = NORMAL_HEIGHT;
        m_position.y = GROUND_Y - NORMAL_HEIGHT;  // 490 - 80 = 410
        m_bodyShape.setSize(sf::Vector2f(36.0f, NORMAL_HEIGHT));
        m_action = PlayerAction::IDLE;
    }
    updateVisuals();
}

// ============================================================
//  takeDamage() — -1 coeur + periode d'invincibilite
// ============================================================
void Player::takeDamage() {
    if (m_invincibilityTimer > 0.0f) return;
    m_health -= 1;
    m_invincibilityTimer = Constants::INVINCIBILITY_AFTER_HIT;
}

bool Player::isInvincible() const { return m_invincibilityTimer > 0.0f; }
bool Player::isDead()       const { return m_health <= 0; }
int  Player::getHealth()    const { return m_health; }
PlayerAction Player::getAction() const { return m_action; }

// ============================================================
//  Methodes internes
// ============================================================

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

// ============================================================
//  updateVisuals() — positionne les formes selon l'etat
//
//  Repere : m_position.y = sommet de la hitbox (getBounds().top)
//
//  DEBOUT (h=80, pos.y=410) :
//    Corps    : y = 410 + (80-42) = 448  →  448..490
//    Casque   : y = 410 - 30      = 380  →  380..408
//    Visiere  : y = 385           = 385
//
//  ACCROUPI (h=35, pos.y=455) :
//    Corps    : y = 455 + (35-35) = 455  →  455..490  (corp aplati)
//    Casque   : masque — tete dans les epaules
//    Barre    : y = 455            (aplatie en haut)
// ============================================================
void Player::updateVisuals() {
    // Corps : toujours en bas de la hitbox
    m_bodyShape.setPosition(
        m_position.x,
        m_position.y + (m_size.y - m_bodyShape.getSize().y)
    );

    // Casque : juste au-dessus du sommet de la hitbox
    m_helmetShape.setPosition(m_position.x + 3.0f, m_position.y - 30.0f);

    // Visiere : centree sur le casque
    m_visorShape.setPosition(m_position.x + 8.0f, m_position.y - 26.0f);
}
