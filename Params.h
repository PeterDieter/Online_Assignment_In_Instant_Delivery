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
struct Order;

// Structure of a Client, including its index, position, and quadrant
struct Client
{
	int clientID;				// IDex of the client
	double lat;					// Latitude
	double lon;					// Longitude 
	int inQuadrantID;			// IDex of quadrant in which the client is located
	Quadrant* inQuadrant;		// pointer to quadrant in which the client is located
};

// Structure of a Warehouse, including its index, position
struct Warehouse
{
	int wareID;											// ID of the warehouse
	int initialNbCouriers;								// Initial number of couriers
	int initialNbPickers;								// Initial number of pickers
	std::vector< Courier*> couriersAssigned; 			// vector of pointers to couriers which are assigned to the warehouse
	std::vector< Picker*> pickersAssigned; 				// vector of pointers to pickers which are assigned to the warehouse
	std::vector< Order*> ordersNotAssignedToCourier;	// vector to orders that are assigned to the warehouse, but not to a courier yet
	std::vector< Quadrant*> assignedQuadrants; 			// vector of pointers to quadrants which are assigned to the warehouse
	double lat;											// Latitude
	double lon;											// Longitude 
};


// Structure of a Warehouse, including its index, position
struct Order
{
	int orderID;					// ID of the warehouse
	Client* client; 				// pointer to couriers which are currently available at the warehouse
	Warehouse* assignedWarehouse; 	// pointer to quadrants which are assigned to the warehouse
	Courier* assignedCourier;		// Courier assigned to order
	Picker* assignedPicker;			// Picker assigned to order
	int orderTime;					// Time the order arrives in the system
	int timeToComission;			// Time it takes to comission the order
	int arrivalTime;				// time the courier arrives at the client, i.e., the client is served
};

// Structure of a Courier, including its index, position, etc.
struct Courier
{
	int courierID;							// ID of the courier
	int timeWhenAvailable; 					// Gives the time when the courier is available again, i.e., he is (back) at a warehouse
	Order* assignedToOrder;					// pointer to order to which the courier is assigned to
	Warehouse* assignedToWarehouse;			// Warehouse where the courier is located or where he is heading to
};

// Structure of a Courier, including its index, position, etc.
struct Picker
{
	int pickerID;							// ID of the picker
	int timeWhenAvailable;					// Gives the time when the picker is available again, i.e., completed all his tasks
	std::vector< Order*> assignedToOrders;	// Vector of pointer to orders to which the picker is assigned to. Pickers can be assigned to multiple orders at the same time
	Warehouse* assignedToWarehouse;			// Warehouse where the picker is located to
};

// Structure of a quadrant, including its index, clients and possibly position?
struct Quadrant
{
	int quadrantID;								// ID of the quadrant
	Warehouse* assignedToWarehouse;				// Pointer to warehouse the quadrant is assigned to
	std::vector< Client*> clientsInQuadrant; 	// vector of pointers to clients which are in the quadrant;
};



class Params
{
public:
	Params(std::string instanceName);
	// Data of the problem instance
	int nbClients;							// Number of clients
	int nbQuadrants;						// Number of quadrants
	int nbWarehouses;						// Number of warehouses
	int nbCouriers;							// Total Number of couriers
	int nbPickers;							// Total number of pickers
	double interArrivalTime;				// Inter arrival time of incoming orders
	double meanCommissionTime;				// Mean time it takes to commission an order (exponential distributed)
	std::vector<Client> paramClients;		// Vector containing information on each client
	std::vector<Quadrant> paramQuadrants;	// Vector containing information on each quadrant
	std::vector<Warehouse> paramWarehouses;	// Vector containing information on each warehouse
	Matrix travelTime;						// Distance matrix from clients to warehouses (symetric)
	XorShift128 rng;						// Fast random number generator
};


#endif