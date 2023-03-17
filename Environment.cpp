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
    nbOrdersServed = 0;
    couriers = std::vector<Courier>(params->nbCouriers);
    pickers = std::vector<Picker>(params->nbCouriers);
    orders = std::vector<Order*>(0);
    warehouses = std::vector<Warehouse>(params->nbWarehouses);

    for (int wID = 0; wID < params->nbWarehouses; wID++)
    {
        warehouses[wID].wareID = params->paramWarehouses[wID].wareID;
        warehouses[wID].lat = params->paramWarehouses[wID].lat;
        warehouses[wID].lon = params->paramWarehouses[wID].lon;
        warehouses[wID].initialNbCouriers = params->paramWarehouses[wID].initialNbCouriers;
        warehouses[wID].initialNbPickers = params->paramWarehouses[wID].initialNbPickers;
        for (int cID = 0; cID < warehouses[wID].initialNbCouriers; cID++)
        {
            couriers[courierCounter].courierID = courierCounter;
            couriers[courierCounter].assignedToWarehouse = &warehouses[wID];
            couriers[courierCounter].assignedToOrder = nullptr;
            couriers[courierCounter].timeWhenAvailable = 0;
            warehouses[wID].couriersAssigned.push_back(&couriers[courierCounter]);
            courierCounter ++;    
        }
        for (int pID = 0; pID < warehouses[wID].initialNbPickers; pID++)
        {
            pickers[pickerCounter].pickerID = pickerCounter;
            pickers[pickerCounter].assignedToWarehouse = &warehouses[wID];
            pickers[pickerCounter].timeWhenAvailable = 0;
            warehouses[wID].pickersAssigned.push_back(&pickers[pickerCounter]);
            pickerCounter ++;    
        }
        warehouses[wID].workLoad = 0;
    }

}

void Environment::initOrder(int currentTime, Order* o)
{
    o->orderID = orders.size();
    o->comissionTime = 120;
    o->assignedCourier = nullptr;
    o->assignedPicker = nullptr;
    o->assignedWarehouse = nullptr;
    int randomIDex = params->rng() % params->nbClients;
    o->client = &params->paramClients[randomIDex];
    o->orderTime = currentTime;
}

int Environment::drawInterArrivalTime()
{
    // Random number generator based on poisson process
    double lambda = 1/params->interArrivalTime;
    std::exponential_distribution<double> exp (lambda);
    return round(exp.operator() (params->rng));
}

void Environment::chooseWarehouseForOrder(Order* newOrder)
{
    // For now we just assign the order to the closest warehouse
    int indexClosestWarehouse;
    distancesToWarehouses = params->travelTime.getRow(newOrder->client->clientID);
    indexClosestWarehouse = std::min_element(distancesToWarehouses.begin(), distancesToWarehouses.end())-distancesToWarehouses.begin();
    newOrder->assignedWarehouse = &warehouses[indexClosestWarehouse];
}

void Environment::choosePickerForOrder(Order* newOrder) 
{
    // We choose the picker who is available fastest
    newOrder->assignedPicker = getFastestAvailablePicker(newOrder->assignedWarehouse);
    // We set the time the picker is available again to the maximum of either the previous availability time or the current time, plus the time needed to comission the order
    newOrder->assignedPicker->timeWhenAvailable = std::max(newOrder->assignedPicker->timeWhenAvailable, currentTime) + newOrder->comissionTime;
    newOrder->assignedPicker->assignedToOrders.push_back(newOrder);
}

void Environment::chooseCourierForOrder(Order* newOrder)
{
    // We choose the courier who is available fastest
    newOrder->assignedCourier = getFastestAvailableCourier(newOrder->assignedWarehouse);
    // We set the time the courier is arriving at the order to the maximum of either the current time, or the time the picker or couriers are available (comission time for picker has already been accounted for before). We then add the distance to the warehouse
    newOrder->arrivalTime = std::max(currentTime, std::max(newOrder->assignedCourier->timeWhenAvailable, newOrder->assignedPicker->timeWhenAvailable)) + params->travelTime.get(newOrder->client->clientID, newOrder->assignedWarehouse->wareID);
    newOrder->assignedCourier->assignedToOrder = newOrder;
    if(newOrder->arrivalTime<timeNextCourierArrivesAtOrder){
        timeNextCourierArrivesAtOrder = newOrder->arrivalTime;
        nextOrderBeingServed = newOrder;
    }

    // Remove order from vector of orders that have not been assigned to a courier yet (If applicable)   
    RemoveOrderFromVector(newOrder->assignedWarehouse->ordersNotAssignedToCourier,newOrder);
    // Remove courier from vector of couriers assigned to warehouse
    RemoveCourierFromVector(newOrder->assignedWarehouse->couriersAssigned, newOrder->assignedCourier);
    newOrder->assignedCourier->assignedToWarehouse = nullptr;
    newOrder->assignedCourier->timeWhenAvailable = INT_MAX;
}


