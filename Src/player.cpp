#include "../include/Player.hpp"
#include "../include/Utils.hpp"
#include <cmath>

// ============================================================
//  Player.cpp — Robot joueur : physique + animation complète
//
//  PRINCIPE CLÉ : m_position.y = sommet de la hitbox
//  ─────────────────────────────────────────────────────────
//  Toutes les positions visuelles (torse, tête, membres) sont
//  calculées RELATIVEMENT à m_position.y, PAS depuis GROUND_Y.
//
//  Si on calcule depuis GROUND_Y : le sprite reste cloué au sol
//  même quand la hitbox monte → bug visuel constaté.
//
//  Si on calcule depuis py (= m_position.y) : quand la physique
//  fait monter m_position.y, le sprite entier monte avec elle.
//
//  SCHÉMA VERTICAL (robot debout, py = 410) :
//
//    py - 16  = 394 ──► pointe antenne
//    py - 12  = 398 ──► base antenne
//    py - 28  = 382 ──► haut de la tête
//    py       = 410 ──► sommet hitbox / bas de la tête
//    py + 10  = 420 ──► haut du torse (offset fixe)
//    py + 52  = 462 ──► bas du torse  (torse = 42px de haut)
//    py + 52  = 462 ──► hanches (jonction torse/jambes)
//    py + 80  = 490 ──► GROUND_Y (bas hitbox, sol)
//
//  HITBOX (inchangée, prouvée mathématiquement) :
//    Debout   : top=410, bottom=490  (NORMAL_HEIGHT = 80)
//    Accroupi : top=455, bottom=490  (CROUCH_HEIGHT = 35)
//    Saut apex: top≈268, bottom≈348  (JUMP_FORCE=-560, G=1100)
// ============================================================

// ------------------------------------------------------------
//  Constructeur
// ------------------------------------------------------------
Player::Player(float startX, float startY)
    : Entity(startX, startY, 36.0f, NORMAL_HEIGHT),
      m_action(PlayerAction::IDLE),
      m_velocity(0.0f, 0.0f),
      m_isOnGround(true),
      m_isRunning(false),
      m_health(Constants::MAX_HEALTH),
      m_invincibilityTimer(0.0f),
      m_animTimer(0.0f),
      m_colorBody(160, 175, 205),
      m_colorHead(195, 210, 230),
      m_colorVisor(0, 210, 255, 200)
{
    // Torse principal (partie basse du corps)
    m_torsoShape.setSize(sf::Vector2f(36.0f, 42.0f));
    m_torsoShape.setFillColor(m_colorBody);
    m_torsoShape.setOutlineColor(sf::Color(90, 115, 160));
    m_torsoShape.setOutlineThickness(2.0f);

    // Tête / casque
    m_headShape.setSize(sf::Vector2f(30.0f, 28.0f));
    m_headShape.setFillColor(m_colorHead);
    m_headShape.setOutlineColor(sf::Color(90, 115, 160));
    m_headShape.setOutlineThickness(2.0f);

    // Visière lumineuse (cercle cyan)
    m_visorShape.setRadius(10.0f);
    m_visorShape.setFillColor(m_colorVisor);
    m_visorShape.setOutlineColor(sf::Color(0, 235, 255));
    m_visorShape.setOutlineThickness(1.5f);

    // Antenne (tige + sphère au sommet)
    m_antennaShape.setSize(sf::Vector2f(3.0f, 12.0f));
    m_antennaShape.setFillColor(Constants::COLOR_ACCENT_CYAN);

    m_antennaTip.setRadius(4.0f);
    m_antennaTip.setFillColor(Constants::COLOR_ACCENT_CYAN);
    m_antennaTip.setOrigin(4.0f, 4.0f);
}

Player::~Player() {}

// ============================================================
//  update() — physique frame par frame
//
//  Appelé chaque frame avec deltaTime (temps écoulé en secondes).
//  La physique n'est active qu'une fois m_isRunning=true,
//  ce qui évite que le robot bouge sur l'écran READY.
// ============================================================
void Player::update(float deltaTime) {

    // ── Physique (gravity + déplacement vertical) ─────────────
    // Uniquement quand la course a démarré (touche ESPACE sur READY)
    if (m_isRunning) {
        applyGravity(deltaTime);

        // Intégration Euler : on applique la vitesse à la position
        // En SFML, Y augmente vers le bas, donc une vitesse négative
        // fait monter le robot (c'est pourquoi JUMP_FORCE est négatif).
        m_position.y += m_velocity.y * deltaTime;

        // Empêche le robot de traverser le sol
        clampToGround();
    }

    // ── Timer d'animation ─────────────────────────────────────
    // S'incrémente en continu pendant la course (sauf accroupi).
    // Utilisé par sin() pour animer les membres de façon cyclique.
    if (m_isRunning && m_action != PlayerAction::CROUCHING)
        m_animTimer += deltaTime;

    // ── Invincibilité post-collision ──────────────────────────
    // Décrémente le timer ; quand il atteint 0, le robot peut
    // de nouveau être touché par un obstacle.
    if (m_invincibilityTimer > 0.0f) {
        m_invincibilityTimer -= deltaTime;
        if (m_invincibilityTimer < 0.0f)
            m_invincibilityTimer = 0.0f;
    }

    // Met à jour les couleurs selon la santé actuelle
    updateColors();
}

