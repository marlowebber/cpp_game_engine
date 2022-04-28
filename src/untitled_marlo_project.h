#ifndef MARLO_H
#define MARLO_H

void startSimulation();
void model();

void camera();

 const int worldSize = 4096;

extern unsigned int cameraPositionX ;//= (worldSize / 2);
extern unsigned int cameraPositionY ;//= (worldSize / 2);
extern bool playerInControl;


extern int mousePositionX;
extern int mousePositionY;

extern bool lockfps;

     const       int cameraPanSpeed = 10;
extern	float fps ;

extern  int playerCreature ;
void spawnPlayer();

unsigned int getPlayerDestination();
void setPlayerDestination(unsigned int newDestination);

void rebuildGameMenus();

// extern unsigned int playerDestination;

void save();
void load();

void model();

void drawGameInterfaceText();

 #endif //MARLO_H