#include "../include/Game.hpp"
#include "../include/Utils.hpp"
#include <cmath>
#include <algorithm>

// ============================================================
//  Game.cpp — Station Evac : Hull Breach
//
//  REGLES DU JEU :
//  1. Une barre ROUGE compte le temps avant l'explosion (90s).
//     Si elle atteint zero : ecran d'explosion, mission echouee.
//  2. Une barre VERTE compte la progression vers la capsule (20s).
//     A 100% : la capsule apparait, il faut la toucher.
//  3. Le robot a 3 coeurs. Chaque collision en retire un.
//     A zero coeurs : Game Over classique.
//  4. La difficulte augmente : obstacles plus rapides, plus frequents.
//  5. Victoire = toucher la capsule => tout s'arrete.
// ============================================================

Game::Game()
    : m_window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
               Constants::WINDOW_TITLE,
               sf::Style::Close | sf::Style::Titlebar),
      m_state(GameState::MAIN_MENU),
      m_selectedButton(MenuButton::PLAY),
      m_explosionTimer(Constants::EXPLOSION_TIME),
      m_survivalTime(0.0f),
      m_scrollSpeed(Constants::SCROLL_SPEED_BASE),
      m_spawnInterval(Constants::SPAWN_INTERVAL_START),
      m_spawnTimer(0.0f),
      m_gameWon(false),
      m_finalScore(0.0f),
      m_shakeActive(false),
      m_shakeTimer(0.0f),
      m_shakeOffset(0.0f, 0.0f),
      m_capsuleVisible(false),
      m_capsuleX(static_cast<float>(Constants::WINDOW_WIDTH) + 100.0f)
{
    m_window.setFramerateLimit(Constants::TARGET_FPS);
    loadFont();

    // --- Sol ---
    m_floorRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 60.0f));
    m_floorRect.setPosition(0.0f, Constants::GROUND_Y);
    m_floorRect.setFillColor(sf::Color(25, 35, 65));
    m_floorRect.setOutlineColor(sf::Color(0, 180, 255, 80));
    m_floorRect.setOutlineThickness(1.0f);

    // --- Plafond ---
    m_ceilingRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 20.0f));
    m_ceilingRect.setPosition(0.0f, 260.0f);
    m_ceilingRect.setFillColor(sf::Color(20, 28, 55));
    m_ceilingRect.setOutlineColor(sf::Color(0, 180, 255, 60));
    m_ceilingRect.setOutlineThickness(1.0f);

    // -------------------------------------------------------
    //  Barre ROUGE : temps avant explosion (haut gauche)
    // -------------------------------------------------------
    m_timerBarBg.setSize(sf::Vector2f(260.0f, 18.0f));
    m_timerBarBg.setPosition(10.0f, 12.0f);
    m_timerBarBg.setFillColor(sf::Color(30, 10, 10));
    m_timerBarBg.setOutlineColor(sf::Color(150, 40, 40));
    m_timerBarBg.setOutlineThickness(1.5f);

    m_timerBarFill.setPosition(10.0f, 12.0f);
    m_timerBarFill.setFillColor(sf::Color(220, 40, 40));

    // -------------------------------------------------------
    //  Barre VERTE : progression capsule (haut droite)
    // -------------------------------------------------------
    m_progressBarBg.setSize(sf::Vector2f(260.0f, 18.0f));
    m_progressBarBg.setPosition(Constants::WINDOW_WIDTH - 270.0f, 12.0f);
    m_progressBarBg.setFillColor(sf::Color(10, 30, 10));
    m_progressBarBg.setOutlineColor(sf::Color(40, 150, 40));
    m_progressBarBg.setOutlineThickness(1.5f);

    m_progressBarFill.setPosition(Constants::WINDOW_WIDTH - 270.0f, 12.0f);
    m_progressBarFill.setFillColor(Constants::COLOR_ACCENT_GREEN);

    // --- Capsule ---
    m_capsule.setSize(sf::Vector2f(110.0f, 80.0f)); // grande capsule futuriste
    m_capsule.setFillColor(sf::Color(40, 55, 85));
    m_capsule.setOutlineColor(Constants::COLOR_ACCENT_GREEN);
    m_capsule.setOutlineThickness(3.0f);

    m_capsuleWindow.setRadius(12.0f);
    m_capsuleWindow.setFillColor(sf::Color(0, 200, 255, 160));
    m_capsuleWindow.setOutlineColor(sf::Color(0, 220, 255));
    m_capsuleWindow.setOutlineThickness(2.0f);
    m_capsuleWindow.setOrigin(12.0f, 12.0f);

    // --- Etoiles couche 1 (lointaine) ---
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

    // --- Etoiles couche 2 (proche) ---
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

    // --- Panneaux metalliques ---
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
        bool cyan = (Utils::randomInt(0, 1) == 0);
        panel.color = cyan ? sf::Color(30, 45, 80, 100) : sf::Color(0, 80, 120, 80);
        panel.rect.setFillColor(panel.color);
        panel.speed = Utils::randomFloat(80.0f, 150.0f);
        m_bgPanels.push_back(panel);
    }

    resetGame();
}

Game::~Game() {}

// ============================================================
//  Boucle principale
// ============================================================
void Game::run() {
    while (m_window.isOpen()) {
        float deltaTime = m_clock.restart().asSeconds();
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        processEvents();
        update(deltaTime);
        render();
    }
}

