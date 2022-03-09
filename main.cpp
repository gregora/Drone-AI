#include "include/network.h"
#include "include/drone.h"
#include "include/misc.h"
#include <stdio.h>
#include <iostream>
#include <limits>
#include <chrono>
#include <string.h>
#include <string>
#include <thread>
#include <vector>
#include <ctime>


void evaluate(int size, nnlib::Network ** networks, float * scores, float time, float sx, float sy, bool display = true, bool record = false);
float calculate_score(Drone* drone);
int min(int size, float * arr);
void sort_scores(int size, float* scores, nnlib::Network ** networks);

//default values
int POPULATION_SIZE = 1000;
int SAMPLE_NUM = 5;
int MUTATIONS = 2;
int MAX_GEN = 1000;

bool MULTITHREADING = false;

bool DISPLAY = false;
bool RECORD = false;
bool SHOW_BEST = false;

int WIDTH = 1000;
int HEIGHT = 500;
float PPM = 10;

int main(int argsn, char** args){
	//initial creation of networks
	nnlib::Network * networks[POPULATION_SIZE];

	char* load = nullptr;

	for(int i = 0; i < argsn; i++){
		if(strcmp(args[i], "-population") == 0){
			POPULATION_SIZE = atol(args[i + 1]);
		}else if(strcmp(args[i], "-samples") == 0){
			SAMPLE_NUM = atol(args[i + 1]);
		}else if(strcmp(args[i], "-mutations") == 0){
			MUTATIONS = atol(args[i + 1]);
		}else if(strcmp(args[i], "-max") == 0){
			MAX_GEN = atol(args[i + 1]);
		}else if(strcmp(args[i], "-display") == 0){
			//turn on display
			DISPLAY = true;
		}else if(strcmp(args[i], "-record") == 0){
			//record
			RECORD = true;
		}else if(strcmp(args[i], "-showbest") == 0){
			//show only the best drone
			SHOW_BEST = true;
		}else if(strcmp(args[i], "-multithreading") == 0){
			MULTITHREADING = true;
		}else if(strcmp(args[i], "-load") == 0){
			load = args[i + 1];
		}
	}

	for(int i = 0; i < POPULATION_SIZE; i++){
		nnlib::Network * network = new nnlib::Network();

		if(load == nullptr){
			nnlib::Dense * layer1 = new nnlib::Dense(6, 4);
			layer1 -> randomize(0, 0);
			nnlib::Dense * layer2 = new nnlib::Dense(4, 2);
			layer1 -> randomize(0, 0);

			network -> addLayer(layer1);
			network -> addLayer(layer2);
		}else{
			std::string path = "saves/generation";
			std::string generation(load);
			network -> load(path + generation + "/" + std::to_string(i) + ".AI");
		}

		networks[i] = network;
	}

	int first_gen = 1;
	if(load != nullptr){
		first_gen = atoi(load);
	}

	for(int i = first_gen; i <= 1000; i++){
		//simulate generation
		float avg_scores[POPULATION_SIZE];
		for(int j = 0; j < POPULATION_SIZE; j++){
			avg_scores[j] = 0;
		}

		printf("\nGENERATION %d\n", i);
		time_t start = time(0);

		std::vector<std::thread> threads;
		float * all_scores[SAMPLE_NUM];

		//run simulation multiple times with multiple random positions
		for(int j = 0; j < SAMPLE_NUM; j++){
			float * scores = new float[POPULATION_SIZE];
			all_scores[j] = scores;

			if(!DISPLAY){
				if(MULTITHREADING){
					threads.push_back(std::thread(evaluate, POPULATION_SIZE, &networks[0], scores, 20, j*9372 % 100 - 50, j*4383 % 50 - 25, false, false));
				}else{
					evaluate(POPULATION_SIZE, &networks[0], scores, 20, j*9372 % 100 - 50, j*4383 % 50 - 25, false, false);
				}
			}else{
				evaluate(POPULATION_SIZE, &networks[0], scores, 20, nnlib::randomInt(-50, 50), nnlib::randomInt(-25, 25), true, RECORD);
				return 0;
			}

		}

		//join threads
		for(int j = 0; j < SAMPLE_NUM; j++){
			if(MULTITHREADING){
				threads[j].join();
			}

			for(int k = 0; k < POPULATION_SIZE; k++){
				avg_scores[k] += all_scores[j][k];
			}
			delete[] all_scores[j];
		}

		//find the best performer
		int min_score = min(POPULATION_SIZE, &avg_scores[0]);
		printf("Min score: %f\n", sqrt(avg_scores[min_score] / SAMPLE_NUM));

		//sort scores in order
		sort_scores(POPULATION_SIZE, avg_scores, networks);

		//save networks
		for(int j = 0; j < POPULATION_SIZE; j++){
			networks[j] -> save("saves/generation"+std::to_string(i)+"/"+std::to_string(j)+".AI");
		}

		int half_population = (POPULATION_SIZE + 1) / 2;

		//replace losers with new drones
		for(int i = 0; i < half_population; i++){
			if(i + half_population < POPULATION_SIZE){

				//delete network
				delete networks[i+half_population];

				//create a child
				networks[i+half_population] = new nnlib::Network();

				//choose random parents
				int parent1 = nnlib::randomInt(0, half_population - 1);
				int parent2 = nnlib::randomInt(0, half_population - 1);

				nnlib::Dense * layer1_1 = ((nnlib::Dense *)(networks[parent1] -> getLayer(0)));
				nnlib::Dense * layer1_2 = ((nnlib::Dense *)(networks[parent2] -> getLayer(0)));

				nnlib::Dense * layer2_1 = ((nnlib::Dense *)(networks[parent1] -> getLayer(1)));
				nnlib::Dense * layer2_2 = ((nnlib::Dense *)(networks[parent2] -> getLayer(1)));

				networks[i+half_population] -> addLayer(layer1_1 -> crossover(layer1_2));
				networks[i+half_population] -> addLayer(layer2_1 -> crossover(layer2_2));

				//mutate child
				for(int j = 0; j < MUTATIONS; j++){
					((nnlib::Dense*)(networks[i+half_population] -> getLayer(0))) -> mutate(-1, 1);
					((nnlib::Dense*)(networks[i+half_population] -> getLayer(1))) -> mutate(-1, 1);
				}

			}
		}

		time_t end = time(0);
		printf("Computation done in %ld s\n", end - start);
	}
}



