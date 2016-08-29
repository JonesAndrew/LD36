#pragma once

#include "scene.hpp"

class Director {
private:
    Scene *current;
    Scene *nextScene;
    sf::RenderWindow *window;
public:
    void startWithScene(Scene *scene);
};