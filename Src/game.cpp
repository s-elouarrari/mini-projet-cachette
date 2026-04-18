#include "game.hpp"
#include "utils.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

// ============================================================
//  Game.cpp — Logique principale du jeu Station Evac
//
//  SYSTEME DE PROGRESSION :
//    - Le joueur survit et accumule du temps (m_survivalTime)
//    - La barre de progression = m_survivalTime / SURVIVAL_TIME_FOR_CAPSULE
//    - A 100% : la capsule entre par la droite de l'ecran
//    - Si le joueur touche la capsule : ecran WIN
//
//  COLLISIONS :
//    - On utilise FloatRect + .intersects() (simple et lisible)
//    - Un seul contact = GAME OVER immediat
// ============================================================

Game::Game()
    : m_window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
               Constants::WINDOW_TITLE,
               sf::Style::Close | sf::Style::Titlebar),
      m_state(GameState::MAIN_MENU),
      m_survivalTime(0.0f),
      m_scrollSpeed(Constants::SCROLL_SPEED_BASE),
      m_spawnTimer(0.0f),
      m_nextSpawnInterval(2.0f),
      m_finalScore(0.0f),
      m_capsuleVisible(false),
      m_capsuleX(static_cast<float>(Constants::WINDOW_WIDTH) + 100.0f),
      m_shakeActive(false),
      m_shakeTimer(0.0f),
      m_shakeOffset(0.0f, 0.0f)
{
    m_window.setFramerateLimit(Constants::TARGET_FPS);
    loadFont();

    // --- Sol de la station ---
    m_floorRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 60.0f));
    m_floorRect.setPosition(0.0f, Constants::GROUND_Y);
    m_floorRect.setFillColor(sf::Color(25, 35, 65));
    m_floorRect.setOutlineColor(sf::Color(0, 180, 255, 80));
    m_floorRect.setOutlineThickness(1.0f);

    // --- Plafond de la station ---
    m_ceilingRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 20.0f));
    m_ceilingRect.setPosition(0.0f, 260.0f);
    m_ceilingRect.setFillColor(sf::Color(20, 28, 55));
    m_ceilingRect.setOutlineColor(sf::Color(0, 180, 255, 60));
    m_ceilingRect.setOutlineThickness(1.0f);

    // --- Barre de progression (fond gris fonce) ---
    m_progressBarBg.setSize(sf::Vector2f(300.0f, 20.0f));
    m_progressBarBg.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - 150.0f, 14.0f
    );
    m_progressBarBg.setFillColor(sf::Color(20, 30, 60));
    m_progressBarBg.setOutlineColor(sf::Color(0, 150, 200));
    m_progressBarBg.setOutlineThickness(1.5f);

    // --- Barre de progression (remplissage vert) ---
    m_progressBarFill.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - 150.0f, 14.0f
    );
    m_progressBarFill.setFillColor(Constants::COLOR_ACCENT_GREEN);

    // --- Capsule de sauvetage ---
    m_capsule.setSize(sf::Vector2f(80.0f, 50.0f));
    m_capsule.setFillColor(sf::Color(50, 60, 90));
    m_capsule.setOutlineColor(Constants::COLOR_ACCENT_GREEN);
    m_capsule.setOutlineThickness(2.5f);

    m_capsuleWindow.setRadius(10.0f);
    m_capsuleWindow.setFillColor(sf::Color(0, 200, 255, 150));
    m_capsuleWindow.setOutlineColor(sf::Color(0, 220, 255));
    m_capsuleWindow.setOutlineThickness(1.5f);
    m_capsuleWindow.setOrigin(10.0f, 10.0f);

    // --- Etoiles : couche 1 (loin, lente) ---
    m_starLayers[0].speed = 30.0f;
    for (int i = 0; i < 60; ++i) {
        sf::CircleShape star;
        star.setRadius(1.0f);
        star.setFillColor(sf::Color(200, 220, 255, 120));
        star.setPosition(
            Utils::randomFloat(0.0f, Constants::WINDOW_WIDTH),
            Utils::randomFloat(0.0f, Constants::GROUND_Y)
        );
        m_starLayers[0].stars.push_back(star);
    }

    // --- Etoiles : couche 2 (proche, rapide) ---
    m_starLayers[1].speed = 70.0f;
    for (int i = 0; i < 30; ++i) {
        sf::CircleShape star;
        star.setRadius(1.5f);
        star.setFillColor(sf::Color(200, 220, 255, 200));
        star.setPosition(
            Utils::randomFloat(0.0f, Constants::WINDOW_WIDTH),
            Utils::randomFloat(0.0f, Constants::GROUND_Y)
        );
        m_starLayers[1].stars.push_back(star);
    }

    // --- Panneaux metalliques de la station (decor) ---
    for (int i = 0; i < 12; ++i) {
        PanelStrip panel;
        panel.rect.setSize(sf::Vector2f(
            Utils::randomFloat(30.0f, 80.0f),
            Utils::randomFloat(8.0f, 20.0f)
        ));
        panel.rect.setPosition(
            Utils::randomFloat(0.0f, Constants::WINDOW_WIDTH),
            Utils::randomFloat(270.0f, Constants::GROUND_Y - 10.0f)
        );
        bool isCyan = (Utils::randomInt(0, 1) == 0);
        panel.color = isCyan
            ? sf::Color(30, 45, 80, 100)
            : sf::Color(0,  80, 120, 80);
        panel.rect.setFillColor(panel.color);
        panel.speed = Utils::randomFloat(80.0f, 150.0f);
        m_bgPanels.push_back(panel);
    }

    resetGame();
}

