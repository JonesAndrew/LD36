#include "director.hpp"

#include <iostream>

void Director::startWithScene(Scene *scene) {
    window = new sf::RenderWindow(sf::VideoMode(480,270),"LD36");
    window->setVerticalSyncEnabled(true);

    current = scene;

    sf::Clock timer;

    double acc = 0;
    double dt = 1.0 / 60.0;

    while (true) {

        sf::Event event;
        bool exit = false;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                exit = true;
                break;
            } else {
                current->handleEvent(event);
            }
        }
        if (exit)
            break;

        acc += timer.restart().asSeconds();

        if (acc > 0.5)
            acc = 0.5;

        while (acc > 0) {
            current->update();
            acc -= dt;
        }

        window->clear();
        current->render(*window);
        window->display();
    }
}