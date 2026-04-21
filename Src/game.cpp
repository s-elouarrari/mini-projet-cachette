#include "../include/Game.hpp"
#include "../include/Utils.hpp"
#include <cmath>
#include <algorithm>

// ============================================================
//  Game.cpp — Station Evac : Hull Breach
//
//  FLUX DES ETATS :
//    MAIN_MENU ──[JOUER]──► READY (robot a l'arret, attend ESPACE)
//               ──[A PROPOS]──► ABOUT
//               ──[QUITTER]──► ferme
//    READY     ──[ESPACE]──► PLAYING (musique demarre)
//    PLAYING   ──► VICTORY / GAME_OVER / EXPLOSION
//    Fin       ──[ENTREE]──► MAIN_MENU
//
//  AUDIO : les sons sont charges depuis assets/
//    Si les fichiers sont absents, le jeu tourne sans son.
// ============================================================

// ──────────────────────────────────────────────────────────────
//  Constructeur
// ──────────────────────────────────────────────────────────────
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
      m_capsuleX(static_cast<float>(Constants::WINDOW_WIDTH) + 100.0f),
      m_capsuleGlowTimer(0.0f),
      m_moonX(620.0f),
      m_moonY(80.0f),
      m_audioLoaded(false)
{
    m_window.setFramerateLimit(Constants::TARGET_FPS);
    loadFont();
    loadAudio();

    // ── Sol ──────────────────────────────────────────────────
    m_floorRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 60.0f));
    m_floorRect.setPosition(0.0f, Constants::GROUND_Y);
    m_floorRect.setFillColor(sf::Color(22, 32, 62));
    m_floorRect.setOutlineColor(sf::Color(0, 180, 255, 100));
    m_floorRect.setOutlineThickness(2.0f);

    // ── Plafond ───────────────────────────────────────────────
    m_ceilingRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 22.0f));
    m_ceilingRect.setPosition(0.0f, 258.0f);
    m_ceilingRect.setFillColor(sf::Color(18, 26, 52));
    m_ceilingRect.setOutlineColor(sf::Color(0, 160, 220, 80));
    m_ceilingRect.setOutlineThickness(1.5f);

    // ── Barre rouge (explosion) ───────────────────────────────
    m_timerBarBg.setSize(sf::Vector2f(260.0f, 18.0f));
    m_timerBarBg.setPosition(10.0f, 12.0f);
    m_timerBarBg.setFillColor(sf::Color(30, 10, 10));
    m_timerBarBg.setOutlineColor(sf::Color(150, 40, 40));
    m_timerBarBg.setOutlineThickness(1.5f);
    m_timerBarFill.setPosition(10.0f, 12.0f);
    m_timerBarFill.setFillColor(sf::Color(220, 40, 40));

    // ── Barre verte (capsule) ─────────────────────────────────
    m_progressBarBg.setSize(sf::Vector2f(260.0f, 18.0f));
    m_progressBarBg.setPosition(Constants::WINDOW_WIDTH - 270.0f, 12.0f);
    m_progressBarBg.setFillColor(sf::Color(10, 30, 10));
    m_progressBarBg.setOutlineColor(sf::Color(40, 150, 40));
    m_progressBarBg.setOutlineThickness(1.5f);
    m_progressBarFill.setPosition(Constants::WINDOW_WIDTH - 270.0f, 12.0f);
    m_progressBarFill.setFillColor(Constants::COLOR_ACCENT_GREEN);

    // ── Capsule ───────────────────────────────────────────────
    m_capsule.setSize(sf::Vector2f(110.0f, 80.0f));
    m_capsule.setFillColor(sf::Color(40, 55, 85));
    m_capsule.setOutlineColor(Constants::COLOR_ACCENT_GREEN);
    m_capsule.setOutlineThickness(3.0f);
    m_capsuleWindow.setRadius(12.0f);
    m_capsuleWindow.setFillColor(sf::Color(0, 200, 255, 160));
    m_capsuleWindow.setOutlineColor(sf::Color(0, 220, 255));
    m_capsuleWindow.setOutlineThickness(2.0f);
    m_capsuleWindow.setOrigin(12.0f, 12.0f);

    // ── Lune ──────────────────────────────────────────────────
    m_moon.setRadius(55.0f);
    m_moon.setFillColor(sf::Color(190, 195, 210));
    m_moon.setOutlineColor(sf::Color(150, 155, 175));
    m_moon.setOutlineThickness(2.0f);
    m_moon.setOrigin(55.0f, 55.0f);

    // Crateres : positions relatives au centre de la lune
    struct CraterDef { float dx, dy, r; };
    CraterDef defs[] = {
        {-20.0f, -15.0f, 9.0f},
        { 18.0f,  10.0f, 12.0f},
        { -5.0f,  22.0f, 6.0f},
        { 28.0f, -20.0f, 5.0f},
        {-30.0f,  18.0f, 7.0f},
        { 10.0f,  -8.0f, 4.0f},
    };
    for (auto& d : defs) {
        Crater c;
        c.offset = sf::Vector2f(d.dx, d.dy);
        c.radius = d.r;
        m_moonCraters.push_back(c);
    }

    // ── Etoiles couche 1 (lointaines) ────────────────────────
    m_starLayers[0].speed = 15.0f;
    for (int i = 0; i < 80; ++i) {
        sf::CircleShape s;
        s.setRadius(Utils::randomFloat(0.5f, 1.5f));
        s.setFillColor(sf::Color(200, 215, 255,
            static_cast<sf::Uint8>(Utils::randomInt(80, 160))));
        s.setPosition(Utils::randomFloat(0, Constants::WINDOW_WIDTH),
                      Utils::randomFloat(0, Constants::GROUND_Y));
        m_starLayers[0].stars.push_back(s);
    }

    // ── Etoiles couche 2 (proches) ───────────────────────────
    m_starLayers[1].speed = 50.0f;
    for (int i = 0; i < 40; ++i) {
        sf::CircleShape s;
        s.setRadius(Utils::randomFloat(1.0f, 2.5f));
        s.setFillColor(sf::Color(220, 230, 255,
            static_cast<sf::Uint8>(Utils::randomInt(140, 220))));
        s.setPosition(Utils::randomFloat(0, Constants::WINDOW_WIDTH),
                      Utils::randomFloat(0, Constants::GROUND_Y));
        m_starLayers[1].stars.push_back(s);
    }

    // ── Panneaux metalliques ──────────────────────────────────
    for (int i = 0; i < 14; ++i) {
        PanelStrip p;
        p.rect.setSize(sf::Vector2f(Utils::randomFloat(30.0f, 85.0f),
                                    Utils::randomFloat(7.0f, 18.0f)));
        p.rect.setPosition(Utils::randomFloat(0, Constants::WINDOW_WIDTH),
                           Utils::randomFloat(270.0f, Constants::GROUND_Y - 10.0f));
        bool cyan = Utils::randomInt(0, 1) == 0;
        p.color = cyan ? sf::Color(28, 42, 78, 110) : sf::Color(0, 75, 115, 90);
        p.rect.setFillColor(p.color);
        p.speed = Utils::randomFloat(70.0f, 140.0f);
        m_bgPanels.push_back(p);
    }

    resetGame();
}