// ============================================================
//  processEvents() — Clavier
// ============================================================
void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            m_window.close();

        if (event.type == sf::Event::KeyPressed) {

            if (m_state == GameState::MAIN_MENU) {
                if (event.key.code == sf::Keyboard::Up) {
                    int i = (static_cast<int>(m_selectedButton) - 1 + 3) % 3;
                    m_selectedButton = static_cast<MenuButton>(i);
                }
                if (event.key.code == sf::Keyboard::Down) {
                    int i = (static_cast<int>(m_selectedButton) + 1) % 3;
                    m_selectedButton = static_cast<MenuButton>(i);
                }
                if (event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Space) {
                    if (m_selectedButton == MenuButton::PLAY)  startGame();
                    if (m_selectedButton == MenuButton::ABOUT) m_state = GameState::ABOUT;
                    if (m_selectedButton == MenuButton::QUIT)  m_window.close();
                }
                if (event.key.code == sf::Keyboard::Escape) m_window.close();
            }

            else if (m_state == GameState::ABOUT) {
                if (event.key.code == sf::Keyboard::Escape ||
                    event.key.code == sf::Keyboard::Return  ||
                    event.key.code == sf::Keyboard::Space) {
                    m_state = GameState::MAIN_MENU;
                }
            }

            else if (m_state == GameState::PLAYING) {
                if (event.key.code == sf::Keyboard::Space ||
                    event.key.code == sf::Keyboard::Up) {
                    m_player->jump();
                }
            }

            else if (m_state == GameState::VICTORY   ||
                     m_state == GameState::GAME_OVER  ||
                     m_state == GameState::EXPLOSION) {
                if (event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Space) {
                    resetGame();
                    m_state = GameState::MAIN_MENU;
                }
            }
        }
    }

    // Accroupissement maintenu
    if (m_state == GameState::PLAYING) {
        bool duck = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        m_player->crouch(duck);
    }
}

// ============================================================
//  update()
// ============================================================
void Game::update(float deltaTime) {
    if (m_state == GameState::PLAYING)
        updatePlaying(deltaTime);
    updateShake(deltaTime);
}

// ============================================================
//  updatePlaying() — Toute la logique de jeu
// ============================================================
void Game::updatePlaying(float deltaTime) {

    // --- Victoire figee : rien ne bouge ---
    if (m_gameWon) return;

    // -------------------------------------------------------
    //  1. COMPTE A REBOURS D'EXPLOSION (barre rouge)
    //     Si elle atteint 0 => ecran explosion
    // -------------------------------------------------------
    m_explosionTimer -= deltaTime;
    if (m_explosionTimer <= 0.0f) {
        m_explosionTimer = 0.0f;
        triggerScreenShake(1.0f);
        m_state = GameState::EXPLOSION;
        return;
    }

    // -------------------------------------------------------
    //  2. PROGRESSION VERS LA CAPSULE (barre verte)
    //     Augmente chaque seconde, plafonnee a 100%
    // -------------------------------------------------------
    m_survivalTime += deltaTime;
    float progressRatio = m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;
    if (progressRatio > 1.0f) progressRatio = 1.0f;

    // A 100% : la capsule apparait une seule fois
    if (progressRatio >= 1.0f && !m_capsuleVisible) {
        m_capsuleVisible = true;
        m_capsuleX = static_cast<float>(Constants::WINDOW_WIDTH) + 80.0f;
    }

    // -------------------------------------------------------
    //  3. DIFFICULTE PROGRESSIVE
    //     La vitesse augmente jusqu'au max
    //     L'intervalle de spawn diminue jusqu'au min
    // -------------------------------------------------------
    if (m_scrollSpeed < Constants::SCROLL_SPEED_MAX)
        m_scrollSpeed += Constants::SPEED_INCREMENT * deltaTime;

    // L'intervalle de spawn se reduit avec le temps
    m_spawnInterval -= Constants::SPAWN_REDUCTION * deltaTime;
    if (m_spawnInterval < Constants::SPAWN_INTERVAL_MIN)
        m_spawnInterval = Constants::SPAWN_INTERVAL_MIN;

    // -------------------------------------------------------
    //  4. DECOR
    // -------------------------------------------------------
    updateParallax(deltaTime);

    // -------------------------------------------------------
    //  5. JOUEUR
    // -------------------------------------------------------
    m_player->update(deltaTime);

    // -------------------------------------------------------
    //  6. SPAWN D'OBSTACLES (bloque quand la capsule est visible)
    // -------------------------------------------------------
    if (!m_capsuleVisible) {
        m_spawnTimer += deltaTime;
        if (m_spawnTimer >= m_spawnInterval) {
            m_spawnTimer = 0.0f;
            spawnObstacle();
        }
    }

    // -------------------------------------------------------
    //  7. MISE A JOUR DES OBSTACLES avec la vitesse actuelle
    // -------------------------------------------------------
    for (auto& obs : m_obstacles)
        obs->updateWithSpeed(deltaTime, m_scrollSpeed);

    // -------------------------------------------------------
    //  8. COLLISIONS joueur <-> obstacles
    //     Un contact retire 1 coeur (pas de mort immediate)
    //     Apres MAX_HEALTH collisions : Game Over
    // -------------------------------------------------------
    checkCollisions();

    // Mort par epuisement des coeurs
    if (m_player->isDead()) {
        triggerScreenShake(0.6f);
        m_state = GameState::GAME_OVER;
        return;
    }

    // -------------------------------------------------------
    //  9. NETTOYAGE
    // -------------------------------------------------------
    cleanObstacles();

    // -------------------------------------------------------
    //  10. CAPSULE DE SAUVETAGE
    //
    //  La capsule entre par la droite et avance vers le robot.
    //  capsuleY = GROUND_Y - 60 = 490 - 60 = 430
    //  => Meme niveau que le robot debout (Y=430..490)
    //  => Le robot la touche naturellement en courant
    //
    //  Contact => m_gameWon=true => tout s'arrete immediatement
    //  Message : "Merci de m'avoir sauve !"
    // -------------------------------------------------------
    if (m_capsuleVisible) {
        m_capsuleX -= (m_scrollSpeed * 0.40f) * deltaTime;

        // Capsule posee sur le sol (meme niveau que le robot)
        const float capsuleW = 110.0f;  // largeur de la grande capsule
        const float capsuleH = 80.0f;   // hauteur → capsuleY = 490-80 = 410 = niveau robot
        float capsuleY = Constants::GROUND_Y - capsuleH;  // 490 - 80 = 410

        m_capsule.setPosition(m_capsuleX, capsuleY);
        m_capsuleWindow.setPosition(m_capsuleX + 55.0f, capsuleY + 35.0f);

        // Detection de contact FloatRect
        sf::FloatRect capsuleBounds(m_capsuleX, capsuleY, capsuleW, capsuleH);
        sf::FloatRect playerBounds  = m_player->getBounds();

        if (playerBounds.intersects(capsuleBounds)) {
            // VICTOIRE : tout s'arrete, message affiché
            m_gameWon    = true;
            m_finalScore = m_survivalTime;
            m_state      = GameState::VICTORY;
            m_obstacles.clear();
        }
    }
}

