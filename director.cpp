#include "director.hpp"

#include <iostream>

void Director::startWithScene(Scene *scene) {
    window = new sf::RenderWindow(sf::VideoMode(480*2,300*2),"Lost Kas");
    window->setVerticalSyncEnabled(true);
    window->setKeyRepeatEnabled(false);

    current = scene;
    nextScene = NULL;

    sf::Clock timer;

    double acc = 0;
    double dt = 1.0 / 60.0;

    while (true) {

        if (nextScene != NULL) {
            delete current;
            current = nextScene;
            nextScene = NULL;
        }

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
            nextScene = current->update();
            acc -= dt;
        }

        window->clear();
        current->render(*window);
        window->display();
    }

    delete current;
}