void Environment::chooseWarehouseForCourier(Courier* courier)
{
    int indexClosestWarehouse;
    distancesToWarehouses = params->travelTime.getRow(courier->assignedToOrder->client->clientID);
    indexClosestWarehouse = std::min_element(distancesToWarehouses.begin(), distancesToWarehouses.end())-distancesToWarehouses.begin();
    courier->assignedToWarehouse = &warehouses[indexClosestWarehouse];
           
    // Compute the time the courier is available again, i.e., can leave the warehouse that we just assigned him to
    courier->timeWhenAvailable = nextOrderBeingServed->arrivalTime + params->travelTime.get(nextOrderBeingServed->client->clientID, courier->assignedToWarehouse->wareID);
    // Add the courier to the vector of assigned couriers at the respective warehouse
    courier->assignedToWarehouse->couriersAssigned.push_back(courier);
    // Increment the number of order that have been served
    nbOrdersServed ++;
    // Remove the order from the order that have not been served
    RemoveOrderFromVector(ordersAssignedToCourierButNotServed, nextOrderBeingServed);
    // Update the order that will be served next
    updateOrderBeingServedNext();
    courier->assignedToOrder = nullptr;

}

void Environment::RemoveOrderFromVector(std::vector<Order*> & V, Order* orderToDelete) {
    V.erase(
        std::remove_if(V.begin(), V.end(), [&](Order* const & o) {
            return o->orderID == orderToDelete->orderID;
        }),
        V.end());
}

void Environment::RemoveCourierFromVector(std::vector<Courier*> & V, Courier* courierToDelete) {
    V.erase(
        std::remove_if(V.begin(), V.end(), [&](Courier* const & o) {
            return o->courierID == courierToDelete->courierID;
        }),
        V.end());
}

Picker* Environment::getFastestAvailablePicker(Warehouse* war){
    int timeAvailable = INT_MAX;
    Picker* fastestAvailablePicker = war->pickersAssigned[0];
    for (auto picker : war->pickersAssigned){
        if(picker->timeWhenAvailable < timeAvailable){
            timeAvailable = picker->timeWhenAvailable;
            fastestAvailablePicker = picker;
        }
    }
    return fastestAvailablePicker;
}

Courier* Environment::getFastestAvailableCourier(Warehouse* war){
    int timeAvailable = INT_MAX;
    Courier* fastestAvailableCourier = war->couriersAssigned[0];
    for (auto courier : war->couriersAssigned){
        if(courier->timeWhenAvailable < timeAvailable){
            timeAvailable = courier->timeWhenAvailable;
            fastestAvailableCourier = courier;
        }
    }
    return fastestAvailableCourier;
}

void Environment::updateOrderBeingServedNext(){
    int highestArrivalTime = INT_MAX;
    if (ordersAssignedToCourierButNotServed.size() < 1){
        nextOrderBeingServed = nullptr;
        timeNextCourierArrivesAtOrder = INT_MAX;
    }else{
        for (auto order : ordersAssignedToCourierButNotServed){
            if(order->arrivalTime < highestArrivalTime){
                nextOrderBeingServed = order;
                highestArrivalTime = order->arrivalTime;
            }
        }
        timeNextCourierArrivesAtOrder = nextOrderBeingServed->arrivalTime;
    }
}

void Environment::simulate(int timeLimit)
{
    std::cout<<"----- Simulation starts -----"<<std::endl;
    // Initialize data structures
    initialize();
    
    // Start with simulation
    currentTime = 0;
    timeCustomerArrives = 0;
    timeNextCourierArrivesAtOrder = INT_MAX;
    while (currentTime <= timeLimit){
        // Keep track of current time
        currentTime = std::min(timeCustomerArrives, timeNextCourierArrivesAtOrder);
    
        if (timeCustomerArrives < timeNextCourierArrivesAtOrder){
            timeCustomerArrives += drawInterArrivalTime();
            // Draw new order and assign it to warehouse, picker and courier. MUST BE IN THAT ORDER!!!
            Order* newOrder = new Order;
            initOrder(currentTime, newOrder);
            orders.push_back(newOrder);
            // We immediately assign the order to a warehouse and a picker
            chooseWarehouseForOrder(newOrder);
            choosePickerForOrder(newOrder);
            // If there are couriers assigned to the warehouse, we can assign a courier to the order
            if (newOrder->assignedWarehouse->couriersAssigned.size()>0){
                chooseCourierForOrder(newOrder);
                ordersAssignedToCourierButNotServed.push_back(newOrder);
            }else{ // else we add the order to list of orders that have not been assigned to a courier yet
                newOrder->assignedWarehouse->ordersNotAssignedToCourier.push_back(newOrder);    
            }
        }else{ // when a courier arrives at an order
            Courier* c = nextOrderBeingServed->assignedCourier;
            // We choose a warehouse for the courier
            chooseWarehouseForCourier(c);
            // If the chosen warehouse has order that have not been assigned to a courier yet, we can now assign the order to a courier
            if (c->assignedToWarehouse->ordersNotAssignedToCourier.size()>0){
                Order* orderToAssignToCourier = c->assignedToWarehouse->ordersNotAssignedToCourier[0];
                chooseCourierForOrder(orderToAssignToCourier);
                ordersAssignedToCourierButNotServed.push_back(orderToAssignToCourier);
            }
        }
    }
    std::cout<<"----- Simulation finished -----"<<std::endl;
    std::cout<<"----- Number of orders who arrived: " << orders.size() << " and served: " << nbOrdersServed << " -----" <<std::endl;
}


