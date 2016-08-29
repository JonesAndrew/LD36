#include "game.hpp"

#include <iostream>
#include <algorithm>

double xmin = 0;
double xmax = 0;

double ymin = 0;
double ymax = 0;

void Actor::takeDamage(int damage,sf::Vector2f p) {
    if (hp > 0) {
        hp -= damage;
        if (hp <= 0) {
            SoundPlayer::getInstance()->playSound("monsterDyingSFX.wav");
            if (rand() % 8 == 0) {
                game->actors.push_back(std::make_shared<Heart>(game));
                game->actors.back()->pos = pos;
            }
        } else {
            SoundPlayer::getInstance()->playSound("monsterHurtSFX.wav");

            sf::Vector2f dif = pos - p;
            double mag = sqrt(dif.x * dif.x + dif.y * dif.y);

            dif.x /= mag;
            dif.y /= mag;

            kb = dif;

            kb.x *= 3;
            kb.y *= 3;
        }
    }
}

void Game::handleEvent(sf::Event event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Space && !transitioning && !player->dead() && !player->breaking) {
            player->breakCount += 40;
            spells.emplace_back(player);

            cord++;
            if (cord > 4) {
                cord = 1;
            }

            SoundPlayer::getInstance()->playSound("twangsitar"+std::to_string(cord)+"SFX.wav");
        }
    }
}

void Player::update() {
        Actor::update();

        if (nodamage > 0) {
            nodamage--;
        }

        if (breakCount > 0) {
            if (breakCount > 104) {
                breaking = true;
                playAnimation("break");
                breakCount = fmin(104,8*12);

                SoundPlayer::getInstance()->clearSounds();
                SoundPlayer::getInstance()->playSound("stringBreakSFX.wav");

                for (int i=0;i<game->spells.size();i++) {
                    if (game->spells[i].anchor.get() == this) {
                        game->spells.erase(game->spells.begin()+i);
                        i--;
                    }
                }
            }
            breakCount -= 1;
        } else {
            breakCount = 0;
            if (curr != "break") {
                breaking = false;
            }
        }

        if (fallCount > 0) {
            sf::Vector2f o = sprite.getOrigin();

            sf::Vector2f dif = o-sf::Vector2f(16,16);
            double mag = sqrt(dif.x*dif.x + dif.y*dif.y);

            dif.x /= mag;
            dif.y /= mag;

            mag = (48-fallCount)*1.5+4;

            dif.x *= mag;
            dif.y *= mag;

            sprite.setOrigin(sf::Vector2f(16,16)+dif);

            fallCount--;
            if (fallCount == 0) {
                frameLength = 12;
                sprite.setOrigin(16,16);
                if (dif.x != 0 && sprite.getScale().x == -1) {
                    takeDamage(1,pos+dif);
                } else {
                    takeDamage(1,pos-dif);
                }

                if (dead()) {
                    return;
                }
            }
        }

        if (!breaking && !game->transitioning && fallCount <= 0) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                vel += sf::Vector2f(-1,0);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                vel += sf::Vector2f(1,0);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                vel += sf::Vector2f(0,1);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                vel += sf::Vector2f(0,-1);
            }
            if (vel != sf::Vector2f(0,0)) {
                double mag = sqrt(vel.x * vel.x + vel.y * vel.y);
                vel.x /= mag;
                vel.y /= mag;

                vel.x *= speed;
                vel.y *= speed;

                setIdle("walk");
            } else {
                setIdle("idle");
            }
        }
    }

void Enemy::update() {
    Actor::update();
    if (state == moving) {
        if (target == pos) {
            state = idle;
        } else {
            sf::Vector2f res = game->room->findPath(pos,target);
            if (res == sf::Vector2f(-1000,-1000)) {
                state = idle;
                vel = sf::Vector2f(0,0);
                target = pos;
                return;
            }
            sf::Vector2f dif = res - pos;
            double mag = sqrt(dif.x * dif.x + dif.y * dif.y);
            if (mag > speed) {
                dif.x /= mag;
                dif.y /= mag;

                dif.x *= speed;
                dif.y *= speed;
                vel = dif;
            } else {
                pos = res;
                vel = sf::Vector2f(0,0);
            }
        }
    }
}