// ============================================================
//  draw() — dessin complet du robot
//
//  CORRECTION VISUELLE DU SAUT :
//  Toutes les coordonnées Y sont calculées depuis :
//      py = m_position.y   (sommet de la hitbox)
//
//  Quand la physique fait monter m_position.y (pendant le saut),
//  py descend (car Y augmente vers le bas en SFML), donc TOUTES
//  les parties du robot bougent ensemble.
//
//  NE PAS utiliser GROUND_Y comme référence ici : ce serait une
//  valeur fixe, et le robot semblerait figé visuellement même
//  quand la hitbox monte correctement.
// ============================================================
void Player::draw(sf::RenderWindow& window) {

    // ── Clignotement pendant l'invincibilité ──────────────────
    // Alterne visible/invisible toutes les 0.1s pour signaler
    // l'état de grâce après une collision.
    if (m_invincibilityTimer > 0.0f) {
        int frame = static_cast<int>(m_invincibilityTimer * 10.0f);
        if (frame % 2 == 0) return; // frame invisible → on ne dessine rien
    }

    // ── Référence de position ─────────────────────────────────
    // px, py = coin supérieur gauche de la hitbox.
    // py CHANGE à chaque frame pendant le saut → tout le robot bouge.
    float px = m_position.x;   // bord gauche (fixe pendant le jeu)
    float py = m_position.y;   // sommet hitbox (VARIABLE : monte en saut)

    // Centre horizontal du robot (utilisé pour centrer tête, visière…)
    float cx = px + 18.0f;

    // ── Positions verticales calculées DEPUIS py ──────────────
    // Ces offsets sont fixes par rapport au sommet de la hitbox.
    // Quand py monte (saut), tout le robot monte avec lui.
    const float TORSE_OFFSET_TOP = 10.0f;   // py + 10 = haut du torse
    const float TORSE_HEIGHT     = 42.0f;   // hauteur du torse
    const float HEAD_HEIGHT      = 28.0f;   // hauteur de la tête

    float torsoTop    = py + TORSE_OFFSET_TOP;           // haut du torse
    float torsoBottom = torsoTop + TORSE_HEIGHT;         // bas du torse = hanches
    float headTop     = py - HEAD_HEIGHT;                // haut de la tête (au-dessus de py)

    // ── MODE ACCROUPI : robot "en boule" ─────────────────────
    if (m_action == PlayerAction::CROUCHING) {
        // Corps aplati (hauteur réduite = CROUCH_HEIGHT)
        // py est déjà à 455 (= GROUND_Y - CROUCH_HEIGHT)
        sf::RectangleShape flat(sf::Vector2f(40.0f, 35.0f));
        flat.setFillColor(m_colorBody);
        flat.setOutlineColor(sf::Color(90, 115, 160));
        flat.setOutlineThickness(2.0f);
        flat.setPosition(px - 2.0f, py); // positionné DEPUIS py
        window.draw(flat);

        // Tête compressée qui dépasse à peine au-dessus du torse
        sf::RectangleShape headFlat(sf::Vector2f(26.0f, 14.0f));
        headFlat.setFillColor(m_colorHead);
        headFlat.setOutlineColor(sf::Color(90, 115, 160));
        headFlat.setOutlineThickness(1.5f);
        headFlat.setPosition(px + 5.0f, py - 14.0f); // DEPUIS py
        window.draw(headFlat);

        // Petite visière visible malgré l'accroupissement
        sf::CircleShape smallVisor(5.0f);
        smallVisor.setFillColor(m_colorVisor);
        smallVisor.setOrigin(5.0f, 5.0f);
        smallVisor.setPosition(cx, py - 7.0f); // DEPUIS py
        window.draw(smallVisor);

        // Bras gauche replié en protection (angle -80° = horizontal vers la gauche)
        drawLimb(window, px - 2.0f, py + 10.0f, -80.0f, 8.0f, 22.0f,
                 sf::Color(130, 145, 180));
        // Bras droit replié en miroir
        drawLimb(window, px + 38.0f, py + 10.0f, -100.0f, 8.0f, 22.0f,
                 sf::Color(130, 145, 180));

        // Jambes en boule = deux cercles sous le corps aplati
        sf::CircleShape kneL(9.0f);
        kneL.setFillColor(sf::Color(120, 135, 170));
        kneL.setOutlineColor(sf::Color(80, 100, 150));
        kneL.setOutlineThickness(1.5f);
        kneL.setOrigin(9.0f, 9.0f);
        kneL.setPosition(px + 8.0f,  py + 30.0f); // DEPUIS py
        window.draw(kneL);

        sf::CircleShape kneR(9.0f);
        kneR.setFillColor(sf::Color(120, 135, 170));
        kneR.setOutlineColor(sf::Color(80, 100, 150));
        kneR.setOutlineThickness(1.5f);
        kneR.setOrigin(9.0f, 9.0f);
        kneR.setPosition(px + 28.0f, py + 30.0f); // DEPUIS py
        window.draw(kneR);

        return; // Dessin duck terminé
    }

    // ── MODE DEBOUT / COURSE / SAUT ──────────────────────────

    // Calcul des angles d'animation des membres
    // Les angles sont en degrés ; drawLimb() applique une rotation SFML.
    float legAngleL = 0.0f;
    float legAngleR = 0.0f;
    float armAngleL = 0.0f;
    float armAngleR = 0.0f;

    if (m_action == PlayerAction::JUMPING) {
        // Pendant le saut : jambes légèrement écartées (impulsion),
        // bras levés vers le haut pour l'équilibre.
        legAngleL =  15.0f;   // jambe gauche légèrement vers l'avant
        legAngleR = -15.0f;   // jambe droite légèrement vers l'arrière
        armAngleL = -55.0f;   // bras gauche levé
        armAngleR =  55.0f;   // bras droit levé
    }
    else if (m_isRunning) {
        // Pendant la course : oscillation sinusoïdale alternée.
        // std::sin(animTimer * 9.0f) produit une onde entre -1 et +1
        // à ~1.4 Hz (9 rad/s ≈ 1.43 Hz), ce qui donne un pas fluide.
        float wave = std::sin(m_animTimer * 9.0f);
        legAngleL =  32.0f * wave;   // jambe gauche : en phase avec wave
        legAngleR = -32.0f * wave;   // jambe droite : opposition naturelle
        armAngleL = -22.0f * wave;   // bras gauche : opposé aux jambes
        armAngleR =  22.0f * wave;   // bras droit  : opposé aux jambes
    }
    else {
        // À l'arrêt (écran READY) : léger balancement d'attente (2 rad/s)
        float idleWave = std::sin(m_animTimer * 2.0f) * 5.0f;
        armAngleL = -5.0f + idleWave;
        armAngleR =  5.0f - idleWave;
    }

    // ── Jambe gauche (en arrière-plan) ────────────────────────
    // Point d'attache = hanche gauche (bas du torse - 8px à gauche du centre)
    drawLimb(window,
             cx - 8.0f, torsoBottom,   // origine : hanche gauche
             legAngleL,
             9.0f, 32.0f,              // largeur, longueur
             sf::Color(115, 130, 168));

    // ── Jambe droite (au premier plan) ───────────────────────
    drawLimb(window,
             cx + 8.0f, torsoBottom,   // origine : hanche droite
             legAngleR,
             9.0f, 32.0f,
             sf::Color(130, 145, 180));

    // ── Torse ────────────────────────────────────────────────
    // torsoTop = py + 10 → se déplace AVEC py pendant le saut
    m_torsoShape.setFillColor(m_colorBody);
    m_torsoShape.setPosition(px, torsoTop);
    window.draw(m_torsoShape);

    // Panneau lumineux central sur le torse (détail cosmétique)
    sf::RectangleShape panel(sf::Vector2f(22.0f, 10.0f));
    panel.setFillColor(sf::Color(0, 160, 220, 150));
    panel.setOutlineColor(sf::Color(0, 200, 255, 100));
    panel.setOutlineThickness(1.0f);
    panel.setPosition(px + 7.0f, torsoTop + 14.0f);
    window.draw(panel);

    // ── Bras gauche (en arrière-plan) ─────────────────────────
    // Épaule = haut du torse + 6px, décalée à gauche du centre
    drawLimb(window,
             cx - 16.0f, torsoTop + 6.0f,  // origine : épaule gauche
             armAngleL,
             7.0f, 26.0f,
             sf::Color(115, 130, 168));

    // ── Bras droit (au premier plan) ──────────────────────────
    drawLimb(window,
             cx + 16.0f, torsoTop + 6.0f,  // origine : épaule droite
             armAngleR,
             7.0f, 26.0f,
             sf::Color(130, 145, 180));

    // ── Tête ──────────────────────────────────────────────────
    // headTop = py - HEAD_HEIGHT → au-dessus de la hitbox, monte avec py
    m_headShape.setFillColor(m_colorHead);
    m_headShape.setPosition(px + 3.0f, headTop);
    window.draw(m_headShape);

    // ── Visière ───────────────────────────────────────────────
    m_visorShape.setFillColor(m_colorVisor);
    m_visorShape.setOrigin(10.0f, 10.0f);
    // Centrée horizontalement sur la tête, à mi-hauteur
    m_visorShape.setPosition(cx, headTop + 14.0f);
    window.draw(m_visorShape);

    // Petit reflet blanc dans le coin supérieur gauche de la visière
    sf::CircleShape reflet(4.0f);
    reflet.setFillColor(sf::Color(255, 255, 255, 120));
    reflet.setOrigin(4.0f, 4.0f);
    reflet.setPosition(cx - 5.0f, headTop + 7.0f);
    window.draw(reflet);

    // ── Antenne ───────────────────────────────────────────────
    // Tige + sphère lumineuse au sommet de la tête
    float antX = px + 16.0f;
    float antY = headTop - 12.0f;  // au-dessus de la tête, suit py
    m_antennaShape.setPosition(antX, antY);
    window.draw(m_antennaShape);

    // Couleur de la sphère antenne = indicateur de santé
    // Cyan = pleine santé, orange = blessé, rouge = critique
    sf::Color tipColor;
    if (m_health == 1)      tipColor = sf::Color(255, 50,  50);   // rouge
    else if (m_health == 2) tipColor = sf::Color(255, 180, 0);    // orange
    else                    tipColor = Constants::COLOR_ACCENT_CYAN; // cyan
    m_antennaTip.setFillColor(tipColor);
    m_antennaTip.setPosition(antX + 1.5f, antY - 4.0f);
    window.draw(m_antennaTip);
}

