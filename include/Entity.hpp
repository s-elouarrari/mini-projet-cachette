#pragma once
#include <SFML/Graphics.hpp>

// ============================================================
//  Entity.hpp — Classe mère abstraite du jeu
//
//  RÔLE : Base commune pour toutes les entités (Player, Obstacle).
//
//  HÉRITAGE ET POLYMORPHISME :
//    Entity (abstraite)
//      ├── Player        (le robot joueur)
//      └── Obstacle      (classe mère des obstacles)
//            ├── MagneticContainer  (obstacle au sol, → SAUTER)
//            └── SecurityDrone      (obstacle aérien, → SE BAISSER)
//
//  Les méthodes virtuelles pures update() et draw() forcent
//  chaque sous-classe à fournir sa propre implémentation.
//  Game peut alors traiter tous les obstacles de façon uniforme :
//    for (auto& obs : m_obstacles) obs->draw(m_window);
//
//  DESTRUCTEUR VIRTUEL :
//    Obligatoire pour que delete sur un pointeur Entity*
//    appelle le bon destructeur de la sous-classe.
// ============================================================

class Entity {
public:
    // ── Cycle de vie ─────────────────────────────────────────
    Entity(float x, float y, float width, float height);
    virtual ~Entity();  // DOIT être virtual pour le polymorphisme

    // ── Interface polymorphique (=0 = virtuelle pure) ────────
    // Chaque sous-classe DOIT implémenter ces deux méthodes.
    virtual void update(float deltaTime) = 0;  // logique/physique
    virtual void draw(sf::RenderWindow& window) = 0;  // rendu SFML

    // ── Collision ────────────────────────────────────────────
    // Retourne la boîte de collision (position + taille).
    // Utilisé avec sf::FloatRect::intersects() dans checkCollisions().
    sf::FloatRect getBounds() const;

    // ── État ─────────────────────────────────────────────────
    bool isAlive() const;
    void setAlive(bool alive);

    // ── Position ─────────────────────────────────────────────
    float getX() const;
    float getY() const;
    void  setPosition(float x, float y);

protected:
    // Ces membres sont accessibles aux sous-classes (protected),
    // mais pas depuis l'extérieur (pas public).

    sf::Vector2f m_position;  // position dans le monde (x, y)
    sf::Vector2f m_size;      // dimensions de la hitbox (largeur, hauteur)
    bool         m_isAlive;   // false = à supprimer de la liste

    // Forme rectangulaire de base (utilisable par les sous-classes
    // pour le rendu ; Player utilise ses propres formes à la place)
    sf::RectangleShape m_shape;
};