void Game::transition(int d) {
    if (transitioning == false) {
        amount = 0;
        transitioning = true;

        for (int i=0;i<actors.size();i++) {
            if (actors[i] != player) {
                actors.erase(actors.begin()+i);
                i--;
            }
        }

        for (int i=0;i<spells.size();) {
            spells.erase(spells.begin());
        }

        target = view.getCenter();

        if (d == 0) {
            roomPos += sf::Vector2i(1,0);
            nextRoom = rooms[roomPos.x][roomPos.y];
            target.x += 480;
            startMag = 480;
            nextRoom->pos.x = 480;
        } else if (d == 1) {
            roomPos += sf::Vector2i(0,1);
            nextRoom = rooms[roomPos.x][roomPos.y];
            target.y += 480; 
            startMag = 480;
            nextRoom->pos.y = 480;
        } else if (d == 2) {
            roomPos += sf::Vector2i(-1,0);
            nextRoom = rooms[roomPos.x][roomPos.y];
            target.x -= 480;
            startMag = 480;
            nextRoom->pos.x = -480;
        } else {
            roomPos += sf::Vector2i(0,-1);
            nextRoom = rooms[roomPos.x][roomPos.y];
            target.y -= 480;
            startMag = 480;
            nextRoom->pos.y = -480;
        }

        dir = d;
    }
}

void Game::shake(int a) {
    amount = a;
}

Game::Game() {
    cord = 1;

    gui = TextureLoader::getInstance()->getSprite("heart.png");

    title = TextureLoader::getInstance()->getSprite("KasText.png");

    music.setLoop(true);
    music.openFromFile("sfx/coolingoffhotchillymeal.wav");
    music.play();

    srand(time(0));

    view.setSize(480,300);
    view.setCenter((w/2.0)*32,(h/2.0)*32);

    guiView.setSize(480,320);
    guiView.setCenter((w/2.0)*32,(h/2.0)*32);

    target = sf::Vector2f((w/2.0)*32,(h/2.0)*32);

    player = std::make_shared<Player>(this);
    actors.push_back(player);
    actors.back()->pos = sf::Vector2f(48,7*32+16);
    // actors.push_back(std::make_shared<Mummy>(this));
    // actors.back()->pos = sf::Vector2f(150,200);

    // actors.push_back(std::make_shared<Doot>(this));
    // actors.back()->pos = sf::Vector2f(200,200);
    // actors.push_back(std::make_shared<Doot>(this));
    // actors.back()->pos = sf::Vector2f(250,200);

    // actors.push_back(std::make_shared<Doot>(this));
    // actors.back()->pos = sf::Vector2f(200,64);
    // actors.push_back(std::make_shared<Doot>(this));
    // actors.back()->pos = sf::Vector2f(250,64);

    SoundPlayer::getInstance();

    for (int i=0;i<2;i++) {
        rooms[0][i+1] = new Room(this,"13");
    }

    rooms[1][2] = new Room(this,"TitleRoom");
    rooms[0][0] = new Room(this,"bossroom");
    rooms[1][3] = new Room(this,"14");
    rooms[0][3] = new Room(this,"12");

    roomPos = sf::Vector2i(1,2);
    room = rooms[1][2];
    nextRoom = nullptr;

    transitioning = false;
    amount = 0;

    woop = true;
}

void Game::castSpell(std::shared_ptr<Actor> actor) {
    spells.emplace_back(actor);
}

