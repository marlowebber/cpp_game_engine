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

Rendering is done with OpenGL. It is a modern implementation using proper vbo philosophy. It can be made faster if you don't clumsily iterate through the list of objects (twice), but it is easy to understand this way.

Threading is done with boost thread.
File IO, for saving and importing, also comes from boost.
Game physics is done with Box2D.
keyboard, mouse, windowing, and audio is provided by SDL2. This means great portability.

Menus are provided by my source code, which has a lightweight custom implementation. You can set up menus with callback functions when they are clicked on, or display the value of a variable.
The ability to draw text was taken from the OpenGL tutorial. http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-11-2d-text/
It uses bitmap text, which is fast and efficient, but inflexible. You must alter the image to change the font or the color.

## How to use it

### Requirements

You need to make sure boost, SDL2, and openGL are installed. A portable copy of box2d is provided.

### Building

Building is done with cmake and make.
	1a. clone this repository.
	1b. install its requirements.
	1c. cmake .
	1d. make
	1e. ./game

	if it worked, a scene will show up where a black square falls onto a black rectangle. Hit escape to close it.


### Customization

You want to make a game but don't want to spend forever choosing a sensible set of tools and getting them all set up. Use this instead to hit the ground running.

1. build and make this project, so you know it works.

2. add your custom game logic to 'src/game.cpp'. The function 'threadGame' gets run once every frame, and 'initializeGame' gets run once at startup. 
	If you want to use the physics, a class for physical bodies is provided, with functions to cleanly create and destroy it from the world.

3. add your graphics logic to the function 'threadGraphics' in 'src/game.cpp'. This also gets run once every frame. Inside it, you must instruct the computer how to draw the vertices of your game objects. They must be transcribed into the format (r, g, b, a, x, y). R, G, and B are colors between 0 and 1. A is the alpha, or transparency, which is between 0 and 1. X and Y are the position, and can be any real number. All six numbers are floats.

4. add key controls in 'main.cpp'. 

5. That's it, build and run your game.