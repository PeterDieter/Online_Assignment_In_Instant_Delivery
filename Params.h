#ifndef PARAMS_H
#define PARAMS_H

#include <assert.h>
#include <string>
#include <vector>
#include <limits.h>
#include <iostream>
#include <ctime>
#include <chrono>

#include "Matrix.h"
#include "Params.h"

struct Quadrant;

// Structure of a Client, including its index, position, and quadrant
struct Client
{
	int clientInd;				// Index of the client
	double coordLat;			// Latitude
	double coordLon;			// Longitude 
	Quadrant* inQuadrant;		// pointer to quadrant in which the client is located
};

// Structure of a quadrant, including its index, clients and possibly position?
struct Quadrant
{
	int quadrantInd;							// Index of the quadrant
	std::vector< Client*> clientsInQuadrant; 	// vector of pointers to clients which are in the quadrant;
};

// Structure of a Warehouse, including its index, position
struct Warehouse
{
	int wareInd;			// Index of the warehouse
	int coordLat;			// Latitude
	int coordLon;			// Longitude 
};

class Params
{
public:
	Params();
	// Data of the problem instance
	int nbClients;						// Number of clients
	int nbQuadrants;					// Number of quadrants
	int nbWarehouses;					// Number of warehouses
	std::vector<Client> clients;		// Vector containing information on each client (including the depot!)
	std::vector<Quadrant> quadrants;	// Vector containing information on each client (including the depot!)
	std::vector<Warehouse> warehouses;	// Vector containing information on each client (including the depot!)
	Matrix travelTime;					// Distance matrix from client to warehouse (symetric)
};


#endif