#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class StudentWorld;

class Actor : public GraphObject{
public:
    Actor(StudentWorld* stud, int imageID, int startX, int startY, int dir = 0, int depth = 1, double size = 1.0);
    virtual ~Actor();
    virtual void doSomething()=0;       //similarities: some check isAlive, some don't
    virtual void bonk();
    virtual bool doesBlock() const;
    virtual bool isDamageable() const;      //only Peach and enemies damageable
    virtual void getDamaged();              //only Peach and enemies can get damaged
    
    //regular
    StudentWorld* getWorld() const;
    bool isAlive() const;
    void setDead();
    int getNewX(int steps) const;        //returns new X after taking # of steps in current dir
    
private:
    bool m_isAlive;
    StudentWorld* m_stud;
};

//-----------------------------------------------------------------------------------
//for blocks and pipes
class Stationary : public Actor{
public:
    Stationary(StudentWorld* stud, int imageID, int startX, int startY);
    virtual bool doesBlock() const;
    virtual void doSomething();
};


class Block : public Stationary{
public:
    Block(StudentWorld* stud, int startX, int startY, std::string goodieType);
    virtual void bonk();
    
private:
    bool m_hasGoodie;
    std::string m_goodieType;
};

class Pipe : public Stationary{
public:
    Pipe(StudentWorld* stud, int startX, int startY);
};

//-----------------------------------------------------------------------------------
//for flags and Mario
class Goal : public Actor{
public:
    Goal(StudentWorld* stud, int imageID, int startX, int startY);
    virtual void doSomething();
    virtual void changeLvlStatus()=0;
};

class Flag : public Goal{
public:
    Flag(StudentWorld* stud, int startX, int startY);
    virtual void changeLvlStatus();
};

class Mario : public Goal{
public:
    Mario(StudentWorld* stud, int startX, int startY);
    virtual void changeLvlStatus();
};

//-----------------------------------------------------------------------------------
//for enemies, goodies, and weapons
class MovingActors : public Actor{
public:
    MovingActors(StudentWorld* stud, int imageID, int startX, int startY, int dir, int depth, double size);
    void reverseDirection();      //reverses direction of Actor
};

//-----------------------------------------------------------------------------------
//for goodies and weapons
class Items : public MovingActors{
public:
    Items(StudentWorld* stud, int imageID, int startX, int startY, int dir, int depth, double size);
    virtual void doSomething();
    virtual bool passedInteractionCheck();
    virtual void interactWithActor()=0;
    virtual void reactToBlockingObject()=0;
};

//-----------------------------------------------------------------------------------
//for flowers, mushrooms, and stars
class Goodies : public Items{
public:
    Goodies(StudentWorld* stud, int imageID, int startX, int startY);
    virtual void interactWithActor();
    virtual void reactToBlockingObject();
    virtual void addPointsAndPower() const =0;
};

class Flower : public Goodies{
public:
    Flower(StudentWorld* stud, int startX, int startY);
    virtual void addPointsAndPower() const;
};

class Mushroom : public Goodies{
public:
    Mushroom(StudentWorld* stud, int startX, int startY);
    virtual void addPointsAndPower() const;
};

class Star : public Goodies{
public:
    Star(StudentWorld* stud, int startX, int startY);
    virtual void addPointsAndPower() const;
};

//-----------------------------------------------------------------------------------
//for weapons
class Weapons : public Items{
public:
    Weapons(StudentWorld* stud, int imageID, int startX, int startY, int dir);
    virtual void reactToBlockingObject();
};

//for Piranha fireball
class PiranhaFireball : public Weapons{
public:
    PiranhaFireball(StudentWorld* stud, int startX, int startY, int dir);
    virtual void interactWithActor();
};

//-----------------------------------------------------------------------------------
//for Peach fireball and Shell
class PeachWeapons : public Weapons{
public:
    PeachWeapons(StudentWorld* stud, int imageID, int startX, int startY, int dir);
    virtual bool passedInteractionCheck();
    virtual void interactWithActor();
};

class PeachFireball : public PeachWeapons{
public:
    PeachFireball(StudentWorld* stud, int startX, int startY, int dir);
};

class Shell : public PeachWeapons{
public:
    Shell(StudentWorld* stud, int startX, int startY, int dir);
};

//-----------------------------------------------------------------------------------
//Enemies Implementation
class Enemies : public MovingActors{
public:
    Enemies(StudentWorld* stud, int imageID, int startX, int startY, int dir);
    virtual bool isDamageable() const;
    virtual void getDamaged();
    virtual void doSomething();
    virtual void doDifferentThing1()=0;
    virtual void doDifferentThing2()=0;
    virtual void bonk();
};

class Piranha : public Enemies{
public:
    Piranha(StudentWorld* stud, int startX, int startY, int dir);
    virtual void doDifferentThing1();
    virtual void doDifferentThing2();
private:
    int firing_delay;
};

//-----------------------------------------------------------------------------------
//MovingEnemies Implementation
class MovingEnemies : public Enemies{
public:
    MovingEnemies(StudentWorld* stud, int imageID, int startX, int startY, int dir);
    virtual void doDifferentThing1();
    virtual void doDifferentThing2();
};

class Goomba : public MovingEnemies{
public:
    Goomba(StudentWorld* stud, int startX, int startY, int dir);
};

class Koopa : public MovingEnemies{
public:
    Koopa(StudentWorld* stud, int startX, int startY, int dir);
    virtual void getDamaged();
    virtual void bonk();
};

//-----------------------------------------------------------------------------------
class Peach : public Actor{
public:
    Peach(StudentWorld* stud, int startX, int startY);
    
    virtual void doSomething();
    virtual void bonk();
    virtual void getDamaged();
    virtual bool isDamageable() const;

    //getters; needed for status line in StudentWorld
    bool hasShootPower() const;
    bool hasJumpPower() const;
    bool hasStarPower() const;
    //setters
    void setShootPower(bool set);       //flower
    void setJumpPower(bool set);        //mushroom
    void setStarPower(bool set);        //star

    void setHitPoints(int hp);
    void setTicksLeftWithStarPower(int ticks);
    
private:
    int m_hitPoints;
    bool m_isTempInvincible;        //temp invincibility
    bool m_hasShootPower;
    bool m_hasJumpPower;
    bool m_hasStarPower;            //'permanant' invincibility
    int ticks_left_with_star_power;
    int ticks_left_with_temp_invincibility;
    double remaining_jump_distance;
    int time_to_recharge_before_next_fire;
};

#endif // ACTOR_H_