Game::~Game() {}

// ──────────────────────────────────────────────────────────────
//  Boucle principale
// ──────────────────────────────────────────────────────────────
void Game::run() {
    while (m_window.isOpen()) {
        float dt = m_clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;
        processEvents();
        update(dt);
        render();
    }
}

// ──────────────────────────────────────────────────────────────
//  processEvents()
// ──────────────────────────────────────────────────────────────
void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            m_window.close();

        if (event.type == sf::Event::KeyPressed) {

            // ── Menu principal ────────────────────────────────
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
                    if (m_selectedButton == MenuButton::PLAY) {
                        startGame();  // va dans READY (pas PLAYING)
                    } else if (m_selectedButton == MenuButton::ABOUT) {
                        m_state = GameState::ABOUT;
                    } else {
                        m_window.close();
                    }
                }
                if (event.key.code == sf::Keyboard::Escape)
                    m_window.close();
            }

            // ── A propos ──────────────────────────────────────
            else if (m_state == GameState::ABOUT) {
                if (event.key.code == sf::Keyboard::Escape ||
                    event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Space) {
                    m_state = GameState::MAIN_MENU;
                }
            }

            // ── READY : robot a l'arret, attend le signal ─────
            else if (m_state == GameState::READY) {
                if (event.key.code == sf::Keyboard::Space ||
                    event.key.code == sf::Keyboard::Up    ||
                    event.key.code == sf::Keyboard::Return) {
                    // Le joueur donne le depart : la course commence
                    m_state = GameState::PLAYING;
                    m_player->startRunning();
                    // Demarrer la musique
                    if (m_audioLoaded)
                        m_music.play();
                }
                if (event.key.code == sf::Keyboard::Escape) {
                    resetGame();
                    m_state = GameState::MAIN_MENU;
                }
            }

            // ── En jeu ────────────────────────────────────────
            else if (m_state == GameState::PLAYING) {
                if (event.key.code == sf::Keyboard::Space ||
                    event.key.code == sf::Keyboard::Up) {
                    m_player->jump();
                    if (m_audioLoaded) m_sndJump.play();
                }
            }

            // ── Fins de partie ────────────────────────────────
            else if (m_state == GameState::VICTORY  ||
                     m_state == GameState::GAME_OVER ||
                     m_state == GameState::EXPLOSION) {
                if (event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Space) {
                    resetGame();
                    m_state = GameState::MAIN_MENU;
                }
            }
        }
    }

    // ── Accroupissement continu (touche BAS maintenue) ────────
    if (m_state == GameState::PLAYING) {
        bool duck = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::S);

        bool wasCrouching = (m_player->getAction() == PlayerAction::CROUCHING);
        m_player->crouch(duck);
        bool isCrouching  = (m_player->getAction() == PlayerAction::CROUCHING);

        // Son duck : joue une seule fois au moment ou on s'accroupit
        if (!wasCrouching && isCrouching && m_audioLoaded)
            m_sndDuck.play();
    }
}

// ──────────────────────────────────────────────────────────────
//  update()
// ──────────────────────────────────────────────────────────────
void Game::update(float dt) {
    // Le joueur est mis a jour dans tous les etats visuels
    if (m_state == GameState::READY || m_state == GameState::PLAYING)
        m_player->update(dt);

    if (m_state == GameState::PLAYING)
        updatePlaying(dt);

    updateShake(dt);
    m_capsuleGlowTimer += dt;
}

