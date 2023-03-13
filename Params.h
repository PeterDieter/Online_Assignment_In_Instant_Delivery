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
struct Courier;

// Structure of a Client, including its index, position, and quadrant
struct Client
{
	int clientInd;				// Index of the client
	double lat;					// Latitude
	double lon;					// Longitude 
	int inQuadrantInd;			// Index of quadrant in which the client is located
	Quadrant* inQuadrant;		// pointer to quadrant in which the client is located
};

// Structure of a Warehouse, including its index, position
struct Warehouse
{
	int wareInd;								// Index of the warehouse
	int workLoad;								// The work load in needed to comission all assigned orders in seconds
	int initialNbCouriers;						// Initial number of couriers available
	std::vector< Courier*> couriersAvailable; 	// vector of pointers to couriers which are currently available at the warehouse
	std::vector< Quadrant*> assignedQuadrants; 	// vector of pointers to quadrants which are assigned to the warehouse
	double lat;									// Latitude
	double lon;									// Longitude 
};


// Structure of a Warehouse, including its index, position
struct Order
{
	int orderInd;					// Index of the warehouse
	Client* client; 				// pointer to couriers which are currently available at the warehouse
	Warehouse* assignedWarehouse; 	// pointer to quadrants which are assigned to the warehouse
	Courier* assignedCourier;		// Courier assigned to order
	int orderTime;					// Time the order arrives in the system
	int comissionTime;				// Time it takes to comission the order
	int arrivalTime;				// time the courier arrives at the customer, i.e., the customer is served
};

// Structure of a Courier, including its index, position, etc.
struct Courier
{
	int courierInd;					// Index of the courier
	double currentLat;				// Current Latitude
	double currentLon;				// Current Longitude 
	bool available;					// States if courier is currently avaible to be assigned to an order
	Order* assignedToOrder;			// Order to which the courier is assigned to
	Warehouse* assignedToWarehouse;	// Warehouse where the courier is located or where he is heading to
};

// Structure of a quadrant, including its index, clients and possibly position?
struct Quadrant
{
	int quadrantInd;							// Index of the quadrant
	std::vector< Client*> clientsInQuadrant; 	// vector of pointers to clients which are in the quadrant;
};



class Params
{
public:
	Params();
	// Data of the problem instance
	int nbClients;						// Number of clients
	int nbQuadrants;					// Number of quadrants
	int nbWarehouses;					// Number of warehouses
	int nbCouriers;						// Number of couriers
	std::vector<Client> paramClients;		// Vector containing information on each client
	std::vector<Quadrant> paramQuadrants;	// Vector containing information on each quadrant
	std::vector<Warehouse> paramWarehouses;	// Vector containing information on each warehouse
	std::vector<Courier> paramCouriers;		// Vector containing information on each courier
	Matrix travelTime;					// Distance matrix from clients to warehouses (symetric)
};


#endif