Game::~Game() {
    // unique_ptr et vector liberent la memoire automatiquement
}

// ============================================================
//  Boucle principale
// ============================================================

void Game::run() {
    while (m_window.isOpen()) {
        // deltaTime = temps ecoule depuis la derniere frame (en secondes)
        float deltaTime = m_clock.restart().asSeconds();

        // Protection contre les deltas trop grands (ex: fenetre deplacee)
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        processEvents();
        update(deltaTime);
        render();
    }
}

// ============================================================
//  Gestion des evenements clavier / fenetre
// ============================================================

void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {

        // Fermeture de la fenetre
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }

        if (event.type == sf::Event::KeyPressed) {
            switch (m_state) {

                // --- Menu principal ---
                case GameState::MAIN_MENU:
                    if (event.key.code == sf::Keyboard::Return ||
                        event.key.code == sf::Keyboard::Space) {
                        startGame();
                    }
                    if (event.key.code == sf::Keyboard::Escape) {
                        m_window.close();
                    }
                    break;

                // --- Jeu en cours ---
                case GameState::PLAYING:
                    // Saut : ESPACE ou fleche HAUT
                    if (event.key.code == sf::Keyboard::Space ||
                        event.key.code == sf::Keyboard::Up) {
                        m_player->jump();
                    }
                    break;

                // --- Ecran GAME OVER ou WIN ---
                case GameState::GAME_OVER:
                case GameState::VICTORY:
                    if (event.key.code == sf::Keyboard::Return ||
                        event.key.code == sf::Keyboard::Space) {
                        resetGame();
                        m_state = GameState::MAIN_MENU;
                    }
                    break;

                default: break;
            }
        }
    }

    // Accroupissement maintenu en continu (fleche BAS ou S)
    if (m_state == GameState::PLAYING) {
        bool crouchHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
                          sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        m_player->crouch(crouchHeld);
    }
}

// ============================================================
//  update() — Aiguille vers la bonne logique selon l'etat
// ============================================================

void Game::update(float deltaTime) {
    if (m_state == GameState::PLAYING) {
        updatePlaying(deltaTime);
    }
    updateShake(deltaTime);
}

// ============================================================
//  updatePlaying() — LOGIQUE PRINCIPALE PENDANT LA PARTIE
// ============================================================