// ============================================================
//  checkCollisions() — Detection de collision avec FloatRect
//
//  PRINCIPE :
//    On recupere la hitbox du robot (getBounds), on reduit
//    legerement les bords GAUCHE et DROIT (tolerance de 4px)
//    pour ne pas punir les tout petits frôlements visuels.
//
//    On ne touche PAS au haut et au bas de la hitbox :
//    les obstacles ont ete calibres en consequence dans
//    Obstacle.cpp pour que le saut et l'accroupissement
//    fonctionnent exactement comme prevu.
//
//    Un contact = 1 coeur perdu + invincibilite temporaire.
//    Apres 3 chocs = Game Over.
// ============================================================
void Game::checkCollisions() {
    // Hitbox du robot : exactement getBounds() avec tolerance laterale
    sf::FloatRect player = m_player->getBounds();
    player.left  += 4.0f;  // tolerance bord gauche
    player.width -= 8.0f;  // tolerance bord droit (4px de chaque cote)
    // Note : on ne touche PAS top/height pour garder la calibration
    //        verticale des obstacles (sol et aerien)

    for (auto& obs : m_obstacles) {
        sf::FloatRect obstacle = obs->getBounds();

        // .intersects() retourne true si les deux rectangles se chevauchent
        if (player.intersects(obstacle)) {
            m_player->takeDamage();   // -1 coeur (ignore si invincible)
            triggerScreenShake(0.35f);
            break; // un seul choc traite par frame
        }
    }
}

// ============================================================
//  spawnObstacle() — 2 types, choix aleatoire
// ============================================================
void Game::spawnObstacle() {
    float spawnX = static_cast<float>(Constants::WINDOW_WIDTH) + 50.0f;
    if (Utils::randomInt(0, 1) == 0)
        m_obstacles.push_back(std::make_unique<MagneticContainer>(spawnX));
    else
        m_obstacles.push_back(std::make_unique<SecurityDrone>(spawnX));
}

// ============================================================
//  cleanObstacles()
// ============================================================
void Game::cleanObstacles() {
    m_obstacles.erase(
        std::remove_if(m_obstacles.begin(), m_obstacles.end(),
            [](const std::unique_ptr<Obstacle>& o) { return o->isOffScreen(); }),
        m_obstacles.end()
    );
}

// ============================================================
//  updateParallax() — Inchange
// ============================================================
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

// ============================================================
//  updateShake()
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
//  render()
// ============================================================
void Game::render() {
    m_window.clear(Constants::COLOR_BACKGROUND);

    sf::View view = m_window.getDefaultView();
    if (m_shakeActive)
        view.setCenter(Constants::WINDOW_WIDTH / 2.0f + m_shakeOffset.x,
                       Constants::WINDOW_HEIGHT / 2.0f + m_shakeOffset.y);
    m_window.setView(view);

    switch (m_state) {
        case GameState::MAIN_MENU:
            drawBackground();
            drawMainMenu();
            break;
        case GameState::ABOUT:
            drawBackground();
            drawAboutScreen();
            break;
        case GameState::PLAYING:
            drawBackground();
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            if (m_capsuleVisible) drawCapsule();
            for (auto& obs : m_obstacles) obs->draw(m_window);
            m_player->draw(m_window);
            drawHUD();
            break;
        case GameState::VICTORY:
            drawBackground();
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            drawCapsule();
            m_player->draw(m_window);
            drawHUD();
            drawVictoryScreen();
            break;
        case GameState::EXPLOSION:
            drawExplosionScreen();
            break;
        case GameState::GAME_OVER:
            drawBackground();
            drawGameOverScreen();
            break;
    }

    m_window.display();
}

// ============================================================
//  drawBackground()
// ============================================================
void Game::drawBackground() {
    for (int layer = 0; layer < 2; ++layer)
        for (const auto& star : m_starLayers[layer].stars)
            m_window.draw(star);
    for (const auto& panel : m_bgPanels)
        m_window.draw(panel.rect);
}

