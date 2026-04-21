#include "../include/Obstacle.hpp"
#include "../include/Utils.hpp"
#include <cmath>

// ============================================================
//  Obstacle.cpp
//
//  RAPPEL HITBOX DU ROBOT (Player.hpp) :
//  ─────────────────────────────────────
//  GROUND_Y = 490
//  Debout   : top=410, bottom=490  (NORMAL_HEIGHT=80)
//  Accroupi : top=455, bottom=490  (CROUCH_HEIGHT=35)
//  Saut apex: JUMP_FORCE=-560, GRAVITY=1100
//    t_apex = 560/1100 = 0.509s
//    h_apex = 560*0.509 - 0.5*1100*0.509^2 = 142px
//    top_apex  = 410 - 142 = 268
//    bot_apex  = 268 + 80  = 348
//
//  ═══════════════════════════════════════════════════════════
//  BLOC SOL — MagneticContainer
//  ═══════════════════════════════════════════════════════════
//  Hitbox : top=390, bottom=490, width=56, height=100
//
//  Robot DEBOUT   410..490 vs 390..490 → overlap 80px → TOUCHE ✓
//  Robot ACCROUPI 455..490 vs 390..490 → overlap 35px → TOUCHE ✓
//    (impossible de l'eviter en se baissant — il faut SAUTER)
//  Robot EN SAUT  bot_apex=348 < top_bloc=390 → PAS de touche ✓
//    (le saut est suffisant pour passer par-dessus)
//
//  ═══════════════════════════════════════════════════════════
//  DRONE AERIEN — SecurityDrone
//  ═══════════════════════════════════════════════════════════
//  Hitbox : top=350, bottom=445, width=60, height=95
//
//  Robot DEBOUT   410..490 vs 350..445 → overlap 410..445=35px → TOUCHE ✓
//  Robot ACCROUPI 455..490 vs 350..445 → 455>445 → PAS de touche ✓
//    (la seule facon de passer est de SE BAISSER)
//  Robot EN SAUT  bot_apex=348 < top_drone=350 → PAS de touche ✓
//    (le saut passe juste mais ne sert a rien : le drone est trop bas
//     pour sauter par-dessus de facon fiable)
//
//  NOTE : on laisse le saut presque fonctionner sur le drone exprès
//  pour que le joueur ait une legere marge, mais SE BAISSER reste
//  la solution propre et fiable.
// ============================================================

// ──────────────────────────────────────────────────────────────
//  Obstacle — classe mere
// ──────────────────────────────────────────────────────────────
Obstacle::Obstacle(float x, float y, float width, float height, ObstacleType type)
    : Entity(x, y, width, height),
      m_currentSpeed(Constants::SCROLL_SPEED_BASE),
      m_type(type)
{}

Obstacle::~Obstacle() {}

void Obstacle::update(float deltaTime) {
    m_position.x -= m_currentSpeed * deltaTime;
}

void Obstacle::draw(sf::RenderWindow& window) {
    m_shape.setPosition(m_position.x, m_position.y);
    window.draw(m_shape);
}

void Obstacle::updateWithSpeed(float deltaTime, float speed) {
    m_currentSpeed = speed;
    m_position.x  -= speed * deltaTime;
}

ObstacleType Obstacle::getType()     const { return m_type; }
bool         Obstacle::isOffScreen() const { return (m_position.x + m_size.x) < 0.0f; }

// ──────────────────────────────────────────────────────────────
//  MagneticContainer — OBSTACLE AU SOL
//  Hitbox  : Y=390, h=100  →  zone 390..490
//  Visuel  : grand bloc cyan/bleu avec bandes lumineuses
//  Action  : SAUTER
// ──────────────────────────────────────────────────────────────
MagneticContainer::MagneticContainer(float x)
    : Obstacle(x, 390.0f, 56.0f, 100.0f, ObstacleType::GROUND),
      m_glowTimer(0.0f)
{
    // Corps principal : zone 390..490
    m_shape.setSize(sf::Vector2f(56.0f, 100.0f));
    m_shape.setFillColor(sf::Color(30, 50, 100));
    m_shape.setOutlineColor(sf::Color(0, 180, 255));
    m_shape.setOutlineThickness(3.0f);

    // Barre lumineuse sur le dessus (alerte visuelle)
    m_topBar.setSize(sf::Vector2f(56.0f, 12.0f));
    m_topBar.setFillColor(sf::Color(0, 200, 255));
}

MagneticContainer::~MagneticContainer() {}

void MagneticContainer::updateWithSpeed(float deltaTime, float speed) {
    m_position.x -= speed * deltaTime;
    m_glowTimer  += deltaTime;
}