void Game::updatePlaying(float deltaTime) {

    // --- 1. PROGRESSION PAR SURVIE ---
    // Chaque seconde de survie remplit la barre
    m_survivalTime += deltaTime;

    // Ratio entre 0.0 et 1.0 (0% a 100%)
    float progressRatio = m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;

    // Quand la barre atteint 100% : la capsule apparait
    if (progressRatio >= 1.0f && !m_capsuleVisible) {
        m_capsuleVisible = true;
        m_capsuleX = static_cast<float>(Constants::WINDOW_WIDTH) + 60.0f;
    }

    // --- 2. VITESSE CROISSANTE ---
    // Le jeu accelere progressivement jusqu'a la vitesse max
    if (m_scrollSpeed < Constants::SCROLL_SPEED_MAX) {
        m_scrollSpeed += Constants::SPEED_INCREMENT * deltaTime;
    }

    // --- 3. DECOR ---
    updateParallax(deltaTime);

    // --- 4. MISE A JOUR DU JOUEUR ---
    m_player->update(deltaTime);

    // --- 5. APPARITION DES OBSTACLES ---
    m_spawnTimer += deltaTime;
    if (m_spawnTimer >= m_nextSpawnInterval) {
        m_spawnTimer = 0.0f;
        spawnObstacle();
        // On tire aleatoirement le prochain intervalle
        m_nextSpawnInterval = Utils::randomFloat(
            Constants::OBSTACLE_SPAWN_INTERVAL_MIN,
            Constants::OBSTACLE_SPAWN_INTERVAL_MAX
        );
    }

    // --- 6. MISE A JOUR DES OBSTACLES ---
    for (auto& obstacle : m_obstacles) {
        obstacle->update(deltaTime);
    }

    // --- 7. DETECTION DES COLLISIONS ---
    // Si le joueur touche un obstacle : GAME OVER immediat
    checkCollisions();

    // Sortie anticipee si GAME OVER declenche dans checkCollisions
    if (m_state == GameState::GAME_OVER) return;

    // --- 8. NETTOYAGE ---
    cleanObstacles();

    // --- 9. CAPSULE DE SAUVETAGE ---
    if (m_capsuleVisible) {
        // La capsule se deplace de la droite vers la gauche
        m_capsuleX -= m_scrollSpeed * 0.5f * deltaTime;

        float capsuleY = Constants::GROUND_Y - 60.0f;
        m_capsule.setPosition(m_capsuleX, capsuleY);
        m_capsuleWindow.setPosition(m_capsuleX + 20.0f, capsuleY + 25.0f);

        // Boite de collision de la capsule
        sf::FloatRect capsuleBounds(m_capsuleX, capsuleY, 80.0f, 50.0f);

        // Boite de collision du joueur
        sf::FloatRect playerBounds = m_player->getBounds();

        // Si le joueur touche la capsule => VICTOIRE
        if (playerBounds.intersects(capsuleBounds)) {
            m_finalScore = m_survivalTime;
            m_state = GameState::VICTORY;
        }
    }
}

// ============================================================
//  checkCollisions() — Detection obstacle par obstacle
//  On utilise FloatRect et .intersects() : simple et lisible
// ============================================================

void Game::checkCollisions() {
    // Recupere la boite de collision du joueur
    sf::FloatRect playerBounds = m_player->getBounds();

    // On reduit legerement la hitbox pour plus de fairness
    playerBounds.left   += 5.0f;
    playerBounds.top    += 5.0f;
    playerBounds.width  -= 10.0f;
    playerBounds.height -= 10.0f;

    // On teste chaque obstacle un par un
    for (auto& obstacle : m_obstacles) {
        sf::FloatRect obstacleBounds = obstacle->getBounds();

        // .intersects() retourne true si les deux rectangles se chevauchent
        if (playerBounds.intersects(obstacleBounds)) {
            // Collision detectee : tremblement et GAME OVER immediat
            triggerScreenShake(0.5f);
            m_state = GameState::GAME_OVER;
            return; // On sort immediatement de la boucle
        }
    }
}

// ============================================================
//  spawnObstacle() — Crée un obstacle aleatoire
// ============================================================

void Game::spawnObstacle() {
    float spawnX = static_cast<float>(Constants::WINDOW_WIDTH) + 50.0f;

    // On choisit un type d'obstacle aleatoirement (0 a 3)
    int type = Utils::randomInt(0, 3);

    switch (type) {
        case 0: m_obstacles.push_back(std::make_unique<MagneticContainer>(spawnX)); break;
        case 1: m_obstacles.push_back(std::make_unique<PlasmaLeak>(spawnX));        break;
        case 2: m_obstacles.push_back(std::make_unique<SecurityDrone>(spawnX));     break;
        case 3: m_obstacles.push_back(std::make_unique<TornDuct>(spawnX));          break;
        default: break;
    }
}

// ============================================================
//  cleanObstacles() — Supprime les obstacles hors ecran
// ============================================================