void evaluate(int size, nnlib::Network ** networks, float * scores, float time, float sx, float sy, bool display, bool record){
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

	sf::Texture centre_t;
	sf::Sprite centre;

	if(DISPLAY){
		centre_t.loadFromFile("img/target.png");
		centre.setTexture(centre_t);
		centre.setOrigin(10, 10);
		centre.setPosition(WIDTH/2, HEIGHT/2);
	}

	Drone* drones[size];

	//set initial positions
	for(int i = 0; i < size; i++){
		drones[i] = new Drone("img/drone.png", display);
		drones[i]->setColor(255*i/size, 255*i/size, 255*i/size);
		drones[i]->x = sx;
		drones[i]->y = sy;
	}


	//main physics + rendering loop
	int frame = -1;
	float elapsed = 0;
	float delta = 1.0f / 60.0f;

	while(elapsed < time){
		frame++;
		elapsed += delta;

		if(display){
			renderTexture -> clear();
			renderTexture -> draw(centre);
		}

		//apply network + physics + drawing for each drone
		for(int i = size - 1; i >= 0; i--){
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

				if(i != 0){
					drones[i] -> setOpacity(50);
				}

				if(!SHOW_BEST || (i == 0)){
					renderTexture -> draw(*drones[i]);
				}
			}

		}

		if(SHOW_BEST && display){

			int h1 = ((float) 120)*(drones[0] -> left_power);
			int h2 = ((float) 120)*(drones[0] -> right_power);

			sf::RectangleShape power1(sf::Vector2f(40, 120));
			sf::RectangleShape power2(sf::Vector2f(40, 120));

			power1.setPosition(WIDTH - 60, HEIGHT - 20 - 120);
			power2.setPosition(WIDTH - 120, HEIGHT - 20 - 120);

			power1.setFillColor(sf::Color(50, 50, 50));
			power2.setFillColor(sf::Color(50, 50, 50));

			renderTexture -> draw(power1);
			renderTexture -> draw(power2);

			power1.setSize(sf::Vector2f(40, h1));
			power2.setSize(sf::Vector2f(40, h2));

			power1.setFillColor(sf::Color(255, 255, 255));
			power2.setFillColor(sf::Color(255, 255, 255));

			power1.setPosition(WIDTH - 60, HEIGHT - 20 - h1);
			power2.setPosition(WIDTH - 120, HEIGHT - 20 - h2);

			renderTexture -> draw(power1);
			renderTexture -> draw(power2);
		}

		//display
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

		//save frame to file
		if(display && record){
			renderTexture -> getTexture().copyToImage().saveToFile("frames/" + std::to_string(frame) + ".png");
			printf("Frame %d rendered!\n", frame);
		}
	}

	for(int i = 0; i < size; i++){
		scores[i] = calculate_score(drones[i]);
		delete drones[i];
	}


	if(window != nullptr)
		delete window;

}

float calculate_score(Drone* drone){
	float score = 0;
	float x = drone -> x;
	float y = drone -> y;
	float speedx = drone -> speedx;
	float speedy = drone -> speedy;
	float angle = drone -> angle;
	float angular_velocity = drone -> angular_velocity;

	score = y*y +x*x;
	return score;
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
