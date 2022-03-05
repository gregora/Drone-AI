#include "include/network.h"
#include "include/drone.h"
#include "include/misc.h"
#include <stdio.h>

float * evaluate(int size, nnlib::Network ** networks, float time, float display = false);

int POPULATION_SIZE = 100;
int WIDTH = 1000;
int HEIGHT = 500;

float PPM = 10;

int main(){
	//initial creation of networks
	nnlib::Network * networks[POPULATION_SIZE];

	for(int i = 0; i < POPULATION_SIZE; i++){
		nnlib::Network * network = new nnlib::Network();
		nnlib::Dense * layer1 = new nnlib::Dense(6, 2);
		network -> addLayer(layer1);
		networks[i] = network;
	}

	evaluate(POPULATION_SIZE, &networks[0], 20, true);
}



float * evaluate(int size, nnlib::Network ** networks, float time, float display){

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderTexture* renderTexture = new sf::RenderTexture();
	sf::RenderWindow* window;

	if(display){
		renderTexture->create(WIDTH, HEIGHT);
		window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Drones", sf::Style::Default, settings);
		window->clear();
		window->display();
	}



	float* scores = (float *) calloc(size, sizeof(float));
	Drone drones[size];

	float elapsed = 0;
	float delta = 1.0f / 60.0f;

	int sx = nnlib::randomInt(-50, 50);
	int sy = nnlib::randomInt(-25, 25);

	//set initial position
	for(int i = 0; i < size; i++){
		drones[i].x = sx;
		drones[i].y = sy;
	}


	while(elapsed < time){
		elapsed += delta;

		if(display){
			renderTexture -> clear();
		}

		for(int i = 0; i < size; i++){
			nnlib::Matrix input(1,6);
			input.setValue(0, 0, drones[i].x);
			input.setValue(0, 1, drones[i].y);
			input.setValue(0, 2, drones[i].speedx);
			input.setValue(0, 3, drones[i].speedy);
			input.setValue(0, 4, drones[i].angle);
			input.setValue(0, 5, drones[i].angular_velocity);

			nnlib::Matrix output = networks[i] -> eval(&input);

			float left = output.getValue(0, 0);
			float right = output.getValue(0, 1);

			drones[i].setPower(left, right);
			drones[i].physics(delta, false);

			if(display){
				drones[i].setPosition(WIDTH/2 + drones[i].x*PPM, HEIGHT/2 - drones[i].y*PPM);
				drones[i].setRotation(drones[i].angle * 180/3.14);
				renderTexture -> draw(drones[i]);
			}

		}

		if(display){
			renderTexture -> display();
			sf::Texture screen_texture = renderTexture -> getTexture();

			window->clear();
			window->draw(sf::Sprite(renderTexture -> getTexture()));
			window->display();

			sf::Event event;
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed){
					window->close();
					exit(1);
				}
			}
		}
	}

	delete window;

}