void MagneticContainer::draw(sf::RenderWindow& window) {
    float glow = (std::sin(m_glowTimer * 5.0f) + 1.0f) * 0.5f;
    sf::Color gc(
        0,
        static_cast<sf::Uint8>(120 + glow * 135),
        static_cast<sf::Uint8>(180 + glow * 75)
    );

    // Corps
    m_shape.setOutlineColor(gc);
    m_shape.setPosition(m_position.x, m_position.y);
    window.draw(m_shape);

    // Barre lumineuse en haut (au-dessus du corps, bien visible)
    m_topBar.setPosition(m_position.x, m_position.y - 12.0f);
    m_topBar.setFillColor(gc);
    window.draw(m_topBar);

    // Bandes horizontales decoratives
    for (int i = 0; i < 3; ++i) {
        sf::RectangleShape band(sf::Vector2f(56.0f, 6.0f));
        band.setFillColor(sf::Color(0, static_cast<sf::Uint8>(100 + i*30), 200, 160));
        band.setPosition(m_position.x, m_position.y + 15.0f + i * 28.0f);
        window.draw(band);
    }

    // Symbole "M" magnetique : deux barres laterales
    sf::RectangleShape sL(sf::Vector2f(8.0f, 40.0f));
    sL.setFillColor(sf::Color(0, 170, 255, 200));
    sL.setPosition(m_position.x + 5.0f, m_position.y + 12.0f);
    window.draw(sL);

    sf::RectangleShape sR(sf::Vector2f(8.0f, 40.0f));
    sR.setFillColor(sf::Color(0, 170, 255, 200));
    sR.setPosition(m_position.x + 43.0f, m_position.y + 12.0f);
    window.draw(sR);

    // Label "JUMP" au-dessus pour guider le joueur
    // (pas de sf::Text ici car on n'a pas la police — on utilise
    //  le hint dans drawHUD() a la place)
}

// ──────────────────────────────────────────────────────────────
//  SecurityDrone — OBSTACLE AERIEN
//  Hitbox  : Y=350, h=95  →  zone 350..445
//  Visuel  : drone rouge avec helices + faisceau laser
//  Action  : SE BAISSER
// ──────────────────────────────────────────────────────────────
SecurityDrone::SecurityDrone(float x)
    : Obstacle(x, 350.0f, 60.0f, 95.0f, ObstacleType::AERIAL),
      m_hoverTimer(0.0f),
      m_baseY(350.0f)
{
    // Corps du drone : cercle rouge/gris
    m_body.setRadius(24.0f);
    m_body.setFillColor(sf::Color(65, 65, 95));
    m_body.setOutlineColor(sf::Color(220, 40, 40));
    m_body.setOutlineThickness(3.0f);
    m_body.setOrigin(24.0f, 24.0f);

    // Helices (deux rectangles en croix)
    m_blades[0].setSize(sf::Vector2f(56.0f, 8.0f));
    m_blades[0].setOrigin(28.0f, 4.0f);
    m_blades[0].setFillColor(sf::Color(130, 130, 165, 210));

    m_blades[1].setSize(sf::Vector2f(8.0f, 56.0f));
    m_blades[1].setOrigin(4.0f, 28.0f);
    m_blades[1].setFillColor(sf::Color(130, 130, 165, 210));
}

SecurityDrone::~SecurityDrone() {}

void SecurityDrone::updateWithSpeed(float deltaTime, float speed) {
    m_position.x  -= speed * deltaTime;
    m_hoverTimer  += deltaTime;
    // Oscillation tres douce (2px) pour rester dans la zone calculee
    m_position.y = m_baseY + std::sin(m_hoverTimer * 2.0f) * 2.0f;
}

void SecurityDrone::draw(sf::RenderWindow& window) {
    // Centre du drone : milieu de la hitbox
    float cx = m_position.x + 30.0f;
    float cy = m_position.y + 28.0f;

    // Helices en rotation rapide
    float angle = m_hoverTimer * 320.0f;
    m_blades[0].setPosition(cx, cy); m_blades[0].setRotation(angle);
    m_blades[1].setPosition(cx, cy); m_blades[1].setRotation(angle + 45.0f);
    window.draw(m_blades[0]);
    window.draw(m_blades[1]);

    // Corps
    m_body.setPosition(cx, cy);
    window.draw(m_body);

    // Oeil rouge clignotant au centre
    float blink = (std::sin(m_hoverTimer * 9.0f) + 1.0f) * 0.5f;
    sf::CircleShape eye(6.0f);
    eye.setOrigin(6.0f, 6.0f);
    eye.setPosition(cx, cy);
    eye.setFillColor(sf::Color(255, 20, 20, static_cast<sf::Uint8>(150 + blink * 105)));
    window.draw(eye);

    // Faisceau laser descendant — zone de danger visuelle
    // Montre jusqu'ou le drone est dangereux (jusqu'a Y=445)
    float laserBot = m_position.y + 95.0f;  // = bottom de la hitbox
    sf::RectangleShape laser(sf::Vector2f(3.0f, laserBot - (cy + 6.0f)));
    laser.setFillColor(sf::Color(255, 20, 20, static_cast<sf::Uint8>(60 + blink * 80)));
    laser.setPosition(cx - 1.5f, cy + 6.0f);
    window.draw(laser);

    // Petit triangle "passe dessous" en bas du faisceau
    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, sf::Vector2f(-8.0f, 0.0f));
    arrow.setPoint(1, sf::Vector2f(8.0f,  0.0f));
    arrow.setPoint(2, sf::Vector2f(0.0f,  10.0f));
    arrow.setFillColor(sf::Color(255, 60, 60, 200));
    arrow.setPosition(cx, laserBot - 12.0f);
    window.draw(arrow);
}
