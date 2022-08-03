#ifndef MARLO_H
#define MARLO_H


#include "utilities.h"


const unsigned int nNeighbours     = 8;
const int worldSize = 2400;// 2400;
const unsigned int worldSquareSize       = worldSize * worldSize;
const int NUMBER_OF_CONNECTIONS = 8;
const unsigned int numberOfAnimals = 20000;//100000;//6000;  //20000;
const unsigned int numberOfSpecies = 12;
const unsigned int animalSquareSize      = 256;
const unsigned int displayNameSize = 32;
const unsigned int nLogs = 32;
const unsigned int logLength = 64;
const float panSpeed = 0.1f;
const unsigned int numberOfSpeakerChannels = 16;
const unsigned int numberOfEcologySettings = 12;

const float viewPanLimit = 250.0f;

const float  maxZoom = 450.0f;
const float  minZoom = 25.0f;

const bool variedGrowthCost      = true;
const bool variedUpkeep          = false;

const extern float playerSpeed;

extern bool mainMenu   ;
extern bool flagQuit   ;
extern bool flagCreate ;
extern bool flagLoad   ;
extern bool flagReady  ;
extern bool flagReturn ;
extern bool flagSave ;
extern int mouseX;
extern int mouseY;
extern int worldCreationStage;
extern unsigned int longestMenu;

// extern bool fastCam;

const unsigned int plantGenomeSize = 24;
const float lightEnergy = 1.0f;




#define PLANTS

struct Square
{
    unsigned int wall;      //  material filling the volume of the square. You may not be able to move through it.
    unsigned int terrain;   // the floor itself. If it is not solid, you may have to swim in it.
    int identity;           // id of the last animal to cross the tile
    int occupyingCell;      // id of the last cell to cross this tile
    float trail;            // movement direction of the last animal to cross the tile
    float height;
    Color light;
    // float pheromoneIntensity;
    unsigned int downhillNeighbour;
    int pheromoneChannel;

    Color grassColor;
    Color seedColor;

    char seedGenes[plantGenomeSize];
    int seedIdentity;
    int seedState;
    Color seedColorMoving;

    char plantGenes[plantGenomeSize];
    int plantState;
    int geneCursor;
    int plantIdentity;
    float energy;
    // float energyDebt;
    float nutrients;
    int sequenceNumber;
    int sequenceReturn;
    bool grown;
    bool growthMatrix[nNeighbours];
    bool branching;
    bool aquaticPlant;
    // #endif
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
    float workingValue;
    int speakerChannel;

    Color color;
    char  name;


    int eyeLookX;
    int eyeLookY;


    int localPosX;
    int localPosY;


    unsigned int worldPositionI;
    float damage;
    int grabbedCreature;
    Connection connections[NUMBER_OF_CONNECTIONS];
};


const unsigned int numberOfStatusEffects = 4;
const unsigned int animalGenomeSize = 1024;

struct Animal
{
    Cell body[animalSquareSize];
    float mass;
    unsigned int numberOfTimesReproduced;
    unsigned int damageDone;
    unsigned int damageReceived;
    unsigned int birthLocation;
    unsigned int age;
    unsigned int lifespan;
    unsigned int generation;
    int parentIdentity;
    bool retired;
    float offspringEnergy;
    float energy;
    float maxEnergy;
    float energyDebt;

    unsigned int position;

    float fPosX;
    float fPosY;

    float fVelX;
    float fVelY; 
    float fVelA; // rotational velocity

    float fAngle;                   // the direction the animal is facing
    float fAngleSin;
    float fAngleCos;
    bool parentAmnesty;
    // unsigned int landMuscle;
    unsigned int totalMuscle;
    unsigned int lastTouchedStranger;
    unsigned int lastTouchedKin;
    unsigned int cellsUsed;
    char displayName[displayNameSize];
    float lastfposx ;
    float lastfposy;
    bool isMachine;
    unsigned int machineCallback;

    // char statusEffects[numberOfStatusEffects];
    // unsigned int statusEffect;

