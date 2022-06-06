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

 #endif //MARLO_H