#include "../include/Game.hpp"
#include "../include/Utils.hpp"
#include <cmath>
#include <algorithm>

// ============================================================
//  Game.cpp — Coordinateur principal du jeu Station Evac
//
//  CONCEPTS POO ILLUSTRÉS DANS CE FICHIER :
//
//  1. COMPOSITION (avoir) :
//     Game "a un" Background, "a un" HUD, "a un" Player.
//     → Les classes dédiées gèrent leur propre logique.
//
//  2. POLYMORPHISME :
//     m_obstacles contient des unique_ptr<Obstacle>.
//     obs->draw(window) appelle MagneticContainer::draw()
//     ou SecurityDrone::draw() selon le vrai type.
//     → Le bon code s'exécute sans if/switch.
//
//  3. STL (Bibliothèque Standard) :
//     std::vector<unique_ptr<Obstacle>> : liste d'obstacles
//     std::remove_if + erase : suppression efficace
//     std::make_unique : allocation sécurisée (pas de new/delete)
//
//  4. ENCAPSULATION :
//     Tous les membres sont private.
//     L'extérieur n'appelle que run().
// ============================================================

// ──────────────────────────────────────────────────────────────
//  Constructeur
//  Initialise tous les membres dans l'ordre : d'abord la liste
//  d'initialisation (avant {}), puis le corps du constructeur.
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
      m_background(),       // construit Background (étoiles, lune, panneaux)
      m_hud(m_font),        // construit HUD avec référence à la police
      m_audioLoaded(false)
{
    m_window.setFramerateLimit(Constants::TARGET_FPS);
    loadFont();   // doit être appelé AVANT m_hud si m_hud stocke la référence
    loadAudio();

    // ── Sol de la station ─────────────────────────────────────
    m_floorRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 60.0f));
    m_floorRect.setPosition(0.0f, Constants::GROUND_Y);
    m_floorRect.setFillColor(sf::Color(22, 32, 62));
    m_floorRect.setOutlineColor(sf::Color(0, 180, 255, 100));
    m_floorRect.setOutlineThickness(2.0f);

    // ── Plafond de la station ─────────────────────────────────
    m_ceilingRect.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, 22.0f));
    m_ceilingRect.setPosition(0.0f, 258.0f);
    m_ceilingRect.setFillColor(sf::Color(18, 26, 52));
    m_ceilingRect.setOutlineColor(sf::Color(0, 160, 220, 80));
    m_ceilingRect.setOutlineThickness(1.5f);

    // ── Capsule de sauvetage ──────────────────────────────────
    m_capsule.setSize(sf::Vector2f(110.0f, 80.0f));
    m_capsule.setFillColor(sf::Color(40, 55, 85));
    m_capsule.setOutlineColor(Constants::COLOR_ACCENT_GREEN);
    m_capsule.setOutlineThickness(3.0f);
    m_capsuleWindow.setRadius(12.0f);
    m_capsuleWindow.setFillColor(sf::Color(0, 200, 255, 160));
    m_capsuleWindow.setOutlineColor(sf::Color(0, 220, 255));
    m_capsuleWindow.setOutlineThickness(2.0f);
    m_capsuleWindow.setOrigin(12.0f, 12.0f);

    resetGame();
}

// Destructeur : les membres SFML (sf::Music, sf::Sound, etc.)
// ont leurs propres destructeurs qui libèrent les ressources audio.
// std::unique_ptr libère Player et les Obstacles automatiquement.
// → Aucun delete manuel nécessaire = pas de fuite mémoire.
Game::~Game() {}

// ──────────────────────────────────────────────────────────────
//  run() — boucle principale (Event → Update → Render)
//
//  Pattern classique des jeux en temps réel :
//    1. Lire les événements
//    2. Mettre à jour la logique (avec le temps écoulé)
//    3. Dessiner la frame
// ──────────────────────────────────────────────────────────────
void Game::run() {
    while (m_window.isOpen()) {
        // deltaTime = secondes écoulées depuis la dernière frame
        // Cela rend le jeu indépendant du framerate (même vitesse
        // à 30fps ou 60fps).
        float dt = m_clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // protection si la fenêtre est déplacée

        processEvents();
        update(dt);
        render();
    }
}

