#pragma once
#include <SFML/Graphics.hpp>

// ============================================================
//  Entity.hpp — Classe mère abstraite (Héritage + Virtuel)
// ============================================================

class Entity {
public:
    Entity(float x, float y, float width, float height);
    virtual ~Entity();  // Destructeur virtuel OBLIGATOIRE

    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    sf::FloatRect getBounds() const;
    bool isAlive() const;
    void setAlive(bool alive);

    float getX() const;
    float getY() const;
    void setPosition(float x, float y);

protected:
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    bool         m_isAlive;
    sf::RectangleShape m_shape;
};