// ============================================================
//  drawHUD() — 2 barres + coeurs
// ============================================================
void Game::drawHUD() {

    // ---- Barre rouge EXPLOSION (haut gauche) ----
    float timeRatio = m_explosionTimer / Constants::EXPLOSION_TIME;
    if (timeRatio < 0.0f) timeRatio = 0.0f;

    m_timerBarFill.setSize(sf::Vector2f(260.0f * timeRatio, 18.0f));

    // Passe de vert->orange->rouge selon urgence
    sf::Color timerColor = Utils::lerpColor(
        Constants::COLOR_ACCENT_RED,
        sf::Color(0, 200, 80),
        timeRatio
    );
    m_timerBarFill.setFillColor(timerColor);

    m_window.draw(m_timerBarBg);
    m_window.draw(m_timerBarFill);

    sf::Text timerLabel = makeText("EXPLOSION", 11, timerColor);
    timerLabel.setPosition(10.0f, 33.0f);
    m_window.draw(timerLabel);

    // Secondes restantes
    std::string secStr = std::to_string(static_cast<int>(m_explosionTimer)) + "s";
    sf::Text secText = makeText(secStr, 13, timerColor);
    secText.setPosition(275.0f, 11.0f);
    m_window.draw(secText);

    // ---- Barre verte CAPSULE (haut droite) ----
    float progressRatio = m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;
    if (progressRatio > 1.0f) progressRatio = 1.0f;

    m_progressBarFill.setSize(sf::Vector2f(260.0f * progressRatio, 18.0f));
    sf::Color progColor = m_capsuleVisible
        ? sf::Color(0, 255, 120)
        : Utils::lerpColor(sf::Color(0, 100, 255), Constants::COLOR_ACCENT_GREEN, progressRatio);
    m_progressBarFill.setFillColor(progColor);

    m_window.draw(m_progressBarBg);
    m_window.draw(m_progressBarFill);

    std::string capLabel = m_capsuleVisible
        ? "CAPSULE EN APPROCHE !"
        : "CAPSULE : " + std::to_string(static_cast<int>(progressRatio * 100)) + "%";
    sf::Text capText = makeText(capLabel, 11, progColor);
    // Aligner a droite du label
    capText.setPosition(
        Constants::WINDOW_WIDTH - 270.0f,
        33.0f
    );
    m_window.draw(capText);

    // ----------------------------------------------------------------
    //  COEURS (sante) — centre en haut, grandes et lisibles
    //
    //  Chaque coeur est dessine avec 5 rectangles/cercles :
    //    - 2 cercles superieurs (les deux bosses du coeur)
    //    - 1 triangle inferieur (la pointe)
    //  Un coeur perdu devient gris fonce (vide).
    //  Espacement : 46px entre chaque icone (bien espace).
    // ----------------------------------------------------------------
    int health = m_player->getHealth();

    // Largeur totale des 3 coeurs = 3 * 36px + 2 * 10px de gap = 128px
    // On centre sur WINDOW_WIDTH/2
    const float heartW    = 36.0f;  // largeur d'un coeur
    const float heartGap  = 10.0f;  // espace entre deux coeurs
    const float heartTotalW = Constants::MAX_HEALTH * heartW + (Constants::MAX_HEALTH - 1) * heartGap;
    float hStartX = Constants::WINDOW_WIDTH / 2.0f - heartTotalW / 2.0f;
    float hY      = 7.0f;  // Y du haut des coeurs

    for (int i = 0; i < Constants::MAX_HEALTH; ++i) {
        bool full = (i < health);

        // Couleur : rouge vif si plein, gris fonce si vide
        sf::Color cFill    = full ? sf::Color(230, 50,  70)  : sf::Color(55, 30, 40);
        sf::Color cOutline = full ? sf::Color(255, 100, 120) : sf::Color(80, 50, 60);

        float hx = hStartX + i * (heartW + heartGap);

        // Bosse gauche du coeur (cercle)
        sf::CircleShape bLeft(9.0f);
        bLeft.setFillColor(cFill);
        bLeft.setOutlineColor(cOutline);
        bLeft.setOutlineThickness(1.5f);
        bLeft.setOrigin(9.0f, 9.0f);
        bLeft.setPosition(hx + 9.0f, hY + 9.0f);
        m_window.draw(bLeft);

        // Bosse droite du coeur (cercle)
        sf::CircleShape bRight(9.0f);
        bRight.setFillColor(cFill);
        bRight.setOutlineColor(cOutline);
        bRight.setOutlineThickness(1.5f);
        bRight.setOrigin(9.0f, 9.0f);
        bRight.setPosition(hx + 27.0f, hY + 9.0f);
        m_window.draw(bRight);

        // Corps central (rectangle qui lie les deux bosses)
        sf::RectangleShape body(sf::Vector2f(heartW, 14.0f));
        body.setFillColor(cFill);
        body.setPosition(hx, hY + 5.0f);
        m_window.draw(body);

        // Pointe inferieure du coeur (triangle)
        sf::ConvexShape tip;
        tip.setPointCount(3);
        tip.setPoint(0, sf::Vector2f(0.0f,     0.0f));
        tip.setPoint(1, sf::Vector2f(heartW,   0.0f));
        tip.setPoint(2, sf::Vector2f(heartW/2.0f, 16.0f));
        tip.setFillColor(cFill);
        tip.setOutlineColor(cOutline);
        tip.setOutlineThickness(1.0f);
        tip.setPosition(hx, hY + 16.0f);
        m_window.draw(tip);
    }

    // ---- Instructions bas d'ecran ----
    sf::Text ctrl = makeText("[ESPACE/HAUT] Sauter    [BAS] Se baisser", 12, sf::Color(80, 110, 160));
    ctrl.setPosition(10.0f, Constants::WINDOW_HEIGHT - 20.0f);
    m_window.draw(ctrl);

    // ---- Hint au-dessus de chaque obstacle proche ----
    // Aide le joueur a savoir quelle touche utiliser
    for (auto& obs : m_obstacles) {
        float ox = obs->getX();
        // Afficher seulement si l obstacle est dans la zone visible a droite
        if (ox > 50.0f && ox < static_cast<float>(Constants::WINDOW_WIDTH)) {
            std::string hint = (obs->getType() == ObstacleType::GROUND)
                ? "SAUTER !" : "SE BAISSER !";
            sf::Color hintColor = (obs->getType() == ObstacleType::GROUND)
                ? sf::Color(0, 220, 255, 200) : sf::Color(255, 160, 0, 200);
            sf::Text hintText = makeText(hint, 13, hintColor);
            hintText.setPosition(ox, 370.0f);
            m_window.draw(hintText);
        }
    }
}

