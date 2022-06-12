#ifndef MARLO_H
#define MARLO_H

void startSimulation();
void model();
void camera();

const int worldSize = 4096;

extern unsigned int cameraPositionX ;
extern unsigned int cameraPositionY ;
extern bool playerInControl;


extern int mousePositionX;
extern int mousePositionY;


extern float fmousePositionX;
extern float fmousePositionY;

extern bool lockfps;
const  int cameraPanSpeed = 10;
extern float fps ;

extern  int playerCreature ;

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

#define VISUALIZER_TRUECOLOR           1001
#define VISUALIZER_TRACKS              1003
#define VISUALIZER_IDENTITY            1004
#define VISUALIZER_NEURALACTIVITY      1006


extern int visualizer ;//= VISUALIZER_TRUECOLOR;

 #endif //MARLO_H