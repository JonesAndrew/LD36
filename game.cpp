#include "game.hpp"

Game::Game() {

}

Scene *Game::update() {
    return NULL;
}

void Game::render(sf::RenderWindow &window) {
    sf::CircleShape s(30);
    window.draw(s);
}