// ============================================================
//  drawCapsule() — Capsule futuriste grande et lumineuse
//
//  La capsule est dessinee entiererement en code SFML :
//    - Corps principal : grand rectangle vert negatif avec
//      cadre cyan epais et effet de lueur pulsante
//    - Hublot : grand cercle cyan avec reflet blanc
//    - Moteurs : deux rectangles oranges en bas
//    - Flammes : cercles qui pulsent selon m_capsuleGlow
//    - Label "CAPSULE" clignotant au-dessus
//
//  La capsule fait 110x80px pour etre bien visible.
// ============================================================
void Game::drawCapsule() {
    float cx = m_capsuleX;
    float cy = m_capsule.getPosition().y;  // Y deja calcule dans updatePlaying

    // Lueur pulsante (timer stocke dans m_capsule via getPosition abuse —
    // on utilise m_shakeTimer comme proxy car il est dispo, sinon on
    // calcule depuis le temps de jeu)
    static float glowT = 0.0f;
    glowT += 0.016f;  // ~60fps
    float glow = (std::sin(glowT * 4.0f) + 1.0f) * 0.5f;
    sf::Uint8 glowAlpha = static_cast<sf::Uint8>(100 + glow * 155);
    sf::Color glowColor(0, static_cast<sf::Uint8>(200 + glow*55), 100, glowAlpha);

    // ---- Halo exterieur (fond lumineux) ----
    sf::RectangleShape halo(sf::Vector2f(120.0f, 90.0f));
    halo.setFillColor(sf::Color(0, 255, 120, static_cast<sf::Uint8>(20 + glow*30)));
    halo.setPosition(cx - 5.0f, cy - 5.0f);
    m_window.draw(halo);

    // ---- Corps principal ----
    sf::RectangleShape body(sf::Vector2f(110.0f, 80.0f));
    body.setFillColor(sf::Color(20, 45, 75));
    body.setOutlineColor(glowColor);
    body.setOutlineThickness(3.5f);
    body.setPosition(cx, cy);
    m_window.draw(body);

    // ---- Bandes laterales (details metalliques) ----
    sf::RectangleShape stripeL(sf::Vector2f(10.0f, 80.0f));
    stripeL.setFillColor(sf::Color(0, static_cast<sf::Uint8>(160 + glow*60), 100, 180));
    stripeL.setPosition(cx + 2.0f, cy);
    m_window.draw(stripeL);

    sf::RectangleShape stripeR(sf::Vector2f(10.0f, 80.0f));
    stripeR.setFillColor(sf::Color(0, static_cast<sf::Uint8>(160 + glow*60), 100, 180));
    stripeR.setPosition(cx + 98.0f, cy);
    m_window.draw(stripeR);

    // ---- Grand hublot central ----
    sf::CircleShape porthole(24.0f);
    porthole.setFillColor(sf::Color(0, static_cast<sf::Uint8>(180 + glow*75), 255, 220));
    porthole.setOutlineColor(sf::Color(0, 240, 255));
    porthole.setOutlineThickness(3.0f);
    porthole.setOrigin(24.0f, 24.0f);
    porthole.setPosition(cx + 55.0f, cy + 35.0f);
    m_window.draw(porthole);

    // Reflet blanc sur le hublot (coin haut-gauche)
    sf::CircleShape reflet(8.0f);
    reflet.setFillColor(sf::Color(255, 255, 255, 120));
    reflet.setOrigin(8.0f, 8.0f);
    reflet.setPosition(cx + 43.0f, cy + 22.0f);
    m_window.draw(reflet);

    // ---- Moteurs en bas ----
    sf::RectangleShape motor1(sf::Vector2f(22.0f, 14.0f));
    motor1.setFillColor(sf::Color(60, 60, 80));
    motor1.setOutlineColor(sf::Color(255, 140, 0));
    motor1.setOutlineThickness(2.0f);
    motor1.setPosition(cx + 15.0f, cy + 80.0f);
    m_window.draw(motor1);

    sf::RectangleShape motor2(sf::Vector2f(22.0f, 14.0f));
    motor2.setFillColor(sf::Color(60, 60, 80));
    motor2.setOutlineColor(sf::Color(255, 140, 0));
    motor2.setOutlineThickness(2.0f);
    motor2.setPosition(cx + 73.0f, cy + 80.0f);
    m_window.draw(motor2);

    // Flammes des moteurs (pulsent)
    sf::Uint8 flameA = static_cast<sf::Uint8>(180 + glow * 75);
    sf::CircleShape flame1(static_cast<float>(6 + glow * 5));
    flame1.setFillColor(sf::Color(255, 120, 0, flameA));
    flame1.setOrigin(flame1.getRadius(), 0.0f);
    flame1.setPosition(cx + 26.0f, cy + 94.0f);
    m_window.draw(flame1);

    sf::CircleShape flame2(static_cast<float>(6 + glow * 5));
    flame2.setFillColor(sf::Color(255, 120, 0, flameA));
    flame2.setOrigin(flame2.getRadius(), 0.0f);
    flame2.setPosition(cx + 84.0f, cy + 94.0f);
    m_window.draw(flame2);

    // ---- Label "CAPSULE DE SAUVETAGE" clignotant ----
    if (static_cast<int>(glowT * 3.0f) % 2 == 0) {
        sf::Text label = makeText("CAPSULE DE SAUVETAGE", 14, glowColor);
        label.setPosition(
            cx + 55.0f - label.getLocalBounds().width / 2.0f,
            cy - 26.0f
        );
        m_window.draw(label);
    }
}