// ============================================================
//  drawLimb() — dessine un membre (bras ou jambe) pivoté
//
//  Le membre est un rectangle qui pivote autour de son bord
//  supérieur central (l'articulation).
//
//  ox, oy : point d'attache (épaule ou hanche) — en pixels SFML
//  angle  : rotation en degrés (0 = vertical vers le bas)
//           Positif = vers l'avant, négatif = vers l'arrière
//  w, h   : largeur et longueur du membre
//  color  : couleur de remplissage
// ============================================================
void Player::drawLimb(sf::RenderWindow& window,
                      float ox, float oy,
                      float angle,
                      float w, float h,
                      sf::Color color) const
{
    sf::RectangleShape limb(sf::Vector2f(w, h));
    limb.setFillColor(color);
    limb.setOutlineColor(sf::Color(70, 90, 140));
    limb.setOutlineThickness(1.5f);

    // setOrigin place le pivot au centre du bord supérieur.
    // SFML effectue ensuite la rotation autour de ce point
    // quand on appelle setRotation().
    limb.setOrigin(w / 2.0f, 0.0f);
    limb.setPosition(ox, oy);
    limb.setRotation(angle);

    window.draw(limb);
}

// ============================================================
//  jump() — déclenche un saut si le robot est au sol
// ============================================================
void Player::jump() {
    // On ne peut sauter que si on est au sol ET que la course a commencé.
    // Cela évite de "sauter" sur l'écran READY avant d'appuyer sur ESPACE.
    if (m_isOnGround && m_isRunning) {
        // JUMP_FORCE est négatif : en SFML, Y augmente vers le bas,
        // donc une vitesse négative fait monter le robot vers le haut.
        m_velocity.y = JUMP_FORCE;   // ex: -560 pixels/seconde
        m_isOnGround = false;
        m_action     = PlayerAction::JUMPING;
    }
}

