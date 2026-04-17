#include "Game.hpp"
#include "Utils.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

// ============================================================
//  Game.cpp — Boucle principale, états, rendu complet
// ============================================================

Game::Game()
    : m_window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
               Constants::WINDOW_TITLE,
               sf::Style::Close | sf::Style::Titlebar),
      m_state(GameState::MAIN_MENU),
      m_pressureTimer(Constants::PRESSURE_MAX),
      m_distanceTraveled(0.0f),
      m_scrollSpeed(Constants::SCROLL_SPEED_BASE),
      m_spawnTimer(0.0f),
      m_nextSpawnInterval(2.0f),
      m_cinematicTimer(0.0f),
      m_cinematicStep(0),
      m_finalScore(0.0f),
      m_shakeActive(false),
      m_shakeTimer(0.0f),
      m_shakeOffset(0.0f, 0.0f),
      m_capsuleVisible(false),
      m_capsuleX(static_cast<float>(Constants::WINDOW_WIDTH) + 100.0f)
{
    m_window.setFramerateLimit(Constants::TARGET_FPS);
    loadFont();

    // --- Sol et plafond ---
    m_floorRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 60.0f));
    m_floorRect.setPosition(0.0f, Constants::GROUND_Y);
    m_floorRect.setFillColor(sf::Color(25, 35, 65));
    m_floorRect.setOutlineColor(sf::Color(0, 180, 255, 80));
    m_floorRect.setOutlineThickness(1.0f);

    m_ceilingRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 20.0f));
    m_ceilingRect.setPosition(0.0f, 260.0f);
    m_ceilingRect.setFillColor(sf::Color(20, 28, 55));
    m_ceilingRect.setOutlineColor(sf::Color(0, 180, 255, 60));
    m_ceilingRect.setOutlineThickness(1.0f);

    // --- HUD - Barre pression ---
    m_pressureBarBg.setSize(sf::Vector2f(220.0f, 18.0f));
    m_pressureBarBg.setPosition(20.0f, 20.0f);
    m_pressureBarBg.setFillColor(sf::Color(20, 30, 60));
    m_pressureBarBg.setOutlineColor(sf::Color(0, 150, 200));
    m_pressureBarBg.setOutlineThickness(1.5f);

    m_pressureBarFill.setPosition(20.0f, 20.0f);
    m_pressureBarFill.setFillColor(Constants::COLOR_PRESSURE_HIGH);

    // --- HUD - Barre progression ---
    m_progressBarBg.setSize(sf::Vector2f(220.0f, 12.0f));
    m_progressBarBg.setPosition(20.0f, 50.0f);
    m_progressBarBg.setFillColor(sf::Color(20, 30, 60));
    m_progressBarBg.setOutlineColor(sf::Color(0, 150, 200));
    m_progressBarBg.setOutlineThickness(1.5f);

    m_progressBarFill.setPosition(20.0f, 50.0f);
    m_progressBarFill.setFillColor(Constants::COLOR_ACCENT_GREEN);

    // --- Étoiles fond (2 couches parallaxe) ---
    for (int layer = 0; layer < 2; ++layer) {
        m_starLayers[layer].speed = (layer == 0) ? 30.0f : 70.0f;
        int starCount = (layer == 0) ? 60 : 30;
        for (int i = 0; i < starCount; ++i) {
            sf::CircleShape star;
            float radius = (layer == 0) ? 1.0f : 1.5f;
            star.setRadius(radius);
            star.setFillColor(sf::Color(200, 220, 255, (layer == 0) ? 120 : 200));
            star.setPosition(
                Utils::randomFloat(0.0f, static_cast<float>(Constants::WINDOW_WIDTH)),
                Utils::randomFloat(0.0f, static_cast<float>(Constants::GROUND_Y))
            );
            m_starLayers[layer].stars.push_back(star);
        }
    }

    // --- Panneaux de fond de station (parallaxe) ---
    for (int i = 0; i < 12; ++i) {
        PanelStrip panel;
        panel.rect.setSize(sf::Vector2f(
            Utils::randomFloat(30.0f, 80.0f),
            Utils::randomFloat(8.0f, 20.0f)
        ));
        panel.rect.setPosition(
            Utils::randomFloat(0.0f, static_cast<float>(Constants::WINDOW_WIDTH)),
            Utils::randomFloat(270.0f, Constants::GROUND_Y - 10.0f)
        );
        panel.color = (Utils::randomInt(0, 1) == 0)
            ? sf::Color(30, 45, 80, 100)
            : sf::Color(0, 80, 120, 80);
        panel.rect.setFillColor(panel.color);
        panel.speed = Utils::randomFloat(80.0f, 150.0f);
        m_bgPanels.push_back(panel);
    }

    // --- Capsule de sauvetage ---
    m_capsule.setSize(sf::Vector2f(80.0f, 50.0f));
    m_capsule.setFillColor(sf::Color(50, 60, 90));
    m_capsule.setOutlineColor(Constants::COLOR_ACCENT_GREEN);
    m_capsule.setOutlineThickness(2.0f);

    m_capsuleWindow.setRadius(10.0f);
    m_capsuleWindow.setFillColor(sf::Color(0, 200, 255, 150));
    m_capsuleWindow.setOutlineColor(sf::Color(0, 220, 255));
    m_capsuleWindow.setOutlineThickness(1.5f);
    m_capsuleWindow.setOrigin(10.0f, 10.0f);

    resetGame();
}

