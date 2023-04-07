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

#include <torch/torch.h>
#include "Matrix.h"
#include "Data.h"
#include "Environment.h"

class Environment
{
public:
	// Constructor 
	Environment(Data* data);

	// Function to perform a simulation
	void simulate(std::string policy, int timeLimit);

private:
	Data* data;												// Problem parameters
	std::vector<Order*> orders;									// Vector of pointers to orders. containing information on each order
	std::vector<Order*> ordersAssignedToCourierButNotServed;	// Vector of orders that have not been served yet
	std::vector<Warehouse*> warehouses;							// Vector of pointers containing information on each warehouse
	std::vector<Courier*> couriers;								// Vector of pointers containing  information on each courier
	std::vector<Picker*> pickers;								// Vector of pointers  containing information on each picker
	std::vector<Route*> routes;									// Vector of pointers  containing information on each route
	Order* nextOrderBeingServed;								// Order that will be served next. Needed as we have two types of decision epoch: Order arriving and order being served (courier needs to be reassigned)
	int currentTime;
	int nbOrdersServed;
	int timeCustomerArrives;
	int timeNextCourierArrivesAtOrder;
	int totalWaitingTime;
	int highestWaitingTimeOfAnOrder;
	int latestArrivalTime;
	int penaltyForNotServing;


	// In this method we apply the nearest warehouse policy.
	void nearestWarehousePolicy(int timelimit);
	// In this method we train a REINFORCE algorithm
	void trainREINFORCE(int timelimit);

	// In this method we initialize the rest of the Data, such as warehouses, couriers, etc.
	void initialize();

	// Function to initialize the values of an order
	void initOrder(int currentTime, Order* o);

	// Functions that assigns order to a warehouse, picker and courier, respectively
	void chooseWarehouseForOrder(Order* newOrder);
	void choosePickerForOrder(Order* newOrder);
	void chooseCourierForOrder(Order* newOrder);

	// Function that assigns a courier to a warehouse
	void chooseWarehouseForCourier(Courier* courier);

	// Function that deletes order from ordersNotServed vector
	void RemoveOrderFromVector(std::vector<Order*> & V, Order* orderToDelete);

	// Function that deletes order from ordersNotServed vector
	void RemoveCourierFromVector(std::vector<Courier*> & V, Courier* courierToDelete);

	// Function that returns the fastest available picker at a warehouse
	Picker* getFastestAvailablePicker(Warehouse* warehouse);

	// Function that returns the fastest available courier assigned to a warehouse
	Courier* getFastestAvailableCourier(Warehouse* warehouse);

	// Function that updates the order that will be served next
	void updateOrderBeingServedNext();

	// Function that saves a route to the list of routes
	void saveRoute(int startTime, int arrivalTime, double fromLat, double fromLon, double toLat, double toLon);

	// Function that writes routes to file
	void writeRoutesAndOrdersToFile(std::string fileNameRoutes, std::string fileNameOrders);

	// Function to draw an inter arrival time based on rate specified in data
	int drawFromExponentialDistribution(double lambda);

	// Function that returns the objective value (waiting time + penalty)
	int getObjValue();

	// Define a new Module.
	struct neuralNetwork : torch::nn::Module {
		neuralNetwork(int64_t inputSize, int64_t outputSize) {
			fc1 = register_module("fc1", torch::nn::Linear(inputSize, 64));
			fc2 = register_module("fc2", torch::nn::Linear(64, 32));
			fc3 = register_module("fc3", torch::nn::Linear(32, outputSize));
		}

		// Implement the Net's algorithm.
		torch::Tensor forward(torch::Tensor x) {
			// Use one of many tensor manipulation functions.
			x = torch::layer_norm(x, (x.size(1)));
			x = torch::leaky_relu(fc1->forward(x));
			x = torch::dropout(x, /*p=*/0.1, /*train=*/is_training());
			x = torch::leaky_relu(fc2->forward(x));
			x = fc3->forward(x);
			x = torch::softmax(x, /*dim=*/1);
			return x;
		}

		// Use one of many "standard library" modules.
		torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};
	};


	// Function that assigns order to a warehouse with the REINFORCE algorithm
	void warehouseForOrderREINFORCE(Order* newOrder, neuralNetwork& n);
	// Function that returns the state as a tensor
	torch::Tensor getState(Order* order);
	// Function that returns the costs of each action
	torch::Tensor getRewardVector();
	torch::Tensor states;
	torch::Tensor actions;

};

struct CustomLoss : public torch::nn::Module {
public:
    CustomLoss() {}

    torch::Tensor forward(torch::Tensor probabilities, torch::Tensor rewards) {
        // Calculate the custom loss function
        torch::Tensor loss = (torch::mul(-torch::log(probabilities),rewards)).mean();
		// loss.requires_grad_(true);
        return loss;
    }
};

#endif