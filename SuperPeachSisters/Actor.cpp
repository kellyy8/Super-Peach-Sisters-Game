#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

//-----------------------------------------------------------------------------------
//Actor implementation
Actor::Actor(StudentWorld* stud, int imageID, int startX, int startY, int dir, int depth, double size):
    GraphObject(imageID, startX, startY, dir, depth, size)
{
    m_isAlive = true;
    m_stud = stud;
}

Actor::~Actor(){}

void Actor::bonk(){}                                //override for Peach, block, and enemies
bool Actor::doesBlock() const{return false;}        //override for block and Pipe
bool Actor::isDamageable() const{return false;}     //override for Peach and enemies
void Actor::getDamaged(){}                          //override for Peach and enemies
StudentWorld* Actor::getWorld() const{return m_stud;}
bool Actor::isAlive() const{return m_isAlive;}
void Actor::setDead(){m_isAlive = false;}

//returns new X location computed based on dir and desired # of steps in that dir
int Actor::getNewX(int steps) const{
    int dir = getDirection();
    if(dir == 0)
        return getX() + steps;
    else
        return getX() - steps;  //only other dir=180
}

//-----------------------------------------------------------------------------------
//Stationary implementation
Stationary::Stationary(StudentWorld* stud, int imageID, int startX, int startY):
    Actor(stud, imageID, startX, startY, 0, 2, 1.0) {}

//all pipes and blocks block movement
bool Stationary::doesBlock() const{
    return true;
}

//all pipes and blocks do not do anything during each tick
void Stationary::doSomething(){}

//Block implementation
Block::Block(StudentWorld* stud, int startX, int startY, std::string goodieType):
    Stationary(stud, IID_BLOCK, startX, startY)
{
    m_goodieType = goodieType;
    if(m_goodieType == "none")
        m_hasGoodie = false;
    else
        m_hasGoodie = true;
}

void Block::bonk(){
    if(!m_hasGoodie){        //no Goodie to begin with or already released Goodie
        getWorld()->playSound(SOUND_PLAYER_BONK);
    }
    else{
        getWorld()->playSound(SOUND_POWERUP_APPEARS);
        getWorld()->addGoodie(m_goodieType, getX(), getY()+8);
        m_hasGoodie = false;    //only release 1 Goodie
    }
}

Pipe::Pipe(StudentWorld* stud, int startX, int startY):
    Stationary(stud, IID_PIPE, startX, startY) {}

//-----------------------------------------------------------------------------------
//Goals implementation
Goal::Goal(StudentWorld* stud, int imageID, int startX, int startY):
    Actor(stud, imageID, startX, startY, 0, 1, 1.0) {}

void Goal::doSomething(){
    if(!isAlive())
        return;
    
    if(getWorld()->isOverlappingPeach(getX(), getY())){
        getWorld()->increaseScore(1000);
        setDead();
        changeLvlStatus();
    }
}

Flag::Flag(StudentWorld* stud, int startX, int startY):
    Goal(stud, IID_FLAG, startX, startY) {}

void Flag::changeLvlStatus(){
    getWorld()->setLvlStatus("completed");
}

Mario::Mario(StudentWorld* stud, int startX, int startY):
    Goal(stud, IID_MARIO, startX, startY) {}

void Mario::changeLvlStatus(){
    getWorld()->setLvlStatus("won");
}

//-----------------------------------------------------------------------------------
//Moving Actors implementation

MovingActors::MovingActors(StudentWorld* stud, int imageID, int startX, int startY, int dir, int depth, double size):
    Actor(stud, imageID, startX, startY, dir, depth, size){}

void MovingActors::reverseDirection(){
    if(getDirection() == 0)
        setDirection(180);
    else if(getDirection() == 180)
        setDirection(0);
}

//-----------------------------------------------------------------------------------
//Items implementation
Items::Items(StudentWorld* stud, int imageID, int startX, int startY, int dir, int depth, double size):
    MovingActors(stud, imageID, startX, startY, dir, depth, size){}

