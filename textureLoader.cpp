//
//  TextureLoader.cpp
//  GameClient
//
//  Created by Andrew Jones on 9/28/14.
//  Copyright (c) 2014 ThreeLives. All rights reserved.
//

#include "textureLoader.hpp"
#ifdef _WIN32
#else
//#include "ResourcePath.hpp"
#endif

TextureLoader *TextureLoader::instance = NULL;

TextureLoader *TextureLoader::getInstance() {
    if (!instance) {
        instance = new TextureLoader();
    }
    return instance;
}

sf::Texture *TextureLoader::getTexture(std::string file) {
    sf::Texture *tex = textures[file];

    if (tex == NULL) {
        tex = new sf::Texture();
        tex->loadFromFile("imgs/"+file);
        textures[file] = tex;
    }

    return tex;
}

sf::Sprite TextureLoader::getSprite(std::string file) {
    sf::Sprite sprite;
    sprite.setTexture(*getTexture(file));
    return sprite;
}