// ──────────────────────────────────────────────────────────────
//  processEvents() — lit les entrées clavier et fenêtre
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
                    if      (m_selectedButton == MenuButton::PLAY)  startGame();
                    else if (m_selectedButton == MenuButton::ABOUT) m_state = GameState::ABOUT;
                    else                                             m_window.close();
                }
                if (event.key.code == sf::Keyboard::Escape) m_window.close();
            }

            // ── Écran À Propos ────────────────────────────────
            else if (m_state == GameState::ABOUT) {
                if (event.key.code == sf::Keyboard::Escape ||
                    event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Space)
                    m_state = GameState::MAIN_MENU;
            }

            // ── Écran READY : robot à l'arrêt, attend ESPACE ─
            else if (m_state == GameState::READY) {
                if (event.key.code == sf::Keyboard::Space  ||
                    event.key.code == sf::Keyboard::Up     ||
                    event.key.code == sf::Keyboard::Return) {
                    m_state = GameState::PLAYING;
                    m_player->startRunning(); // POLYMORPHISME via pointeur
                    if (m_audioLoaded) m_music.play();
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

            // ── Fins de partie : retour au menu ───────────────
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

    // ── Accroupissement maintenu (touche BAS enfoncée) ────────
    if (m_state == GameState::PLAYING) {
        bool duck = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        bool wasCrouching = (m_player->getAction() == PlayerAction::CROUCHING);
        m_player->crouch(duck);
        bool isCrouching  = (m_player->getAction() == PlayerAction::CROUCHING);
        // Son duck : joue seulement au moment de l'accroupissement
        if (!wasCrouching && isCrouching && m_audioLoaded)
            m_sndDuck.play();
    }
}

// ──────────────────────────────────────────────────────────────
//  update() — délègue selon l'état actuel
// ──────────────────────────────────────────────────────────────
void Game::update(float dt) {
    // Le joueur est mis à jour sur READY (animation de balancement)
    // et sur PLAYING (physique complète)
    if (m_state == GameState::READY || m_state == GameState::PLAYING)
        m_player->update(dt);

    if (m_state == GameState::PLAYING)
        updatePlaying(dt);

    updateShake(dt);
    m_capsuleGlowTimer += dt;
}

// ──────────────────────────────────────────────────────────────
//  updatePlaying() — toute la logique pendant la partie
// ──────────────────────────────────────────────────────────────
void Game::updatePlaying(float dt) {
    if (m_gameWon) return; // victoire figée : rien ne bouge

    // ── 1. Compte à rebours explosion ────────────────────────
    m_explosionTimer -= dt;
    if (m_explosionTimer <= 0.0f) {
        m_explosionTimer = 0.0f;
        triggerScreenShake(1.0f);
        if (m_audioLoaded) m_music.stop();
        m_state = GameState::EXPLOSION;
        return;
    }

    // ── 2. Progression vers la capsule ───────────────────────
    m_survivalTime += dt;
    float progressRatio = m_survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;
    if (progressRatio > 1.0f) progressRatio = 1.0f;
    if (progressRatio >= 1.0f && !m_capsuleVisible) {
        m_capsuleVisible = true;
        m_capsuleX = static_cast<float>(Constants::WINDOW_WIDTH) + 80.0f;
    }

    // ── 3. Difficulté progressive ─────────────────────────────
    if (m_scrollSpeed < Constants::SCROLL_SPEED_MAX)
        m_scrollSpeed += Constants::SPEED_INCREMENT * dt;
    m_spawnInterval -= Constants::SPAWN_REDUCTION * dt;
    if (m_spawnInterval < Constants::SPAWN_INTERVAL_MIN)
        m_spawnInterval = Constants::SPAWN_INTERVAL_MIN;

    // ── 4. Décor (délégué à Background) ──────────────────────
    m_background.update(dt);

    // ── 5. Spawn d'obstacles (bloqué quand la capsule est visible) ─
    if (!m_capsuleVisible) {
        m_spawnTimer += dt;
        if (m_spawnTimer >= m_spawnInterval) {
            m_spawnTimer = 0.0f;
            spawnObstacle();
        }
    }

    // ── 6. Mise à jour des obstacles via POLYMORPHISME ────────
    // obs->updateWithSpeed() appelle la bonne méthode selon le type
    // (MagneticContainer ou SecurityDrone) sans if/switch
    for (auto& obs : m_obstacles)
        obs->updateWithSpeed(dt, m_scrollSpeed);

    // ── 7. Détection des collisions ───────────────────────────
    checkCollisions();
    if (m_player->isDead()) {
        triggerScreenShake(0.7f);
        if (m_audioLoaded) {
            m_music.stop();
            m_sndGameOver.play(); // Son game over au moment de la mort
        }
        m_state = GameState::GAME_OVER;
        return;
    }

    // ── 8. Nettoyage des obstacles hors-écran ─────────────────
    cleanObstacles();

    // ── 9. Capsule de sauvetage ───────────────────────────────
    if (m_capsuleVisible) {
        m_capsuleX -= m_scrollSpeed * 0.40f * dt;
        const float capsuleW = 110.0f;
        const float capsuleH = 80.0f;
        float capsuleY = Constants::GROUND_Y - capsuleH; // 410

        m_capsule.setPosition(m_capsuleX, capsuleY);
        m_capsuleWindow.setPosition(m_capsuleX + 55.0f, capsuleY + 35.0f);

        // Collision avec FloatRect (syntaxe simple, niveau L2)
        sf::FloatRect capsuleBounds(m_capsuleX, capsuleY, capsuleW, capsuleH);
        sf::FloatRect playerBounds = m_player->getBounds();

        if (playerBounds.intersects(capsuleBounds)) {
            m_gameWon    = true;
            m_finalScore = m_survivalTime;
            m_state      = GameState::VICTORY;
            m_obstacles.clear(); // écran propre pour la victoire
            if (m_audioLoaded) {
                m_music.stop();
                m_sndWin.play();
            }
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  checkCollisions() — détection avec sf::FloatRect
//
//  CONCEPT : collision AABB (Axis-Aligned Bounding Box)
//  On réduit légèrement la hitbox latérale (±4px) pour être
//  "fair" avec le joueur sur les frôlements visuels.
// ──────────────────────────────────────────────────────────────
void Game::checkCollisions() {
    sf::FloatRect player = m_player->getBounds();
    player.left  += 4.0f;  // tolérance bord gauche
    player.width -= 8.0f;  // tolérance bord droit

    for (auto& obs : m_obstacles) {
        // POLYMORPHISME : obs->getBounds() fonctionne pour tout obstacle
        if (player.intersects(obs->getBounds())) {
            m_player->takeDamage(); // retire 1 cœur + invincibilité
            if (m_audioLoaded) m_sndHit.play();
            triggerScreenShake(0.35f);
            break; // un seul choc par frame (pas de rafale)
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  spawnObstacle() — crée un obstacle aléatoire
//
//  STL : std::make_unique alloue sur le tas sans new/delete.
//  POLYMORPHISME : le vecteur stocke des Obstacle* mais contient
//  des MagneticContainer ou SecurityDrone concrets.
// ──────────────────────────────────────────────────────────────
void Game::spawnObstacle() {
    float spawnX = static_cast<float>(Constants::WINDOW_WIDTH) + 50.0f;
    if (Utils::randomInt(0, 1) == 0)
        m_obstacles.push_back(std::make_unique<MagneticContainer>(spawnX));
    else
        m_obstacles.push_back(std::make_unique<SecurityDrone>(spawnX));
}

// ──────────────────────────────────────────────────────────────
//  cleanObstacles() — supprime les obstacles sortis à gauche
//
//  IDIOME STL : erase-remove_if (motif classique C++)
//  remove_if déplace les éléments "à supprimer" en fin de vecteur,
//  puis erase les supprime réellement.
// ──────────────────────────────────────────────────────────────
void Game::cleanObstacles() {
    m_obstacles.erase(
        std::remove_if(m_obstacles.begin(), m_obstacles.end(),
            [](const std::unique_ptr<Obstacle>& o) {
                return o->isOffScreen(); // lambda = fonction anonyme
            }),
        m_obstacles.end());
}

// ──────────────────────────────────────────────────────────────
//  updateShake() — anime le tremblement d'écran
// ──────────────────────────────────────────────────────────────
void Game::updateShake(float dt) {
    if (!m_shakeActive) return;
    m_shakeTimer -= dt;
    if (m_shakeTimer <= 0.0f) {
        m_shakeActive = false;
        m_shakeOffset = sf::Vector2f(0.0f, 0.0f);
        return;
    }
    float intensity = m_shakeTimer * 8.0f;
    m_shakeOffset = sf::Vector2f(Utils::randomFloat(-intensity, intensity),
                                 Utils::randomFloat(-intensity, intensity));
}

// ──────────────────────────────────────────────────────────────
//  render() — dessine la frame complète
// ──────────────────────────────────────────────────────────────
void Game::render() {
    m_window.clear(Constants::COLOR_BACKGROUND);

    // Applique le tremblement en décalant la vue SFML
    sf::View view = m_window.getDefaultView();
    if (m_shakeActive)
        view.setCenter(Constants::WINDOW_WIDTH  / 2.0f + m_shakeOffset.x,
                       Constants::WINDOW_HEIGHT / 2.0f + m_shakeOffset.y);
    m_window.setView(view);

    switch (m_state) {
        case GameState::MAIN_MENU:
            m_background.draw(m_window);
            drawMainMenu();
            break;
        case GameState::ABOUT:
            m_background.draw(m_window);
            drawAboutScreen();
            break;
        case GameState::READY:
            m_background.draw(m_window);
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            m_player->draw(m_window);
            drawReadyScreen();
            break;
        case GameState::PLAYING: {
            m_background.draw(m_window);
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            if (m_capsuleVisible) drawCapsule();
            // POLYMORPHISME : obs->draw() appelle la bonne sous-classe
            for (auto& obs : m_obstacles) obs->draw(m_window);
            m_player->draw(m_window);
            // Prépare les hints pour le HUD
            std::vector<std::pair<float,bool>> hints;
            for (const auto& obs : m_obstacles)
                hints.push_back({obs->getX(), obs->getType() == ObstacleType::GROUND});
            m_hud.draw(m_window, m_explosionTimer, m_survivalTime,
                       m_capsuleVisible, m_player->getHealth(), hints);
            break;
        }
        case GameState::VICTORY:
            m_background.draw(m_window);
            m_window.draw(m_floorRect);
            m_window.draw(m_ceilingRect);
            drawCapsule();
            m_player->draw(m_window);
            {
                std::vector<std::pair<float,bool>> empty;
                m_hud.draw(m_window, m_explosionTimer, m_survivalTime,
                           true, m_player->getHealth(), empty);
            }
            drawVictoryScreen();
            break;
        case GameState::EXPLOSION:
            drawExplosionScreen();
            break;
        case GameState::GAME_OVER:
            m_background.draw(m_window);
            drawGameOverScreen();
            break;
    }

    m_window.display();
}

// ──────────────────────────────────────────────────────────────
//  drawCapsule() — capsule futuriste animée
// ──────────────────────────────────────────────────────────────
void Game::drawCapsule() {
    float cx = m_capsuleX;
    float cy = m_capsule.getPosition().y;

    float glow = (std::sin(m_capsuleGlowTimer * 4.0f) + 1.0f) * 0.5f;
    sf::Uint8 ga = static_cast<sf::Uint8>(100 + glow * 155);
    sf::Color gc(0, static_cast<sf::Uint8>(200 + glow * 55), 100, ga);

    // Halo de lueur
    sf::RectangleShape halo(sf::Vector2f(122.0f, 92.0f));
    halo.setFillColor(sf::Color(0, 255, 120, static_cast<sf::Uint8>(18 + glow*28)));
    halo.setPosition(cx - 6.0f, cy - 6.0f);
    m_window.draw(halo);

    // Corps principal
    sf::RectangleShape body(sf::Vector2f(110.0f, 80.0f));
    body.setFillColor(sf::Color(18, 42, 72));
    body.setOutlineColor(gc);
    body.setOutlineThickness(3.5f);
    body.setPosition(cx, cy);
    m_window.draw(body);

    // Bandes latérales
    for (float bx : {cx + 2.0f, cx + 98.0f}) {
        sf::RectangleShape band(sf::Vector2f(10.0f, 80.0f));
        band.setFillColor(sf::Color(0, static_cast<sf::Uint8>(155 + glow*60), 95, 190));
        band.setPosition(bx, cy);
        m_window.draw(band);
    }

    // Hublot
    sf::CircleShape port(24.0f);
    port.setFillColor(sf::Color(0, static_cast<sf::Uint8>(175 + glow*80), 255, 225));
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

    // Moteurs + flammes
    for (float mx : {cx + 15.0f, cx + 73.0f}) {
        sf::RectangleShape mot(sf::Vector2f(22.0f, 14.0f));
        mot.setFillColor(sf::Color(55, 55, 78));
        mot.setOutlineColor(sf::Color(255, 140, 0));
        mot.setOutlineThickness(2.0f);
        mot.setPosition(mx, cy + 80.0f);
        m_window.draw(mot);
        float fr = 6.0f + glow * 5.0f;
        sf::CircleShape flame(fr);
        flame.setFillColor(sf::Color(255, 110, 0, static_cast<sf::Uint8>(170 + glow*85)));
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
//  drawRobotPreview() — robot complet pour les écrans statiques
// ──────────────────────────────────────────────────────────────
void Game::drawRobotPreview(float cx, float cy, float scale) {
    sf::Color body  = sf::Color(160, 175, 205);
    sf::Color head  = sf::Color(195, 210, 230);
    sf::Color visor = sf::Color(0, 210, 255, 200);
    sf::Color limb  = sf::Color(130, 145, 180);
    sf::Color boot  = Constants::COLOR_ACCENT_ORANGE;
    sf::Color cyan  = Constants::COLOR_ACCENT_CYAN;

    // Lambda locaux pour dessiner formes centrées
    auto R = [&](float x, float y, float w, float h, sf::Color f,
                 sf::Color o = sf::Color::Transparent) {
        sf::RectangleShape r(sf::Vector2f(w*scale, h*scale));
        r.setFillColor(f);
        if (o != sf::Color::Transparent) { r.setOutlineColor(o); r.setOutlineThickness(1.5f*scale); }
        r.setOrigin(w*scale/2.0f, h*scale/2.0f);
        r.setPosition(cx + x*scale, cy + y*scale);
        m_window.draw(r);
    };
    auto C = [&](float x, float y, float r, sf::Color f) {
        sf::CircleShape c(r*scale);
        c.setFillColor(f);
        c.setOrigin(r*scale, r*scale);
        c.setPosition(cx + x*scale, cy + y*scale);
        m_window.draw(c);
    };
    auto L = [&](float x, float y, float angle, float w, float h, sf::Color f) {
        sf::RectangleShape r(sf::Vector2f(w*scale, h*scale));
        r.setFillColor(f);
        r.setOrigin(w*scale/2.0f, 0.0f);
        r.setPosition(cx + x*scale, cy + y*scale);
        r.setRotation(angle);
        m_window.draw(r);
    };

    R(0, -56, 3, 14, cyan);            // antenne tige
    C(0, -66, 5, cyan);                // antenne pointe
    R(0, -32, 30, 26, head, sf::Color(80,110,160)); // tête
    C(0, -32, 10, visor);              // visière
    C(-6, -38, 4, sf::Color(255,255,255,110)); // reflet visière
    L(-22, -10, -12, 7, 24, limb);    // bras gauche
    L( 22, -10,  12, 7, 24, limb);    // bras droit
    R(0, 8, 36, 42, body, sf::Color(80,110,160)); // torse
    R(0, 6, 20, 10, sf::Color(0,160,220,150));    // panneau torse
    L(-10, 30, -8, 9, 30, limb);      // jambe gauche
    L( 10, 30,  8, 9, 30, limb);      // jambe droite
    R(-10, 56, 14, 8, boot);          // botte gauche
    R( 10, 56, 14, 8, boot);          // botte droite
}

// ──────────────────────────────────────────────────────────────
//  drawButton() — bouton de menu avec surlignage
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
    txt.setPosition(Constants::WINDOW_WIDTH/2.0f - txt.getLocalBounds().width/2.0f,
                    y + bh/2.0f - txt.getLocalBounds().height/2.0f - 3.0f);
    m_window.draw(txt);
}

// ──────────────────────────────────────────────────────────────
//  Écrans de jeu
// ──────────────────────────────────────────────────────────────
void Game::drawMainMenu() {
    sf::Text title = makeText("STATION EVAC", 50, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(Constants::WINDOW_WIDTH/2.0f - title.getLocalBounds().width/2.0f, 28.0f);
    m_window.draw(title);
    sf::Text sub = makeText("HULL BREACH", 24, Constants::COLOR_ACCENT_ORANGE);
    sub.setPosition(Constants::WINDOW_WIDTH/2.0f - sub.getLocalBounds().width/2.0f, 88.0f);
    m_window.draw(sub);
    sf::RectangleShape line(sf::Vector2f(380.0f, 2.0f));
    line.setFillColor(sf::Color(0, 100, 145));
    line.setPosition(Constants::WINDOW_WIDTH/2.0f - 190.0f, 122.0f);
    m_window.draw(line);
    drawRobotPreview(178.0f, 330.0f, 1.35f);
    sf::Text rname = makeText("R0-B0T", 16, Constants::COLOR_ACCENT_CYAN);
    rname.setPosition(178.0f - rname.getLocalBounds().width/2.0f, 422.0f);
    m_window.draw(rname);
    float sy = 258.0f, gap = 62.0f;
    drawButton("[ JOUER ]",    sy,         m_selectedButton == MenuButton::PLAY);
    drawButton("[ A PROPOS ]", sy + gap,   m_selectedButton == MenuButton::ABOUT);
    drawButton("[ QUITTER ]",  sy + gap*2, m_selectedButton == MenuButton::QUIT);
    sf::Text nav = makeText("[HAUT/BAS] Naviguer    [ENTREE] Confirmer",
                             12, sf::Color(65, 95, 135));
    nav.setPosition(Constants::WINDOW_WIDTH/2.0f - nav.getLocalBounds().width/2.0f,
                    Constants::WINDOW_HEIGHT - 26.0f);
    m_window.draw(nav);
}

void Game::drawAboutScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 185));
    m_window.draw(overlay);
    sf::Text title = makeText("A PROPOS", 36, Constants::COLOR_ACCENT_CYAN);
    title.setPosition(Constants::WINDOW_WIDTH/2.0f - title.getLocalBounds().width/2.0f, 38.0f);
    m_window.draw(title);
    sf::Text h1 = makeText("La station spatiale Omega-7 est en train d'exploser !", 19, sf::Color(255,200,100));
    h1.setPosition(Constants::WINDOW_WIDTH/2.0f - h1.getLocalBounds().width/2.0f, 92.0f);
    m_window.draw(h1);
    sf::Text h2 = makeText("Evitez les debris spatiaux en sautant ou en vous baissant.", 16, Constants::COLOR_HUD_TEXT);
    h2.setPosition(Constants::WINDOW_WIDTH/2.0f - h2.getLocalBounds().width/2.0f, 124.0f);
    m_window.draw(h2);
    sf::Text h3 = makeText("Atteignez la capsule avant l'explosion !", 16, sf::Color(200,180,255));
    h3.setPosition(Constants::WINDOW_WIDTH/2.0f - h3.getLocalBounds().width/2.0f, 148.0f);
    m_window.draw(h3);
    sf::RectangleShape sep(sf::Vector2f(500.0f, 1.0f));
    sep.setFillColor(sf::Color(45, 75, 120));
    sep.setPosition(Constants::WINDOW_WIDTH/2.0f - 250.0f, 185.0f);
    m_window.draw(sep);
    sf::Text ctrlTitle = makeText("CONTROLES", 19, Constants::COLOR_ACCENT_ORANGE);
    ctrlTitle.setPosition(Constants::WINDOW_WIDTH/2.0f - ctrlTitle.getLocalBounds().width/2.0f, 196.0f);
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
        sf::Text k = makeText(l.key,  15, Constants::COLOR_ACCENT_CYAN);
        sf::Text d = makeText(l.desc, 15, Constants::COLOR_HUD_TEXT);
        k.setPosition(135.0f, y);
        d.setPosition(365.0f, y);
        m_window.draw(k);
        m_window.draw(d);
        y += 32.0f;
    }
    drawRobotPreview(Constants::WINDOW_WIDTH/2.0f, 460.0f, 0.95f);
    sf::Text back = makeText("[ ECHAP ou ENTREE ] Retour", 14, sf::Color(75, 115, 158));
    back.setPosition(Constants::WINDOW_WIDTH/2.0f - back.getLocalBounds().width/2.0f,
                     Constants::WINDOW_HEIGHT - 26.0f);
    m_window.draw(back);
}

void Game::drawReadyScreen() {
    sf::RectangleShape panel(sf::Vector2f(480.0f, 130.0f));
    panel.setFillColor(sf::Color(5, 12, 30, 210));
    panel.setOutlineColor(Constants::COLOR_ACCENT_CYAN);
    panel.setOutlineThickness(2.5f);
    panel.setPosition(Constants::WINDOW_WIDTH/2.0f - 240.0f, 165.0f);
    m_window.draw(panel);
    sf::Text ready = makeText("PRET POUR LA MISSION ?", 28, Constants::COLOR_ACCENT_CYAN);
    ready.setPosition(Constants::WINDOW_WIDTH/2.0f - ready.getLocalBounds().width/2.0f, 178.0f);
    m_window.draw(ready);
    sf::Text sub = makeText("Appuyez sur ESPACE pour lancer la course !", 18, Constants::COLOR_HUD_TEXT);
    sub.setPosition(Constants::WINDOW_WIDTH/2.0f - sub.getLocalBounds().width/2.0f, 220.0f);
    m_window.draw(sub);
    sf::Text esc = makeText("[ ECHAP ] Retour au menu", 14, sf::Color(80, 110, 155));
    esc.setPosition(Constants::WINDOW_WIDTH/2.0f - esc.getLocalBounds().width/2.0f, 252.0f);
    m_window.draw(esc);
    static float blinkT = 0.0f;
    blinkT += 0.016f;
    if (std::sin(blinkT * 5.0f) > 0.0f) {
        sf::Text arrow = makeText(">>>", 22, Constants::COLOR_ACCENT_GREEN);
        arrow.setPosition(Constants::PLAYER_START_X + 50.0f, Constants::PLAYER_START_Y - 30.0f);
        m_window.draw(arrow);
    }
}

void Game::drawVictoryScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 20, 10, 165));
    m_window.draw(overlay);
    sf::RectangleShape topLine(sf::Vector2f(Constants::WINDOW_WIDTH, 4.0f));
    topLine.setFillColor(Constants::COLOR_ACCENT_GREEN);
    topLine.setPosition(0.0f, 100.0f);
    m_window.draw(topLine);
    sf::Text title = makeText("MISSION REUSSIE !", 50, Constants::COLOR_ACCENT_GREEN);
    title.setPosition(Constants::WINDOW_WIDTH/2.0f - title.getLocalBounds().width/2.0f, 115.0f);
    m_window.draw(title);
    sf::Text thanks = makeText("Merci de m'avoir sauve !", 32, sf::Color(255,230,80));
    thanks.setPosition(Constants::WINDOW_WIDTH/2.0f - thanks.getLocalBounds().width/2.0f, 185.0f);
    m_window.draw(thanks);
    std::string sc = "Temps de survie : " + Utils::formatTime(m_finalScore);
    sf::Text score = makeText(sc, 24, Constants::COLOR_ACCENT_CYAN);
    score.setPosition(Constants::WINDOW_WIDTH/2.0f - score.getLocalBounds().width/2.0f, 282.0f);
    m_window.draw(score);
    sf::RectangleShape botLine(sf::Vector2f(Constants::WINDOW_WIDTH, 4.0f));
    botLine.setFillColor(Constants::COLOR_ACCENT_GREEN);
    botLine.setPosition(0.0f, 368.0f);
    m_window.draw(botLine);
    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(95, 200, 115));
    replay.setPosition(Constants::WINDOW_WIDTH/2.0f - replay.getLocalBounds().width/2.0f, 382.0f);
    m_window.draw(replay);
}

void Game::drawExplosionScreen() {
    m_window.clear(sf::Color(75, 4, 4));
    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape streak(sf::Vector2f(Utils::randomFloat(80,480), Utils::randomFloat(2,12)));
        streak.setFillColor(sf::Color(255, Utils::randomInt(40,120), 0, 170));
        streak.setPosition(Utils::randomFloat(0,Constants::WINDOW_WIDTH),
                           Utils::randomFloat(0,Constants::WINDOW_HEIGHT));
        m_window.draw(streak);
    }
    sf::Text boom = makeText("KABOOM !", 70, sf::Color(255,175,0));
    boom.setPosition(Constants::WINDOW_WIDTH/2.0f - boom.getLocalBounds().width/2.0f, 95.0f);
    m_window.draw(boom);
    sf::Text msg = makeText("Robot detruit dans l'explosion.", 26, sf::Color(255,215,175));
    msg.setPosition(Constants::WINDOW_WIDTH/2.0f - msg.getLocalBounds().width/2.0f, 208.0f);
    m_window.draw(msg);
    float pct = std::min((m_survivalTime/Constants::SURVIVAL_TIME_FOR_CAPSULE)*100.0f, 100.0f);
    sf::Text prog = makeText("Progression : " + std::to_string(static_cast<int>(pct)) + "%", 18, sf::Color(215,175,95));
    prog.setPosition(Constants::WINDOW_WIDTH/2.0f - prog.getLocalBounds().width/2.0f, 265.0f);
    m_window.draw(prog);
    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(195,95,75));
    replay.setPosition(Constants::WINDOW_WIDTH/2.0f - replay.getLocalBounds().width/2.0f, 395.0f);
    m_window.draw(replay);
}

