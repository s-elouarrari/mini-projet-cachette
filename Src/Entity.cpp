#include "../include/Entity.hpp"

// ============================================================
//  Entity.cpp — Classe mère abstraite
//
//  Entity est la base de TOUTES les entités du jeu :
//    Player, Obstacle (MagneticContainer, SecurityDrone).
//
//  Elle centralise :
//    - La position dans le monde (m_position)
//    - Les dimensions de la hitbox (m_size)
//    - La forme de base (m_shape, utilisée par les sous-classes)
//    - Les deux méthodes virtuelles pures update() et draw()
//      qui forcent chaque sous-classe à les implémenter.
//
//  POLYMORPHISME : Game stocke les obstacles comme
//    std::vector<std::unique_ptr<Obstacle>>
//  et appelle obs->update() / obs->draw() sans savoir quel
//  type concret est derrière. C'est le polymorphisme en action.
// ============================================================

// ------------------------------------------------------------
//  Constructeur
//  Initialise position, taille, et prépare la forme de base.
// ------------------------------------------------------------
Entity::Entity(float x, float y, float width, float height)
    : m_position(x, y),
      m_size(width, height),
      m_isAlive(true)
{
    // On prépare la forme rectangulaire de base.
    // Les sous-classes peuvent utiliser m_shape directement,
    // ou bien créer leurs propres formes dans leur constructeur.
    m_shape.setSize(sf::Vector2f(width, height));
    m_shape.setPosition(x, y);
}

// ------------------------------------------------------------
//  Destructeur virtuel
//  OBLIGATOIRE quand une classe mère a des méthodes virtuelles.
//  Sans ça, supprimer un Obstacle* pointant sur un MagneticContainer
//  n'appellerait pas le destructeur de MagneticContainer → fuite mémoire.
// ------------------------------------------------------------
Entity::~Entity() {
    // Pas de ressources dynamiques à libérer ici.
    // Les unique_ptr des membres se nettoient automatiquement.
}

// ------------------------------------------------------------
//  getBounds() — retourne la hitbox comme FloatRect
//
//  Utilisé par checkCollisions() dans Game.cpp :
//    sf::FloatRect playerBounds = m_player->getBounds();
//    if (playerBounds.intersects(obstacleBounds)) { ... }
//
//  IMPORTANT : getBounds() retourne m_position et m_size,
//  pas les dimensions de m_shape. On modifie m_position.y
//  pendant le saut, et getBounds() reflète toujours la
//  vraie position de la hitbox.
// ------------------------------------------------------------
sf::FloatRect Entity::getBounds() const {
    return sf::FloatRect(m_position.x, m_position.y, m_size.x, m_size.y);
}

// Accesseurs simples (getters)
bool  Entity::isAlive() const  { return m_isAlive; }
void  Entity::setAlive(bool a) { m_isAlive = a; }
float Entity::getX()    const  { return m_position.x; }
float Entity::getY()    const  { return m_position.y; }

// ------------------------------------------------------------
//  setPosition() — déplace l'entité dans le monde
//  Met à jour m_position ET la position de m_shape en même temps.
// ------------------------------------------------------------
void Entity::setPosition(float x, float y) {
    m_position.x = x;
    m_position.y = y;
    m_shape.setPosition(x, y);
}
