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
	Params* params;							// Problem parameters
	std::vector<Order> orders;				// Vector containing information on each order
	std::vector<Order*> ordersNotServed;	// Vector pointing to orders that have not been served yet
	std::vector<Warehouse> warehouses;		// Vector containing information on each warehouse
	std::vector<Courier> couriers;			// Vector containing information on each courier
	std::vector<Picker> pickers;			// Vector containing information on each courier
	Order* nextOrderBeingServed;
	std::vector<int> newOrderDistancesToWarehouses; 
	int currentTime;
	int nbOrdersServed;
	int timeCustomerArrives;
	int timeNextCourierArrivesAtOrder;

	// In this method we initialize the rest of the Data, such as warehouses, couriers, etc.
	void initialize();

	// Function to create a new order
	Order createOrder(int currentTime);

	// Functions that assigns order to a warehouse, picker and courier, respectively
	void chooseWarehouseForOrder(Order* newOrder);
	void choosePickerForOrder(Order* newOrder);
	void chooseCourierForOrder(Order* newOrder);

	// Function that assigns a courier to a warehouse
	void chooseWarehouseForCourier(Courier* courier);

	// Function that deletes order from ordersNotServed vector
	void RemoveOrderFromNotServedVector(std::vector<Order*> & V, Order* orderToDelete);

	// Function that returns the fastest available picker
	Picker* getFastestAvailablePicker(Warehouse* warehouse);

	// Function that returns the fastest available courier
	Courier* getFastestAvailableCourier(Warehouse* warehouse);

	// Function that updates the order that will be served next
	void updateOrderBeingServedNext();

	// Function to draw an inter arrival time based on rate specified in params
	int drawInterArrivalTime();

};


#endif