void Game::drawGameOverScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 205));
    m_window.draw(overlay);
    sf::Text title = makeText("GAME OVER", 54, Constants::COLOR_ACCENT_RED);
    title.setPosition(Constants::WINDOW_WIDTH/2.0f - title.getLocalBounds().width/2.0f, 138.0f);
    m_window.draw(title);
    sf::Text sub = makeText("Le robot a subi trop de collisions.", 20, sf::Color(195,145,145));
    sub.setPosition(Constants::WINDOW_WIDTH/2.0f - sub.getLocalBounds().width/2.0f, 215.0f);
    m_window.draw(sub);
    float pct = std::min((m_survivalTime/Constants::SURVIVAL_TIME_FOR_CAPSULE)*100.0f, 100.0f);
    sf::Text prog = makeText("Progression : " + std::to_string(static_cast<int>(pct)) + "%", 20, sf::Color(175,175,215));
    prog.setPosition(Constants::WINDOW_WIDTH/2.0f - prog.getLocalBounds().width/2.0f, 272.0f);
    m_window.draw(prog);
    sf::Text replay = makeText("[ ENTREE ] Retour au menu", 18, sf::Color(175,95,95));
    replay.setPosition(Constants::WINDOW_WIDTH/2.0f - replay.getLocalBounds().width/2.0f, 368.0f);
    m_window.draw(replay);
}