Scene *Game::update() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
        if (woop == false) {
            Scene *next = new Game();
            return next;
        }
        woop = true;
    } else {
        woop = false;
    }

    if (player->dead()) {
        sf::Vector2f dif = player->pos - sf::Vector2f(0,4) - view.getCenter();
        dif.x /= 50;
        dif.y /= 50;
        view.move(dif);
        view.zoom(0.99);
        view.rotate(1);
    }
    else if (amount != 0) {
        amount--;
        if (amount < 0) {
            amount = 0;
        }

        float ang = rand() % 1000;

        ang = ang * M_PI / 500;

        view.setCenter((w/2.0)*32+cos(ang)*amount,(h/2.0)*32+sin(ang)*amount);
    } else if (view.getCenter() != target) {
        sf::Vector2f dif = target - view.getCenter();
        
        double mag = sqrt(dif.x * dif.x + dif.y * dif.y);

        if (mag > startMag-64) {
            if (dir == 0) {
                player->pos = sf::Vector2f(11*32 + 16 + (startMag-mag)/2, 4*32 + 16);
            } else if (dir == 1) {
                player->pos = sf::Vector2f(32*6 + 16, 7*32 + 16 + (startMag-mag)/2);
            } else if (dir == 2) {
                player->pos = sf::Vector2f(48 - (startMag-mag)/2, 4*32 + 16);
            } else {
                player->pos = sf::Vector2f(32*6 + 16, 32-(startMag-mag)/4);
            }
        } else {
            if (dir == 0) {
                if (mag == startMag-64) {
                    player->pos = sf::Vector2f(12*32 + 16, 4*32 + 16);
                    player->setIdle("none");
                    player->playAnimation("leave");
                }
                if (mag <= 4*12*4+64) {
                    player->pos = sf::Vector2f(16 + 480, 4*32 + 16); //+ 16 * (4*12*4 - mag) / 4*12*4);
                    player->playAnimation("enter");
                } 
                if (mag <= 64) {
                    player->pos = sf::Vector2f(32 - mag/2 + 16 + 480,  4*32 + 16);
                    player->setIdle("walk");
                }
            } else if (dir == 1) {
                if (mag == startMag-64) {
                    player->pos = sf::Vector2f(32*6 + 16, 8*32 + 16);
                    player->setIdle("none");
                    player->playAnimation("leave");
                }
                if (mag <= 4*12*4+64) {
                    player->pos = sf::Vector2f(32*6 + 16, 16 + 480); //+ 16 * (4*12*4 - mag) / 4*12*4);
                    player->playAnimation("enter");
                } 
                if (mag <= 64) {
                    player->pos = sf::Vector2f(32*6 + 16, 480 + 16 + 16 - mag/4);
                    player->setIdle("walk");
                }
            } else if (dir == 2) {
                if (mag == startMag-64) {
                    player->pos = sf::Vector2f(16, 4*32 + 16);
                    player->setIdle("none");
                    player->playAnimation("leave");
                }
                if (mag <= 4*12*4+64) {
                    player->pos = sf::Vector2f(12*32 + 16 - 480, 4*32 + 16); //+ 16 * (4*12*4 - mag) / 4*12*4);
                    player->playAnimation("enter");
                } 
                if (mag <= 64) {
                    player->pos = sf::Vector2f(11*32 + 16 + (mag)/2 - 480,  4*32 + 16);
                    player->setIdle("walk");
                }
            } else {
                if (mag == startMag-64) {
                    player->pos = sf::Vector2f(32*6 + 16, 16);
                    player->setIdle("none");
                    player->playAnimation("leave");
                }
                if (mag <= 4*12*4+64) {
                    player->pos = sf::Vector2f(32*6 + 16, 8*32 + 16 - startMag); //+ 16 * (4*12*4 - mag) / 4*12*4);
                    player->playAnimation("enter");
                } 
                if (mag <= 64) {
                    player->pos = sf::Vector2f(32*6 + 16, 8*32 + 16 - 32 + mag/2 - startMag);
                    player->setIdle("walk");
                }
            }
        }

        if (mag <= 4) {
            view.setCenter((w/2.0)*32,(h/2.0)*32);
            target = view.getCenter();

            transitioning = false;

            if (dir == 0) {
                player->pos = sf::Vector2f(48, 4*32 + 16);
            } else if (dir == 1) {
                player->pos = sf::Vector2f(32*6 + 16, 32);
            } else if (dir == 2) {
                player->pos = sf::Vector2f(11*32 + 16, 4*32 + 16);
            } else {
                player->pos = sf::Vector2f(32*6 + 16, 7*32+16);
            }

            room = nextRoom;
            nextRoom = nullptr;

            room->pos = sf::Vector2f(0,0);

            int count=rand()%3+1;
            int bigCount = count + rand()%3;

            int c=0;

            if (!room->cleared) {
                if (room->name == "bossroom") {
                    actors.push_back(std::make_shared<Doot>(this));
                    actors.back()->pos = sf::Vector2f(48,32);

                    actors.push_back(std::make_shared<Doot>(this));
                    actors.back()->pos = sf::Vector2f(12*32+16,32);

                    actors.push_back(std::make_shared<Doot>(this));
                    actors.back()->pos = sf::Vector2f(48,7*32+16);

                    actors.push_back(std::make_shared<Doot>(this));
                    actors.back()->pos = sf::Vector2f(12*32+16,7*32+16);
                } else {
                    while (c<count) {
                        int x = rand() % w;
                        int y = rand() % h;

                        if (room->values[x][y] == 3 || room->values[x][y] == 2) {
                            sf::Vector2f p = sf::Vector2f(x*32+16,y*32+16);

                            sf::Vector2f dif = player->pos - p;

                            double mag = sqrt(dif.x*dif.x + dif.y*dif.y);

                            if (mag >= 64) {
                                actors.push_back(std::make_shared<Mummy>(this));
                                actors.back()->pos = p;
                                c++;
                            }
                        }
                    }
                    while (c<bigCount) {
                        int x = rand() % w;
                        int y = rand() % h;

                        sf::Vector2f p = sf::Vector2f(x*32+16,y*32+16);

                        sf::Vector2f dif = player->pos - p;

                        double mag = sqrt(dif.x*dif.x + dif.y*dif.y);

                        if (mag >= 64) {
                            actors.push_back(std::make_shared<Bat>(this));
                            actors.back()->pos = p;
                            c++;
                        }
                    }
                }
            }

        } else {
            dif.x /= mag;
            dif.y /= mag;

            dif.x *= 4;
            dif.y *= 4;

            view.move(dif);
        }
    }

    for (int i=0;i<actors.size();i++) {
        if (!actors[i]->dead()) {
            actors[i]->update();

            if (!transitioning)
                actors[i]->step(true);
            if (!transitioning && !actors[i]->fly)
                room->update(*actors[i],true);
            if (!transitioning)
                actors[i]->step(false);
            if (!transitioning && !actors[i]->fly)
                room->update(*actors[i],false);
            
        } else {
            actors[i]->deadUpdate();
        }
    }

    for (auto &spell : spells) {
        spell.update();
    }
    for (int i=0;i<spells.size();i++) {
        for (int t=i+1;t<spells.size();t++) {

            if (spells[i].anchor == spells[t].anchor && 
                abs(spells[i].getR() - spells[t].getR()) < 2 &&
                spells[i].getR() != 0 && spells[t].getR() != 0) {

                if (spells[i].anchor == player) {
                    shake(5);
                }

                for (int a=0;a<actors.size();a++) {
                    if (actors[a]->heart == false) {
                        if ((spells[i].anchor == player && actors[a] != player) ||
                            (spells[i].anchor != player && actors[a] == player)) {
                            spells[i].explode(*actors[a]);
                        }
                    }
                }

                spells[i].kill();
                spells[t].kill();
                break;
            }
        }
    }
    for (int i=0;i<spells.size();i++) {
        if (spells[i].getDir() == 0) {
            spells.erase(spells.begin()+i);
            i--;
        }
    }

    for (int i=0;i<actors.size();i++) {
        if (actors[i]->destroy()) {
            actors.erase(actors.begin()+i);
            i--;
        }
    }
    return NULL;
}

void Game::render(sf::RenderWindow &window) {
    window.setView(view);

    room->render(window);
    if (nextRoom != nullptr) {
        nextRoom->render(window);
    }

    std::sort(actors.begin(),actors.end(),PointerCompare());

    for (int i=0;i<actors.size();i++) {
        actors[i]->render(window);
    }

    for (auto &spell : spells) {
        spell.render(window);
    }

    if (room->name ==  "TitleRoom") {
        title.setPosition(65,0);
        window.draw(title);
    }

    window.setView(guiView);

    for (int i=0;i<3;i++) {
        if (player->hp > i) {
            gui.setTextureRect(sf::IntRect(0,0,32,32));
        } else {
            gui.setTextureRect(sf::IntRect(32,0,32,32));
        }
        gui.setPosition(i*24 - 5, 280);
        window.draw(gui);
    }

    window.setView(window.getDefaultView());
}