    char genes[animalGenomeSize];
};



// these parts need to be recorded in between sessions.
struct GameState
{
    char saveName[displayNameSize];
    char version[displayNameSize];

    // these variables are how the player drives their character, and what they get back from it.
    int playerActiveGrabber;
    bool playerGrabState ;
    bool playerCanSee ;
    bool playerCanHear ;
    bool playerCanSmell ;
    bool palette ;
    bool playerCanPickup ;
    int playerCanPickupItem;
    bool lockfps;
    bool paused ;
    int mousePositionX ;
    int mousePositionY ;
    bool playerLMBDown;

    // these variables keep track of the main characters in the game world.
    int playerCreature ;
    int adversary = -1;
    unsigned int adversaryRespawnPos;
    int selectedAnimal ;
    int selectedPlant ;
    int cursorAnimal ;
    unsigned int playerRespawnPos;

    Animal champions[numberOfSpecies];
    int championScores[numberOfSpecies] ;
    // float championEnergies[numberOfSpecies];

    bool adversaryDefeated;
    bool adversaryCreated;

    // camera view
    unsigned int cameraPositionX;
    unsigned int cameraPositionY;
    int cameraTargetCreature ;

    // these variables govern the display of menus and other texts.
    bool showInstructions ;
    bool ecologyComputerDisplay ;
    int visualizer ;
    bool computerdisplays[5];
    char logs[logLength][nLogs];
    unsigned int paletteSelectedOrgan ;

    bool speciesVacancies [numberOfSpecies];
    unsigned int speciesPopulationCounts [numberOfSpecies];
    unsigned int populationCountUpdates  [numberOfSpecies];

    float speakerChannels[numberOfSpeakerChannels];
    float speakerChannelsLastTurn[numberOfSpeakerChannels];

    Animal animals     [numberOfAnimals];
    struct Square world[worldSquareSize];

    float ecoSettings[numberOfEcologySettings];


    unsigned int activeEcoSetting;
} ;


void incrementSideText();
void decrementSideText();

// the following are used as an interface to the game.
void setupRandomWorld();
void startSimulation();
void activateGrabbedMachine();
void selectCursorAnimal();
void rightClickCallback ();
void viewAdversary();
void toggleInstructions();
void resetMouseCursor(  );
void togglePause ();
void incrementSelectedOrgan();
void decrementSelectedOrgan();
// void playerGrab();
// void playerDrop();

void incrementVisualizer();
 
void spawnAnimalAtCursor();   
void toggleRain();

extern bool playerGrab ;
extern bool playerDrop ;

void adjustPlayerPos(Vec_f2 pos);
void load();
void save();
void setMousePosition(Vec_i2 in);
void saveSelectedAnimal ( );
void toggleFPSLimit();
bool getFPSLimit();
void spawnPlayer();
void camera();
void model();
void drawGameInterfaceText();
void drawMainMenuText();
void incrementCameraPos(Vec_i2 in);
void incrementSelectedGrabber();
void notifyLMBUp();
// void drawInterfacePanel();

void scrambleSelectedAnimal();

void setupPlantAtCursor();
void lookAtNextNonretiredAnimal();
// void drawPalette2();

void resetGameState();

void paintAnimals ();
bool getPause();
bool test_all();

void checkLongestMenu(std::string in);
void fastReset();
// the following are used privately to create the game content.
void animalAppendCell(int animalIndex, unsigned int organType);
void setupCreatureFromCharArray( int animalIndex,  char * start, unsigned int len, unsigned int width, std::string newName, int newMachineCallback );
void resetAnimal(int animalIndex);

void paintCreatureFromCharArray( int animalIndex,  char * start, unsigned int len, unsigned int width );

void setupStructureFromCharArray(int worldPositionI, char * walls,char * floors,char * lights, unsigned int len, unsigned int width, unsigned int scale);
void appendCell(int animalIndex, unsigned int organType, Vec_i2 newPosition);
#endif //MARLO_H