// ============================================================
//  drawRobotPreview() — Robot dessin en formes SFML
// ============================================================
void Game::drawRobotPreview(float cx, float cy, float scale) {
    auto rect = [&](float x, float y, float w, float h,
                    sf::Color fill, sf::Color outline = sf::Color::Transparent) {
        sf::RectangleShape r(sf::Vector2f(w * scale, h * scale));
        r.setFillColor(fill);
        if (outline != sf::Color::Transparent) {
            r.setOutlineColor(outline);
            r.setOutlineThickness(1.5f * scale);
        }
        r.setOrigin(w * scale / 2.0f, h * scale / 2.0f);
        r.setPosition(cx + x * scale, cy + y * scale);
        m_window.draw(r);
    };
    auto circ = [&](float x, float y, float r, sf::Color fill) {
        sf::CircleShape c(r * scale);
        c.setFillColor(fill);
        c.setOrigin(r * scale, r * scale);
        c.setPosition(cx + x * scale, cy + y * scale);
        m_window.draw(c);
    };

    // Antenne
    rect(0, -52, 3, 16, Constants::COLOR_ACCENT_CYAN);
    circ(0, -62, 5, Constants::COLOR_ACCENT_CYAN);

    // Tete
    rect(0, -30, 36, 28, sf::Color(190, 205, 225), sf::Color(80, 110, 160));
    // Visiere
    circ(0, -30, 11, sf::Color(0, 200, 255, 200));

    // Corps
    rect(0, 10, 40, 46, sf::Color(160, 175, 200), sf::Color(80, 110, 160));
    // Detail centre corps
    rect(0, 8, 20, 10, sf::Color(0, 180, 255, 120));

    // Jambe gauche + droite
    rect(-12, 46, 14, 22, sf::Color(130, 145, 175));
    rect( 12, 46, 14, 22, sf::Color(130, 145, 175));

    // Bottes orange (magnetiques)
    rect(-12, 62, 16, 8, Constants::COLOR_ACCENT_ORANGE);
    rect( 12, 62, 16, 8, Constants::COLOR_ACCENT_ORANGE);
}

// ============================================================
//  drawButton() — Bouton menu avec surlignage
// ============================================================
void Game::drawButton(const std::string& label, float y, bool selected) {
    float bw = 280.0f, bh = 46.0f;
    float bx = Constants::WINDOW_WIDTH / 2.0f - bw / 2.0f;

    sf::RectangleShape btn(sf::Vector2f(bw, bh));
    btn.setPosition(bx, y);
    btn.setFillColor(selected ? sf::Color(0, 70, 120) : sf::Color(15, 25, 50));
    btn.setOutlineColor(selected ? Constants::COLOR_ACCENT_CYAN : sf::Color(50, 80, 120));
    btn.setOutlineThickness(selected ? 2.5f : 1.5f);
    m_window.draw(btn);

    sf::Color tc = selected ? Constants::COLOR_ACCENT_CYAN : sf::Color(140, 170, 210);
    sf::Text txt = makeText(label, selected ? 22u : 19u, tc);
    txt.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - txt.getLocalBounds().width / 2.0f,
        y + bh / 2.0f - txt.getLocalBounds().height / 2.0f - 3.0f
    );
    m_window.draw(txt);
}

// ============================================================
//  drawMainMenu()
// ============================================================
void Game::drawMainMenu() {
    // Titre
    sf::Text title = makeText("STATION EVAC", 50, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 28.0f);
    m_window.draw(title);

    sf::Text sub = makeText("HULL BREACH", 24, Constants::COLOR_ACCENT_ORANGE);
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 88.0f);
    m_window.draw(sub);

    sf::RectangleShape line(sf::Vector2f(380.0f, 2.0f));
    line.setFillColor(sf::Color(0, 100, 150));
    line.setPosition(Constants::WINDOW_WIDTH / 2.0f - 190.0f, 122.0f);
    m_window.draw(line);

    // Robot a gauche
    drawRobotPreview(175.0f, 330.0f, 1.35f);
    sf::Text robotName = makeText("R0-B0T", 16, Constants::COLOR_ACCENT_CYAN);
    robotName.setPosition(175.0f - robotName.getLocalBounds().width / 2.0f, 420.0f);
    m_window.draw(robotName);

    // 3 boutons a droite
    float startY = 255.0f, gap = 62.0f;
    drawButton("[ JOUER ]",    startY,        m_selectedButton == MenuButton::PLAY);
    drawButton("[ A PROPOS ]", startY + gap,  m_selectedButton == MenuButton::ABOUT);
    drawButton("[ QUITTER ]",  startY + gap*2, m_selectedButton == MenuButton::QUIT);

    sf::Text nav = makeText("[HAUT/BAS] Naviguer    [ENTREE] Confirmer", 12, sf::Color(70, 100, 140));
    nav.setPosition(Constants::WINDOW_WIDTH / 2.0f - nav.getLocalBounds().width / 2.0f,
                    Constants::WINDOW_HEIGHT - 26.0f);
    m_window.draw(nav);
}

