#pragma once

#include "scene.hpp"

class Director {
private:
    Scene *current;
    Scene *next;
    sf::RenderWindow *window;
public:
    void startWithScene(Scene *scene);
};