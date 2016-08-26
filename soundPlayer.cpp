//
//  TextureLoader.cpp
//  GameClient
//
//  Created by Andrew Jones on 9/28/14.
//  Copyright (c) 2014 ThreeLives. All rights reserved.
//

#include "soundPlayer.hpp"
#ifdef _WIN32
#else
//#include "ResourcePath.hpp"
#endif

SoundPlayer *SoundPlayer::instance = NULL;

SoundPlayer *SoundPlayer::getInstance() {
    if (!instance) {
        instance = new SoundPlayer();
        instance->loadSound("hit.wav");
        instance->loadSound("shoot.wav");
        instance->loadSound("explosion.wav");
    }
    return instance;
}

void SoundPlayer::loadSound(std::string file) {
    sf::SoundBuffer *sound = sounds[file];

    if (sound == NULL) {
        sound = new sf::SoundBuffer();
        sound->loadFromFile("sounds/"+file);
        sounds[file] = sound;
    }
}

void SoundPlayer::playSound(std::string file) {
    for (size_t i=0;i<playing_sounds.size();i++) {
        if (playing_sounds[i]->getStatus() == sf::Sound::Stopped) {
            delete playing_sounds[i];
            playing_sounds.erase(playing_sounds.begin()+i);
            i--;
        }
    }

    sf::SoundBuffer *sound = sounds[file];

    if (sound == NULL) {
        sound = new sf::SoundBuffer();
        sound->loadFromFile("sounds/"+file);
        sounds[file] = sound;
    }

    sf::Sound *s = new sf::Sound;
    s->setBuffer(*sound);
    s->play();
    playing_sounds.push_back(s);
}
