#pragma once

#include <iostream>
#include <math.h>

#include "scene.hpp"
#include "textureLoader.hpp"
#include "soundPlayer.hpp"

typedef int OutCode;

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)

// ASSUME THAT xmax, xmin, ymax and ymin are global constants.

extern double xmin;
extern double xmax;

extern double ymin;
extern double ymax;

inline OutCode ComputeOutCode(double x, double y)
{
    OutCode code;

    code = INSIDE;          // initialised as being inside of [[clip window]]

    if (x < xmin)           // to the left of clip window
        code |= LEFT;
    else if (x > xmax)      // to the right of clip window
        code |= RIGHT;
    if (y < ymin)           // below the clip window
        code |= BOTTOM;
    else if (y > ymax)      // above the clip window
        code |= TOP;

    return code;
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
// diagonal from (xmin, ymin) to (xmax, ymax).
inline bool CohenSutherlandLineClipAndDraw(double x0, double y0, double x1, double y1)
{
    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    OutCode outcode0 = ComputeOutCode(x0, y0);
    OutCode outcode1 = ComputeOutCode(x1, y1);
    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
            accept = true;
            break;
        } else if (outcode0 & outcode1) { // Bitwise AND is not 0. Trivially reject and get out of loop
            break;
        } else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            double x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

            // Now find the intersection point;
            // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
            if (outcodeOut & TOP) {           // point is above the clip rectangle
                x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
                y = ymax;
            } else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
                x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                y = ymin;
            } else if (outcodeOut & RIGHT) {  // point is to the right of clip rectangle
                y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                x = xmax;
            } else if (outcodeOut & LEFT) {   // point is to the left of clip rectangle
                y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
                x = xmin;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = ComputeOutCode(x0, y0);
            } else {
                x1 = x;
                y1 = y;
                outcode1 = ComputeOutCode(x1, y1);
            }
        }
    }
    return accept;
}

class Actor;
class Player;
class Spell;
class Room;

class Game : public Scene
{
    sf::Vector2f anchor;

    sf::View view;
    sf::View guiView;

    sf::Vector2f target;
    double startMag;
    int amount;

    bool woop;

    sf::Sprite gui;
public:
    std::vector<std::shared_ptr<Actor>> actors;
    std::shared_ptr<Player> player;
    std::vector<Spell> spells;
    Room *room;
    Room *nextRoom;
    bool transitioning;
    int dir;
    int cord;

    sf::Music music;

    Game();
    virtual ~Game() {
        music.stop();
    };
    
    virtual Scene *update();
    virtual void render(sf::RenderWindow &window);
    virtual void handleEvent(sf::Event event);

    void castSpell(std::shared_ptr<Actor> actor);

    void transition(int dir);

    void shake(int amount);
};

const int w = 13;
const int h = 9;

class Actor : public std::enable_shared_from_this<Actor> {
protected:
    sf::Sprite sprite;
    sf::Sprite poof;
    std::map<std::string, std::vector<sf::IntRect> > animations;
    int ft,frame;
    std::string idle;
    std::string curr;

    Game *game;

    bool remove;

    int frameLength;
    double speed;
public:
    bool touchy;
    bool fly;
    int hp;

    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Vector2f kb;

    Actor(Game *g) {
        ft = frame = 0;
        idle = curr = "idle";
        hp = 1;
        pos.x = 40;
        pos.y = 40;

        frameLength = 12;

        game = g;

        touchy = false;

        poof = TextureLoader::getInstance()->getSprite("poof.png");
        poof.setOrigin(16,16);

        animations["poof"].emplace_back(0,0,32,32);
        animations["poof"].emplace_back(32,0,32,32);
        animations["poof"].emplace_back(64,0,32,32);

        remove = false;
        fly = false;
    }

    bool operator < (const Actor& actor) const
    {
        return (pos.y < actor.pos.y);
    }

    void animUpdate() {
        ft++;
        if (ft > frameLength) {
            ft = 0;
            frame++;
            if (frame >= animations[curr].size()) {
                frame = 0;
                curr = idle;
            }
        }
    }

    virtual void update() {
        vel = sf::Vector2f(0,0);

        double mag = sqrt(kb.x * kb.x + kb.y * kb.y);
        if (mag > 0.3) {
            kb.x -= kb.x / mag * 0.3;
            kb.y -= kb.y / mag * 0.3;
        } else {
            kb = sf::Vector2f(0,0);
        }

        animUpdate();
    }

