#pragma once
#include "Entity.hpp"

// ============================================================
//  Player.hpp — Classe du robot joueur
//
//  HÉRITAGE : Entity → Player
//    Player hérite de Entity qui fournit :
//      - m_position (sf::Vector2f) : position dans le monde
//      - m_size     (sf::Vector2f) : dimensions de la hitbox
//      - getBounds() : retourne un sf::FloatRect pour les collisions
//
//  PRINCIPE DU SAUT (correction du bug visuel) :
//    La physique modifie m_position.y à chaque frame.
//    Dans draw(), TOUTES les formes sont placées relativement
//    à m_position.y (appelé "py").
//    → Quand py monte (saut), TOUT le robot monte visuellement.
//    → NE PAS utiliser GROUND_Y fixe comme référence dans draw().
//
//  HITBOX GARANTIES (calculées, pas arbitraires) :
//    GROUND_Y     = 490    (Y du sol, bord supérieur)
//    NORMAL_HEIGHT= 80     → debout   : getBounds() Y 410..490
//    CROUCH_HEIGHT= 35     → accroupi : getBounds() Y 455..490
//    JUMP_FORCE   = -560   → apex saut: getBounds() Y ~268..348
//
//  Drone aérien  (Y 350..445) :
//    debout   410..490 vs 350..445 → overlap → TOUCHE ✓
//    accroupi 455..490 vs 350..445 → 455>445  → PASSE ✓
//    saut     268..348 vs 350..445 → 348<350  → PASSE ✓
//
//  Bloc sol      (Y 390..490) :
//    debout   410..490 vs 390..490 → overlap → TOUCHE ✓
//    saut     268..348 vs 390..490 → 348<390  → PASSE ✓
// ============================================================

// États possibles du robot (utilisés par draw() pour choisir l'animation)
enum class PlayerAction {
    IDLE,       // À l'arrêt (écran READY, avant le signal de départ)
    RUNNING,    // En course (après avoir appuyé sur ESPACE)
    JUMPING,    // En l'air (saut en cours)
    CROUCHING   // Accroupi (touche BAS maintenue)
};

class Player : public Entity {
public:
    // ── Cycle de vie ─────────────────────────────────────────
    Player(float startX, float startY);
    ~Player() override;

    // ── Interface Entity (méthodes virtuelles pures à implémenter) ──
    void update(float deltaTime) override;  // physique + animation
    void draw(sf::RenderWindow& window) override;  // rendu SFML

    // ── Actions du joueur ────────────────────────────────────
    void jump();                   // déclenche un saut (si au sol)
    void crouch(bool isCrouching); // active/désactive l'accroupissement
    void startRunning();           // démarre la course (depuis READY)

    // ── Système de santé ─────────────────────────────────────
    void takeDamage();          // -1 cœur + invincibilité temporaire
    bool isInvincible() const;  // true pendant la période de grâce
    bool isDead()       const;  // true quand health ≤ 0
    int  getHealth()    const;  // retourne le nombre de cœurs restants

    // ── Accesseurs d'état ────────────────────────────────────
    PlayerAction getAction() const;
    bool         isRunning() const;

private:
    // ── Constantes physiques ──────────────────────────────────
    // Toutes regroupées ici pour faciliter le réglage du gameplay.

    static constexpr float GRAVITY       = 1100.0f;
    // Accélération gravitationnelle (px/s²). Plus élevé = retombe vite.

    static constexpr float JUMP_FORCE    = -560.0f;
    // Vitesse initiale du saut (px/s, négatif = vers le haut en SFML).
    // Apex = v²/(2g) = 560²/2200 ≈ 143px au-dessus du sol.

    static constexpr float GROUND_Y      = 490.0f;
    // Y du bord supérieur du sol. Doit correspondre à Constants::GROUND_Y.

    static constexpr float NORMAL_HEIGHT = 80.0f;
    // Hauteur hitbox debout. Debout : m_position.y = 490-80 = 410.

    static constexpr float CROUCH_HEIGHT = 35.0f;
    // Hauteur hitbox accroupi. Accroupi : m_position.y = 490-35 = 455.

    // ── État du robot ─────────────────────────────────────────
    PlayerAction m_action;       // action courante (IDLE/RUNNING/JUMPING/CROUCHING)
    sf::Vector2f m_velocity;     // vitesse actuelle (x toujours 0, y varie)
    bool         m_isOnGround;   // true si les pieds touchent le sol
    bool         m_isRunning;    // false sur l'écran READY (avant ESPACE)

    // ── Santé ────────────────────────────────────────────────
    int   m_health;              // cœurs restants (0..MAX_HEALTH)
    float m_invincibilityTimer;  // secondes d'invincibilité restantes

    // ── Animation ────────────────────────────────────────────
    float m_animTimer;           // s'incrémente en continu, utilisé par sin()
    // Les membres oscillent selon sin(animTimer * fréquence)

    // ── Couleurs (changent selon la santé) ───────────────────
    sf::Color m_colorBody;
    sf::Color m_colorHead;
    sf::Color m_colorVisor;

    // ── Formes SFML pré-allouées ─────────────────────────────
    // Allouées une fois dans le constructeur pour éviter les
    // allocations à chaque frame dans draw().
    sf::RectangleShape m_torsoShape;    // torse / corps
    sf::RectangleShape m_headShape;     // tête / casque
    sf::CircleShape    m_visorShape;    // visière lumineuse
    sf::RectangleShape m_antennaShape;  // tige d'antenne
    sf::CircleShape    m_antennaTip;    // sphère au sommet de l'antenne

    // ── Méthodes internes ────────────────────────────────────
    void applyGravity(float deltaTime);  // ajoute GRAVITY à m_velocity.y
    void clampToGround();                // pose le robot sur le sol
    void updateColors();                 // met à jour m_colorBody/Head/Visor

    // Dessine un membre (bras ou jambe) pivoté autour de son attache
    void drawLimb(sf::RenderWindow& window,
                  float ox, float oy,    // point d'attache (épaule ou hanche)
                  float angle,           // rotation en degrés
                  float w, float h,      // largeur, longueur
                  sf::Color color) const;
};
