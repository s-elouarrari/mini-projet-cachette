#include "../include/HUD.hpp"
#include "../include/Utils.hpp"
#include <cmath>

// ============================================================
//  HUD.cpp — Affichage de l'interface utilisateur
//
//  CONCEPT POO : Encapsulation + Passage par référence
//  Le HUD reçoit les données dont il a besoin via ses paramètres.
//  Il ne connaît pas la classe Game, ni Player directement.
// ============================================================

// ------------------------------------------------------------
//  Constructeur — initialise les formes des barres
//  Paramètre par référence constante : on lit la police sans la copier
// ------------------------------------------------------------
HUD::HUD(const sf::Font& font)
    : m_font(font)
{
    // Barre rouge "EXPLOSION" (haut gauche)
    m_timerBarBg.setSize(sf::Vector2f(260.0f, 18.0f));
    m_timerBarBg.setPosition(10.0f, 12.0f);
    m_timerBarBg.setFillColor(sf::Color(30, 10, 10));
    m_timerBarBg.setOutlineColor(sf::Color(150, 40, 40));
    m_timerBarBg.setOutlineThickness(1.5f);
    m_timerBarFill.setPosition(10.0f, 12.0f);
    m_timerBarFill.setFillColor(sf::Color(220, 40, 40));

    // Barre verte "CAPSULE" (haut droite)
    m_progressBarBg.setSize(sf::Vector2f(260.0f, 18.0f));
    m_progressBarBg.setPosition(Constants::WINDOW_WIDTH - 270.0f, 12.0f);
    m_progressBarBg.setFillColor(sf::Color(10, 30, 10));
    m_progressBarBg.setOutlineColor(sf::Color(40, 150, 40));
    m_progressBarBg.setOutlineThickness(1.5f);
    m_progressBarFill.setPosition(Constants::WINDOW_WIDTH - 270.0f, 12.0f);
    m_progressBarFill.setFillColor(Constants::COLOR_ACCENT_GREEN);
}

// ------------------------------------------------------------
//  draw() — point d'entrée principal du HUD
// ------------------------------------------------------------
void HUD::draw(sf::RenderWindow& window,
               float explosionTimer,
               float survivalTime,
               bool  capsuleVisible,
               int   playerHealth,
               const std::vector<std::pair<float,bool>>& obstacleHints)
{
    // Calcul des ratios (valeurs entre 0.0 et 1.0)
    float timeRatio = explosionTimer / Constants::EXPLOSION_TIME;
    if (timeRatio < 0.0f) timeRatio = 0.0f;

    float progressRatio = survivalTime / Constants::SURVIVAL_TIME_FOR_CAPSULE;
    if (progressRatio > 1.0f) progressRatio = 1.0f;

    drawExplosionBar(window, timeRatio);
    drawProgressBar(window, progressRatio, capsuleVisible);
    drawHearts(window, playerHealth);
    drawHints(window, obstacleHints);

    // Instructions en bas de l'écran
    sf::Text ctrl = makeText("[ESPACE/HAUT] Sauter    [BAS] Se baisser",
                              12, sf::Color(70, 100, 150));
    ctrl.setPosition(10.0f, Constants::WINDOW_HEIGHT - 20.0f);
    window.draw(ctrl);
}

// ------------------------------------------------------------
//  drawExplosionBar() — barre rouge qui diminue avec le temps
// ------------------------------------------------------------
void HUD::drawExplosionBar(sf::RenderWindow& window, float ratio) {
    m_timerBarFill.setSize(sf::Vector2f(260.0f * ratio, 18.0f));

    // Interpolation de couleur : vert (sûr) → rouge (danger)
    sf::Color tc = Utils::lerpColor(Constants::COLOR_ACCENT_RED,
                                    sf::Color(0, 200, 80), ratio);
    m_timerBarFill.setFillColor(tc);

    window.draw(m_timerBarBg);
    window.draw(m_timerBarFill);

    // Label texte
    sf::Text label = makeText("EXPLOSION", 11, tc);
    label.setPosition(10.0f, 33.0f);
    window.draw(label);

    // Compteur en secondes
    int sec = static_cast<int>(ratio * Constants::EXPLOSION_TIME);
    sf::Text secT = makeText(std::to_string(sec) + "s", 13, tc);
    secT.setPosition(275.0f, 11.0f);
    window.draw(secT);
}