    void step(bool stepX) {
        if (kb != sf::Vector2f(0,0)) {
            if (stepX) {
                pos.x += kb.x;
            } else {
                pos.y += kb.y;
            }
        } else {
            if (stepX) {
                pos.x += vel.x;
            } else {
                pos.y += vel.y;
            }
        }
    }

    void playAnimation(std::string anim) {
        if (curr != anim) {
            ft = 0;
            frame = 0;
            curr = anim;
        }
    }

    void setIdle(std::string anim) {
        idle = anim;
        playAnimation(anim);
    }

    sf::Vector2f &getPosRef() {
        return pos;
    }

    virtual void deadUpdate() {
        frameLength = 6;
        playAnimation("poof");
        animUpdate();
        if (curr != "poof") {
            remove = true;
        }
    }

    virtual void render(sf::RenderWindow &window) {
        if (dead()) {
            poof.setTextureRect(animations[curr][frame]);
            poof.setPosition(pos);
            window.draw(poof);
        } else {
            if (vel.x > 0) {
                sprite.setScale(1,1);
            } else if (vel.x < 0) {
                sprite.setScale(-1,1);
            }

            sprite.setTextureRect(animations[curr][frame]);
            sprite.setPosition(pos);
            window.draw(sprite);
        }
    }

    bool destroy() {
        return remove;
    }

    bool dead() {
        return hp <= 0;
    }

