#pragma once

#include "SFML/graphics.hpp"

class Scene {
protected:
public:
    virtual ~Scene() {};
    virtual void handleEvent(sf::Event event) {}
    virtual Scene *update() {return NULL;}
    virtual void render(sf::RenderWindow &window) {}
};