// ============================================================
//  drawAboutScreen() — Histoire + controles
// ============================================================
void Game::drawAboutScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 185));
    m_window.draw(overlay);

    sf::Text title = makeText("A PROPOS", 36, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 40.0f);
    m_window.draw(title);

    // Histoire
    sf::Text h1 = makeText("La station spatiale Omega-7 est en train d'exploser !", 19, sf::Color(255, 200, 100));
    h1.setPosition(Constants::WINDOW_WIDTH / 2.0f - h1.getLocalBounds().width / 2.0f, 100.0f);
    m_window.draw(h1);

    sf::Text h2 = makeText("Aidez le robot a atteindre la capsule de sauvetage", 17, Constants::COLOR_HUD_TEXT);
    h2.setPosition(Constants::WINDOW_WIDTH / 2.0f - h2.getLocalBounds().width / 2.0f, 132.0f);
    m_window.draw(h2);

    sf::Text h3 = makeText("avant l'explosion de la station !", 17, Constants::COLOR_HUD_TEXT);
    h3.setPosition(Constants::WINDOW_WIDTH / 2.0f - h3.getLocalBounds().width / 2.0f, 156.0f);
    m_window.draw(h3);

    sf::RectangleShape sep(sf::Vector2f(500.0f, 1.0f));
    sep.setFillColor(sf::Color(50, 80, 120));
    sep.setPosition(Constants::WINDOW_WIDTH / 2.0f - 250.0f, 195.0f);
    m_window.draw(sep);

    // Controles
    sf::Text ctrlTitle = makeText("CONTROLES", 19, Constants::COLOR_ACCENT_ORANGE);
    ctrlTitle.setPosition(Constants::WINDOW_WIDTH / 2.0f - ctrlTitle.getLocalBounds().width / 2.0f, 208.0f);
    m_window.draw(ctrlTitle);

    struct Line { const char* key; const char* desc; };
    Line lines[] = {
        {"[ESPACE] ou [HAUT]", "Sauter par-dessus un obstacle au sol"},
        {"[BAS] ou [S]",       "Se baisser sous un obstacle aerien"},
        {"[HAUT] / [BAS]",     "Naviguer dans le menu"},
        {"[ENTREE]",           "Valider / Rejouer"},
    };
    float y = 248.0f;
    for (auto& l : lines) {
        sf::Text k = makeText(l.key, 15, Constants::COLOR_ACCENT_CYAN);
        k.setPosition(140.0f, y);
        m_window.draw(k);
        sf::Text d = makeText(l.desc, 15, Constants::COLOR_HUD_TEXT);
        d.setPosition(370.0f, y);
        m_window.draw(d);
        y += 32.0f;
    }

    // Sante
    sf::Text healthInfo = makeText("Le robot a 3 coeurs. Chaque collision en retire un.", 15, sf::Color(200, 180, 220));
    healthInfo.setPosition(Constants::WINDOW_WIDTH / 2.0f - healthInfo.getLocalBounds().width / 2.0f, 380.0f);
    m_window.draw(healthInfo);

    sf::Text healthInfo2 = makeText("A zero : Game Over. Survivez 60s pour faire apparaitre la capsule !", 14, sf::Color(170, 170, 200));
    healthInfo2.setPosition(Constants::WINDOW_WIDTH / 2.0f - healthInfo2.getLocalBounds().width / 2.0f, 404.0f);
    m_window.draw(healthInfo2);

    drawRobotPreview(Constants::WINDOW_WIDTH / 2.0f, 470.0f, 0.95f);

    sf::Text back = makeText("[ ECHAP ou ENTREE ] Retour", 15, sf::Color(80, 120, 160));
    back.setPosition(Constants::WINDOW_WIDTH / 2.0f - back.getLocalBounds().width / 2.0f,
                     Constants::WINDOW_HEIGHT - 28.0f);
    m_window.draw(back);
}

// ============================================================
//  drawVictoryScreen() — Overlay "Merci de m'avoir sauve !"
// ============================================================
void Game::drawVictoryScreen() {
    // Fond vert sombre semi-transparent (different du Game Over noir)
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 20, 10, 165));
    m_window.draw(overlay);

    // Ligne decorative haut
    sf::RectangleShape topLine(sf::Vector2f(Constants::WINDOW_WIDTH, 4.0f));
    topLine.setFillColor(Constants::COLOR_ACCENT_GREEN);
    topLine.setPosition(0.0f, 100.0f);
    m_window.draw(topLine);

    // Titre principal — grand et vert
    sf::Text title = makeText("MISSION REUSSIE !", 50, Constants::COLOR_ACCENT_GREEN);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 115.0f);
    m_window.draw(title);

    // Message du robot
    sf::Text thanks = makeText("Merci de m'avoir sauve !", 32, sf::Color(255, 230, 80));
    thanks.setPosition(Constants::WINDOW_WIDTH / 2.0f - thanks.getLocalBounds().width / 2.0f, 185.0f);
    m_window.draw(thanks);

    sf::Text sub = makeText("Le robot est en securite dans la capsule.", 19, Constants::COLOR_HUD_TEXT);
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 235.0f);
    m_window.draw(sub);

    // Ligne separatrice
    sf::RectangleShape sep(sf::Vector2f(400.0f, 2.0f));
    sep.setFillColor(sf::Color(0, 150, 80));
    sep.setPosition(Constants::WINDOW_WIDTH / 2.0f - 200.0f, 272.0f);
    m_window.draw(sep);

    // Score
    std::string sc = "Temps de survie : " + Utils::formatTime(m_finalScore);
    sf::Text score = makeText(sc, 24, Constants::COLOR_ACCENT_CYAN);
    score.setPosition(Constants::WINDOW_WIDTH / 2.0f - score.getLocalBounds().width / 2.0f, 285.0f);
    m_window.draw(score);

    // Ligne decorative bas
    sf::RectangleShape botLine(sf::Vector2f(Constants::WINDOW_WIDTH, 4.0f));
    botLine.setFillColor(Constants::COLOR_ACCENT_GREEN);
    botLine.setPosition(0.0f, 370.0f);
    m_window.draw(botLine);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(100, 200, 120));
    replay.setPosition(Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 385.0f);
    m_window.draw(replay);
}

