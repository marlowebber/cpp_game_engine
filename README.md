# cpp_game_engine

## About

This is sort of like a minimalistic version of unity or some other game-building environment- it gives you a game engine with rendering, threading, game physics, keyboard and mouse, and audio all set up, with places where you can write your own code into it. 
It is meant for 2d games, although only box2d limits it in this regard. It is also meant for linux. You can create a windows executable using mingW32, but I have not yet achieved this, so it is not included in this project yet.

YOU SHOULD WRITE YOUR GAME IN C++. Why you say? Coding in C-like languages is working on the raw coal face of what your computer is doing. If performance matters to you, there's no other way to master it than to understand how your data is stored and moves through memory.

Although this engine is simple, the design choices here reflect the best wisdom I could find out there on the internet, so it is powerful, easy to customize, and easy to understand.
It can be hard to work through all of that information to find the right software if you're not already familiar with it. So hopefully this can let you skip that and get right to the fun parts of experimenting with C++ simulations and games.

I have made this available mostly for my own convenience as I base many small projects off of it. But I would be thrilled if anyone else wanted to use it too. You can use it for whatever you like, including commercial purposes, and you don't have to pay me or even credit me. Please modify it as you see fit.

The commenting and documentation is rudimentary at this point but I hope to do it properly very soon.

One day, I hope this will be the most popular game engine in the world!

## Components

Rendering is done with OpenGL.
Threading is done with boost thread.
Game physics is done with Box2D.
keyboard, mouse, windowing, and audio is provided by SDL2.

## How to use it

### Requirements

You need to make sure boost, SDL2, and openGL are installed. A portable copy of box2d is provided.

### Building

Building is done with cmake and make. Just go in to the project folder and say 'make'.