void Game::cleanObstacles() {
    // On efface les obstacles qui sont passes a gauche de l'ecran
    m_obstacles.erase(
        std::remove_if(
            m_obstacles.begin(),
            m_obstacles.end(),
            [](const std::unique_ptr<Obstacle>& obs) {
                return obs->isOffScreen();
            }
        ),
        m_obstacles.end()
    );
}

// ============================================================
//  updateParallax() — Defilement du decor (etoiles + panneaux)
// ============================================================

void Game::updateParallax(float deltaTime) {
    // Defilement des deux couches d'etoiles
    for (int layer = 0; layer < 2; ++layer) {
        for (auto& star : m_starLayers[layer].stars) {
            sf::Vector2f pos = star.getPosition();
            pos.x -= m_starLayers[layer].speed * deltaTime;

            // Si l'etoile sort a gauche, on la replace a droite
            if (pos.x < -4.0f) {
                pos.x = static_cast<float>(Constants::WINDOW_WIDTH) + 2.0f;
                pos.y = Utils::randomFloat(0.0f, static_cast<float>(Constants::GROUND_Y));
            }
            star.setPosition(pos);
        }
    }

    // Defilement des panneaux metalliques
    for (auto& panel : m_bgPanels) {
        sf::Vector2f pos = panel.rect.getPosition();
        pos.x -= panel.speed * deltaTime;
        if (pos.x < -100.0f) {
            pos.x = static_cast<float>(Constants::WINDOW_WIDTH) + 10.0f;
            pos.y = Utils::randomFloat(270.0f, Constants::GROUND_Y - 10.0f);
        }
        panel.rect.setPosition(pos);
    }
}

// ============================================================
//  updateShake() — Animation du tremblement d'ecran
// ============================================================

void Game::updateShake(float deltaTime) {
    if (!m_shakeActive) return;

    m_shakeTimer -= deltaTime;
    if (m_shakeTimer <= 0.0f) {
        m_shakeActive = false;
        m_shakeOffset = sf::Vector2f(0.0f, 0.0f);
        return;
    }
    float intensity = m_shakeTimer * 8.0f;
    m_shakeOffset = sf::Vector2f(
        Utils::randomFloat(-intensity, intensity),
        Utils::randomFloat(-intensity, intensity)
    );
}

// ============================================================
//  render() — Affichage selon l'etat courant
// ============================================================

void Game::render() {
    m_window.clear(Constants::COLOR_BACKGROUND);

    // Application du tremblement d'ecran via la vue SFML
    sf::View view = m_window.getDefaultView();
    if (m_shakeActive) {
        view.setCenter(
            Constants::WINDOW_WIDTH  / 2.0f + m_shakeOffset.x,
            Constants::WINDOW_HEIGHT / 2.0f + m_shakeOffset.y
        );
    }
    m_window.setView(view);

    switch (m_state) {
        case GameState::MAIN_MENU:
            drawBackground();
            drawMainMenu();
            break;

        case GameState::PLAYING:
            drawBackground();
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            if (m_capsuleVisible) drawCapsule();
            for (auto& obstacle : m_obstacles) obstacle->draw(m_window);
            m_player->draw(m_window);
            drawHUD();
            break;

        case GameState::VICTORY:
            drawBackground();
            drawVictoryScreen();
            break;

        case GameState::GAME_OVER:
            drawBackground();
            drawGameOverScreen();
            break;
    }

    m_window.display();
}

// ============================================================
//  drawBackground() — Etoiles + panneaux de station
// ============================================================

void Game::drawBackground() {
    for (int layer = 0; layer < 2; ++layer) {
        for (const auto& star : m_starLayers[layer].stars) {
            m_window.draw(star);
        }
    }
    for (const auto& panel : m_bgPanels) {
        m_window.draw(panel.rect);
    }
}

// ============================================================
//  drawHUD() — Barre de progression centree en haut
// ============================================================

