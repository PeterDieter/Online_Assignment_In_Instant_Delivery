#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <assert.h>
#include <string>
#include <vector>
#include <limits.h>
#include <iostream>
#include <ctime>
#include <chrono>
#include <random>

#include "Matrix.h"
#include "Params.h"
#include "Environment.h"

class Environment
{
public:
	// Constructor 
	Environment(Params* params);

	// Function to perform a simulation
	void simulate(int timeLimit);

private:
	Params* params;			// Problem parameters
	std::vector<Order> orders;			// Vector containing information on each order
	std::vector<Warehouse> warehouses;	// Vector containing information on each warehouse
	std::vector<Courier> couriers;		// Vector containing information on each courier
	std::vector<Picker> pickers;		// Vector containing information on each courier
	Order newOrder;
	std::vector<int> newOrderDistancesToWarehouses; 
	int indexClosestWarehouse;
	
	// In this method we initialize the rest of the Data, such as warehouses, couriers, etc.
	void initialize();

	// Function to create a new order
	Order createOrder(int currentTime);

	// Functions that assigns order to a warehouse, picker and courier, respectively
	Warehouse* chooseWarehouse();
	Picker* choosePicker();
	Courier* chooseCourier();

	// Function to draw an inter arrival time based on rate specified in params
	int drawInterArrivalTime();

};


#endif