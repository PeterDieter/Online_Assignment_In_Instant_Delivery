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
#include "xorshift128.h"

struct Quadrant;
struct Courier;
struct Picker;

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
	int wareInd;										// Index of the warehouse
	int workLoad;										// The work load in needed to comission all assigned orders in seconds
	int initialNbCouriers;								// Initial number of couriers
	int initialNbPickers;								// Initial number of pickers
	std::vector< Courier*> couriersAssigned; 			// vector of pointers to couriers which are assigned to the warehouse
	std::vector< Picker*> pickersAssigned; 				// vector of pointers to pickers which are assigned to the warehouse
	std::vector< Quadrant*> assignedQuadrants; 			// vector of pointers to quadrants which are assigned to the warehouse
	double lat;											// Latitude
	double lon;											// Longitude 
};


// Structure of a Warehouse, including its index, position
struct Order
{
	int orderInd;					// Index of the warehouse
	Client* client; 				// pointer to couriers which are currently available at the warehouse
	Warehouse* assignedWarehouse; 	// pointer to quadrants which are assigned to the warehouse
	Courier* assignedCourier;		// Courier assigned to order
	Picker* assignedPicker;			// Picker assigned to order
	int orderTime;					// Time the order arrives in the system
	int comissionTime;				// Time it takes to comission the order
	int arrivalTime;				// time the courier arrives at the client, i.e., the client is served
};

// Structure of a Courier, including its index, position, etc.
struct Courier
{
	int courierInd;							// Index of the courier
	int timeWhenAvailable; 					// Gives the time when the courier is available again, i.e., he return to a warehouse
	std::vector< Order*> assignedToOrders;	// Vector of pointer to orders to which the courier is assigned to
	Warehouse* assignedToWarehouse;			// Warehouse where the courier is located or where he is heading to
};

// Structure of a Courier, including its index, position, etc.
struct Picker
{
	int pickerInd;							// Index of the picker
	int timeWhenAvailable;					// Gives the time when the picker is available again, i.e., completed all his tasks
	std::vector< Order*> assignedToOrders;	// Vector of pointer to orders to which the picker is assigned to
	Warehouse* assignedToWarehouse;			// Warehouse where the picker is located to
};

// Structure of a quadrant, including its index, clients and possibly position?
struct Quadrant
{
	int quadrantInd;							// Index of the quadrant
	Warehouse* assignedToWarehouse;				// Pointer to warehouse the quadrant is assigned to
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
	int nbCouriers;						// Total Number of couriers
	int nbPickers;						// Total number of pickers
	double interArrivalTime;			// Inter arrival time of incoming orders
	std::vector<Client> paramClients;		// Vector containing information on each client
	std::vector<Quadrant> paramQuadrants;	// Vector containing information on each quadrant
	std::vector<Warehouse> paramWarehouses;	// Vector containing information on each warehouse
	std::vector<Courier> paramCouriers;		// Vector containing information on each courier
	std::vector<Courier> paramPickers;		// Vector containing information on each picker
	Matrix travelTime;					// Distance matrix from clients to warehouses (symetric)
	XorShift128 rng;					// Fast random number generator
};


#endif