void Game::drawHUD() {
    // --- Calcul du ratio de progression ---
    float progressRatio = m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;
    if (progressRatio > 1.0f) progressRatio = 1.0f;

    // --- Mise a jour de la largeur de la barre verte ---
    float fillWidth = 300.0f * progressRatio;
    m_progressBarFill.setSize(sf::Vector2f(fillWidth, 20.0f));

    // La barre passe au vert eclatant a 100%
    sf::Color barColor = (progressRatio >= 1.0f)
        ? sf::Color(0, 255, 120)
        : Utils::lerpColor(sf::Color(0, 120, 255), Constants::COLOR_ACCENT_GREEN, progressRatio);
    m_progressBarFill.setFillColor(barColor);

    m_window.draw(m_progressBarBg);
    m_window.draw(m_progressBarFill);

    // --- Label "CAPSULE" au-dessus de la barre ---
    std::string labelStr = (progressRatio >= 1.0f)
        ? "CAPSULE EN APPROCHE !"
        : "CAPSULE : " + std::to_string(static_cast<int>(progressRatio * 100)) + "%";
    sf::Text label = makeText(labelStr, 13, barColor);
    label.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - label.getLocalBounds().width / 2.0f,
        38.0f
    );
    m_window.draw(label);

    // --- Instructions en bas ---
    sf::Text controls = makeText(
        "[ESPACE / HAUT] Sauter     [BAS] S'accroupir",
        12, sf::Color(100, 130, 180)
    );
    controls.setPosition(10.0f, Constants::WINDOW_HEIGHT - 22.0f);
    m_window.draw(controls);
}

// ============================================================
//  drawCapsule() — La capsule de sauvetage avec son label
// ============================================================

void Game::drawCapsule() {
    m_window.draw(m_capsule);
    m_window.draw(m_capsuleWindow);

    sf::Text label = makeText(">>> CAPSULE <<<", 13, Constants::COLOR_ACCENT_GREEN);
    label.setPosition(
        m_capsuleX + 80.0f / 2.0f - label.getLocalBounds().width / 2.0f,
        m_capsule.getPosition().y - 22.0f
    );
    m_window.draw(label);
}

// ============================================================
//  drawMainMenu() — Ecran d'accueil avec le bouton Start
// ============================================================

void Game::drawMainMenu() {
    // --- Titre du jeu ---
    sf::Text title = makeText("STATION EVAC", 52, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f,
        80.0f
    );
    m_window.draw(title);

    sf::Text subtitle = makeText("HULL BREACH", 26, Constants::COLOR_ACCENT_ORANGE);
    subtitle.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - subtitle.getLocalBounds().width / 2.0f,
        145.0f
    );
    m_window.draw(subtitle);

    // --- Ligne decorative ---
    sf::RectangleShape line(sf::Vector2f(320.0f, 2.0f));
    line.setFillColor(Constants::COLOR_ACCENT_CYAN);
    line.setPosition(Constants::WINDOW_WIDTH / 2.0f - 160.0f, 185.0f);
    m_window.draw(line);

    // --- Histoire du jeu (texte simple et clair) ---
    sf::Text story1 = makeText("La station spatiale est en train d'exploser !", 18, Constants::COLOR_HUD_TEXT);
    story1.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - story1.getLocalBounds().width / 2.0f, 210.0f
    );
    m_window.draw(story1);

    sf::Text story2 = makeText("Aidez le robot a s'echapper avant la fin !", 18, sf::Color(200, 180, 160));
    story2.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - story2.getLocalBounds().width / 2.0f, 240.0f
    );
    m_window.draw(story2);

    sf::Text story3 = makeText("Survivez 20 secondes pour faire apparaitre la capsule de sauvetage.", 15, sf::Color(160, 180, 200));
    story3.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - story3.getLocalBounds().width / 2.0f, 270.0f
    );
    m_window.draw(story3);

    // --- Instructions de controle ---
    sf::Text ctrl1 = makeText("[ESPACE / HAUT]  Sauter par-dessus les obstacles au sol", 15, Constants::COLOR_HUD_TEXT);
    ctrl1.setPosition(Constants::WINDOW_WIDTH / 2.0f - ctrl1.getLocalBounds().width / 2.0f, 315.0f);
    m_window.draw(ctrl1);

    sf::Text ctrl2 = makeText("[BAS]  S'accroupir sous les obstacles aeriens", 15, Constants::COLOR_HUD_TEXT);
    ctrl2.setPosition(Constants::WINDOW_WIDTH / 2.0f - ctrl2.getLocalBounds().width / 2.0f, 340.0f);
    m_window.draw(ctrl2);

    // --- Bouton START ---
    sf::RectangleShape startBtn(sf::Vector2f(260.0f, 55.0f));
    startBtn.setFillColor(sf::Color(0, 60, 100));
    startBtn.setOutlineColor(Constants::COLOR_ACCENT_CYAN);
    startBtn.setOutlineThickness(2.5f);
    startBtn.setPosition(Constants::WINDOW_WIDTH / 2.0f - 130.0f, 400.0f);
    m_window.draw(startBtn);

    sf::Text startText = makeText("[ ENTREE ]  START", 24, Constants::COLOR_ACCENT_CYAN);
    startText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - startText.getLocalBounds().width / 2.0f,
        412.0f
    );
    m_window.draw(startText);

    sf::Text quitText = makeText("[ ECHAP ] Quitter", 15, sf::Color(120, 130, 160));
    quitText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - quitText.getLocalBounds().width / 2.0f,
        490.0f
    );
    m_window.draw(quitText);
}