// ──────────────────────────────────────────────────────────────
//  updatePlaying()
// ──────────────────────────────────────────────────────────────
void Game::updatePlaying(float dt) {
    if (m_gameWon) return;

    // 1. Compte a rebours explosion
    m_explosionTimer -= dt;
    if (m_explosionTimer <= 0.0f) {
        m_explosionTimer = 0.0f;
        triggerScreenShake(1.0f);
        if (m_audioLoaded) m_music.stop();
        m_state = GameState::EXPLOSION;
        return;
    }

    // 2. Progression capsule
    m_survivalTime += dt;
    float progressRatio = m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;
    if (progressRatio > 1.0f) progressRatio = 1.0f;
    if (progressRatio >= 1.0f && !m_capsuleVisible) {
        m_capsuleVisible = true;
        m_capsuleX = static_cast<float>(Constants::WINDOW_WIDTH) + 80.0f;
    }

    // 3. Difficulte progressive
    if (m_scrollSpeed < Constants::SCROLL_SPEED_MAX)
        m_scrollSpeed += Constants::SPEED_INCREMENT * dt;
    m_spawnInterval -= Constants::SPAWN_REDUCTION * dt;
    if (m_spawnInterval < Constants::SPAWN_INTERVAL_MIN)
        m_spawnInterval = Constants::SPAWN_INTERVAL_MIN;

    // 4. Decor
    updateParallax(dt);
    updateMoon(dt);

    // 5. Spawn obstacles
    if (!m_capsuleVisible) {
        m_spawnTimer += dt;
        if (m_spawnTimer >= m_spawnInterval) {
            m_spawnTimer = 0.0f;
            spawnObstacle();
        }
    }

    // 6. Mise a jour obstacles
    for (auto& obs : m_obstacles)
        obs->updateWithSpeed(dt, m_scrollSpeed);

    // 7. Collisions
    checkCollisions();
    if (m_player->isDead()) {
        triggerScreenShake(0.7f);
        if (m_audioLoaded) m_music.stop();
        m_state = GameState::GAME_OVER;
        return;
    }

    // 8. Nettoyage
    cleanObstacles();

    // 9. Capsule
    if (m_capsuleVisible) {
        m_capsuleX -= m_scrollSpeed * 0.40f * dt;
        const float capsuleW = 110.0f;
        const float capsuleH = 80.0f;
        float capsuleY = Constants::GROUND_Y - capsuleH;  // 410

        m_capsule.setPosition(m_capsuleX, capsuleY);
        m_capsuleWindow.setPosition(m_capsuleX + 55.0f, capsuleY + 35.0f);

        sf::FloatRect cb(m_capsuleX, capsuleY, capsuleW, capsuleH);
        sf::FloatRect pb = m_player->getBounds();

        if (pb.intersects(cb)) {
            m_gameWon    = true;
            m_finalScore = m_survivalTime;
            m_state      = GameState::VICTORY;
            m_obstacles.clear();
            if (m_audioLoaded) {
                m_music.stop();
                m_sndWin.play();
            }
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  checkCollisions()
// ──────────────────────────────────────────────────────────────
void Game::checkCollisions() {
    sf::FloatRect player = m_player->getBounds();
    player.left  += 4.0f;
    player.width -= 8.0f;

    for (auto& obs : m_obstacles) {
        if (player.intersects(obs->getBounds())) {
            m_player->takeDamage();
            if (m_audioLoaded) m_sndHit.play();
            triggerScreenShake(0.35f);
            break;
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  spawnObstacle()
// ──────────────────────────────────────────────────────────────
void Game::spawnObstacle() {
    float sx = static_cast<float>(Constants::WINDOW_WIDTH) + 50.0f;
    if (Utils::randomInt(0, 1) == 0)
        m_obstacles.push_back(std::make_unique<MagneticContainer>(sx));
    else
        m_obstacles.push_back(std::make_unique<SecurityDrone>(sx));
}

void Game::cleanObstacles() {
    m_obstacles.erase(
        std::remove_if(m_obstacles.begin(), m_obstacles.end(),
            [](const std::unique_ptr<Obstacle>& o){ return o->isOffScreen(); }),
        m_obstacles.end());
}

// ──────────────────────────────────────────────────────────────
//  updateParallax() — etoiles + panneaux
// ──────────────────────────────────────────────────────────────
void Game::updateParallax(float dt) {
    for (int layer = 0; layer < 2; ++layer) {
        for (auto& star : m_starLayers[layer].stars) {
            sf::Vector2f pos = star.getPosition();
            pos.x -= m_starLayers[layer].speed * dt;
            if (pos.x < -4.0f) {
                pos.x = static_cast<float>(Constants::WINDOW_WIDTH) + 2.0f;
                pos.y = Utils::randomFloat(0.0f, static_cast<float>(Constants::GROUND_Y));
            }
            star.setPosition(pos);
        }
    }
    for (auto& p : m_bgPanels) {
        sf::Vector2f pos = p.rect.getPosition();
        pos.x -= p.speed * dt;
        if (pos.x < -100.0f) {
            pos.x = static_cast<float>(Constants::WINDOW_WIDTH) + 10.0f;
            pos.y = Utils::randomFloat(270.0f, Constants::GROUND_Y - 10.0f);
        }
        p.rect.setPosition(pos);
    }
}

// ──────────────────────────────────────────────────────────────
//  updateMoon() — lune se deplace tres lentement (parallaxe)
// ──────────────────────────────────────────────────────────────
void Game::updateMoon(float dt) {
    // Vitesse tres lente : 4px/s seulement
    m_moonX -= 4.0f * dt;
    // Quand la lune sort a gauche, elle revient a droite
    if (m_moonX < -80.0f)
        m_moonX = static_cast<float>(Constants::WINDOW_WIDTH) + 80.0f;
}

// ──────────────────────────────────────────────────────────────
//  updateShake()
// ──────────────────────────────────────────────────────────────
void Game::updateShake(float dt) {
    if (!m_shakeActive) return;
    m_shakeTimer -= dt;
    if (m_shakeTimer <= 0.0f) {
        m_shakeActive = false;
        m_shakeOffset = sf::Vector2f(0.0f, 0.0f);
        return;
    }
    float i = m_shakeTimer * 8.0f;
    m_shakeOffset = sf::Vector2f(Utils::randomFloat(-i, i), Utils::randomFloat(-i, i));
}

// ──────────────────────────────────────────────────────────────
//  render()
// ──────────────────────────────────────────────────────────────
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
        case GameState::READY:
            drawBackground();
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            m_player->draw(m_window);
            drawReadyScreen();
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

// ──────────────────────────────────────────────────────────────
//  drawBackground() — etoiles + panneaux + lune
// ──────────────────────────────────────────────────────────────
void Game::drawBackground() {
    // Etoiles couche 1 (lointaines)
    for (const auto& s : m_starLayers[0].stars) m_window.draw(s);
    // Lune
    drawMoon();
    // Etoiles couche 2 (proches, devant la lune)
    for (const auto& s : m_starLayers[1].stars) m_window.draw(s);
    // Panneaux metalliques
    for (const auto& p : m_bgPanels) m_window.draw(p.rect);
}

// ──────────────────────────────────────────────────────────────
//  drawMoon() — grande lune grise avec crateres
// ──────────────────────────────────────────────────────────────
void Game::drawMoon() {
    m_moon.setPosition(m_moonX, m_moonY);
    m_window.draw(m_moon);

    // Crateres : cercles plus fonces
    for (const auto& c : m_moonCraters) {
        sf::CircleShape crater(c.radius);
        crater.setFillColor(sf::Color(155, 158, 175));
        crater.setOutlineColor(sf::Color(130, 134, 155));
        crater.setOutlineThickness(1.0f);
        crater.setOrigin(c.radius, c.radius);
        crater.setPosition(m_moonX + c.offset.x, m_moonY + c.offset.y);
        m_window.draw(crater);
    }

    // Petit reflet blanc en haut a gauche de la lune
    sf::CircleShape shine(14.0f);
    shine.setFillColor(sf::Color(230, 235, 245, 60));
    shine.setOrigin(14.0f, 14.0f);
    shine.setPosition(m_moonX - 28.0f, m_moonY - 28.0f);
    m_window.draw(shine);
}

// ──────────────────────────────────────────────────────────────
//  drawHUD() — barres + coeurs
// ──────────────────────────────────────────────────────────────
void Game::drawHUD() {
    // Barre rouge (explosion)
    float tRatio = m_explosionTimer / Constants::EXPLOSION_TIME;
    if (tRatio < 0.0f) tRatio = 0.0f;
    m_timerBarFill.setSize(sf::Vector2f(260.0f * tRatio, 18.0f));
    sf::Color tc = Utils::lerpColor(Constants::COLOR_ACCENT_RED, sf::Color(0, 200, 80), tRatio);
    m_timerBarFill.setFillColor(tc);
    m_window.draw(m_timerBarBg);
    m_window.draw(m_timerBarFill);

    sf::Text tlabel = makeText("EXPLOSION", 11, tc);
    tlabel.setPosition(10.0f, 33.0f);
    m_window.draw(tlabel);

    std::string secStr = std::to_string(static_cast<int>(m_explosionTimer)) + "s";
    sf::Text secT = makeText(secStr, 13, tc);
    secT.setPosition(275.0f, 11.0f);
    m_window.draw(secT);

    // Barre verte (capsule)
    float pRatio = m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;
    if (pRatio > 1.0f) pRatio = 1.0f;
    m_progressBarFill.setSize(sf::Vector2f(260.0f * pRatio, 18.0f));
    sf::Color pc = m_capsuleVisible
        ? sf::Color(0, 255, 120)
        : Utils::lerpColor(sf::Color(0, 100, 255), Constants::COLOR_ACCENT_GREEN, pRatio);
    m_progressBarFill.setFillColor(pc);
    m_window.draw(m_progressBarBg);
    m_window.draw(m_progressBarFill);

    std::string capStr = m_capsuleVisible
        ? "CAPSULE EN APPROCHE !"
        : "CAPSULE : " + std::to_string(static_cast<int>(pRatio * 100)) + "%";
    sf::Text capT = makeText(capStr, 11, pc);
    capT.setPosition(Constants::WINDOW_WIDTH - 270.0f, 33.0f);
    m_window.draw(capT);

    // ── Coeurs (sante) — grands, bien espaces ─────────────────
    int health = m_player->getHealth();
    const float heartW   = 36.0f;
    const float heartGap = 12.0f;
    const float totalW   = Constants::MAX_HEALTH * heartW + (Constants::MAX_HEALTH - 1) * heartGap;
    float hx0 = Constants::WINDOW_WIDTH / 2.0f - totalW / 2.0f;
    float hy   = 7.0f;

    for (int i = 0; i < Constants::MAX_HEALTH; ++i) {
        bool full = (i < health);
        sf::Color cf = full ? sf::Color(230, 50, 70)  : sf::Color(55, 30, 40);
        sf::Color co = full ? sf::Color(255, 100, 120) : sf::Color(80, 50, 60);
        float hx = hx0 + i * (heartW + heartGap);

        sf::CircleShape bL(9.0f);
        bL.setFillColor(cf); bL.setOutlineColor(co); bL.setOutlineThickness(1.5f);
        bL.setOrigin(9.0f, 9.0f); bL.setPosition(hx + 9.0f, hy + 9.0f);
        m_window.draw(bL);

        sf::CircleShape bR(9.0f);
        bR.setFillColor(cf); bR.setOutlineColor(co); bR.setOutlineThickness(1.5f);
        bR.setOrigin(9.0f, 9.0f); bR.setPosition(hx + 27.0f, hy + 9.0f);
        m_window.draw(bR);

        sf::RectangleShape body(sf::Vector2f(heartW, 14.0f));
        body.setFillColor(cf); body.setPosition(hx, hy + 5.0f);
        m_window.draw(body);

        sf::ConvexShape tip;
        tip.setPointCount(3);
        tip.setPoint(0, sf::Vector2f(0.0f,         0.0f));
        tip.setPoint(1, sf::Vector2f(heartW,        0.0f));
        tip.setPoint(2, sf::Vector2f(heartW / 2.0f, 16.0f));
        tip.setFillColor(cf);
        tip.setOutlineColor(co); tip.setOutlineThickness(1.0f);
        tip.setPosition(hx, hy + 16.0f);
        m_window.draw(tip);
    }

    // ── Hint d'action ─────────────────────────────────────────
    for (auto& obs : m_obstacles) {
        float ox = obs->getX();
        if (ox > 60.0f && ox < static_cast<float>(Constants::WINDOW_WIDTH) - 50.0f) {
            std::string hint = (obs->getType() == ObstacleType::GROUND) ? "SAUTER !" : "SE BAISSER !";
            sf::Color hc = (obs->getType() == ObstacleType::GROUND)
                ? sf::Color(0, 220, 255, 220) : sf::Color(255, 160, 0, 220);
            sf::Text ht = makeText(hint, 13, hc);
            ht.setPosition(ox, 360.0f);
            m_window.draw(ht);
        }
    }

    // Controles en bas
    sf::Text ctrl = makeText("[ESPACE/HAUT] Sauter    [BAS] Se baisser", 12, sf::Color(70, 100, 150));
    ctrl.setPosition(10.0f, Constants::WINDOW_HEIGHT - 20.0f);
    m_window.draw(ctrl);
}

// ──────────────────────────────────────────────────────────────
//  drawCapsule() — capsule futuriste lumineuse
// ──────────────────────────────────────────────────────────────
void Game::drawCapsule() {
    float cx = m_capsuleX;
    float cy = m_capsule.getPosition().y;

    float glow = (std::sin(m_capsuleGlowTimer * 4.0f) + 1.0f) * 0.5f;
    sf::Uint8 ga = static_cast<sf::Uint8>(100 + glow * 155);
    sf::Color gc(0, static_cast<sf::Uint8>(200 + glow * 55), 100, ga);

    // Halo
    sf::RectangleShape halo(sf::Vector2f(122.0f, 92.0f));
    halo.setFillColor(sf::Color(0, 255, 120, static_cast<sf::Uint8>(18 + glow * 28)));
    halo.setPosition(cx - 6.0f, cy - 6.0f);
    m_window.draw(halo);

    // Corps
    sf::RectangleShape body(sf::Vector2f(110.0f, 80.0f));
    body.setFillColor(sf::Color(18, 42, 72));
    body.setOutlineColor(gc);
    body.setOutlineThickness(3.5f);
    body.setPosition(cx, cy);
    m_window.draw(body);

    // Bandes laterales
    for (float bx : {cx + 2.0f, cx + 98.0f}) {
        sf::RectangleShape band(sf::Vector2f(10.0f, 80.0f));
        band.setFillColor(sf::Color(0, static_cast<sf::Uint8>(155 + glow * 60), 95, 190));
        band.setPosition(bx, cy);
        m_window.draw(band);
    }

    // Grand hublot
    sf::CircleShape port(24.0f);
    port.setFillColor(sf::Color(0, static_cast<sf::Uint8>(175 + glow * 80), 255, 225));
    port.setOutlineColor(sf::Color(0, 240, 255));
    port.setOutlineThickness(3.0f);
    port.setOrigin(24.0f, 24.0f);
    port.setPosition(cx + 55.0f, cy + 35.0f);
    m_window.draw(port);

    sf::CircleShape reflet(8.0f);
    reflet.setFillColor(sf::Color(255, 255, 255, 110));
    reflet.setOrigin(8.0f, 8.0f);
    reflet.setPosition(cx + 43.0f, cy + 21.0f);
    m_window.draw(reflet);

    // Moteurs
    for (float mx : {cx + 15.0f, cx + 73.0f}) {
        sf::RectangleShape mot(sf::Vector2f(22.0f, 14.0f));
        mot.setFillColor(sf::Color(55, 55, 78));
        mot.setOutlineColor(sf::Color(255, 140, 0));
        mot.setOutlineThickness(2.0f);
        mot.setPosition(mx, cy + 80.0f);
        m_window.draw(mot);

        // Flamme
        float fr = 6.0f + glow * 5.0f;
        sf::CircleShape flame(fr);
        flame.setFillColor(sf::Color(255, 110, 0, static_cast<sf::Uint8>(170 + glow * 85)));
        flame.setOrigin(fr, 0.0f);
        flame.setPosition(mx + 11.0f, cy + 94.0f);
        m_window.draw(flame);
    }

    // Label clignotant
    if (static_cast<int>(m_capsuleGlowTimer * 3.0f) % 2 == 0) {
        sf::Text label = makeText("CAPSULE DE SAUVETAGE", 13, gc);
        label.setPosition(cx + 55.0f - label.getLocalBounds().width / 2.0f, cy - 25.0f);
        m_window.draw(label);
    }
}

// ──────────────────────────────────────────────────────────────
//  drawReadyScreen() — robot a l'arret, attend START
// ──────────────────────────────────────────────────────────────
void Game::drawReadyScreen() {
    // Panneau semi-transparent
    sf::RectangleShape panel(sf::Vector2f(480.0f, 130.0f));
    panel.setFillColor(sf::Color(5, 12, 30, 210));
    panel.setOutlineColor(Constants::COLOR_ACCENT_CYAN);
    panel.setOutlineThickness(2.5f);
    panel.setPosition(Constants::WINDOW_WIDTH / 2.0f - 240.0f, 165.0f);
    m_window.draw(panel);

    sf::Text ready = makeText("PRET POUR LA MISSION ?", 28, Constants::COLOR_ACCENT_CYAN);
    ready.setPosition(Constants::WINDOW_WIDTH / 2.0f - ready.getLocalBounds().width / 2.0f, 178.0f);
    m_window.draw(ready);

    sf::Text sub = makeText("Appuyez sur ESPACE pour lancer la course !", 18, Constants::COLOR_HUD_TEXT);
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 220.0f);
    m_window.draw(sub);

    sf::Text esc = makeText("[ ECHAP ] Retour au menu", 14, sf::Color(80, 110, 155));
    esc.setPosition(Constants::WINDOW_WIDTH / 2.0f - esc.getLocalBounds().width / 2.0f, 252.0f);
    m_window.draw(esc);

    // Fleches clignotantes autour du robot
    static float blinkT = 0.0f;
    blinkT += 0.016f;
    if (std::sin(blinkT * 5.0f) > 0.0f) {
        sf::Text arrow = makeText(">>>", 22, Constants::COLOR_ACCENT_GREEN);
        arrow.setPosition(Constants::PLAYER_START_X + 50.0f, Constants::PLAYER_START_Y - 30.0f);
        m_window.draw(arrow);
    }
}

// ──────────────────────────────────────────────────────────────
//  drawRobotPreview() — robot complet pour le menu
// ──────────────────────────────────────────────────────────────
void Game::drawRobotPreview(float cx, float cy, float scale) {
    sf::Color body  = sf::Color(160, 175, 205);
    sf::Color head  = sf::Color(195, 210, 230);
    sf::Color visor = sf::Color(0, 210, 255, 200);
    sf::Color limb  = sf::Color(130, 145, 180);
    sf::Color boot  = Constants::COLOR_ACCENT_ORANGE;
    sf::Color cyan  = Constants::COLOR_ACCENT_CYAN;

    auto R = [&](float x, float y, float w, float h, sf::Color f,
                 sf::Color o = sf::Color::Transparent) {
        sf::RectangleShape r(sf::Vector2f(w * scale, h * scale));
        r.setFillColor(f);
        if (o != sf::Color::Transparent) { r.setOutlineColor(o); r.setOutlineThickness(1.5f * scale); }
        r.setOrigin(w * scale / 2.0f, h * scale / 2.0f);
        r.setPosition(cx + x * scale, cy + y * scale);
        m_window.draw(r);
    };
    auto C = [&](float x, float y, float r, sf::Color f) {
        sf::CircleShape c(r * scale);
        c.setFillColor(f);
        c.setOrigin(r * scale, r * scale);
        c.setPosition(cx + x * scale, cy + y * scale);
        m_window.draw(c);
    };
    auto L = [&](float x, float y, float angle, float w, float h, sf::Color f) {
        sf::RectangleShape r(sf::Vector2f(w * scale, h * scale));
        r.setFillColor(f);
        r.setOrigin(w * scale / 2.0f, 0.0f);
        r.setPosition(cx + x * scale, cy + y * scale);
        r.setRotation(angle);
        m_window.draw(r);
    };

    // Antenne
    R(0, -56, 3, 14, cyan);
    C(0, -66, 5, cyan);

    // Tete
    R(0, -32, 30, 26, head, sf::Color(80, 110, 160));
    C(0, -32, 10, visor);
    C(-6, -38, 4, sf::Color(255, 255, 255, 110));

    // Bras
    L(-22, -10, -12, 7, 24, limb);
    L( 22, -10,  12, 7, 24, limb);

    // Torse
    R(0, 8, 36, 42, body, sf::Color(80, 110, 160));
    R(0, 6, 20, 10, sf::Color(0, 160, 220, 150));

    // Jambes
    L(-10, 30, -8, 9, 30, limb);
    L( 10, 30,  8, 9, 30, limb);

    // Bottes
    R(-10, 56, 14, 8, boot);
    R( 10, 56, 14, 8, boot);
}

// ──────────────────────────────────────────────────────────────
//  drawButton()
// ──────────────────────────────────────────────────────────────
void Game::drawButton(const std::string& label, float y, bool selected) {
    float bw = 280.0f, bh = 46.0f;
    float bx = Constants::WINDOW_WIDTH / 2.0f - bw / 2.0f;
    sf::RectangleShape btn(sf::Vector2f(bw, bh));
    btn.setPosition(bx, y);
    btn.setFillColor(selected ? sf::Color(0, 70, 120) : sf::Color(14, 24, 50));
    btn.setOutlineColor(selected ? Constants::COLOR_ACCENT_CYAN : sf::Color(45, 75, 118));
    btn.setOutlineThickness(selected ? 2.5f : 1.5f);
    m_window.draw(btn);

    sf::Color tc = selected ? Constants::COLOR_ACCENT_CYAN : sf::Color(135, 165, 210);
    sf::Text txt = makeText(label, selected ? 22u : 19u, tc);
    txt.setPosition(Constants::WINDOW_WIDTH / 2.0f - txt.getLocalBounds().width / 2.0f,
                    y + bh / 2.0f - txt.getLocalBounds().height / 2.0f - 3.0f);
    m_window.draw(txt);
}

// ──────────────────────────────────────────────────────────────
//  drawMainMenu()
// ──────────────────────────────────────────────────────────────
void Game::drawMainMenu() {
    sf::Text title = makeText("STATION EVAC", 50, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 28.0f);
    m_window.draw(title);

    sf::Text sub = makeText("HULL BREACH", 24, Constants::COLOR_ACCENT_ORANGE);
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 88.0f);
    m_window.draw(sub);

    sf::RectangleShape line(sf::Vector2f(380.0f, 2.0f));
    line.setFillColor(sf::Color(0, 100, 145));
    line.setPosition(Constants::WINDOW_WIDTH / 2.0f - 190.0f, 122.0f);
    m_window.draw(line);

    // Robot avec membres complets a gauche
    drawRobotPreview(178.0f, 330.0f, 1.35f);
    sf::Text rname = makeText("R0-B0T", 16, Constants::COLOR_ACCENT_CYAN);
    rname.setPosition(178.0f - rname.getLocalBounds().width / 2.0f, 422.0f);
    m_window.draw(rname);

    // 3 boutons a droite
    float sy = 258.0f, gap = 62.0f;
    drawButton("[ JOUER ]",    sy,        m_selectedButton == MenuButton::PLAY);
    drawButton("[ A PROPOS ]", sy + gap,  m_selectedButton == MenuButton::ABOUT);
    drawButton("[ QUITTER ]",  sy + gap*2, m_selectedButton == MenuButton::QUIT);

    sf::Text nav = makeText("[HAUT/BAS] Naviguer    [ENTREE] Confirmer", 12, sf::Color(65, 95, 135));
    nav.setPosition(Constants::WINDOW_WIDTH / 2.0f - nav.getLocalBounds().width / 2.0f,
                    Constants::WINDOW_HEIGHT - 26.0f);
    m_window.draw(nav);
}

// ──────────────────────────────────────────────────────────────
//  drawAboutScreen() — histoire mise a jour
// ──────────────────────────────────────────────────────────────
void Game::drawAboutScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 185));
    m_window.draw(overlay);

    sf::Text title = makeText("A PROPOS", 36, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 38.0f);
    m_window.draw(title);

    // Histoire (texte mis a jour)
    sf::Text h1 = makeText("La station spatiale Omega-7 est en train d'exploser !", 19, sf::Color(255, 200, 100));
    h1.setPosition(Constants::WINDOW_WIDTH / 2.0f - h1.getLocalBounds().width / 2.0f, 92.0f);
    m_window.draw(h1);

    sf::Text h2 = makeText("Evitez les debris spatiaux en sautant ou en vous baissant.", 16, Constants::COLOR_HUD_TEXT);
    h2.setPosition(Constants::WINDOW_WIDTH / 2.0f - h2.getLocalBounds().width / 2.0f, 124.0f);
    m_window.draw(h2);

    sf::Text h3 = makeText("Atteignez la capsule avant l'explosion !", 16, sf::Color(200, 180, 255));
    h3.setPosition(Constants::WINDOW_WIDTH / 2.0f - h3.getLocalBounds().width / 2.0f, 148.0f);
    m_window.draw(h3);

    sf::RectangleShape sep(sf::Vector2f(500.0f, 1.0f));
    sep.setFillColor(sf::Color(45, 75, 120));
    sep.setPosition(Constants::WINDOW_WIDTH / 2.0f - 250.0f, 185.0f);
    m_window.draw(sep);

    sf::Text ctrlTitle = makeText("CONTROLES", 19, Constants::COLOR_ACCENT_ORANGE);
    ctrlTitle.setPosition(Constants::WINDOW_WIDTH / 2.0f - ctrlTitle.getLocalBounds().width / 2.0f, 196.0f);
    m_window.draw(ctrlTitle);

    struct CtrlLine { const char* key; const char* desc; };
    CtrlLine lines[] = {
        {"[ESPACE] ou [HAUT]", "Sauter par-dessus un obstacle au sol"},
        {"[BAS] ou [S]",       "Se baisser sous un obstacle aerien"},
        {"[HAUT] / [BAS]",     "Naviguer dans le menu"},
        {"[ENTREE]",           "Valider / Rejouer"},
    };
    float y = 232.0f;
    for (auto& l : lines) {
        sf::Text k = makeText(l.key, 15, Constants::COLOR_ACCENT_CYAN);
        k.setPosition(135.0f, y);
        m_window.draw(k);
        sf::Text d = makeText(l.desc, 15, Constants::COLOR_HUD_TEXT);
        d.setPosition(365.0f, y);
        m_window.draw(d);
        y += 32.0f;
    }

    sf::Text hi = makeText("Le robot a 3 coeurs. Chaque collision en retire un.", 14, sf::Color(195, 175, 220));
    hi.setPosition(Constants::WINDOW_WIDTH / 2.0f - hi.getLocalBounds().width / 2.0f, 368.0f);
    m_window.draw(hi);

    sf::Text hi2 = makeText("Survivez 60s pour faire apparaitre la capsule de sauvetage !", 13, sf::Color(165, 165, 200));
    hi2.setPosition(Constants::WINDOW_WIDTH / 2.0f - hi2.getLocalBounds().width / 2.0f, 390.0f);
    m_window.draw(hi2);

    drawRobotPreview(Constants::WINDOW_WIDTH / 2.0f, 460.0f, 0.95f);

    sf::Text back = makeText("[ ECHAP ou ENTREE ] Retour", 14, sf::Color(75, 115, 158));
    back.setPosition(Constants::WINDOW_WIDTH / 2.0f - back.getLocalBounds().width / 2.0f,
                     Constants::WINDOW_HEIGHT - 26.0f);
    m_window.draw(back);
}

// ──────────────────────────────────────────────────────────────
//  drawVictoryScreen()
// ──────────────────────────────────────────────────────────────
void Game::drawVictoryScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 20, 10, 165));
    m_window.draw(overlay);

    sf::RectangleShape topLine(sf::Vector2f(Constants::WINDOW_WIDTH, 4.0f));
    topLine.setFillColor(Constants::COLOR_ACCENT_GREEN);
    topLine.setPosition(0.0f, 100.0f);
    m_window.draw(topLine);

    sf::Text title = makeText("MISSION REUSSIE !", 50, Constants::COLOR_ACCENT_GREEN);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 115.0f);
    m_window.draw(title);

    sf::Text thanks = makeText("Merci de m'avoir sauve !", 32, sf::Color(255, 230, 80));
    thanks.setPosition(Constants::WINDOW_WIDTH / 2.0f - thanks.getLocalBounds().width / 2.0f, 185.0f);
    m_window.draw(thanks);

    sf::Text sub = makeText("Le robot est en securite dans la capsule.", 18, Constants::COLOR_HUD_TEXT);
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 232.0f);
    m_window.draw(sub);

    sf::RectangleShape sep(sf::Vector2f(400.0f, 2.0f));
    sep.setFillColor(sf::Color(0, 145, 78));
    sep.setPosition(Constants::WINDOW_WIDTH / 2.0f - 200.0f, 270.0f);
    m_window.draw(sep);

    std::string sc = "Temps de survie : " + Utils::formatTime(m_finalScore);
    sf::Text score = makeText(sc, 24, Constants::COLOR_ACCENT_CYAN);
    score.setPosition(Constants::WINDOW_WIDTH / 2.0f - score.getLocalBounds().width / 2.0f, 282.0f);
    m_window.draw(score);

    sf::RectangleShape botLine(sf::Vector2f(Constants::WINDOW_WIDTH, 4.0f));
    botLine.setFillColor(Constants::COLOR_ACCENT_GREEN);
    botLine.setPosition(0.0f, 368.0f);
    m_window.draw(botLine);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(95, 200, 115));
    replay.setPosition(Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 382.0f);
    m_window.draw(replay);
}