Game::~Game() {
    // unique_ptr et vector nettoient automatiquement
}

// ============================================================
//  Boucle principale
// ============================================================

void Game::run() {
    while (m_window.isOpen()) {
        float deltaTime = m_clock.restart().asSeconds();
        // Protection contre les très grands deltas (ex: breakpoint)
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        processEvents();
        update(deltaTime);
        render();
    }
}

// ============================================================
//  Gestion des événements
// ============================================================

void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }

        if (event.type == sf::Event::KeyPressed) {
            switch (m_state) {
                case GameState::MAIN_MENU:
                    if (event.key.code == sf::Keyboard::Return ||
                        event.key.code == sf::Keyboard::Space) {
                        startGame();
                    }
                    if (event.key.code == sf::Keyboard::Escape) {
                        m_window.close();
                    }
                    break;

                case GameState::INTRO_CINEMATIC:
                    if (event.key.code == sf::Keyboard::Space ||
                        event.key.code == sf::Keyboard::Return) {
                        m_cinematicTimer = 999.0f; // Skip
                    }
                    break;

                case GameState::PLAYING:
                    if (event.key.code == sf::Keyboard::Space ||
                        event.key.code == sf::Keyboard::Up) {
                        m_player->jump();
                    }
                    if (event.key.code == sf::Keyboard::P) {
                        m_state = GameState::PAUSED;
                    }
                    break;

                case GameState::PAUSED:
                    if (event.key.code == sf::Keyboard::P ||
                        event.key.code == sf::Keyboard::Escape) {
                        m_state = GameState::PLAYING;
                    }
                    break;

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

        if (event.type == sf::Event::KeyReleased) {
            if (m_state == GameState::PLAYING) {
                if (event.key.code == sf::Keyboard::Down) {
                    m_player->crouch(false);
                }
            }
        }
    }

    // Crouch maintenu en continu
    if (m_state == GameState::PLAYING) {
        bool crouchHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
                          sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        m_player->crouch(crouchHeld);
    }
}

// ============================================================
//  Update
// ============================================================

void Game::update(float deltaTime) {
    switch (m_state) {
        case GameState::INTRO_CINEMATIC:
            updateCinematic(deltaTime);
            break;
        case GameState::PLAYING:
            updatePlaying(deltaTime);
            break;
        default: break;
    }
    updateShake(deltaTime);
}