// ============================================================
//  drawVictoryScreen() — Ecran WIN
// ============================================================

void Game::drawVictoryScreen() {
    // Fond sombre semi-transparent
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    m_window.draw(overlay);

    sf::Text title = makeText("MISSION REUSSIE !", 46, Constants::COLOR_ACCENT_GREEN);
    title.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 150.0f
    );
    m_window.draw(title);

    sf::Text sub = makeText("Le robot a atteint la capsule de sauvetage.", 22, Constants::COLOR_HUD_TEXT);
    sub.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 225.0f
    );
    m_window.draw(sub);

    // Affichage du temps de survie
    std::string scoreStr = "Temps de survie : " + Utils::formatTime(m_finalScore);
    sf::Text score = makeText(scoreStr, 28, Constants::COLOR_ACCENT_CYAN);
    score.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - score.getLocalBounds().width / 2.0f, 290.0f
    );
    m_window.draw(score);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 20, sf::Color(140, 180, 140));
    replay.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 400.0f
    );
    m_window.draw(replay);
}

// ============================================================
//  drawGameOverScreen() — Ecran GAME OVER
// ============================================================

void Game::drawGameOverScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 190));
    m_window.draw(overlay);

    sf::Text title = makeText("GAME OVER", 54, Constants::COLOR_ACCENT_RED);
    title.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 150.0f
    );
    m_window.draw(title);

    sf::Text sub = makeText("Le robot a percute un obstacle.", 22, sf::Color(200, 160, 160));
    sub.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 230.0f
    );
    m_window.draw(sub);

    // Affichage de la progression atteinte avant la collision
    float pct = (m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE) * 100.0f;
    if (pct > 100.0f) pct = 100.0f;
    std::string progressStr = "Progression atteinte : " + std::to_string(static_cast<int>(pct)) + "%";
    sf::Text progressText = makeText(progressStr, 22, sf::Color(180, 180, 220));
    progressText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - progressText.getLocalBounds().width / 2.0f, 285.0f
    );
    m_window.draw(progressText);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 20, sf::Color(180, 100, 100));
    replay.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 400.0f
    );
    m_window.draw(replay);
}

// ============================================================
//  Fonctions utilitaires
// ============================================================

void Game::startGame() {
    resetGame();
    m_state = GameState::PLAYING;
}

void Game::resetGame() {
    m_survivalTime       = 0.0f;
    m_scrollSpeed        = Constants::SCROLL_SPEED_BASE;
    m_spawnTimer         = 0.0f;
    m_nextSpawnInterval  = 2.0f;
    m_finalScore         = 0.0f;
    m_shakeActive        = false;
    m_shakeTimer         = 0.0f;
    m_capsuleVisible     = false;
    m_capsuleX           = static_cast<float>(Constants::WINDOW_WIDTH) + 100.0f;

    // On recrée le joueur et on vide la liste des obstacles
    m_obstacles.clear();
    m_player = std::make_unique<Player>(Constants::PLAYER_START_X, Constants::PLAYER_START_Y);
}

void Game::triggerScreenShake(float duration) {
    m_shakeActive = true;
    m_shakeTimer  = duration;
}

bool Game::loadFont() {
    // On essaie plusieurs chemins selon l'OS de l'utilisateur
    const std::vector<std::string> fontPaths = {
        "assets/font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Courier.ttc",
        "C:/Windows/Fonts/cour.ttf"
    };
    for (const auto& path : fontPaths) {
        if (m_font.loadFromFile(path)) return true;
    }
    return false;
}

sf::Text Game::makeText(const std::string& str, unsigned int size, sf::Color color) {
    sf::Text text;
    text.setFont(m_font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    return text;
}
