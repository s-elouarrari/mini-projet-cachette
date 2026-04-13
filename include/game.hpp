#ifndef GAME_HPP
#define GAME_HPP

class Game {
public:
    Game();
    void run();

private:
    void update();
    void render();
    void processEvents();
};

#endif