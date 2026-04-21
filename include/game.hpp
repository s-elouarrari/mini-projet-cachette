#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include "Player.hpp"
#include "Obstacle.hpp"
#include "Utils.hpp"

// ============================================================
//  Game.hpp — Coeur du jeu Station Evac
//
//  ETATS DU JEU :
//    MAIN_MENU → ABOUT  (retour au menu)
//    MAIN_MENU → READY  (JOUER clique : robot a l'arret, attend ESPACE)
//    READY     → PLAYING (joueur appuie sur ESPACE)
//    PLAYING   → VICTORY / GAME_OVER / EXPLOSION
//
//  AUDIO : SFML Audio (sf::SoundBuffer + sf::Sound + sf::Music)
//    Fichiers attendus dans assets/ :
//      jump.wav, duck.wav, hit.wav, win.wav, music.ogg
// ============================================================

enum class GameState {
    MAIN_MENU,
    ABOUT,
    READY,      // Robot a l'arret sur la piste, attend ESPACE
    PLAYING,
    VICTORY,
    EXPLOSION,
    GAME_OVER
};

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
    // ── Fenetre ───────────────────────────────────────────────
    sf::RenderWindow m_window;
    sf::Clock        m_clock;
    sf::Font         m_font;

    // ── Etat ──────────────────────────────────────────────────
    GameState  m_state;
    MenuButton m_selectedButton;

    // ── Gameplay ──────────────────────────────────────────────
    float m_explosionTimer;
    float m_survivalTime;
    float m_scrollSpeed;
    float m_spawnInterval;
    float m_spawnTimer;
    bool  m_gameWon;
    float m_finalScore;

    // ── Screen shake ──────────────────────────────────────────
    bool         m_shakeActive;
    float        m_shakeTimer;
    sf::Vector2f m_shakeOffset;

    // ── Entites ───────────────────────────────────────────────
    std::unique_ptr<Player>                m_player;
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;

    // ── Capsule ───────────────────────────────────────────────
    bool               m_capsuleVisible;
    float              m_capsuleX;
    sf::RectangleShape m_capsule;
    sf::CircleShape    m_capsuleWindow;
    float              m_capsuleGlowTimer; // timer local pour la capsule

    // ── Decor : etoiles (2 couches) ───────────────────────────
    struct StarLayer {
        std::vector<sf::CircleShape> stars;
        float speed;
    };
    StarLayer m_starLayers[2];

    // ── Decor : panneaux metalliques ──────────────────────────
    struct PanelStrip {
        sf::RectangleShape rect;
        sf::Color          color;
        float              speed;
    };
    std::vector<PanelStrip> m_bgPanels;

    // ── Decor : lune (parallaxe lente) ────────────────────────
    struct Crater {
        sf::Vector2f offset;  // position relative au centre de la lune
        float        radius;
    };
    sf::CircleShape      m_moon;
    std::vector<Crater>  m_moonCraters;
    float                m_moonX;
    float                m_moonY;

    // ── Visuels fixes ─────────────────────────────────────────
    sf::RectangleShape m_floorRect;
    sf::RectangleShape m_ceilingRect;
    sf::RectangleShape m_timerBarBg;
    sf::RectangleShape m_timerBarFill;
    sf::RectangleShape m_progressBarBg;
    sf::RectangleShape m_progressBarFill;

    // ── Audio ─────────────────────────────────────────────────
    // SoundBuffers : stockent les donnees audio en RAM
    sf::SoundBuffer m_bufJump;
    sf::SoundBuffer m_bufDuck;
    sf::SoundBuffer m_bufHit;
    sf::SoundBuffer m_bufWin;

    // Sounds : jouent un buffer (on peut en jouer plusieurs en meme temps)
    sf::Sound m_sndJump;
    sf::Sound m_sndDuck;
    sf::Sound m_sndHit;
    sf::Sound m_sndWin;

    // Music : streame depuis le disque (plus efficace pour les longs morceaux)
    sf::Music m_music;

    bool m_audioLoaded;  // true si au moins les fichiers sont trouves

    // ── Methodes ──────────────────────────────────────────────
    void processEvents();
    void update(float deltaTime);
    void render();

    void updatePlaying(float deltaTime);
    void updateParallax(float deltaTime);
    void updateMoon(float deltaTime);
    void updateShake(float deltaTime);
    void checkCollisions();
    void spawnObstacle();
    void cleanObstacles();

    void drawBackground();    // etoiles + panneaux + lune
    void drawMoon();
    void drawHUD();
    void drawCapsule();
    void drawReadyScreen();
    void drawMainMenu();
    void drawAboutScreen();
    void drawVictoryScreen();
    void drawExplosionScreen();
    void drawGameOverScreen();
    void drawButton(const std::string& label, float y, bool selected);
    void drawRobotPreview(float cx, float cy, float scale);

    void startGame();
    void resetGame();
    void triggerScreenShake(float duration);
    void loadAudio();

    bool     loadFont();
    sf::Text makeText(const std::string& str, unsigned int size, sf::Color color);
};
