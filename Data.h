#ifndef DATA_H
#define DATA_H

#include <assert.h>
#include <string>
#include <vector>
#include <limits.h>
#include <iostream>
#include <ctime>
#include <chrono>

#include "Matrix.h"
#include "Data.h"
#include "xorshift128.h"

struct Courier;
struct Picker;
struct Order;

// Structure of a Client, including its index, position
struct Client
{
	int clientID;				// IDex of the client
	double lat;					// Latitude
	double lon;					// Longitude 
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
	double lat;											// Latitude
	double lon;											// Longitude 
};


// Structure of a Warehouse, including its index, position
struct Order
{
	int orderID;					// ID of the warehouse
	Client* client; 				// pointer to couriers which are currently available at the warehouse
	bool accepted;					// states if the order has been accepted or not
	Warehouse* assignedWarehouse; 	// pointer to warehouse the order is assigned to
	Courier* assignedCourier;		// Courier assigned to order
	Picker* assignedPicker;			// Picker assigned to order
	int orderTime;					// Time the order arrives in the system
	int timeToComission;			// Time it takes to comission the order
	int serviceTimeAtClient;		// Time it takes to serve the client at the door
	int arrivalTime;				// time the courier arrives at the client, i.e., the client is served
	float actionProbability;		// Probability of the chosen action according to policy network (Only relevant for Policy Gradient methods!)
};

// Structure of a route. This is stored mainly for plotting purposes later on
struct Route
{
	double fromlLat;				// From latitude
	double fromlon;					// From longitude
	double tolLat;					// From latitude
	double tolon;					// From longitude
	int startTime;					// 
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
	Warehouse* assignedToWarehouse;			// Warehouse where the picker is located to
};

class Data
{
public:
	Data(std::string instanceName);
	// Data of the problem instance
	int nbClients;							// Number of clients
	int nbWarehouses;						// Number of warehouses
	int nbCouriers;							// Total Number of couriers
	int nbPickers;							// Total number of pickers
	double interArrivalTime;				// Inter arrival time of incoming orders
	double meanCommissionTime;				// Mean time it takes to commission an order (exponential distributed)
	double meanServiceTimeAtClient;			// Mean time it takes to serivce an order (at the client) (exponential distributed)
	std::vector<Client> paramClients;		// Vector containing information on each client
	std::vector<Warehouse> paramWarehouses;	// Vector containing information on each warehouse
	Matrix travelTime;						// Distance matrix from clients to warehouses (symetric)
	XorShift128 rng;						// Fast random number generator
};


#endif