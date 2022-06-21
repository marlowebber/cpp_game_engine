#ifndef MARLO_H
#define MARLO_H


#include "utilities.h"


const int worldSize = 4096;
const unsigned int worldSquareSize       = worldSize * worldSize;
const int NUMBER_OF_CONNECTIONS = 8;
const unsigned int numberOfAnimals = 2500;
const unsigned int numberOfSpecies = 8;
const unsigned int animalSquareSize      = 512;
const unsigned int displayNameSize = 32;
const unsigned int nLogs = 32;
const unsigned int logLength = 64;
const float playerSpeed = 0.3f;
const float panSpeed = 0.1f;
const unsigned int numberOfSpeakerChannels = 16;


const float  maxZoom = 450.0f;
const float  minZoom = 50.0f;


const float growthEnergyScale      = 1.0f;         // a multiplier for how much it costs game.animals to make new cells.
const bool variedGrowthCost      = false;
const bool variedUpkeep          = false;

extern bool mainMenu   ;
extern bool flagQuit   ;
extern bool flagCreate ;
extern bool flagLoad   ;
extern bool flagReady  ;
extern bool flagReturn ;
extern int mouseX;
extern int mouseY;
extern int worldCreationStage;




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


// these parts need to be recorded in between sessions.
struct GameState
{
    char saveName[displayNameSize];
    char version[displayNameSize];

    // these variables are how the player drives their character, and what they get back from it.
    unsigned int playerActiveGrabber;
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

    // these variables keep track of the main characters in the game world.
    int playerCreature ;
    int championScore ;
    float championEnergyScore ;
    int adversary = -1;
    unsigned int adversaryRespawnPos;
    int selectedAnimal ;
    int cursorAnimal ;
    unsigned int playerRespawnPos;
    Animal champion;
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
    unsigned int speciesAttacksPerTurn   [numberOfSpecies];

    float speakerChannels[numberOfSpeakerChannels];
    float speakerChannelsLastTurn[numberOfSpeakerChannels];

    Animal animals[numberOfAnimals];

    struct Square world[worldSquareSize];
} ;


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
void playerGrab();
void playerDrop();
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

// the following are used privately to create the game content.
void animalAppendCell(unsigned int animalIndex, unsigned int organType);
void setupCreatureFromCharArray( unsigned int animalIndex, char * start, unsigned int len, unsigned int width, std::string newName, int newMachineCallback );
void resetAnimal(unsigned int animalIndex);

void paintCreatureFromCharArray( unsigned int animalIndex, char * start, unsigned int len, unsigned int width );

#endif //MARLO_H