void Game::updatePlaying(float deltaTime) {
    // Pression (oxygène)
    m_pressureTimer -= deltaTime;
    if (m_pressureTimer <= 0.0f) {
        m_pressureTimer = 0.0f;
        m_state = GameState::GAME_OVER;
        return;
    }

    // Progression
    m_distanceTraveled += m_scrollSpeed * deltaTime;

    // Vitesse croissante
    updateScrollSpeed(deltaTime);

    // Défilement arrière-plan
    updateParallax(deltaTime);

    // Joueur
    m_player->update(deltaTime);

    if (!m_player->isAlive()) {
        triggerScreenShake(0.6f);
        m_state = GameState::GAME_OVER;
        return;
    }

    // Apparition obstacles
    m_spawnTimer += deltaTime;
    if (m_spawnTimer >= m_nextSpawnInterval) {
        m_spawnTimer = 0.0f;
        spawnObstacle();
        m_nextSpawnInterval = Utils::randomFloat(
            Constants::OBSTACLE_SPAWN_INTERVAL_MIN,
            Constants::OBSTACLE_SPAWN_INTERVAL_MAX
        );
    }

    // Mise à jour obstacles
    for (auto& obs : m_obstacles) {
        obs->update(deltaTime);
    }

    // Collisions
    checkCollisions();

    // Nettoyage des obstacles hors écran
    cleanObstacles();

    // Capsule : apparaît à ~80% du trajet
    float progress = m_distanceTraveled / Constants::DISTANCE_TO_CAPSULE;
    if (progress >= 0.80f && !m_capsuleVisible) {
        m_capsuleVisible = true;
        m_capsuleX = static_cast<float>(Constants::WINDOW_WIDTH) + 60.0f;
    }

    if (m_capsuleVisible) {
        m_capsuleX -= m_scrollSpeed * deltaTime;
        float capsuleY = Constants::GROUND_Y - 60.0f;
        m_capsule.setPosition(m_capsuleX, capsuleY);
        m_capsuleWindow.setPosition(m_capsuleX + 20.0f, capsuleY + 25.0f);

        // Contact joueur / capsule => victoire
        sf::FloatRect capsuleBounds(m_capsuleX, capsuleY, 80.0f, 50.0f);
        if (m_player->getBounds().intersects(capsuleBounds)) {
            m_finalScore = m_pressureTimer;
            m_state = GameState::VICTORY;
            m_cinematicTimer = 0.0f;
            m_cinematicStep = 0;
        }
    }

    // Victoire automatique si distance atteinte sans capsule visible
    if (m_distanceTraveled >= Constants::DISTANCE_TO_CAPSULE && !m_capsuleVisible) {
        m_finalScore = m_pressureTimer;
        m_state = GameState::VICTORY;
    }
}

void Game::updateCinematic(float deltaTime) {
    m_cinematicTimer += deltaTime;
    // La cinématique dure 6s (3 étapes de 2s), ou est skippée
    if (m_cinematicTimer >= 6.0f) {
        m_state = GameState::PLAYING;
        m_cinematicTimer = 0.0f;
    }
}

void Game::updateShake(float deltaTime) {
    if (!m_shakeActive) return;
    m_shakeTimer -= deltaTime;
    if (m_shakeTimer <= 0.0f) {
        m_shakeActive = false;
        m_shakeOffset = sf::Vector2f(0.0f, 0.0f);
        return;
    }
    float intensity = m_shakeTimer * 6.0f;
    m_shakeOffset = sf::Vector2f(
        Utils::randomFloat(-intensity, intensity),
        Utils::randomFloat(-intensity, intensity)
    );
}

void Game::spawnObstacle() {
    float spawnX = static_cast<float>(Constants::WINDOW_WIDTH) + 50.0f;
    int obstacleType = Utils::randomInt(0, 3);

    switch (obstacleType) {
        case 0: m_obstacles.push_back(std::make_unique<MagneticContainer>(spawnX)); break;
        case 1: m_obstacles.push_back(std::make_unique<PlasmaLeak>(spawnX));        break;
        case 2: m_obstacles.push_back(std::make_unique<SecurityDrone>(spawnX));     break;
        case 3: m_obstacles.push_back(std::make_unique<TornDuct>(spawnX));          break;
        default: break;
    }
}

void Game::checkCollisions() {
    sf::FloatRect playerBounds = m_player->getBounds();
    // Réduction légère de la hitbox pour plus de fairness
    playerBounds.left   += 4.0f;
    playerBounds.top    += 4.0f;
    playerBounds.width  -= 8.0f;
    playerBounds.height -= 8.0f;

    for (auto& obs : m_obstacles) {
        if (!obs->isAlive()) continue;
        sf::FloatRect obsBounds = obs->getBounds();
        // Réduction hitbox obstacle aussi
        obsBounds.left   += 3.0f;
        obsBounds.top    += 3.0f;
        obsBounds.width  -= 6.0f;
        obsBounds.height -= 6.0f;

        if (playerBounds.intersects(obsBounds)) {
            m_player->takeDamage();
            triggerScreenShake(0.3f);
            obs->setAlive(false); // L'obstacle disparaît après collision
            break;
        }
    }
}

