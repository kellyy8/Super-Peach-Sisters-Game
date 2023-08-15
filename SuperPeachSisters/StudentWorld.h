#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include <string>
#include <list>

class Actor;
class Peach;

class StudentWorld : public GameWorld{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    void setBonkAtCoords(double x, double y);
    void setDamageAtCoords(double x, double y);
    
    bool isBlockingOrDamageableObjectAt(double tryX, double tryY, Actor* a, char type); //overlap
    void bonkBlockingObjectAt();
    void bonkEnemies();
    bool peachHasStarPower() const;
    bool isOverlappingPeach(double x, double y) const;      //overlap
    void setLvlStatus(std::string s);
    void addPower(std::string power);
    void damagePeach() const;
    void damageEnemies();
    void addPeachFireball(int startX, int startY, int dir);
    void addShell(Actor* a);
    void addGoodie(std::string goodieType, int startX, int startY);
    
    void bonkPeach();
    bool platformExists(double tryX, double tryY, Actor* a) const;
    
    bool isPeachWithinThisHeight(double y);
    bool isPeachToLeft(double x);
    bool isPeachToRight(double x);
    bool isPeachWithinThisWidth(double x);
    void addPiranhaFireball(double x, double y, int dir);

private:
    std::list<Actor*> actorContainer;
    Peach* m_peach;
    double bonkAtX, bonkAtY;
    double damageAtX, damageAtY;
    std::string m_lvlStatus;
};

#endif // STUDENTWORLD_H_