// ──────────────────────────────────────────────────────────────
//  drawExplosionScreen()
// ──────────────────────────────────────────────────────────────
void Game::drawExplosionScreen() {
    m_window.clear(sf::Color(75, 4, 4));

    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape streak(sf::Vector2f(
            Utils::randomFloat(80.0f, 480.0f),
            Utils::randomFloat(2.0f, 12.0f)
        ));
        streak.setFillColor(sf::Color(255, Utils::randomInt(40, 120), 0, 170));
        streak.setPosition(Utils::randomFloat(0, Constants::WINDOW_WIDTH),
                           Utils::randomFloat(0, Constants::WINDOW_HEIGHT));
        m_window.draw(streak);
    }

    sf::Text boom = makeText("KABOOM !", 70, sf::Color(255, 175, 0));
    boom.setPosition(Constants::WINDOW_WIDTH / 2.0f - boom.getLocalBounds().width / 2.0f, 95.0f);
    m_window.draw(boom);

    sf::Text msg = makeText("Robot detruit dans l'explosion.", 26, sf::Color(255, 215, 175));
    msg.setPosition(Constants::WINDOW_WIDTH / 2.0f - msg.getLocalBounds().width / 2.0f, 208.0f);
    m_window.draw(msg);

    float pct = std::min((m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE) * 100.0f, 100.0f);
    std::string ps = "Progression atteinte : " + std::to_string(static_cast<int>(pct)) + "%";
    sf::Text prog = makeText(ps, 18, sf::Color(215, 175, 95));
    prog.setPosition(Constants::WINDOW_WIDTH / 2.0f - prog.getLocalBounds().width / 2.0f, 265.0f);
    m_window.draw(prog);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(195, 95, 75));
    replay.setPosition(Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 395.0f);
    m_window.draw(replay);
}