// ──────────────────────────────────────────────────────────────
//  Utilitaires
// ──────────────────────────────────────────────────────────────
void Game::startGame() {
    resetGame();
    m_state = GameState::READY; // robot à l'arrêt, attend ESPACE
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
    // std::make_unique : crée un Player sans new/delete = pas de fuite mémoire
    m_player = std::make_unique<Player>(Constants::PLAYER_START_X, Constants::PLAYER_START_Y);
    if (m_audioLoaded) m_music.stop();
}

void Game::triggerScreenShake(float duration) {
    m_shakeActive = true;
    m_shakeTimer  = duration;
}

// ──────────────────────────────────────────────────────────────
//  loadAudio() — charge les sons depuis assets/
//  Si un fichier est absent, le jeu continue sans audio.
// ──────────────────────────────────────────────────────────────
void Game::loadAudio() {
    bool ok = true;
    if (!m_bufJump.loadFromFile("assets/sound/jump.wav"))     ok = false;
    if (!m_bufDuck.loadFromFile("assets/sound/duck.wav"))     ok = false;
    if (!m_bufHit.loadFromFile("assets/sound/hit.wav"))       ok = false;
    if (!m_bufWin.loadFromFile("assets/sound/win.wav"))       ok = false;
    if (!m_bufGameOver.loadFromFile("assets/sound/gameover.wav")) ok = false; // NOUVEAU

    if (ok) {
        m_sndJump.setBuffer(m_bufJump);
        m_sndDuck.setBuffer(m_bufDuck);
        m_sndHit.setBuffer(m_bufHit);
        m_sndWin.setBuffer(m_bufWin);
        m_sndGameOver.setBuffer(m_bufGameOver); // NOUVEAU

        m_sndJump.setVolume(80.0f);
        m_sndDuck.setVolume(70.0f);
        m_sndHit.setVolume(90.0f);
        m_sndWin.setVolume(85.0f);
        m_sndGameOver.setVolume(90.0f);         // NOUVEAU
    }

    if (m_music.openFromFile("assets/sound/music.wav")) {
        m_music.setLoop(true);
        m_music.setVolume(55.0f);
        m_audioLoaded = ok;
    }
}

bool Game::loadFont() {
    const std::vector<std::string> paths = {
        "assets/fonts/font.ttf",
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
