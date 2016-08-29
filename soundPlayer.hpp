#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class SoundPlayer {
private:

    std::map<std::string, sf::SoundBuffer*> sounds;

    std::vector<sf::Sound *> playing_sounds;

    static SoundPlayer *instance;

    SoundPlayer(){};
    SoundPlayer(SoundPlayer const&){};

public:

    static SoundPlayer *getInstance();
    void playSound(std::string file);
    void loadSound(std::string file);
    void clearSounds();
};
