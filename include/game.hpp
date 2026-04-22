#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include "Player.hpp"
#include "Obstacle.hpp"
#include "Background.hpp"
#include "HUD.hpp"
#include "Utils.hpp"

// ============================================================
//  Game.hpp — Coordinateur principal du jeu
//
//  RÔLE : Game orchestre toutes les autres classes.
//  Il ne fait plus lui-même le rendu du décor (Background)
//  ni du HUD (HUD) — il délègue ces responsabilités.
//
//  CONCEPT POO ILLUSTRÉ : Composition
//  Game "a un" Background, "a un" HUD, "a un" Player.
//  C'est différent de l'héritage ("est un").
//
//  FLUX DES ÉTATS :
//    MAIN_MENU → READY  (bouton JOUER)
//    READY     → PLAYING (touche ESPACE)
//    PLAYING   → VICTORY / GAME_OVER / EXPLOSION
//    Fin       → MAIN_MENU (touche ENTRÉE)
// ============================================================

// Enum des états du jeu (remplace les int magiques)
enum class GameState {
    MAIN_MENU,  // Menu principal avec 3 boutons
    ABOUT,      // Écran "À Propos"
    READY,      // Robot à l'arrêt, attend ESPACE pour démarrer
    PLAYING,    // Partie en cours
    VICTORY,    // Robot a atteint la capsule
    EXPLOSION,  // Temps écoulé, station explose
    GAME_OVER   // Robot à court de cœurs
};

// Enum pour les boutons du menu (plus lisible que 0/1/2)
enum class MenuButton {
    PLAY  = 0,
    ABOUT = 1,
    QUIT  = 2
};

class Game {
public:
    // Constructeur : initialise la fenêtre et toutes les ressources
    Game();

    // Destructeur : SFML libère ses ressources automatiquement
    // (sf::RenderWindow, sf::Sound, sf::Music ont leurs propres destructeurs)
    ~Game();

    // Lance la boucle principale (bloquante jusqu'à fermeture)
    void run();

private:
    // ── Fenêtre et outils SFML ────────────────────────────────
    sf::RenderWindow m_window;  // la fenêtre de rendu
    sf::Clock        m_clock;   // horloge pour le deltaTime
    sf::Font         m_font;    // police partagée entre HUD et menus

    // ── État du jeu ───────────────────────────────────────────
    GameState  m_state;           // état courant
    MenuButton m_selectedButton;  // bouton surligné dans le menu

    // ── Variables de gameplay ─────────────────────────────────
    float m_explosionTimer;   // secondes restantes avant explosion
    float m_survivalTime;     // secondes de survie accumulées
    float m_scrollSpeed;      // vitesse actuelle des obstacles (px/s)
    float m_spawnInterval;    // intervalle entre deux obstacles (s)
    float m_spawnTimer;       // compteur depuis le dernier spawn
    bool  m_gameWon;          // true = robot a touché la capsule
    float m_finalScore;       // score affiché sur l'écran victoire

    // ── Screen shake (tremblement d'écran lors d'une collision) ─
    bool         m_shakeActive;
    float        m_shakeTimer;
    sf::Vector2f m_shakeOffset;

    // ── Entités du jeu ────────────────────────────────────────
    // std::unique_ptr : gestion automatique de la mémoire (pas de delete manuel)
    std::unique_ptr<Player>                m_player;
    // std::vector : conteneur dynamique de la STL
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;

    // ── Capsule de sauvetage ──────────────────────────────────
    bool               m_capsuleVisible;
    float              m_capsuleX;
    float              m_capsuleGlowTimer;
    sf::RectangleShape m_capsule;
    sf::CircleShape    m_capsuleWindow;

    // ── Décor et HUD (classes dédiées) ───────────────────────
    // COMPOSITION : Game utilise Background et HUD comme membres
    Background m_background;
    HUD        m_hud;

    // Visuels fixes du terrain
    sf::RectangleShape m_floorRect;
    sf::RectangleShape m_ceilingRect;

    // ── Audio SFML ────────────────────────────────────────────
    // sf::SoundBuffer : stocke les données audio en mémoire RAM
    sf::SoundBuffer m_bufJump;
    sf::SoundBuffer m_bufDuck;
    sf::SoundBuffer m_bufHit;
    sf::SoundBuffer m_bufWin;
    sf::SoundBuffer m_bufGameOver; // NOUVEAU : son de game over

    // sf::Sound : joue un SoundBuffer (peut se superposer)
    sf::Sound m_sndJump;
    sf::Sound m_sndDuck;
    sf::Sound m_sndHit;
    sf::Sound m_sndWin;
    sf::Sound m_sndGameOver; // NOUVEAU

    // sf::Music : streame depuis le disque (pour les longs morceaux)
    sf::Music m_music;

    bool m_audioLoaded; // false si les fichiers audio sont absents

    // ── Méthodes privées : organisation de la boucle ──────────
    void processEvents();                  // lecture clavier/fenêtre
    void update(float deltaTime);          // logique frame par frame
    void render();                         // dessin de la frame

    void updatePlaying(float deltaTime);   // logique spécifique en jeu
    void checkCollisions();                // détection FloatRect
    void spawnObstacle();                  // crée un obstacle aléatoire
    void cleanObstacles();                 // supprime les hors-écran
    void updateShake(float deltaTime);     // anime le tremblement

    // Dessin des écrans
    void drawCapsule();
    void drawMainMenu();
    void drawAboutScreen();
    void drawReadyScreen();
    void drawVictoryScreen();
    void drawExplosionScreen();
    void drawGameOverScreen();
    void drawRobotPreview(float cx, float cy, float scale);
    void drawButton(const std::string& label, float y, bool selected);

    // Utilitaires
    void     startGame();
    void     resetGame();
    void     triggerScreenShake(float duration);
    void     loadAudio();
    bool     loadFont();
    sf::Text makeText(const std::string& str, unsigned int size, sf::Color color);
};
