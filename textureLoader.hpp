//
//  TextureLoader.h
//  GameClient
//
//  Created by Andrew Jones on 9/28/14.
//  Copyright (c) 2014 ThreeLives. All rights reserved.
//

#ifndef __GameClient__TextureLoader__
#define __GameClient__TextureLoader__

#include <SFML/Graphics.hpp>

class TextureLoader {
private:

    std::map<std::string, sf::Texture*> textures;

    static TextureLoader *instance;

    TextureLoader(){};
    TextureLoader(TextureLoader const&){};

public:

    static TextureLoader *getInstance();
    sf::Texture *getTexture(std::string file);
    sf::Sprite getSprite(std::string file);
};

#endif /* defined(__GameClient__TextureLoader__) */