//default checks for overlapping with Peach; used by goodies & piranha fireball
bool Items::passedInteractionCheck(){
    return getWorld()->isOverlappingPeach(getX(), getY());
}

void Items::doSomething(){
    if(passedInteractionCheck()){
        interactWithActor();
        setDead();
        return;
    }
    else if(!(getWorld()->isBlockingOrDamageableObjectAt(getX(), getY()-2, this, 'b'))){
        moveTo(getX(), getY()-2);   //falling
    }
    
    int newX = getNewX(2);      //going left and right
    if(getWorld()->isBlockingOrDamageableObjectAt(newX, getY(), this, 'b'))
        reactToBlockingObject();
    else
        moveTo(newX, getY());
}

//-----------------------------------------------------------------------------------
//Goodies implementation

Goodies::Goodies(StudentWorld* stud, int imageID, int startX, int startY):
    Items(stud, imageID, startX, startY, 0, 1, 1.0){}

void Goodies::interactWithActor(){
    addPointsAndPower();
}

void Goodies::reactToBlockingObject(){
    reverseDirection();
}

Flower::Flower(StudentWorld* stud, int startX, int startY):
    Goodies(stud, IID_FLOWER, startX, startY){}

void Flower::addPointsAndPower() const{
    getWorld()->increaseScore(50);
    getWorld()->addPower("shoot");
}

Mushroom::Mushroom(StudentWorld* stud, int startX, int startY):
    Goodies(stud, IID_MUSHROOM, startX, startY){}

void Mushroom::addPointsAndPower() const{
    getWorld()->increaseScore(75);
    getWorld()->addPower("jump");
}

Star::Star(StudentWorld* stud, int startX, int startY):
    Goodies(stud, IID_STAR, startX, startY){}

void Star::addPointsAndPower() const{
    getWorld()->increaseScore(100);
    getWorld()->addPower("star");
}

//-----------------------------------------------------------------------------------
//Weapons implementation
Weapons::Weapons(StudentWorld* stud, int imageID, int startX, int startY, int dir):
    Items(stud, imageID, startX, startY, dir, 1, 1.0){}

void Weapons::reactToBlockingObject(){
    setDead();
}

PiranhaFireball::PiranhaFireball(StudentWorld* stud, int startX, int startY, int dir):
    Weapons(stud, IID_PIRANHA_FIRE, startX, startY, dir){}

void PiranhaFireball::interactWithActor(){
    getWorld()->damagePeach();
}

//-----------------------------------------------------------------------------------
PeachWeapons::PeachWeapons(StudentWorld* stud, int imageID, int startX, int startY, int dir):
    Weapons(stud, imageID, startX, startY, dir){}

bool PeachWeapons::passedInteractionCheck(){
    return getWorld()->isBlockingOrDamageableObjectAt(getX(), getY(), this, 'd');
}

void PeachWeapons::interactWithActor(){
    getWorld()->damageEnemies();
}

PeachFireball::PeachFireball(StudentWorld* stud, int startX, int startY, int dir):
    PeachWeapons(stud, IID_PEACH_FIRE, startX, startY, dir){}

Shell::Shell(StudentWorld* stud, int startX, int startY, int dir):
    PeachWeapons(stud, IID_SHELL, startX, startY, dir){}


//-----------------------------------------------------------------------------------
//Enemies implementation
Enemies::Enemies(StudentWorld* stud, int imageID, int startX, int startY, int dir):
    MovingActors(stud, imageID, startX, startY, dir, 0, 1.0){}

bool Enemies::isDamageable() const{return true;}

void Enemies::getDamaged(){
    getWorld()->increaseScore(100);
    setDead();
}

void Enemies::doSomething(){
    if(!isAlive())
        return;

    doDifferentThing1();
    
    if(getWorld()->isOverlappingPeach(getX(), getY())){
        getWorld()->bonkPeach();
        return;
    }
    
    doDifferentThing2();
}

