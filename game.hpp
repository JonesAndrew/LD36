#pragma once

#include "scene.hpp"

class Game : public Scene
{
public:
    Game();
    ~Game() {};
    
    virtual Scene *update();
    virtual void render(sf::RenderWindow &window);
};