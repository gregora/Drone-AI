#include "include/network.h"
#include "include/drone.h"
#include "include/misc.h"
#include <stdio.h>
#include <iostream>
#include <limits>
#include <chrono>


float * evaluate(int size, nnlib::Network ** networks, float time, float sx, float sy, float display = true);
float calculate_score(Drone* drone);
int min(int size, float * arr);
void sort_scores(int size, float* scores, nnlib::Network ** networks);

int POPULATION_SIZE = 1000;
int SAMPLE_NUM = 5;
int MUTATIONS = 2;

int WIDTH = 1000;
int HEIGHT = 500;
float PPM = 10;

int main(){
	//initial creation of networks
	nnlib::Network * networks[POPULATION_SIZE];

	for(int i = 0; i < POPULATION_SIZE; i++){
		nnlib::Network * network = new nnlib::Network();
		nnlib::Dense * layer1 = new nnlib::Dense(6, 4);
		layer1 -> randomize(0, 0);
		nnlib::Dense * layer2 = new nnlib::Dense(4, 2);
		layer1 -> randomize(0, 0);

		network -> addLayer(layer1);
		network -> addLayer(layer2);
		networks[i] = network;
	}

	for(int i = 1; i <= 1000; i++){
		//simulate generation
		float avg_scores[POPULATION_SIZE];
		for(int j = 0; j < POPULATION_SIZE; j++){
			avg_scores[j] = 0;
		}

		printf("Generation %d\n", i);

		//run simulation multiple times with multiple random positions
		for(int j = 0; j < SAMPLE_NUM; j++){
			float * scores = evaluate(POPULATION_SIZE, &networks[0], 20, j*9372 % 100 - 50, j*4383 % 50 - 25, (i%1 == 0) && (j == 1));
			for(int k = 0; k < POPULATION_SIZE; k++){
				avg_scores[k] += scores[k];
			}
			delete[] scores;
		}

		int min_score = min(POPULATION_SIZE, &avg_scores[0]);
		printf("Min score: %f\n", sqrt(avg_scores[min_score] / SAMPLE_NUM));

		sort_scores(POPULATION_SIZE, avg_scores, networks);

		for(int j = 0; j < POPULATION_SIZE; j++){
			networks[j] -> save("saves/generation"+std::to_string(i)+"/"+std::to_string(j)+".AI");
		}

		int half_population = (POPULATION_SIZE + 1) / 2;

		//replace losers with new drones
		for(int i = 0; i < half_population; i++){
			if(i + half_population < POPULATION_SIZE){

				delete networks[i+half_population];

				networks[i+half_population] = new nnlib::Network();

				int parent1 = nnlib::randomInt(0, half_population - 1);
				int parent2 = nnlib::randomInt(0, half_population - 1);

				nnlib::Dense * layer1_1 = ((nnlib::Dense *)(networks[parent1] -> getLayer(0)));
				nnlib::Dense * layer1_2 = ((nnlib::Dense *)(networks[parent2] -> getLayer(0)));

				nnlib::Dense * layer2_1 = ((nnlib::Dense *)(networks[parent1] -> getLayer(1)));
				nnlib::Dense * layer2_2 = ((nnlib::Dense *)(networks[parent2] -> getLayer(1)));


				networks[i+half_population] -> addLayer(layer1_1 -> crossover(layer1_2));
				networks[i+half_population] -> addLayer(layer2_1 -> crossover(layer2_2));

				for(int j = 0; j < MUTATIONS; j++){
					((nnlib::Dense*)(networks[i+half_population] -> getLayer(0))) -> mutate(-1, 1);
					((nnlib::Dense*)(networks[i+half_population] -> getLayer(1))) -> mutate(-1, 1);
				}

			}
		}

	}
}



float * evaluate(int size, nnlib::Network ** networks, float time, float sx, float sy, float display){
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderTexture* renderTexture = new sf::RenderTexture();
	sf::RenderWindow* window = nullptr;

	if(display){
		renderTexture->create(WIDTH, HEIGHT);
		window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Drones", sf::Style::Default, settings);
		window->clear();
		window->display();
	}



	float* scores = (float *) calloc(size, sizeof(float));
	Drone* drones[size];

	float elapsed = 0;
	float delta = 1.0f / 60.0f;


	//set initial position
	for(int i = 0; i < size; i++){
		drones[i] = new Drone("img/drone.png", display);
		drones[i]->x = sx;
		drones[i]->y = sy;
	}


	while(elapsed < time){
		elapsed += delta;

		if(display){
			renderTexture -> clear();
		}

		for(int i = 0; i < size; i++){
			nnlib::Matrix input(1,6);
			input.setValue(0, 0, drones[i]->x);
			input.setValue(0, 1, drones[i]->y);
			input.setValue(0, 2, drones[i]->speedx);
			input.setValue(0, 3, drones[i]->speedy);
			input.setValue(0, 4, drones[i]->angle);
			input.setValue(0, 5, drones[i]->angular_velocity);

			nnlib::Matrix output = networks[i] -> eval(&input);

			float left = output.getValue(0, 0);
			float right = output.getValue(0, 1);

			drones[i]->setPower(left, right);
			drones[i]->physics(delta, false);

			if(display){
				drones[i]->setPosition(WIDTH/2 + drones[i]->x*PPM, HEIGHT/2 - drones[i]->y*PPM);
				drones[i]->setRotation(drones[i]->angle * 180/3.14);
				renderTexture -> draw(*drones[i]);
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

	for(int i = 0; i < size; i++){
		scores[i] = calculate_score(drones[i]);
	}

	if(window != nullptr)
		delete window;

	return scores;
}

float calculate_score(Drone* drone){
	float score = 0;
	float x = drone -> x;
	float y = drone -> y;
	float speedx = drone -> speedx;
	float speedy = drone -> speedy;
	float angle = drone -> angle;
	float angular_velocity = drone -> angular_velocity;

	score = y*y;
}

int min(int size, float * arr){
	float min_i = 0;
	int index = -1;
	for(int i = 0; i < size; i++){
		if(i == 0){
			min_i = arr[i];
			index = 0;
		}else{
			if(min_i > arr[i]){
				min_i = arr[i];
				index = i;
			}
		}
	}

	return index;
}

void sort_scores(int size, float* scores, nnlib::Network ** networks){

	for(int i = 0; i < size; i++){
		int min_index = min(size - i, scores + i) + i;

		//switch networks
		float first_val = scores[i];
		nnlib::Network* first_net = networks[i];

		scores[i] = scores[min_index];
		networks[i] = networks[min_index];

		scores[min_index] = first_val;
		networks[min_index] = first_net;
	}
}
