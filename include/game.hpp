#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Player.hpp"
#include "Obstacle.hpp"
#include "Utils.hpp"

// ============================================================
//  Game.hpp — Coeur du jeu
//
//  NOUVEAUTES :
//    - Barre de temps EXPLOSION (en haut, rouge)
//    - Barre de progression CAPSULE (en haut, verte)
//    - Indicateur de sante (coeurs)
//    - Difficulte progressive (vitesse + spawn)
//    - Victoire figee avec message "Merci de m'avoir sauve !"
//    - Etat EXPLOSION si la barre rouge atteint zero
// ============================================================

enum class GameState {
    MAIN_MENU,  // Menu 3 boutons
    ABOUT,      // Ecran histoire
    PLAYING,    // Partie en cours
    VICTORY,    // Robot dans la capsule => tout arrete
    EXPLOSION,  // Temps ecoule => ecran rouge
    GAME_OVER   // Sante a zero => Game Over classique
};

// Les 3 boutons du menu
enum class MenuButton {
    PLAY  = 0,
    ABOUT = 1,
    QUIT  = 2
};

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    // --- Fenetre ---
    sf::RenderWindow m_window;
    sf::Clock        m_clock;
    sf::Font         m_font;

    // --- Etat ---
    GameState  m_state;
    MenuButton m_selectedButton;

    // --- Chronometre d'explosion (compte a rebours) ---
    float m_explosionTimer;    // Temps restant avant explosion (sec)

    // --- Progression vers la capsule ---
    float m_survivalTime;      // Temps accumule (0 -> SURVIVAL_TIME_FOR_CAPSULE)

    // --- Difficulte progressive ---
    float m_scrollSpeed;       // Vitesse actuelle des obstacles
    float m_spawnInterval;     // Intervalle actuel entre deux spawns
    float m_spawnTimer;        // Compteur depuis le dernier spawn

    // --- Fin de partie ---
    bool  m_gameWon;           // True = robot a touche la capsule
    float m_finalScore;        // Score affiché a la victoire

    // --- Tremblement d'ecran ---
    bool         m_shakeActive;
    float        m_shakeTimer;
    sf::Vector2f m_shakeOffset;

    // --- Entites ---
    std::unique_ptr<Player>                m_player;
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;

    // --- Capsule ---
    bool               m_capsuleVisible;
    float              m_capsuleX;
    sf::RectangleShape m_capsule;
    sf::CircleShape    m_capsuleWindow;

    // --- Decor (inchange) ---
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

    // --- Visuels fixes ---
    sf::RectangleShape m_floorRect;
    sf::RectangleShape m_ceilingRect;

    // Barre rouge : temps avant explosion (haut gauche)
    sf::RectangleShape m_timerBarBg;
    sf::RectangleShape m_timerBarFill;

    // Barre verte : progression capsule (haut droite)
    sf::RectangleShape m_progressBarBg;
    sf::RectangleShape m_progressBarFill;

    // --- Methodes ---
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
    void drawRobotPreview(float cx, float cy, float scale);
    void drawMainMenu();
    void drawAboutScreen();
    void drawVictoryScreen();
    void drawExplosionScreen();
    void drawGameOverScreen();
    void drawButton(const std::string& label, float y, bool selected);

    void     startGame();
    void     resetGame();
    void     triggerScreenShake(float duration);
    bool     loadFont();
    sf::Text makeText(const std::string& str, unsigned int size, sf::Color color);
};
