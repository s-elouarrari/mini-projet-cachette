#include "../include/game.hpp"
#include <iostream>

Game::Game() {}

void Game::run() {
    std::cout << "Game started" << std::endl;

    while (true) {
        processEvents();
        update();
        render();
    }
}

void Game::processEvents() {
    // input later
}

void Game::update() {
    // game logic later
}

void Game::render() {
    // display later
}