// ──────────────────────────────────────────────────────────────
//  drawGameOverScreen()
// ──────────────────────────────────────────────────────────────
void Game::drawGameOverScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 205));
    m_window.draw(overlay);

    sf::Text title = makeText("GAME OVER", 54, Constants::COLOR_ACCENT_RED);
    title.setPosition(Constants::WINDOW_WIDTH / 2.0f - title.getLocalBounds().width / 2.0f, 138.0f);
    m_window.draw(title);

    sf::Text sub = makeText("Le robot a subi trop de collisions.", 20, sf::Color(195, 145, 145));
    sub.setPosition(Constants::WINDOW_WIDTH / 2.0f - sub.getLocalBounds().width / 2.0f, 215.0f);
    m_window.draw(sub);

    float pct = std::min((m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE) * 100.0f, 100.0f);
    std::string ps = "Progression atteinte : " + std::to_string(static_cast<int>(pct)) + "%";
    sf::Text prog = makeText(ps, 20, sf::Color(175, 175, 215));
    prog.setPosition(Constants::WINDOW_WIDTH / 2.0f - prog.getLocalBounds().width / 2.0f, 272.0f);
    m_window.draw(prog);

    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(175, 95, 95));
    replay.setPosition(Constants::WINDOW_WIDTH / 2.0f - replay.getLocalBounds().width / 2.0f, 368.0f);
    m_window.draw(replay);
}

