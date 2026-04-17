#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Player.hpp"
#include "Obstacle.hpp"
#include "Utils.hpp"

// ============================================================
//  Game.hpp — Cœur du jeu (Tableaux + Pointeurs)
// ============================================================

enum class GameState {
    MAIN_MENU,
    INTRO_CINEMATIC,
    PLAYING,
    PAUSED,
    VICTORY,
    GAME_OVER
};

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    // --- Fenêtre et horloge ---
    sf::RenderWindow m_window;
    sf::Clock        m_clock;
    sf::Font         m_font;

    // --- État du jeu ---
    GameState m_state;
    float     m_pressureTimer;       // Oxygène restant
    float     m_distanceTraveled;    // Progression vers la capsule
    float     m_scrollSpeed;         // Vitesse de défilement actuelle
    float     m_spawnTimer;          // Délai avant prochain obstacle
    float     m_nextSpawnInterval;   // Intervalle aléatoire
    float     m_cinematicTimer;      // Durée cinématique intro/fin
    int       m_cinematicStep;       // Étape de la cinématique
    float     m_finalScore;          // Temps restant à la victoire
    bool      m_shakeActive;         // Tremblement d'écran
    float     m_shakeTimer;
    sf::Vector2f m_shakeOffset;

    // --- Entités ---
    std::unique_ptr<Player>              m_player;
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;

    // --- Décor (parallaxe) ---
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

    // --- HUD ---
    sf::RectangleShape m_pressureBarBg;
    sf::RectangleShape m_pressureBarFill;
    sf::RectangleShape m_progressBarBg;
    sf::RectangleShape m_progressBarFill;
    sf::RectangleShape m_floorRect;
    sf::RectangleShape m_ceilingRect;

    // --- Capsule de sauvetage ---
    sf::RectangleShape m_capsule;
    sf::CircleShape    m_capsuleWindow;
    bool               m_capsuleVisible;
    float              m_capsuleX;

    // --- Méthodes privées ---
    void processEvents();
    void update(float deltaTime);
    void render();

    // Sous-méthodes update
    void updatePlaying(float deltaTime);
    void updateCinematic(float deltaTime);
    void updateShake(float deltaTime);
    void spawnObstacle();
    void checkCollisions();
    void cleanObstacles();
    void updateScrollSpeed(float deltaTime);
    void updateParallax(float deltaTime);

    // Sous-méthodes render
    void drawBackground();
    void drawHUD();
    void drawMainMenu();
    void drawIntroCinematic();
    void drawVictoryScreen();
    void drawGameOverScreen();
    void drawCapsule();
    void drawScreenShake();

    // Helpers menu
    void startGame();
    void resetGame();
    void triggerScreenShake(float duration);

    bool loadFont();
    sf::Text makeText(const std::string& str, unsigned int size, sf::Color color);
};
