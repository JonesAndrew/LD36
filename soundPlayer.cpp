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
        instance->loadSound("twangsitar1SFX.wav");
        instance->loadSound("twangsitar2SFX.wav");
        instance->loadSound("twangsitar3SFX.wav");
        instance->loadSound("twangsitar4SFX.wav");
        instance->loadSound("getHurtSFX.wav");
        instance->loadSound("monsterDyingSFX.wav");
        instance->loadSound("monsterHurtSFX.wav");
        instance->loadSound("somethingHeavyMovesSFX.wav");
        instance->loadSound("DootDootSFX.wav");
    }
    return instance;
}

void SoundPlayer::loadSound(std::string file) {
    sf::SoundBuffer *sound = sounds[file];

    if (sound == NULL) {
        sound = new sf::SoundBuffer();
        sound->loadFromFile("sfx/"+file);
        sounds[file] = sound;
    }
}

void SoundPlayer::clearSounds() {
    for (size_t i=0;i<playing_sounds.size();i++) {
        delete playing_sounds[i];
        playing_sounds.erase(playing_sounds.begin()+i);
        i--;
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
        sound->loadFromFile("sfx/"+file);
        sounds[file] = sound;
    }

    sf::Sound *s = new sf::Sound;
    s->setBuffer(*sound);
    s->play();
    playing_sounds.push_back(s);
}