//only Peach will every call this function on enemies
void Enemies::bonk(){
    if(getWorld()->peachHasStarPower()){
        getWorld()->playSound(SOUND_PLAYER_KICK);
        getWorld()->increaseScore(100);
        setDead();
    }
}

//-----------------------------------------------------------------------------------
//MovingEnemies implementation
MovingEnemies::MovingEnemies(StudentWorld* stud, int imageID, int startX, int startY, int dir):
    Enemies(stud, imageID, startX, startY, dir){}

void MovingEnemies::doDifferentThing1(){}

//adding onto the doSomething() for all Enemies
void MovingEnemies::doDifferentThing2(){
    int newX = getNewX(1);
    if(getWorld()->isBlockingOrDamageableObjectAt(newX, getY(), this, 'b')){
        reverseDirection();
    }
    else{
        //check if it can move 1 pixel in current direction w/o stepping partly of fully off edge
        if(!(getWorld()->platformExists(newX, getY(), this)))
            reverseDirection();
    }
    
    //need to getNewX since direction may or may not be changed
    newX = getNewX(1);
    if(getWorld()->isBlockingOrDamageableObjectAt(newX, getY(), this, 'b'))
        return;
    else
        moveTo(newX, getY());
}

Goomba::Goomba(StudentWorld* stud, int startX, int startY, int dir):
    MovingEnemies(stud, IID_GOOMBA, startX, startY, dir){}

Koopa::Koopa(StudentWorld* stud, int startX, int startY, int dir):
    MovingEnemies(stud, IID_KOOPA, startX, startY, dir){}

void Koopa::getDamaged(){
    Enemies::getDamaged();
    getWorld()->addShell(this);
}

void Koopa::bonk(){
    Enemies::bonk();
    if(getWorld()->peachHasStarPower())
        getWorld()->addShell(this);         //check this, in case shell causes repeated loop
}

Piranha::Piranha(StudentWorld* stud, int startX, int startY, int dir):
    Enemies(stud, IID_PIRANHA, startX, startY, dir)
{
    firing_delay = 0;
}

//implement cycling graph image
void Piranha::doDifferentThing1(){
    increaseAnimationNumber();
}

//implement everything else it does
void Piranha::doDifferentThing2(){
    //last function called in Enemies's doSomething()
    if(!(getWorld()->isPeachWithinThisHeight(getY()))){
        return;
    }
    
    //determine Peach's direction; && face Peach
    if(getWorld()->isPeachToLeft(getX()))
        setDirection(180);
    else if(getWorld()->isPeachToRight(getX()))
        setDirection(0);
    
    //determine if Peach's location is greater than or less than Piranha's location
    if(firing_delay > 0){
        firing_delay--;
        return;
    }
    else if(getWorld()->isPeachWithinThisWidth(getX())){
        getWorld()->addPiranhaFireball(getX(), getY(), getDirection());
        getWorld()->playSound(SOUND_PIRANHA_FIRE);
        firing_delay = 40;
    }
}

//-----------------------------------------------------------------------------------
//Peach implementation
Peach::Peach(StudentWorld* stud, int startX, int startY):
    Actor(stud, IID_PEACH, startX, startY, 0, 0, 1.0)
{
    m_hitPoints = 1;
    m_isTempInvincible = false;
    m_hasStarPower = false;
    m_hasShootPower = false;
    m_hasJumpPower = false;
    ticks_left_with_star_power = 0;
    ticks_left_with_temp_invincibility = 0;
    remaining_jump_distance = 0;
    time_to_recharge_before_next_fire = 0;
};

