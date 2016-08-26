#include "director.hpp"
#include "game.hpp"

int main() {
    Director director;
    director.startWithScene(new Game());

    return 0;
}