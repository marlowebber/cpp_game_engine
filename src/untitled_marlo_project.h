#ifndef MARLO_H
#define MARLO_H

void startSimulation();
void model();

void camera();

 const int worldSize = 512;

extern unsigned int cameraPositionX ;//= (worldSize / 2);
extern unsigned int cameraPositionY ;//= (worldSize / 2);


extern	float fps ;
void spawnPlayer();

unsigned int getPlayerDestination();
void setPlayerDestination(unsigned int newDestination);

void rebuildGameMenus();

// extern unsigned int playerDestination;

 #endif //MARLO_H