void Game::cleanObstacles() {
    m_obstacles.erase(
        std::remove_if(m_obstacles.begin(), m_obstacles.end(),
            [](const std::unique_ptr<Obstacle>& obs) {
                return obs->isOffScreen() || !obs->isAlive();
            }),
        m_obstacles.end()
    );
}

void Game::updateScrollSpeed(float deltaTime) {
    if (m_scrollSpeed < Constants::SCROLL_SPEED_MAX) {
        m_scrollSpeed += Constants::SPEED_INCREMENT * deltaTime;
    }
    // Synchroniser la vitesse dans les sous-classes (via le const de base)
    // (On utilise la valeur directement dans Obstacle::update via la constante)
}

void Game::updateParallax(float deltaTime) {
    for (int layer = 0; layer < 2; ++layer) {
        for (auto& star : m_starLayers[layer].stars) {
            sf::Vector2f pos = star.getPosition();
            pos.x -= m_starLayers[layer].speed * deltaTime;
            if (pos.x < -4.0f) {
                pos.x = static_cast<float>(Constants::WINDOW_WIDTH) + 2.0f;
                pos.y = Utils::randomFloat(0.0f, static_cast<float>(Constants::GROUND_Y));
            }
            star.setPosition(pos);
        }
    }

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

void Game::triggerScreenShake(float duration) {
    m_shakeActive = true;
    m_shakeTimer  = duration;
}

// ============================================================
//  Render
// ============================================================

void Game::render() {
    m_window.clear(Constants::COLOR_BACKGROUND);

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
        case GameState::INTRO_CINEMATIC:
            drawBackground();
            drawIntroCinematic();
            break;
        case GameState::PLAYING:
        case GameState::PAUSED:
            drawBackground();
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            if (m_capsuleVisible) drawCapsule();
            for (auto& obs : m_obstacles) obs->draw(m_window);
            m_player->draw(m_window);
            drawHUD();
            if (m_state == GameState::PAUSED) {
                sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
                overlay.setFillColor(sf::Color(0, 0, 0, 140));
                m_window.draw(overlay);
                sf::Text pauseText = makeText("PAUSE", 48, Constants::COLOR_ACCENT_CYAN);
                pauseText.setPosition(
                    Constants::WINDOW_WIDTH / 2.0f - pauseText.getLocalBounds().width / 2.0f,
                    Constants::WINDOW_HEIGHT / 2.0f - 50.0f
                );
                m_window.draw(pauseText);
                sf::Text hint = makeText("[P] pour reprendre", 20, Constants::COLOR_HUD_TEXT);
                hint.setPosition(
                    Constants::WINDOW_WIDTH / 2.0f - hint.getLocalBounds().width / 2.0f,
                    Constants::WINDOW_HEIGHT / 2.0f + 20.0f
                );
                m_window.draw(hint);
            }
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

void Game::drawBackground() {
    // Étoiles parallaxe
    for (int layer = 0; layer < 2; ++layer) {
        for (const auto& star : m_starLayers[layer].stars) {
            m_window.draw(star);
        }
    }
    // Panneaux de la station
    for (const auto& panel : m_bgPanels) {
        m_window.draw(panel.rect);
    }
}

void Game::drawCapsule() {
    m_window.draw(m_capsule);
    m_window.draw(m_capsuleWindow);

    // Label "CAPSULE"
    sf::Text label = makeText("CAPSULE", 11, Constants::COLOR_ACCENT_GREEN);
    label.setPosition(m_capsuleX + 4.0f, m_capsule.getPosition().y - 18.0f);
    m_window.draw(label);
}

void Game::drawHUD() {
    // === Barre pression ===
    float pressureRatio = m_pressureTimer / Constants::PRESSURE_MAX;
    float barWidth = 220.0f * pressureRatio;
    m_pressureBarFill.setSize(sf::Vector2f(std::max(0.0f, barWidth), 18.0f));

    sf::Color barColor = Utils::lerpColor(
        Constants::COLOR_PRESSURE_LOW,
        Constants::COLOR_PRESSURE_HIGH,
        pressureRatio
    );
    m_pressureBarFill.setFillColor(barColor);

    m_window.draw(m_pressureBarBg);
    m_window.draw(m_pressureBarFill);

    sf::Text pressureLabel = makeText("O2 PRESSURE", 11, Constants::COLOR_HUD_TEXT);
    pressureLabel.setPosition(248.0f, 20.0f);
    m_window.draw(pressureLabel);

    sf::Text pressureTime = makeText(Utils::formatTime(m_pressureTimer), 13, barColor);
    pressureTime.setPosition(248.0f, 34.0f);
    m_window.draw(pressureTime);

    // === Barre progression ===
    float progressRatio = std::min(1.0f, m_distanceTraveled / Constants::DISTANCE_TO_CAPSULE);
    m_progressBarFill.setSize(sf::Vector2f(220.0f * progressRatio, 12.0f));
    m_window.draw(m_progressBarBg);
    m_window.draw(m_progressBarFill);

    sf::Text progressLabel = makeText("DISTANCE CAPSULE", 10, Constants::COLOR_HUD_TEXT);
    progressLabel.setPosition(248.0f, 50.0f);
    m_window.draw(progressLabel);

    // === Indicateur état joueur ===
    std::string stateStr;
    sf::Color stateColor;
    switch (m_player->getState()) {
        case PlayerState::HEALTHY: stateStr = "[HEALTHY]"; stateColor = Constants::COLOR_ACCENT_GREEN; break;
        case PlayerState::HURT:    stateStr = "[HURT]";    stateColor = Constants::COLOR_ACCENT_ORANGE; break;
        case PlayerState::DEAD:    stateStr = "[DEAD]";    stateColor = Constants::COLOR_ACCENT_RED; break;
    }
    sf::Text stateText = makeText(stateStr, 13, stateColor);
    stateText.setPosition(Constants::WINDOW_WIDTH - 120.0f, 20.0f);
    m_window.draw(stateText);

    // === Instructions ===
    sf::Text ctrlText = makeText("[ESPACE] Sauter   [BAS] S'accroupir   [P] Pause", 11, sf::Color(100, 130, 180));
    ctrlText.setPosition(10.0f, Constants::WINDOW_HEIGHT - 22.0f);
    m_window.draw(ctrlText);
}

void Game::drawMainMenu() {
    // Titre
    sf::Text title = makeText("STATION EVAC", 54, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f,
        120.0f
    );
    m_window.draw(title);

    sf::Text subtitle = makeText("HULL BREACH", 28, Constants::COLOR_ACCENT_ORANGE);
    subtitle.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - subtitle.getLocalBounds().width / 2.0f,
        185.0f
    );
    m_window.draw(subtitle);

    // Ligne décorative
    sf::RectangleShape line(sf::Vector2f(300.0f, 2.0f));
    line.setFillColor(Constants::COLOR_ACCENT_CYAN);
    line.setPosition(Constants::WINDOW_WIDTH / 2.0f - 150.0f, 225.0f);
    m_window.draw(line);

    // Instructions rapides
    const std::vector<std::string> instructions = {
        "[ESPACE / HAUT]  Saut lunaire (obstacles au sol)",
        "[BAS]            Accroupissement (obstacles aeriens)",
        "[P]              Pause",
        "",
        "Atteignez la capsule avant que la pression chute!"
    };
    float yPos = 250.0f;
    for (const auto& line_str : instructions) {
        sf::Text line_text = makeText(line_str, 16, Constants::COLOR_HUD_TEXT);
        line_text.setPosition(
            Constants::WINDOW_WIDTH / 2.0f - line_text.getLocalBounds().width / 2.0f,
            yPos
        );
        m_window.draw(line_text);
        yPos += 26.0f;
    }

    // Bouton jouer
    sf::RectangleShape playBtn(sf::Vector2f(240.0f, 50.0f));
    playBtn.setFillColor(sf::Color(0, 80, 120));
    playBtn.setOutlineColor(Constants::COLOR_ACCENT_CYAN);
    playBtn.setOutlineThickness(2.0f);
    playBtn.setPosition(Constants::WINDOW_WIDTH / 2.0f - 120.0f, 420.0f);
    m_window.draw(playBtn);

    sf::Text playText = makeText("[ ENTREE ] JOUER", 22, Constants::COLOR_ACCENT_CYAN);
    playText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - playText.getLocalBounds().width / 2.0f,
        430.0f
    );
    m_window.draw(playText);

    sf::Text quitText = makeText("[ ECHAP ] Quitter", 16, sf::Color(120, 140, 180));
    quitText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - quitText.getLocalBounds().width / 2.0f,
        490.0f
    );
    m_window.draw(quitText);
}