    virtual void takeDamage(int damage,sf::Vector2f p) {
        if (hp > 0) {
            hp -= damage;
            if (hp <= 0) {
                SoundPlayer::getInstance()->playSound("monsterDyingSFX.wav");
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
};

struct PointerCompare {
  bool operator()(const std::shared_ptr<Actor> l, const std::shared_ptr<Actor> r) {
    return *l < *r;
  }
};

class Player : public Actor {
public:

    bool breaking;
    int breakCount;
    int nodamage;

    Player(Game *g) : Actor(g) {
        sprite = TextureLoader::getInstance()->getSprite("kas.png");
        sprite.setOrigin(16,16);

        poof = TextureLoader::getInstance()->getSprite("smoke.png");
        poof.setOrigin(16,16);

        animations["idle"].emplace_back(0,0,32,32);
        animations["idle"].emplace_back(32,0,32,32);
        animations["idle"].emplace_back(64,0,32,32);
        animations["idle"].emplace_back(96,0,32,32);

        animations["walk"].emplace_back(0,32,32,32);
        animations["walk"].emplace_back(32,32,32,32);
        animations["walk"].emplace_back(64,32,32,32);
        animations["walk"].emplace_back(96,32,32,32);

        animations["break"].emplace_back(0,64,32,32);
        animations["break"].emplace_back(32,64,32,32);
        animations["break"].emplace_back(64,64,32,32);
        animations["break"].emplace_back(96,64,32,32);
        animations["break"].emplace_back(0,96,32,32);
        animations["break"].emplace_back(32,96,32,32);
        animations["break"].emplace_back(64,96,32,32);
        animations["break"].emplace_back(96,96,32,32);

        animations["leave"].emplace_back(0 ,7*32,32,32);
        animations["leave"].emplace_back(32,7*32,32,32);
        animations["leave"].emplace_back(64,7*32,32,32);
        animations["leave"].emplace_back(96,7*32,32,32);

        animations["enter"].emplace_back(0 ,8*32,32,32);
        animations["enter"].emplace_back(32,8*32,32,32);
        animations["enter"].emplace_back(64,8*32,32,32);
        animations["enter"].emplace_back(96,8*32,32,32);

        animations["dead"].emplace_back(64,4*32,32,32);
        animations["dead"].emplace_back(92,4*32,32,32);
        animations["dead"].emplace_back(0 ,6*32,32,32);

        animations["skel"].emplace_back(32,6*32,32,32);
        animations["skel"].emplace_back(64,6*32,32,32);

        animations["none"].emplace_back(0,0,0,0);

        speed = 1.6;
        hp = 3;
        nodamage = 0;

        breakCount = 0;
        breaking = false;
    }

    virtual void deadUpdate() {
        frameLength = 6;
        animUpdate();
    }

    virtual void render(sf::RenderWindow &window) {
        if (dead()) {
            sprite.setTextureRect(animations[curr][frame]);
            sprite.setPosition(pos);
            window.draw(sprite);

            if (curr == "dead") {
                poof.setTextureRect(animations["poof"][frame]);
                poof.setPosition(pos);
                window.draw(poof);
            }
        } else {
            if (vel.x > 0) {
                sprite.setScale(1,1);
            } else if (vel.x < 0) {
                sprite.setScale(-1,1);
            }

            sprite.setTextureRect(animations[curr][frame]);
            sprite.setPosition(pos);
            window.draw(sprite);
        }
    }

    virtual void update();

    virtual void takeDamage(int d,sf::Vector2f p) {
        if (nodamage <= 0 && hp > 0) {
            hp -= d;
            nodamage = 20;

            sf::Vector2f dif = pos - p;
            double mag = sqrt(dif.x * dif.x + dif.y * dif.y);

            dif.x /= mag;
            dif.y /= mag;

            kb = dif;

            kb.x *= 3;
            kb.y *= 3;

            SoundPlayer::getInstance()->playSound("getHurtSFX.wav");

            if (hp <= 0) {
                setIdle("skel");
                playAnimation("dead");

                game->music.openFromFile("sfx/GameOverSong.wav");
                game->music.play();
            }
        }
    }
};

class Enemy : public Actor {
protected:
    sf::Vector2f target;
    enum State
    {
        idle,
        moving,
    };
    State state;
public:

    Enemy(Game *g) : Actor(g) {
        state = idle;
        speed = 1;
    }

    virtual void update() ;

    void moveTo(sf::Vector2f t) {
        state = moving;
        target = t;
        if (target.x < 32 + 16) {
            target.x = 32 + 16;
        } else if (target.x > (w-1)*32 - 16) {
            target.x = (w-1)*32 - 16;
        }

        if (target.y < 32 + 16) {
            target.y = 32 + 16;
        } else if (target.y > (h-1)*32 - 16) {
            target.y = (h-1)*32 - 16;
        }
    }
};

class Bat : public Enemy {
    int sleep;
public:

    Bat(Game *g) : Enemy(g) {
        sprite = TextureLoader::getInstance()->getSprite("bat.png");
        sprite.setOrigin(16,16);

        animations["idle"].emplace_back(0,0,32,32);
        animations["idle"].emplace_back(32,0,32,32);
        animations["idle"].emplace_back(64,0,32,32);
        animations["idle"].emplace_back(96,0,32,32);

        sleep = rand()%60+20;

        fly = true;
    }

    virtual void update() {
        animUpdate();
        if (sleep > 0) {
            sleep--;
            if (sleep == 0) {
                target = game->player->pos+sf::Vector2f(rand()%40-20,rand()%40-20);
                sf::Vector2f dif = target - pos;
                double mag = sqrt(dif.x*dif.x + dif.y*dif.y);
                dif.x /= mag;
                dif.y /= mag;

                mag += 60+rand()%50;

                dif.x *= mag;
                dif.y *= mag;

                target = pos + dif;
            }
        } else {
            if (target == pos) {
                sleep = rand()%60+20;
            } else {
                sf::Vector2f dif = target - pos;
                double mag = sqrt(dif.x*dif.x + dif.y*dif.y);
                if (mag < 3) {
                    pos = target;
                } else {
                    dif.x /= mag;
                    dif.y /= mag;

                    dif.x *= 3;
                    dif.y *= 3;

                    pos += dif;
                }

                dif = game->player->pos - pos;
                mag = sqrt(dif.x * dif.x + dif.y * dif.y);

                if (mag <= 16) {
                    game->player->takeDamage(1,pos);
                }
            }
        }
    }
};

class Mummy : public Enemy {
    int sleep;
    enum Goal
    {
        attacking,
        running,
    };
    Goal goal;
public:

    Mummy(Game *g) : Enemy(g) {
        sprite = TextureLoader::getInstance()->getSprite("mummy.png");
        sprite.setOrigin(16,16);

        animations["idle"].emplace_back(0,0,32,32);
        animations["idle"].emplace_back(32,0,32,32);
        animations["idle"].emplace_back(64,0,32,32);
        animations["idle"].emplace_back(96,0,32,32);

        sleep = 10;

        hp = 3;

        goal = attacking;

        touchy = true;
    }

    virtual void update() {
        Enemy::update();
        if (sleep <= 0) {
            if (goal == attacking) {
                for (int i=0;i<game->actors.size();i++) {
                    if (game->actors[i].get() != this && game->actors[i]->touchy) {
                        sf::Vector2f dif = game->actors[i]->pos - pos;
                        double mag = sqrt(dif.x * dif.x + dif.y * dif.y);
                        if (mag <= 16) {
                            dif.x /= mag;
                            dif.y /= mag;

                            dif.x *= 10;
                            dif.y *= 10;

                            moveTo(pos - dif);
                            goal = running;
                            return;
                        }
                    }
                }

                moveTo(game->player->pos);

                sf::Vector2f dif = game->player->pos - pos;
                double mag = sqrt(dif.x * dif.x + dif.y * dif.y);

                if (mag <= 16) {
                    goal = running;

                    dif.x /= mag;
                    dif.y /= mag;

                    dif.x *= -40;
                    dif.y *= -40;

                    moveTo(pos+dif);

                    game->player->takeDamage(1,pos);
                }

            } else {
                if (target == pos) {
                    goal = attacking;
                    // sleep = 20;
                }
            }
        } else {
            sleep -= 1;
        }
    }
};

class Doot : public Enemy {
    int runs;
    int cd;
    enum Goal
    {
        attacking,
        running,
    };
    Goal goal;
public:

    Doot(Game *g) : Enemy(g) {
        sprite = TextureLoader::getInstance()->getSprite("doot.png");
        sprite.setOrigin(24,32);

        animations["idle"].emplace_back(0,0,48,48);
        animations["idle"].emplace_back(48,0,48,48);
        animations["idle"].emplace_back(0,0,48,48);
        animations["idle"].emplace_back(96,0,48,48);
        animations["idle"].emplace_back(144,0,48,48);
        animations["idle"].emplace_back(96,0,48,48);

        animations["walk"].emplace_back(0,48,48,48);
        animations["walk"].emplace_back(48,48,48,48);
        animations["walk"].emplace_back(96,48,48,48);
        animations["walk"].emplace_back(144,48,48,48);

        speed = 1.4;

        goal = running;

        runs = rand() % 3 + 1;

        sf::Vector2f dest = sf::Vector2f(rand()%(32*10)+48,rand()%(32*6)+48);

        moveTo(dest);
    }

    virtual void update() {
        Enemy::update();
        if (goal == running) {
            if (target == pos) {
                runs--;
                if (runs == 0) {
                    goal = attacking;
                    playAnimation("idle");
                    game->castSpell(shared_from_this());
                    cd = 36;
                } else {
                    sf::Vector2f dest = game->player->pos + sf::Vector2f(rand()%200-100,rand()%200-100);
                    moveTo(dest);
                }
            }
        } else {
            cd--;
            if (cd == 0) {
                game->castSpell(shared_from_this());
            }
            if (curr == "walk") {
                goal = running;
                runs = rand() % 3 + 1;

                sf::Vector2f dest = sf::Vector2f(rand()%(32*10)+48,rand()%(32*6)+48);
                moveTo(dest);
            }
        }

        if (vel != sf::Vector2f(0,0)) {
            setIdle("walk");
        }
    }
};

struct Node {
    int distance;
    Node *parent;
    std::vector<Node *> adjancent;
    sf::Vector2i point;
};

class Room {
    int values[w][h];
    std::vector<Node> nodes;
    sf::Sprite sprite;
    Game *game;

public:

    sf::Vector2f pos;

    void render(sf::RenderWindow &window) {
        for (int x=0;x<w;x++) {
            for (int y=0;y<h;y++) {
                sprite.setTextureRect(sf::IntRect(32*(values[x][y]%8),32*(values[x][y]/8),32,32));
                sprite.setPosition(pos.x + x*32, pos.y + y*32);
                window.draw(sprite);
            }
        } 
    }

    Room(Game *g) {
        game = g;

        sprite = TextureLoader::getInstance()->getSprite("tiles.png");

        for (int x=0;x<w;x++) {
            for (int y=0;y<h;y++) {
                values[x][y] = 3;

                if (x == 0) {
                    values[x][y] = 9;
                }

                if (y == 0) {
                    values[x][y] = 8;
                }

                if (x == w - 1) {
                    values[x][y] = 10;
                }

                if (y == h - 1) {
                    values[x][y] = 11;
                }

                if (x == 0 && y == 0) {
                    values[x][y] = 4;
                }

                if (x == w - 1 && y == 0) {
                    values[x][y] = 5;
                }

                if (x == 0 && y == h - 1) {
                    values[x][y] = 6;
                }

                if (x == w - 1 && y == h - 1) {
                    values[x][y] = 7;
                }

                if (x == (w-1)/2 && y == 0) {
                    values[x][y] = 12;
                }

                if (y == (h-1)/2 && x == 0) {
                    values[x][y] = 13;
                }

                if (x == (w-1)/2 && y == h - 1) {
                    values[x][y] = 15;
                }

                if (y == (h-1)/2 && x == w - 1) {
                    values[x][y] = 14;
                }

                nodes.emplace_back();
                nodes.back().point = sf::Vector2i(x,y);
            }
        }

        // values[5][3] = 16;
        // values[6][3] = 20;
        // values[7][3] = 17;

        // values[5][4] = 21;
        // values[6][4] = 1;
        // values[7][4] = 22;

        // values[5][5] = 18;
        // values[6][5] = 23;
        // values[7][5] = 19;

        for (int i=0;i<nodes.size();i++) {
            for (int t=i+1;t<nodes.size();t++) {
                sf::Vector2i dif = nodes[i].point - nodes[t].point;
                if ((fabs(dif.x) == 1 && dif.y == 0) || (fabs(dif.y) == 1 && dif.x == 0)) {
                    nodes[i].adjancent.push_back(&nodes[t]);
                    nodes[t].adjancent.push_back(&nodes[i]);
                }
            }
        }
    }

    void update(Actor &actor,bool stepX) {
        if (&actor == game->player.get() && game->actors.size() == 1) {
            if (stepX) {
                int y = 4;

                for (int x=0;x<w;x+=w-1) {
                    if (actor.pos.x-16 >= x*32+32) {
                        continue;
                    }

                    if (actor.pos.x+16 <= x*32) {
                        continue;
                    }

                    if (actor.pos.y+16 <= y*32) {
                        continue;
                    }

                    if (actor.pos.y >= y*32+32) {
                        continue;
                    }

                    game->transition(2 - x/6);
                    return;
                }
            } else {
                int x = 6;
                
                for (int y=0;y<h;y+=h-1) {
                    if (actor.pos.x-16 >= x*32+32) {
                        continue;
                    }

                    if (actor.pos.x+16 <= x*32) {
                        continue;
                    }

                    if (actor.pos.y+16 <= y*32) {
                        continue;
                    }

                    if (actor.pos.y >= y*32+32) {
                        continue;
                    }

                    game->transition(2 - y/4 + 1);
                    return;
                }
            }
        }

        for (int x=0;x<w;x++) {
            for (int y=0;y<h;y++) {
                if (values[x][y] != 3) {
                    if (actor.pos.x-16 >= x*32+32) {
                        continue;
                    }

                    if (actor.pos.x+16 <= x*32) {
                        continue;
                    }

                    if (actor.pos.y+16 <= y*32) {
                        continue;
                    }

                    if (actor.pos.y >= y*32+32) {
                        continue;
                    }

                    sf::Vector2f dif = actor.pos + sf::Vector2f(0,8) - sf::Vector2f(x*32+16,y*32+16);

                    if (stepX) {
                        if (dif.x > 0) {
                            actor.pos.x = x*32+48;
                        } else {
                            actor.pos.x = x*32-16;
                        }
                    } else {
                        if (dif.y > 0) {
                            actor.pos.y = y*32+32;
                        } else {
                            actor.pos.y = y*32-16;
                        }
                    }
                }
            }
        }
    }

    sf::Vector2f findPath(sf::Vector2f a, sf::Vector2f b) {
        double offset = 15.9999;

        if (values[int((b.x+offset)/32)][int(b.y/32)] != 3) {
            return sf::Vector2f(-1000,-1000);
        }

        if (values[int((b.x-offset)/32)][int(b.y/32)] != 3) {
            return sf::Vector2f(-1000,-1000);
        }

        if (values[int((b.x+offset)/32)][int((b.y+offset)/32)] != 3) {
            return sf::Vector2f(-1000,-1000);
        }

        if (values[int((b.x-offset)/32)][int((b.y+offset)/32)] != 3) {
            return sf::Vector2f(-1000,-1000);
        }

        sf::Vector2i tileA = sf::Vector2i(a.x/32,a.y/32);
        sf::Vector2i tileB = sf::Vector2i(b.x/32,b.y/32);

        std::vector<Node *> Q;

        for (int i=0;i<nodes.size();i++) {
            nodes[i].distance = -1;
            nodes[i].parent = nullptr;
            if (nodes[i].point == tileA) {
                Q.push_back(&nodes[i]);
                nodes[i].distance = 0;
            }
        }

        bool found = false;

        Node *final = nullptr;

        if (tileA == tileB) {
            found = true;
            final = Q[0];
        }
        
        while (Q.size() != 0 && found == false) {
            Node *current = Q[0];

            for (int i=0;i<current->adjancent.size();i++) {
                Node *adj = current->adjancent[i];
                if (adj->distance == -1 && values[adj->point.x][adj->point.y] == 3) {
                    current->adjancent[i]->distance = 0;
                    current->adjancent[i]->parent = current;

                    if (current->adjancent[i]->point == tileB) {
                        final = current->adjancent[i];
                        found = true;
                        break;
                    }

                    Q.push_back(current->adjancent[i]);
                }
            }

            Q.erase(Q.begin());
        }

        std::vector<sf::Vector2f> points;
        points.emplace_back(a.x,a.y);
        points.emplace_back(b.x,b.y);
        final = final->parent;

        while (final != nullptr) {
            points.insert(points.begin()+1,sf::Vector2f(final->point.x*32+16,final->point.y*32+16));
            final = final->parent;
        }

        int checkPoint = 0;
        int currentPoint = 1;

        while (currentPoint+1 < points.size()) {
            bool walk = true;

            int sx = fmin(points[currentPoint+1].x/32,points[checkPoint].x/32);
            int ex = fmax(points[currentPoint+1].x/32,points[checkPoint].x/32);

            int sy = fmin(points[currentPoint+1].y/32,points[checkPoint].y/32);
            int ey = fmax(points[currentPoint+1].y/32,points[checkPoint].y/32);

            for (int x = sx-1; x <= ex+1; x++) {
                for (int y = sy-1; y <= ey+1; y++) {
                    if (values[x][y] == 3) {
                        continue;
                    }

                    xmin = x*32;
                    xmax = xmin + 32;

                    ymin = y*32;
                    ymax = ymin + 32;

                    if (CohenSutherlandLineClipAndDraw(points[checkPoint].x-offset,points[checkPoint].y,
                                                       points[currentPoint+1].x-offset,points[currentPoint+1].y)) {

                        walk = false;
                        break;
                    }

                    if (CohenSutherlandLineClipAndDraw(points[checkPoint].x+offset,points[checkPoint].y,
                                                       points[currentPoint+1].x+offset,points[currentPoint+1].y)) {

                        walk = false;
                        break;
                    }

                    if (CohenSutherlandLineClipAndDraw(points[checkPoint].x-offset,points[checkPoint].y+offset,
                                                       points[currentPoint+1].x-offset,points[currentPoint+1].y+offset)) {

                        walk = false;
                        break;
                    }

                    if (CohenSutherlandLineClipAndDraw(points[checkPoint].x+offset,points[checkPoint].y+offset,
                                                       points[currentPoint+1].x+offset,points[currentPoint+1].y+offset)) {

                        walk = false;
                        break;
                    }
                }

                if (!walk) {
                    break;
                }
            }

            if (walk) {
                 points.erase(points.begin()+currentPoint);
            } else {
                checkPoint = currentPoint;
                currentPoint++;
            }
        }

        for (int i=0;i<points.size()-1;i++) {
            if (points[i] == points[i+1]) {
                points.erase(points.begin()+i);
                i--;
            }
        }

        return points[1];
    }
};

class Spell {
    int r;
    int dir;
    int max;
    sf::CircleShape shape;
public:
    std::shared_ptr<Actor> anchor;

    Spell(std::shared_ptr<Actor> anc) {
        r = 0;
        dir = 1;
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(2);
        shape.setPointCount(60);
        anchor = anc;
        max = 80;
    }

    void update() {
        r += dir;
        if (r >= max) {
            dir = -1;
            r = max;
        } else if (r <= 0) {
            kill();
        }
    }

    void render(sf::RenderWindow &window) {
        shape.setRadius(r);
        shape.setOrigin(r,r);
        shape.setPosition(anchor->pos);
        window.draw(shape);
    }

    int getDir() {
        return dir;
    }

    int getR() {
        return r;
    }

    void kill() {
        r = 0;
        dir = 0;
    }

    void explode(Actor &actor) {
        sf::Vector2f dif = actor.pos - anchor->pos;
        double v = fabs(sqrt(dif.x * dif.x + dif.y * dif.y) - r);
        if (v < 16) {
            actor.takeDamage(1,anchor->pos);
        }
    }
};