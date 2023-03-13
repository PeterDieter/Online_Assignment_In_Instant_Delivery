#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <assert.h>
#include <string>
#include <vector>
#include <limits.h>
#include <iostream>
#include <ctime>
#include <chrono>

#include "Matrix.h"
#include "Params.h"
#include "Environment.h"

class Environment
{
public:
	Params* params;			// Problem parameters
	std::vector<Order> orders;			// Vector containing information on each order
	std::vector<Warehouse> warehouses;	// Vector containing information on each warehouse
	std::vector<Courier> couriers;		// Vector containing information on each courier

	// In this method we initialize the rest of the Data, such as warehouses, couriers, etc.
	void InitializeEnvironment(Params* params);

	// Constructor
	Environment(Params* params);

};


#endif