void Peach::doSomething(){
    if(!isAlive())
        return;
    
    if(hasStarPower())
        ticks_left_with_star_power--;
    if(ticks_left_with_star_power == 0)
        setStarPower(false);
    
    //adjusting time left for temp invincibility
    if(m_isTempInvincible)
        ticks_left_with_temp_invincibility--;
    if(ticks_left_with_temp_invincibility == 0)
        m_isTempInvincible = false;
    
    //adjusting time for shooting fireballs
    if(time_to_recharge_before_next_fire > 0)
        time_to_recharge_before_next_fire--;
    
    if(getWorld()->isBlockingOrDamageableObjectAt(getX(), getY(), this, 'b'))
        getWorld()->bonkBlockingObjectAt();
    if(getWorld()->isBlockingOrDamageableObjectAt(getX(), getY(), this, 'd'))
        getWorld()->bonkEnemies();
    
    //adjusting distance for jumping (and falling back down)
    if(remaining_jump_distance > 0){
        if(getWorld()->isBlockingOrDamageableObjectAt(getX(), getY()+4, this, 'b')){
            getWorld()->bonkBlockingObjectAt();
            remaining_jump_distance = 0;
        }
        else{
            moveTo(getX(), getY()+4);
            remaining_jump_distance--;
        }
    }
    else{
        bool down0 = getWorld()->isBlockingOrDamageableObjectAt(getX(), getY(), this, 'b');
        bool down1 = getWorld()->isBlockingOrDamageableObjectAt(getX(), getY()-1, this, 'b');
        bool down2 = getWorld()->isBlockingOrDamageableObjectAt(getX(), getY()-2, this, 'b');
        bool down3 = getWorld()->isBlockingOrDamageableObjectAt(getX(), getY()-3, this, 'b');
        if(!down0 && !down1 && !down2 && !down3)
            moveTo(getX(), getY()-4);
    }
    
    int key;
    if (getWorld()->getKey(key)){
        switch (key){
            case KEY_PRESS_LEFT:
                setDirection(180);
                if(getWorld()->isBlockingOrDamageableObjectAt(getX()-4, getY(), this, 'b')){
                    getWorld()->bonkBlockingObjectAt();
                }
                else
                    moveTo(getX()-4, getY());
                break;
            case KEY_PRESS_RIGHT:
                setDirection(0);
                if(getWorld()->isBlockingOrDamageableObjectAt(getX()+4, getY(), this, 'b')){
                    getWorld()->bonkBlockingObjectAt();
                }
                else
                    moveTo(getX()+4, getY());
                break;
            case KEY_PRESS_UP:
                if(getWorld()->isBlockingOrDamageableObjectAt(getX(), getY()-1, this, 'b')){
                    if(!m_hasJumpPower)
                        remaining_jump_distance = 8;
                    else
                        remaining_jump_distance = 12;
                    getWorld()->playSound(SOUND_PLAYER_JUMP);
                }
                break;
            case KEY_PRESS_SPACE:
                if(!m_hasShootPower || time_to_recharge_before_next_fire > 0)
                    break;
                getWorld()->playSound(SOUND_PLAYER_FIRE);
                time_to_recharge_before_next_fire = 8;
                int newX = getNewX(4);
                getWorld()->addPeachFireball(newX, getY(), getDirection());
                break;
        }
    }
}

void Peach::bonk(){
    if(hasStarPower() || m_isTempInvincible)
        return;

    m_hitPoints--;
    m_isTempInvincible = true;
    ticks_left_with_temp_invincibility = 10;
    
    if(hasShootPower())
        setShootPower(false);
    if(hasJumpPower())
        setJumpPower(false);
    if(m_hitPoints >= 1)
        getWorld()->playSound(SOUND_PLAYER_HURT);
    if(m_hitPoints <= 0)
        setDead();
}

void Peach::getDamaged(){
    bonk();
}

bool Peach::isDamageable() const{return true;}
bool Peach::hasShootPower() const{return m_hasShootPower;}
bool Peach::hasJumpPower() const{return m_hasJumpPower;}
bool Peach::hasStarPower() const{return m_hasStarPower;}

void Peach::setShootPower(bool set){ m_hasShootPower = set;}
void Peach::setJumpPower(bool set){ m_hasJumpPower = set;}
void Peach::setStarPower(bool set){ m_hasStarPower = set;}
void Peach::setHitPoints(int hp){ m_hitPoints = hp;}
void Peach::setTicksLeftWithStarPower(int ticks){ ticks_left_with_star_power = ticks;}
