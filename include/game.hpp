#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "player.hpp"
#include "Obstacle.hpp"
#include "Utils.hpp"

// ============================================================
//  Game.hpp — Cœur du jeu
//  Structure simplifiée, commentée pour niveau L2
// ============================================================

// Les différents états possibles du jeu
enum class GameState {
    MAIN_MENU,   // Écran d'accueil avec le bouton "Start"
    PLAYING,     // Partie en cours
    VICTORY,     // Le robot a atteint la capsule => WIN
    GAME_OVER    // Collision avec un obstacle => GAME OVER
};

class Game {
public:
    Game();
    ~Game();

    void run(); // Boucle principale : events => update => render

private:
    // -------------------------------------------------------
    //  Fenetre, horloge et police
    // -------------------------------------------------------
    sf::RenderWindow m_window;
    sf::Clock        m_clock;
    sf::Font         m_font;

    // -------------------------------------------------------
    //  Etat du jeu
    // -------------------------------------------------------
    GameState m_state;

    // Temps de survie accumule (0 -> SURVIVAL_TIME_FOR_CAPSULE)
    // Quand il atteint 100%, la capsule apparait
    float m_survivalTime;

    // Vitesse de defilement (augmente progressivement)
    float m_scrollSpeed;

    // Timer pour l'apparition des obstacles
    float m_spawnTimer;
    float m_nextSpawnInterval;

    // Score final affiche sur l'ecran de victoire
    float m_finalScore;

    // -------------------------------------------------------
    //  Entites du jeu
    // -------------------------------------------------------
    std::unique_ptr<Player>                m_player;
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;

    // -------------------------------------------------------
    //  Capsule de sauvetage
    // -------------------------------------------------------
    bool               m_capsuleVisible;  // Apparait quand la barre est a 100%
    float              m_capsuleX;        // Position X (entre par la droite)
    sf::RectangleShape m_capsule;
    sf::CircleShape    m_capsuleWindow;

    // -------------------------------------------------------
    //  Decor : etoiles (2 couches de parallaxe)
    // -------------------------------------------------------
    struct StarLayer {
        std::vector<sf::CircleShape> stars;
        float speed;
    };
    StarLayer m_starLayers[2];

    struct PanelStrip {
        sf::RectangleShape rect;
        sf::Color          color;
        float              speed;
    };
    std::vector<PanelStrip> m_bgPanels;

    // -------------------------------------------------------
    //  Elements visuels : sol, plafond, HUD
    // -------------------------------------------------------
    sf::RectangleShape m_floorRect;
    sf::RectangleShape m_ceilingRect;

    sf::RectangleShape m_progressBarBg;
    sf::RectangleShape m_progressBarFill;

    // -------------------------------------------------------
    //  Tremblement d'ecran (screen shake)
    // -------------------------------------------------------
    bool         m_shakeActive;
    float        m_shakeTimer;
    sf::Vector2f m_shakeOffset;

    // -------------------------------------------------------
    //  Methodes privees
    // -------------------------------------------------------
    void processEvents();
    void update(float deltaTime);
    void render();

    void updatePlaying(float deltaTime);
    void checkCollisions();
    void spawnObstacle();
    void cleanObstacles();
    void updateParallax(float deltaTime);
    void updateShake(float deltaTime);

    void drawBackground();
    void drawHUD();
    void drawCapsule();
    void drawMainMenu();
    void drawVictoryScreen();
    void drawGameOverScreen();

    void     startGame();
    void     resetGame();
    void     triggerScreenShake(float duration);
    bool     loadFont();
    sf::Text makeText(const std::string& str, unsigned int size, sf::Color color);
};
