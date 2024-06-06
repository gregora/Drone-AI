# Drone-AI

A project for teaching a neural network to control a drone using genetic algorithm.

## Results
Great results were achieved after only 400 generations.

https://github.com/gregora/Drone-AI/assets/26600857/c0e435fd-adde-4d00-8164-25889f45468b

This is the accompanying Youtube video: [https://youtu.be/l1SYa1WjSYQ](https://youtu.be/l1SYa1WjSYQ)

## Dependencies
* [Drone interface](https://github.com/gregora/Drone) (included)
* [NN-Lib](https://github.com/gregora/NN-Lib) (included)
* [SFML](https://www.sfml-dev.org/)

## Compiling

### Linux
You can simply run `compile.sh` file. Compiling with this method also runs the binary.

## Running
**Before running make sure that `/saves/[generation number]/` folders exist**

Binary accepts these command line arguments:

* Learning algorithm
	* `-population [size]` - specify population size (*default: 1000*)  
	* `-samples [number of samples]` - specify the number of reruns of each generation (*default: 5*)
	* `-mutations [mutations / reproduction]` - frequency of mutations (*default: 2*)
	* `-max [max generation]` - max generation (*default: 1000*)
	* `-time [limit]` - limit the simulation time in seconds (*default: 20*)
	* `-load [generation number]` - load a generation from `saves/[generation number]/`
* Graphics  
	* `-display` - render to screen (**automatically exits after one generation**)
	* `-width [px]` - screen width in pixels
	* `-height [px]` - screen height in pixels
	* `-record` - save frames as .png to the `/frames` folder
	* `-showctr` - visualize engine input
* Other
	* `-multithreading` - use multiple threads for computations (only applicable when not rendering to the screen)
