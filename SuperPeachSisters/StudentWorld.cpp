#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <list>
#include "Level.h"
#include "Actor.h"
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    m_peach = nullptr;
    bonkAtX = -1;
    bonkAtY = -1;
    damageAtX = -1;
    damageAtY = -1;
    m_lvlStatus = "incomplete";
}

StudentWorld::~StudentWorld(){
    cleanUp();
}

int StudentWorld::init(){
    Level lev(assetPath());
    ostringstream oss;
    oss.fill('0');
    oss << "level" << setw(2) << getLevel() << ".txt";
    string level_file = oss.str();
    Level::LoadResult result = lev.loadLevel(level_file);
    
    if (result == Level::load_fail_file_not_found){
        cerr << "Could not find " << level_file << " data file" << endl;
        return GWSTATUS_LEVEL_ERROR;
    }
    else if (result == Level::load_fail_bad_format){
        cerr << level_file << " is improperly formatted" << endl;
        return GWSTATUS_LEVEL_ERROR;
    }
    else if (result == Level::load_success){
        setLvlStatus("incomplete");
        Level::GridEntry ge;        
        for(int x=0; x<GRID_WIDTH; x++){
            for(int y=0; y<GRID_HEIGHT; y++){
                ge = lev.getContentsOf(x, y); // x=5, y=10
                
                switch (ge){
                    case Level::empty:
                        break;
                    case Level::koopa:
                        actorContainer.push_back(new Koopa(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT, randInt(0,1)*180));
                        break;
                    case Level::goomba:
                        actorContainer.push_back(new Goomba(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT, randInt(0,1)*180));
                        break;
                    case Level::piranha:
                        actorContainer.push_back(new Piranha(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT, randInt(0,1)*180));
                        break;
                    case Level::peach:
                        m_peach = new Peach(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        break;
                    case Level::flag:
                        actorContainer.push_back(new Flag(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT));
                        break;
                    case Level::block:
                        actorContainer.push_back(new Block(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT, "none"));
                        break;
                    case Level::pipe:
                        actorContainer.push_back(new Pipe(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT));
                        break;
                    case Level::star_goodie_block:
                        actorContainer.push_back(new Block(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT, "star"));
                        break;
                    case Level::mushroom_goodie_block:
                        actorContainer.push_back(new Block(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT, "mushroom"));
                        break;
                    case Level::flower_goodie_block:
                        actorContainer.push_back(new Block(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT, "flower"));
                        break;
                    case Level::mario:
                        actorContainer.push_back(new Mario(this, x*SPRITE_WIDTH,y*SPRITE_HEIGHT));
                        break;
                }
            }
        }
    }

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    // This code is here merely to allow the game to build, run, and terminate after you hit enter.
    // Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
        
    if(m_peach->isAlive())
        m_peach->doSomething();
    
    list<Actor*>::iterator it = actorContainer.begin();
    for(; it != actorContainer.end(); it++){
        if((*it)->isAlive()){
            (*it)->doSomething();
            
            //someone made Peach die :/
            if(!(m_peach->isAlive())){
                playSound(SOUND_PLAYER_DIE);
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
            if(m_lvlStatus == "completed"){
                playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
            if(m_lvlStatus == "won"){
                playSound(SOUND_GAME_OVER);
                return GWSTATUS_PLAYER_WON;
            }
        }
    }
    
    //delete all dead actors not including Peach; cleanUp will delete Peach for me if Peach has died
    it = actorContainer.begin();
    while(it != actorContainer.end()){
        if(!((*it)->isAlive())){
            delete (*it);
            it = actorContainer.erase(it);      //increments for me
        }
        else
            it++;
    }
    
    //update status line
    ostringstream stat;         //new empty string
    stat << "Lives: " << getLives();
    stat.fill('0');
    stat << "  Level: " << setw(2) << getLevel();
    stat << "  Points: " << setw(6) << getScore();
    
    if(m_peach->hasStarPower())
        stat << " StarPower!";
    if(m_peach->hasShootPower())
        stat << " ShootPower!";
    if(m_peach->hasJumpPower())
        stat << " JumpPower!";
    
    string statusLine = stat.str();
    setGameStatText(statusLine);
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp(){
    delete m_peach;
    
    list<Actor*>::iterator it = actorContainer.begin();
    while(it != actorContainer.end()){
        delete *it;
        it = actorContainer.erase(it);      //increments for me
    }
}

void StudentWorld::setBonkAtCoords(double x, double y){
    bonkAtX = x;
    bonkAtY = y;
}

void StudentWorld::setDamageAtCoords(double x, double y){
    damageAtX = x;
    damageAtY = y;
}

//general; for all Actors to check if a block/pipe blocks movement
//Peach will never be found as a damageable object through this check
//she is not in the actor container; will only identify enemies for damageable
bool StudentWorld::isBlockingOrDamageableObjectAt(double tryX, double tryY, Actor* a, char type){
    list<Actor*>::iterator it = actorContainer.begin();
    double farLeft, farRight, farUp, farDown;
    
    if(type == 'b'){ //check for blocking object
        //identify the pixels occupied by each object
        for(; it != actorContainer.end(); it++){
            if((*it) == a)
                continue;
            farLeft = (*it)->getX();
            farRight = (*it)->getX() + SPRITE_WIDTH-1;
            farUp = (*it)->getY() + SPRITE_HEIGHT-1;
            farDown = (*it)->getY();
            
            //left bottom
            if(farLeft <= tryX && tryX <= farRight && farDown <= tryY && tryY <= farUp
               && (*it)->doesBlock()){
                setBonkAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
            //right bottom
            if(farLeft <= (tryX+SPRITE_WIDTH-1) && (tryX+SPRITE_WIDTH-1) <= farRight
               && farDown <= tryY && tryY <= farUp && (*it)->doesBlock()){
                setBonkAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
            //left top
            if(farLeft <= tryX && tryX <= farRight && farDown <= (tryY+SPRITE_WIDTH-1)
               && (tryY+SPRITE_WIDTH-1) <= farUp && (*it)->doesBlock()){
                setBonkAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
            //right top
            if(farLeft <= (tryX+SPRITE_WIDTH-1) && (tryX+SPRITE_WIDTH-1) <= farRight
               && farDown <= (tryY+SPRITE_WIDTH-1) && (tryY+SPRITE_WIDTH-1) <= farUp
               && (*it)->doesBlock()){
                setBonkAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
        }
    }
    else if(type == 'd'){ //check for damageable object
        for(; it != actorContainer.end(); it++){
            if((*it) == a)
                continue;
            farLeft = (*it)->getX();
            farRight = (*it)->getX() + SPRITE_WIDTH-1;
            farUp = (*it)->getY() + SPRITE_HEIGHT-1;
            farDown = (*it)->getY();
            
            //left bottom
            if(farLeft <= tryX && tryX <= farRight && farDown <= tryY && tryY <= farUp
               && (*it)->isDamageable() && (*it)->isAlive()){
                setDamageAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
            //right bottom
            if(farLeft <= (tryX+SPRITE_WIDTH-1) && (tryX+SPRITE_WIDTH-1) <= farRight
               && farDown <= tryY && tryY <= farUp && (*it)->isDamageable() && (*it)->isAlive()){
                setDamageAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
            //left top
            if(farLeft <= tryX && tryX <= farRight && farDown <= (tryY+SPRITE_WIDTH-1)
               && (tryY+SPRITE_WIDTH-1) <= farUp && (*it)->isDamageable() && (*it)->isAlive()){
                setDamageAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
            //right top
            if(farLeft <= (tryX+SPRITE_WIDTH-1) && (tryX+SPRITE_WIDTH-1) <= farRight
               && farDown <= (tryY+SPRITE_WIDTH-1) && (tryY+SPRITE_WIDTH-1) <= farUp
               && (*it)->isDamageable() && (*it)->isAlive()){
                setDamageAtCoords((*it)->getX(), (*it)->getY());
                return true;
            }
        }
    }
    return false;
}

//for Peach only; checks for blocked object & bonks any existing ones // blocked objects are always alive
void StudentWorld::bonkBlockingObjectAt(){
    if(bonkAtX == -1 && bonkAtY == -1)
        return;
    
    list<Actor*>::iterator it = actorContainer.begin();
    for(; it != actorContainer.end(); it++){
        if((*it)->getX() == bonkAtX && (*it)->getY() == bonkAtY){
            (*it)->bonk();
            setBonkAtCoords(-1, -1);
            return;
        }
    }
}

//enemies only react when bonked by Peach; but Peach is the only who bonks enemies; won't check
//enemies are the only non-damageable actors besides Peach; use damageableObjectAt to identify them
//damageableObjectAt is setting damageCoordinates to be Peach bc it does not check for Peach
//first check if damageable, then bonk the damageable item
void StudentWorld::bonkEnemies(){
    if(damageAtX == -1 && damageAtY == -1)
        return;
    
    list<Actor*>::iterator it = actorContainer.begin();
    for(; it != actorContainer.end(); it++){
        if((*it)->getX() == damageAtX && (*it)->getY() == damageAtY && (*it)->isAlive()){
            (*it)->bonk();
            setDamageAtCoords(-1, -1);
            return;
        }
    }
}

bool StudentWorld::peachHasStarPower() const{
    return m_peach->hasStarPower();
}

//use this if trying to bonk or damage Peach
bool StudentWorld::isOverlappingPeach(double x, double y) const{
    int pLeft, pRight, pBot, pTop;
    pLeft = m_peach->getX();
    pRight = m_peach->getX() + SPRITE_WIDTH - 1;
    pBot = m_peach->getY();
    pTop = m_peach->getY() + SPRITE_WIDTH - 1;
    
    //check if any Actor's coordinates are between Peach's coordinates
    //left bottom
    if(pLeft <= x && x <= pRight && pBot <= y && y <= pTop)
        return true;
    
    //right bottom
    if(pLeft <= (x+SPRITE_WIDTH-1) && (x+SPRITE_WIDTH-1) <= pRight && pBot <= y && y <= pTop)
        return true;
    
    //left top
    if(pLeft <= x && x <= pRight && pBot <= (y+SPRITE_WIDTH-1) && (y+SPRITE_WIDTH-1) <= pTop)
        return true;

    //right top
    if(pLeft <= (x+SPRITE_WIDTH-1) && (x+SPRITE_WIDTH-1) <= pRight
       && pBot <= (y+SPRITE_WIDTH-1) && (y+SPRITE_WIDTH-1) <= pTop)
        return true;
        
    return false;
}

void StudentWorld::setLvlStatus(std::string s){
    m_lvlStatus = s;
}

void StudentWorld::addPower(std::string power){
    if (power == "shoot"){
        m_peach->setShootPower(true);
        m_peach->setHitPoints(2);
    }
    else if(power == "jump"){
        m_peach->setJumpPower(true);
        m_peach->setHitPoints(2);
    }
    else if(power == "star"){
        m_peach->setStarPower(true);
        m_peach->setTicksLeftWithStarPower(150);
    }
    playSound(SOUND_PLAYER_POWERUP);
}

void StudentWorld::damagePeach() const{
    m_peach->getDamaged();
}

//only used by PeachFireball and Shell; only after they checked that Damageable actor exists
void StudentWorld::damageEnemies(){
    //uninitialized == haven't checked if Damageable actor exists
    if(damageAtX == -1 && damageAtY == -1){
        return;
    }

    list<Actor*>::iterator it = actorContainer.begin();
    for(; it != actorContainer.end(); it++){
        if((*it)->getX() == damageAtX && (*it)->getY() == damageAtY && (*it)->isAlive()){
            (*it)->getDamaged();
            setDamageAtCoords(-1, -1);
            return;
        }
    }
}

//add PeachFireball 4 pixels away from Peach in direction Peach is facing
void StudentWorld::addPeachFireball(int startX, int startY, int dir){
    actorContainer.push_back(new PeachFireball(this, startX, startY, dir));
}

//add Shell at same location and direction as Koopa when it dies
void StudentWorld::addShell(Actor* a){
    actorContainer.push_back(new Shell(this, a->getX(), a->getY(), a->getDirection()));
}

void StudentWorld::addGoodie(std::string goodieType, int startX, int startY){
    if(goodieType == "flower")
        actorContainer.push_back(new Flower(this, startX, startY));
    else if(goodieType == "mushroom")
        actorContainer.push_back(new Mushroom(this, startX, startY));
    else if(goodieType == "star")
        actorContainer.push_back(new Star(this, startX, startY));
}

void StudentWorld::bonkPeach(){
    if(m_peach->isAlive())
        m_peach->bonk();
}

//Goombas and Koopas don't want to fall; and they never jump up; so they are always going L and R
bool StudentWorld::platformExists(double tryX, double tryY, Actor* a) const{
    list<Actor*>::const_iterator it = actorContainer.begin();
    double farLeft, farRight, farUp, farDown;
    
    //identify the pixels occupied by each object
    for(; it != actorContainer.end(); it++){
        if((*it) == a)
            continue;
        farLeft = (*it)->getX();
        farRight = (*it)->getX() + SPRITE_WIDTH-1;
        farUp = (*it)->getY() + SPRITE_HEIGHT-1;
        farDown = (*it)->getY();
        
        //might go off right edge; check if platform is underneath new right bottom
        if(a->getDirection() == 0){
            if(farLeft <= (tryX+SPRITE_WIDTH-1) && (tryX+SPRITE_WIDTH-1) <= farRight
               && farDown <= tryY-1 && tryY-1 <= farUp && (*it)->doesBlock()){
                return true;
            }
        }
        //might go off left edge; check if platform is underneath new left bottom
        else if(a->getDirection() == 180){
            if(farLeft <= tryX && tryX <= farRight && farDown <= tryY-1 && tryY-1 <= farUp
               && (*it)->doesBlock()){
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::isPeachWithinThisHeight(double y){
    int peaY = m_peach->getY();
    int botLimit = y - 1.5*SPRITE_HEIGHT;
    int upLimit = y + 1.5*SPRITE_HEIGHT;
    
    if(botLimit < peaY && peaY < upLimit)       //within == not inclusive
        return true;
    return false;
}

bool StudentWorld::isPeachToLeft(double x){
    int peaX = m_peach->getX();
    if(peaX < x)
        return true;
    return false;
}

bool StudentWorld::isPeachToRight(double x){
    int peaX = m_peach->getX();
    if(peaX > x)
        return true;
    return false;
}

bool StudentWorld::isPeachWithinThisWidth(double x){
    int peaX = m_peach->getX();
    int leftLimit = x - 8*SPRITE_WIDTH;
    int rightLimit = x + 8*SPRITE_WIDTH;

    if(leftLimit < peaX && peaX < rightLimit)
        return true;
    return false;
}

void StudentWorld::addPiranhaFireball(double x, double y, int dir){
    actorContainer.push_back(new PiranhaFireball(this, x, y, dir));
}
