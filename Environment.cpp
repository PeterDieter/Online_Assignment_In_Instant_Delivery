#include <algorithm>
#include <set>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <random>

#include "Params.h"
#include "Matrix.h"
#include "Environment.h"


Environment::Environment(Params* params) : params(params)
{   
    std::cout<<"----- Create Environment -----"<<std::endl;
}

void Environment::initialize()
{
    
    // CONSTRUCTOR: First we initialize the environment by assigning 
    int courierCounter = 0;
    int pickerCounter = 0;
    couriers = std::vector<Courier>(params->nbCouriers);
    pickers = std::vector<Picker>(params->nbCouriers);
    orders = std::vector<Order>(0);
    warehouses = std::vector<Warehouse>(params->nbWarehouses);

    for (int wInd = 0; wInd < params->nbWarehouses; wInd++)
    {
        warehouses[wInd].wareInd = params->paramWarehouses[wInd].wareInd;
        warehouses[wInd].lat = params->paramWarehouses[wInd].lat;
        warehouses[wInd].lon = params->paramWarehouses[wInd].lon;
        warehouses[wInd].initialNbCouriers = params->paramWarehouses[wInd].initialNbCouriers;
        warehouses[wInd].initialNbPickers = params->paramWarehouses[wInd].initialNbPickers;
        for (int cInd = 0; cInd < warehouses[wInd].initialNbCouriers; cInd++)
        {
            couriers[courierCounter].courierInd = courierCounter;
            couriers[courierCounter].assignedToWarehouse = &warehouses[wInd];
            warehouses[wInd].couriersAssigned.push_back(&couriers[courierCounter]);
            warehouses[wInd].couriersAvailable.push_back(&couriers[courierCounter]);
            courierCounter ++;    
        }
        for (int pInd = 0; pInd < warehouses[wInd].initialNbPickers; pInd++)
        {
            pickers[pickerCounter].pickerInd = pickerCounter;
            pickers[pickerCounter].assignedToWarehouse = &warehouses[wInd];
            warehouses[wInd].pickersAssigned.push_back(&pickers[pickerCounter]);
            warehouses[wInd].pickersAvailable.push_back(&pickers[pickerCounter]);
            pickerCounter ++;    
        }
        warehouses[wInd].workLoad = 0;
    }

}

Order Environment::createOrder(int currentTime)
{
    Order order;
    order.orderInd = orders.size();
    order.comissionTime = 120;
    int randomIndex = params->rng() % params->nbClients;
    order.client = &params->paramClients[randomIndex];
    order.orderTime = currentTime;

    return order;
}

int Environment::drawInterArrivalTime()
{
    // Random number generator based on poisson process
    double lambda = 1/params->interArrivalTime;
    std::exponential_distribution<double> exp (lambda);
    return round(exp.operator() (params->rng));
}

Warehouse* Environment::chooseWarehouse()
{
    // For now we just assign the order to the closest warehouse
    newOrderDistancesToWarehouses = params->travelTime.getRow(newOrder.client->clientInd);
    indexClosestWarehouse = std::min_element(newOrderDistancesToWarehouses.begin(), newOrderDistancesToWarehouses.end())-newOrderDistancesToWarehouses.begin();
    
    return &warehouses[indexClosestWarehouse];
}

Picker* Environment::choosePicker()
{
    return newOrder.assignedWarehouse->pickersAvailable[0];
}

Courier* Environment::chooseCourier()
{
    return newOrder.assignedWarehouse->couriersAvailable[0];
}


void Environment::simulate(int timeLimit)
{
    // Initialize data structures
    initialize();
    
    // Start with simulation
    int currentTime = 0;
    int timeLastCustomerArrived = 0;
    while (currentTime <= timeLimit){
        // Keep track of current time
        currentTime = timeLastCustomerArrived;

        // Draw new Order and assign it to warehouse, picker and courier
        timeLastCustomerArrived += drawInterArrivalTime();
        newOrder = createOrder(timeLastCustomerArrived);
        newOrder.assignedWarehouse = chooseWarehouse();
        newOrder.assignedPicker = choosePicker();
        newOrder.assignedCourier = chooseCourier();
        orders.push_back(newOrder);

    }
    std::cout<<orders.size()<<std::endl;

}


