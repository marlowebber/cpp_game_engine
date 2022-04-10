#ifndef MARLO_H
#define MARLO_H

void startSimulation();
void model();

void camera();


void spawnPlayer();

unsigned int getPlayerDestination();
void setPlayerDestination(unsigned int newDestination);

 const int worldSize = 512;
void rebuildGameMenus();

// extern unsigned int playerDestination;

 #endif //MARLO_H