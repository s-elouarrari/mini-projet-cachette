#include "Entity.hpp"

// ============================================================
//  Entity.cpp — Implémentation de la classe mère
// ============================================================

Entity::Entity(float x, float y, float width, float height)
    : m_position(x, y),
      m_size(width, height),
      m_isAlive(true)
{
    m_shape.setSize(sf::Vector2f(width, height));
    m_shape.setPosition(x, y);
}

Entity::~Entity() {
    // Pas de ressources dynamiques ici — nettoyage automatique
}

sf::FloatRect Entity::getBounds() const {
    return sf::FloatRect(m_position.x, m_position.y, m_size.x, m_size.y);
}

bool Entity::isAlive() const {
    return m_isAlive;
}

void Entity::setAlive(bool alive) {
    m_isAlive = alive;
}

float Entity::getX() const {
    return m_position.x;
}

float Entity::getY() const {
    return m_position.y;
}

void Entity::setPosition(float x, float y) {
    m_position.x = x;
    m_position.y = y;
    m_shape.setPosition(x, y);
}