// ============================================================
//  crouch() — active/désactive l'accroupissement
//
//  En accroupissement :
//    - m_size.y passe à CROUCH_HEIGHT (35px au lieu de 80)
//    - m_position.y est recalculé pour que le BAS reste au sol
//      (bas = m_position.y + m_size.y = GROUND_Y = 490)
//    → top hitbox monte de 410 à 455 → passe sous le drone (bas=445)
// ============================================================
void Player::crouch(bool isCrouching) {
    // Pas d'action si la course n'a pas encore commencé
    if (!m_isRunning) return;

    if (isCrouching && m_isOnGround) {
        // S'accroupir : réduire la hitbox par le haut
        m_size.y     = CROUCH_HEIGHT;
        m_position.y = GROUND_Y - CROUCH_HEIGHT;  // 490 - 35 = 455
        m_action     = PlayerAction::CROUCHING;
    }
    else if (!isCrouching && m_isOnGround) {
        // Se relever : restaurer la hitbox normale
        m_size.y     = NORMAL_HEIGHT;
        m_position.y = GROUND_Y - NORMAL_HEIGHT;  // 490 - 80 = 410
        if (m_action == PlayerAction::CROUCHING)
            m_action = PlayerAction::RUNNING;
    }
}

// ============================================================
//  startRunning() — appelé quand le joueur appuie sur ESPACE
//                   depuis l'écran READY
// ============================================================
void Player::startRunning() {
    m_isRunning = true;
    m_action    = PlayerAction::RUNNING;
}

