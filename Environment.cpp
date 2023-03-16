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
            couriers[courierCounter].timeWhenAvailable = 0;
            warehouses[wInd].couriersAssigned.push_back(&couriers[courierCounter]);
            courierCounter ++;    
        }
        for (int pInd = 0; pInd < warehouses[wInd].initialNbPickers; pInd++)
        {
            pickers[pickerCounter].pickerInd = pickerCounter;
            pickers[pickerCounter].assignedToWarehouse = &warehouses[wInd];
            pickers[pickerCounter].timeWhenAvailable = 0;
            warehouses[wInd].pickersAssigned.push_back(&pickers[pickerCounter]);
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

void Environment::chooseWarehouseForOrder(Order* newOrder)
{
    // For now we just assign the order to the closest warehouse
    int indexClosestWarehouse;
    newOrderDistancesToWarehouses = params->travelTime.getRow(newOrder->client->clientInd);
    indexClosestWarehouse = std::min_element(newOrderDistancesToWarehouses.begin(), newOrderDistancesToWarehouses.end())-newOrderDistancesToWarehouses.begin();
    newOrder->assignedWarehouse = &warehouses[indexClosestWarehouse];
}

void Environment::choosePickerForOrder(Order* newOrder) 
{
    // We choose the picker who is available fastest
    newOrder->assignedPicker = getFastestAvailablePicker(newOrder->assignedWarehouse);
    // We set the time the picker is available again to the maximum of either the previous availability time or the current time, plus the time needed to comission the order
    newOrder->assignedPicker->timeWhenAvailable = std::max(newOrder->assignedPicker->timeWhenAvailable, currentTime) + newOrder->comissionTime;
    newOrder->assignedPicker->assignedToOrders.push_back(new Order(*newOrder));
}

void Environment::chooseCourierForOrder(Order* newOrder)
{
    // We choose the courier who is available fastest
    newOrder->assignedCourier = getFastestAvailableCourier(newOrder->assignedWarehouse);
    // We set the time the courier is arriving at the order to the maximum of either the current time, or the time the picker or couriers are available (plus comission time for picker). We then add the distance to the warehouse
    newOrder->arrivalTime = std::max(currentTime, std::max(newOrder->assignedCourier->timeWhenAvailable, newOrder->assignedPicker->timeWhenAvailable + newOrder->comissionTime)) + params->travelTime.get(newOrder->client->clientInd, newOrder->assignedWarehouse->wareInd);
    newOrder->assignedCourier->timeWhenAvailable = newOrder->arrivalTime + params->travelTime.get(newOrder->client->clientInd, newOrder->assignedWarehouse->wareInd);
    newOrder->assignedCourier->assignedToOrders.push_back(new Order(*newOrder));
    if(newOrder->arrivalTime<timeNextCourierArrivesAtOrder){
        timeNextCourierArrivesAtOrder = newOrder->arrivalTime;
        nextOrderBeingServed = newOrder;
    }
}

void Environment::RemoveOrderFromNotServedVector(std::vector<Order*> & V, Order* orderToDelete) {
    V.erase(
        std::remove_if(V.begin(), V.end(), [&](Order* const & o) {
            return o->orderInd == orderToDelete->orderInd;
        }),
        V.end());
}

void Environment::chooseWarehouseForCourier(Courier* courier)
{
    courier->assignedToWarehouse = courier->assignedToWarehouse;
    courier->timeWhenAvailable = nextOrderBeingServed->arrivalTime + params->travelTime.get(nextOrderBeingServed->orderInd, courier->assignedToWarehouse->wareInd);
    nbOrdersServed ++;
    RemoveOrderFromNotServedVector(ordersNotServed, nextOrderBeingServed);
    updateOrderBeingServedNext();
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
    if (ordersNotServed.size() < 1){
        nextOrderBeingServed = nullptr;
        timeNextCourierArrivesAtOrder = INT_MAX;
    }else{
        for (auto order : ordersNotServed){
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
            Order newOrder = createOrder(timeCustomerArrives);
            chooseWarehouseForOrder(&newOrder);
            choosePickerForOrder(&newOrder);
            chooseCourierForOrder(&newOrder);
            orders.push_back(newOrder);
            ordersNotServed.push_back(new Order(newOrder));
        }else{
            chooseWarehouseForCourier(nextOrderBeingServed->assignedCourier);
        }
    }
    std::cout<<"----- Number of orders who arrived: " << orders.size() << " and served: " << nbOrdersServed << " -----" <<std::endl;
    for (auto od : orders){
        std::cout<<od.arrivalTime-od.orderTime<<std::endl;
    }
    
}