// ------------------------------------------------------------
//  drawProgressBar() — barre verte de progression vers la capsule
// ------------------------------------------------------------
void HUD::drawProgressBar(sf::RenderWindow& window, float ratio, bool capsuleVisible) {
    m_progressBarFill.setSize(sf::Vector2f(260.0f * ratio, 18.0f));

    sf::Color pc = capsuleVisible
        ? sf::Color(0, 255, 120)
        : Utils::lerpColor(sf::Color(0, 100, 255), Constants::COLOR_ACCENT_GREEN, ratio);
    m_progressBarFill.setFillColor(pc);

    window.draw(m_progressBarBg);
    window.draw(m_progressBarFill);

    // Label dynamique
    std::string capStr = capsuleVisible
        ? "CAPSULE EN APPROCHE !"
        : "CAPSULE : " + std::to_string(static_cast<int>(ratio * 100)) + "%";
    sf::Text capT = makeText(capStr, 11, pc);
    capT.setPosition(Constants::WINDOW_WIDTH - 270.0f, 33.0f);
    window.draw(capT);
}

// ------------------------------------------------------------
//  drawHearts() — icônes de cœur centrées en haut de l'écran
//
//  Chaque cœur = 2 cercles (bosses) + 1 rectangle + 1 triangle
//  Plein = rouge, Vide = gris foncé
// ------------------------------------------------------------
void HUD::drawHearts(sf::RenderWindow& window, int health) {
    const float heartW   = 36.0f;
    const float heartGap = 12.0f;
    const float totalW   = Constants::MAX_HEALTH * heartW + (Constants::MAX_HEALTH-1) * heartGap;
    float hx0 = Constants::WINDOW_WIDTH / 2.0f - totalW / 2.0f;
    float hy   = 7.0f;

    for (int i = 0; i < Constants::MAX_HEALTH; ++i) {
        bool full = (i < health);
        sf::Color cf = full ? sf::Color(230, 50, 70)   : sf::Color(55, 30, 40);
        sf::Color co = full ? sf::Color(255, 100, 120) : sf::Color(80, 50, 60);
        float hx = hx0 + i * (heartW + heartGap);

        // Bosse gauche
        sf::CircleShape bL(9.0f);
        bL.setFillColor(cf); bL.setOutlineColor(co); bL.setOutlineThickness(1.5f);
        bL.setOrigin(9.0f, 9.0f); bL.setPosition(hx + 9.0f, hy + 9.0f);
        window.draw(bL);

        // Bosse droite
        sf::CircleShape bR(9.0f);
        bR.setFillColor(cf); bR.setOutlineColor(co); bR.setOutlineThickness(1.5f);
        bR.setOrigin(9.0f, 9.0f); bR.setPosition(hx + 27.0f, hy + 9.0f);
        window.draw(bR);

        // Corps central
        sf::RectangleShape body(sf::Vector2f(heartW, 14.0f));
        body.setFillColor(cf); body.setPosition(hx, hy + 5.0f);
        window.draw(body);

        // Pointe (triangle vers le bas)
        sf::ConvexShape tip;
        tip.setPointCount(3);
        tip.setPoint(0, sf::Vector2f(0.0f,        0.0f));
        tip.setPoint(1, sf::Vector2f(heartW,       0.0f));
        tip.setPoint(2, sf::Vector2f(heartW/2.0f, 16.0f));
        tip.setFillColor(cf);
        tip.setOutlineColor(co); tip.setOutlineThickness(1.0f);
        tip.setPosition(hx, hy + 16.0f);
        window.draw(tip);
    }
}

// ------------------------------------------------------------
//  drawHints() — affiche "SAUTER !" ou "SE BAISSER !" au-dessus
//  de chaque obstacle visible
// ------------------------------------------------------------
void HUD::drawHints(sf::RenderWindow& window,
                    const std::vector<std::pair<float,bool>>& hints)
{
    for (const auto& h : hints) {
        float ox      = h.first;
        bool  isGround = h.second;

        if (ox > 60.0f && ox < static_cast<float>(Constants::WINDOW_WIDTH) - 50.0f) {
            std::string msg = isGround ? "SAUTER !" : "SE BAISSER !";
            sf::Color col = isGround
                ? sf::Color(0, 220, 255, 220)
                : sf::Color(255, 160, 0, 220);
            sf::Text ht = makeText(msg, 13, col);
            ht.setPosition(ox, 360.0f);
            window.draw(ht);
        }
    }
}

// ------------------------------------------------------------
//  makeText() — helper pour créer un sf::Text configuré
// ------------------------------------------------------------
sf::Text HUD::makeText(const std::string& str, unsigned int size, sf::Color color) const {
    sf::Text t;
    t.setFont(m_font);
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(color);
    return t;
}