void Game::drawIntroCinematic() {
    // 3 étapes de 2 secondes
    int step = static_cast<int>(m_cinematicTimer / 2.0f);

    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    m_window.draw(overlay);

    std::string message;
    switch (step) {
        case 0:
            message = "ALERTE CRITIQUE : BRECHE DETECTEE DANS LA COQUE";
            break;
        case 1:
            message = "PRESSION EN CHUTE. REJOIGNEZ LA CAPSULE DE SAUVETAGE";
            break;
        case 2:
        default:
            message = "ACTIVEZ VOS BOTTES MAGNETIQUES. BONNE CHANCE, ASTRONAUTE";
            break;
    }

    sf::Text msg = makeText(message, 18, Constants::COLOR_ACCENT_ORANGE);
    msg.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - msg.getLocalBounds().width / 2.0f,
        Constants::WINDOW_HEIGHT / 2.0f - 20.0f
    );
    m_window.draw(msg);

    sf::Text skipHint = makeText("[ESPACE] Passer la cinematique", 13, sf::Color(100, 120, 160));
    skipHint.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - skipHint.getLocalBounds().width / 2.0f,
        Constants::WINDOW_HEIGHT - 40.0f
    );
    m_window.draw(skipHint);
}

void Game::drawVictoryScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    sf::Text title = makeText("MISSION ACCOMPLIE", 44, Constants::COLOR_ACCENT_GREEN);
    title.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f,
        160.0f
    );
    m_window.draw(title);

    sf::Text capsuleText = makeText("Capsule ejectee avec succes !", 22, Constants::COLOR_HUD_TEXT);
    capsuleText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - capsuleText.getLocalBounds().width / 2.0f,
        230.0f
    );
    m_window.draw(capsuleText);

    std::string scoreStr = "Pression restante : " + Utils::formatTime(m_finalScore);
    sf::Text scoreText = makeText(scoreStr, 26, Constants::COLOR_ACCENT_CYAN);
    scoreText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - scoreText.getLocalBounds().width / 2.0f,
        300.0f
    );
    m_window.draw(scoreText);

    sf::Text replayText = makeText("[ ENTREE ] Retour au menu", 20, sf::Color(130, 160, 210));
    replayText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - replayText.getLocalBounds().width / 2.0f,
        400.0f
    );
    m_window.draw(replayText);
}