// ============================================================
//  takeDamage() — retire 1 cœur + active l'invincibilité
//
//  L'invincibilité évite les hits en rafale : si le robot
//  reste dans un obstacle plusieurs frames, il ne perd qu'1 cœur.
// ============================================================
void Player::takeDamage() {
    if (m_invincibilityTimer > 0.0f) return; // encore invincible, on ignore
    m_health -= 1;
    m_invincibilityTimer = Constants::INVINCIBILITY_AFTER_HIT; // ex: 1.8s
}

// Accesseurs publics
bool Player::isInvincible() const { return m_invincibilityTimer > 0.0f; }
bool Player::isDead()       const { return m_health <= 0; }
int  Player::getHealth()    const { return m_health; }
PlayerAction Player::getAction() const { return m_action; }
bool         Player::isRunning() const { return m_isRunning; }

// ============================================================
//  applyGravity() — accélère le robot vers le bas
//
//  La gravité n'est appliquée que quand le robot est en l'air.
//  m_velocity.y augmente chaque frame → le robot accélère
//  vers le bas jusqu'à toucher le sol (clampToGround).
// ============================================================
void Player::applyGravity(float deltaTime) {
    if (!m_isOnGround) {
        // GRAVITY = 1100 px/s² → le robot retombe rapidement
        // (gravité spatiale légèrement plus forte qu'en vrai pour
        //  un gameplay dynamique)
        m_velocity.y += GRAVITY * deltaTime;
    }
}

// ============================================================
//  clampToGround() — empêche le robot de traverser le sol
//
//  groundLevel = position.y maximale du sommet de la hitbox :
//    debout   : 490 - 80 = 410
//    accroupi : 490 - 35 = 455
//
//  Si m_position.y dépasse groundLevel, on "pose" le robot
//  exactement sur le sol et on annule la vitesse verticale.
// ============================================================
void Player::clampToGround() {
    float groundLevel = GROUND_Y - m_size.y;  // sol - hauteur courante

    if (m_position.y >= groundLevel) {
        m_position.y = groundLevel; // coller au sol
        m_velocity.y = 0.0f;        // annuler la vitesse (pas de rebond)
        m_isOnGround = true;
        // Retour à l'animation de course après l'atterrissage
        if (m_action == PlayerAction::JUMPING)
            m_action = PlayerAction::RUNNING;
    }
}

// ============================================================
//  updateColors() — met à jour les couleurs selon la santé
//
//  Feedback visuel immédiat pour le joueur :
//    3 cœurs → gris métallique (normal)
//    2 cœurs → orange (attention !)
//    1 cœur  → rouge (danger !)
// ============================================================
void Player::updateColors() {
    if (m_health == 2) {
        m_colorBody  = sf::Color(255, 155, 20);
        m_colorHead  = sf::Color(255, 180, 50);
        m_colorVisor = sf::Color(255, 210, 0, 220);
    } else if (m_health == 1) {
        m_colorBody  = sf::Color(215, 45, 45);
        m_colorHead  = sf::Color(235, 70, 70);
        m_colorVisor = sf::Color(255, 40, 40, 220);
    } else {
        // Pleine santé : palette gris métallique spatiale
        m_colorBody  = sf::Color(160, 175, 205);
        m_colorHead  = sf::Color(195, 210, 230);
        m_colorVisor = sf::Color(0, 210, 255, 200);
    }
}
