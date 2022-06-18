#ifndef MARLO_H
#define MARLO_H

void startSimulation();
void model();
void camera();




const extern int prelimSize ;
const extern int worldSize ;
const extern int cameraPanSpeed ;
extern unsigned int cameraPositionX ;
extern unsigned int cameraPositionY ;
extern bool playerInControl;
extern int mousePositionX;
extern int mousePositionY;
// extern float fmousePositionX;
// extern float fmousePositionY;
extern bool lockfps;
extern float fps ;
extern  int playerCreature ;
extern bool shift;
extern bool mainMenu;
extern unsigned int worldCreationStage;





struct Square
{
    unsigned int wall;      //  material filling the volume of the square. You probably can't move through it.
    unsigned int material;  // a piece of material sitting on the ground. You can move over it, no matter what it is made of.
    unsigned int terrain;   // the floor itself. If it is not solid, you may have to swim in it.
    int identity;           // id of the last animal to cross the tile
    int occupyingCell;      // id of the last cell to cross this tile
    float trail;            // movement direction of the last animal to cross the tile
    float height;
    Color light;
    float temperature;
    float pheromoneIntensity;
    int pheromoneChannel;
    Color grassColor;
};

struct Connection
{
    bool used;
    unsigned int connectedTo;
    float weight;
};

struct Cell
{
    unsigned int organ;
    float signalIntensity;
    Color color;
    unsigned int speakerChannel;
    int eyeLookX;
    int eyeLookY;
    int localPosX;
    int localPosY;
    unsigned int worldPositionI;
    float damage;
    int grabbedCreature;
    Connection connections[NUMBER_OF_CONNECTIONS];
};

struct Animal
{
    Cell body[animalSquareSize];
    Cell genes[animalSquareSize];
    unsigned int mass;
    unsigned int numberOfTimesReproduced;
    unsigned int damageDone;
    unsigned int damageReceived;
    unsigned int birthLocation;
    unsigned int age;
    unsigned int lifespan;
    int parentIdentity;
    bool retired;
    float offspringEnergy;
    float energy;
    float maxEnergy;
    float energyDebt;
    unsigned int position;
    unsigned int uPosX;
    unsigned int uPosY;
    float fPosX;
    float fPosY;
    float fAngle;                   // the direction the animal is facing
    float fAngleSin;
    float fAngleCos;
    bool parentAmnesty;
    unsigned int totalMuscle;
    unsigned int totalGonads;
    unsigned int lastTouchedStranger;
    unsigned int lastTouchedKin;
    unsigned int cellsUsed;
    Color identityColor;
    float temp_limit_low;
    float temp_limit_high;
    char displayName[displayNameSize];
    float lastfposx ;
    float lastfposy;
    bool isMachine;
    unsigned int machineCallback;
};



void spawnPlayer();

void save();
void load();

void model();

void drawGameInterfaceText();

void spawnTournamentAnimals();
void adjustPlayerPos(Vec_f2 pos);

void playerGrab();
void playerDrop();

void togglePause ();

void selectCursorAnimal();

void activateGrabbedMachine();
void resetMouseCursor();

void saveSelectedAnimal();

void viewAdversary();

void incrementSelectedOrgan();

void decrementSelectedOrgan();

void rightClickCallback();


void toggleErodingRain();
void normalizeTerrainHeight();
void toggleInstructions();

void recomputeTerrainLighting();

void gameGraphics();

void setupRandomWorld();


void drawMainMenuText();


 #endif //MARLO_H