void Game::drawGameOverScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    m_window.draw(overlay);

    sf::Text title = makeText("MISSION ECHOUEE", 44, Constants::COLOR_ACCENT_RED);
    title.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f,
        170.0f
    );
    m_window.draw(title);

    std::string cause = (m_pressureTimer <= 0.0f)
        ? "Depressurisation complete. L'astronaute n'a pas survecu."
        : "Collision fatale. L'astronaute est hors combat.";
    sf::Text causeText = makeText(cause, 18, sf::Color(200, 160, 160));
    causeText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - causeText.getLocalBounds().width / 2.0f,
        250.0f
    );
    m_window.draw(causeText);

    sf::Text replayText = makeText("[ ENTREE ] Retour au menu", 20, sf::Color(150, 100, 100));
    replayText.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - replayText.getLocalBounds().width / 2.0f,
        380.0f
    );
    m_window.draw(replayText);
}

// ============================================================
//  Helpers
// ============================================================

void Game::startGame() {
    resetGame();
    m_state = GameState::INTRO_CINEMATIC;
    m_cinematicTimer = 0.0f;
    m_cinematicStep  = 0;
}

void Game::resetGame() {
    m_pressureTimer    = Constants::PRESSURE_MAX;
    m_distanceTraveled = 0.0f;
    m_scrollSpeed      = Constants::SCROLL_SPEED_BASE;
    m_spawnTimer       = 0.0f;
    m_nextSpawnInterval= 2.0f;
    m_finalScore       = 0.0f;
    m_shakeActive      = false;
    m_shakeTimer       = 0.0f;
    m_capsuleVisible   = false;
    m_capsuleX         = static_cast<float>(Constants::WINDOW_WIDTH) + 100.0f;

    m_obstacles.clear();
    m_player = std::make_unique<Player>(Constants::PLAYER_START_X, Constants::PLAYER_START_Y);
}

bool Game::loadFont() {
    // On essaie plusieurs chemins courants selon l'OS
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
    return false; // Fonctionnel sans police (texte invisible)
}

sf::Text Game::makeText(const std::string& str, unsigned int size, sf::Color color) {
    sf::Text text;
    text.setFont(m_font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    return text;
}