// ──────────────────────────────────────────────────────────────
//  Utilitaires
// ──────────────────────────────────────────────────────────────
void Game::startGame() {
    resetGame();
    m_state = GameState::READY;   // ← va dans READY, pas PLAYING
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
    m_capsuleGlowTimer= 0.0f;
    m_obstacles.clear();
    m_player = std::make_unique<Player>(Constants::PLAYER_START_X, Constants::PLAYER_START_Y);
    if (m_audioLoaded) m_music.stop();
}

void Game::triggerScreenShake(float duration) {
    m_shakeActive = true;
    m_shakeTimer  = duration;
}

// ──────────────────────────────────────────────────────────────
//  loadAudio() — charge les sons depuis assets/
//  Si un fichier est absent, on continue sans son.
// ──────────────────────────────────────────────────────────────
void Game::loadAudio() {
    bool ok = true;

    // Chargement des sons courts (SoundBuffer)
    if (!m_bufJump.loadFromFile("assets/jump.wav")) ok = false;
    if (!m_bufDuck.loadFromFile("assets/duck.wav")) ok = false;
    if (!m_bufHit.loadFromFile("assets/hit.wav"))   ok = false;
    if (!m_bufWin.loadFromFile("assets/win.wav"))   ok = false;

    if (ok) {
        // Associer chaque Sound a son SoundBuffer
        m_sndJump.setBuffer(m_bufJump);
        m_sndDuck.setBuffer(m_bufDuck);
        m_sndHit.setBuffer(m_bufHit);
        m_sndWin.setBuffer(m_bufWin);

        // Reglage des volumes
        m_sndJump.setVolume(80.0f);
        m_sndDuck.setVolume(70.0f);
        m_sndHit.setVolume(90.0f);
        m_sndWin.setVolume(85.0f);
    }

    // Musique de fond (streame depuis le disque)
    if (m_music.openFromFile("assets/music.wav")) {
        m_music.setLoop(true);        // boucle en continu
        m_music.setVolume(55.0f);     // volume modere pour ne pas couvrir les sons
        // La musique ne demarre PAS ici — elle demarre dans READY→PLAYING
        m_audioLoaded = ok;  // true seulement si tous les fichiers sont OK
    }
    // Note : si les fichiers sont absents, m_audioLoaded reste false
    // et aucun son ne sera joue. Le jeu fonctionne quand meme.
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