// ============================================================
//  drawExplosionScreen() — Ecran rouge, temps ecoule
// ============================================================
void Game::drawExplosionScreen() {
    // Fond rouge intense pour simuler l'explosion
    m_window.clear(sf::Color(80, 5, 5));

    // Lignes rouges chaotiques en arriere-plan
    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape streak(sf::Vector2f(
            Utils::randomFloat(100.0f, 500.0f), Utils::randomFloat(2.0f, 12.0f)
        ));
        streak.setFillColor(sf::Color(255, Utils::randomInt(50, 120), 0, 180));
        streak.setPosition(
            Utils::randomFloat(0.0f, Constants::WINDOW_WIDTH),
            Utils::randomFloat(0.0f, Constants::WINDOW_HEIGHT)
        );
        m_window.draw(streak);
    }

    sf::Text boom = makeText("KABOOM !", 70, sf::Color(255, 180, 0));
    boom.setPosition(Constants::WINDOW_WIDTH / 2.0f - boom.getLocalBounds().width / 2.0f, 100.0f);
    m_window.draw(boom);

    sf::Text msg = makeText("Robot detruit dans l'explosion.", 26, sf::Color(255, 220, 180));
    msg.setPosition(Constants::WINDOW_WIDTH / 2.0f - msg.getLocalBounds().width / 2.0f, 210.0f);
    m_window.draw(msg);

    sf::Text sub = makeText("La station spatiale n'existe plus.", 18, sf::Color(200, 150, 150));
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 258.0f);
    m_window.draw(sub);

    // Progression atteinte avant la fin
    float pct = (m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE) * 100.0f;
    if (pct > 100.0f) pct = 100.0f;
    std::string ps = "Progression : " + std::to_string(static_cast<int>(pct)) + "%  -  Il manquait "
                   + std::to_string(static_cast<int>(Constants::SURVIVAL_TIME_FOR_CAPSULE - m_survivalTime) + 1)
                   + " secondes...";
    sf::Text prog = makeText(ps, 17, sf::Color(220, 180, 100));
    prog.setPosition(Constants::WINDOW_WIDTH / 2.0f - prog.getLocalBounds().width / 2.0f, 310.0f);
    m_window.draw(prog);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(200, 100, 80));
    replay.setPosition(Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 400.0f);
    m_window.draw(replay);
}

// ============================================================
//  drawGameOverScreen() — Sante a zero
// ============================================================
void Game::drawGameOverScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    m_window.draw(overlay);

    sf::Text title = makeText("GAME OVER", 54, Constants::COLOR_ACCENT_RED);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 140.0f);
    m_window.draw(title);

    sf::Text sub = makeText("Le robot a subi trop de collisions.", 20, sf::Color(200, 150, 150));
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 215.0f);
    m_window.draw(sub);

    float pct = (m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE) * 100.0f;
    if (pct > 100.0f) pct = 100.0f;
    std::string ps = "Progression atteinte : " + std::to_string(static_cast<int>(pct)) + "%";
    sf::Text prog = makeText(ps, 20, sf::Color(180, 180, 220));
    prog.setPosition(Constants::WINDOW_WIDTH / 2.0f - prog.getLocalBounds().width / 2.0f, 270.0f);
    m_window.draw(prog);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(180, 100, 100));
    replay.setPosition(Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 370.0f);
    m_window.draw(replay);
}

// ============================================================
//  Utilitaires
// ============================================================
void Game::startGame() {
    resetGame();
    m_state = GameState::PLAYING;
}

void Game::resetGame() {
    m_explosionTimer  = Constants::EXPLOSION_TIME;
    m_survivalTime    = 0.0f;
    m_scrollSpeed     = Constants::SCROLL_SPEED_BASE;
    m_spawnInterval   = Constants::SPAWN_INTERVAL_START;
    m_spawnTimer      = 0.0f;
    m_gameWon         = false;
    m_finalScore      = 0.0f;
    m_shakeActive     = false;
    m_shakeTimer      = 0.0f;
    m_capsuleVisible  = false;
    m_capsuleX        = static_cast<float>(Constants::WINDOW_WIDTH) + 100.0f;
    m_obstacles.clear();
    m_player = std::make_unique<Player>(Constants::PLAYER_START_X, Constants::PLAYER_START_Y);
}

void Game::triggerScreenShake(float duration) {
    m_shakeActive = true;
    m_shakeTimer  = duration;
}

bool Game::loadFont() {
    const std::vector<std::string> paths = {
        "assets/font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Courier.ttc",
        "C:/Windows/Fonts/cour.ttf"
    };
    for (const auto& p : paths)
        if (m_font.loadFromFile(p)) return true;
    return false;
}

sf::Text Game::makeText(const std::string& str, unsigned int size, sf::Color color) {
    sf::Text t;
    t.setFont(m_font);
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(color);
    return t;
}
