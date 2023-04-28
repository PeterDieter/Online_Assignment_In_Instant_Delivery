#include <algorithm>
#include <set>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <random>

#include <torch/torch.h>
#include <torch/script.h>
#include "Data.h"
#include "Matrix.h"
#include "Environment.h"


Environment::Environment(Data* data) : data(data)
{   
    std::cout<<"----- Create Environment -----"<<std::endl;
}


void Environment::initialize(int timeLimit)
{
    
    // CONSTRUCTOR: First we initialize the environment by assigning 
    int courierCounter = 0;
    int pickerCounter = 0;
    penaltyForNotServing = 2500;
    totalWaitingTime = 0;
    highestWaitingTimeOfAnOrder = 0;
    latestArrivalTime = 0;
    nbOrdersServed = 0;
    ordersAssignedToCourierButNotServed = std::vector<Order*>(0);
    couriers = std::vector<Courier*>(0);
    pickers = std::vector<Picker*>(0);
    orders = std::vector<Order*>(0);
    routes = std::vector<Route*>(0);
    warehouses = std::vector<Warehouse*>(0);

    for (int wID = 0; wID < data->nbWarehouses; wID++)
    {
        Warehouse* newWarehouse = new Warehouse;
        warehouses.push_back(newWarehouse);
        newWarehouse->wareID = data->paramWarehouses[wID].wareID;
        newWarehouse->lat = data->paramWarehouses[wID].lat;
        newWarehouse->lon = data->paramWarehouses[wID].lon;
        newWarehouse->initialNbCouriers = data->paramWarehouses[wID].initialNbCouriers;
        newWarehouse->initialNbPickers = data->paramWarehouses[wID].initialNbPickers;
        newWarehouse->ordersNotAssignedToCourier = std::vector<Order*>(0);
        for (int cID = 0; cID < newWarehouse->initialNbCouriers; cID++)
        {
            Courier* newCourier = new Courier;
            newCourier->courierID = courierCounter;
            newCourier->assignedToWarehouse = warehouses[wID];
            newCourier->assignedToOrder = nullptr;
            newCourier->timeWhenAvailable = 0;
            couriers.push_back(newCourier);
            newWarehouse->couriersAssigned.push_back(newCourier);
            courierCounter ++;    
        }
        for (int pID = 0; pID < newWarehouse->initialNbPickers; pID++)
        {
            Picker* newPicker = new Picker;
            newPicker->pickerID = pickerCounter;
            newPicker->assignedToWarehouse = warehouses[wID];
            newPicker->timeWhenAvailable = 0;
            pickers.push_back(newPicker);
            newWarehouse->pickersAssigned.push_back(newPicker);
            pickerCounter ++;    
        }
    }

    // Now we draw the random numbers
    orderTimes = std::vector<int>(0);
    clientsVector = std::vector<int>(0);
    timesToComission = std::vector<int>(0);
    timesToServe = std::vector<int>(0);
    int currTime = 0;
    int nextTime;
    while (currTime < timeLimit){
        nextTime = drawFromExponentialDistribution(data->interArrivalTime);
        currTime += nextTime;
        orderTimes.push_back(nextTime);
        clientsVector.push_back(data->rng() % data->nbClients);
        timesToComission.push_back(drawFromExponentialDistribution(data->meanCommissionTime));
        timesToServe.push_back(drawFromExponentialDistribution(data->meanServiceTimeAtClient));
    }
}

void Environment::initOrder(int currentTime, Order* o)
{
    o->orderID = orders.size();
    o->timeToComission = timesToComission[o->orderID]; // Follows expoential distribution
    o->assignedCourier = nullptr;
    o->assignedPicker = nullptr;
    o->assignedWarehouse = nullptr;
    o->client = &data->paramClients[clientsVector[o->orderID]];
    o->orderTime = currentTime;
    o->arrivalTime = -1;
}

int Environment::drawFromExponentialDistribution(double lambda)
{
    // Random number generator based on poisson process
    double lambdaInv = 1/lambda;
    std::exponential_distribution<double> exp (lambdaInv);
    return round(exp.operator() (data->rng));
}

void Environment::choosePickerForOrder(Order* newOrder) 
{
    // We choose the picker who is available fastest
    newOrder->assignedPicker = getFastestAvailablePicker(newOrder->assignedWarehouse);
    // We set the time the picker is available again to the maximum of either the previous availability time or the current time, plus the time needed to comission the order
    newOrder->assignedPicker->timeWhenAvailable = std::max(newOrder->assignedPicker->timeWhenAvailable, currentTime) + newOrder->timeToComission;
}

void Environment::chooseCourierForOrder(Order* newOrder)
{
    // We choose the courier who is available fastest
    newOrder->assignedCourier = getFastestAvailableCourier(newOrder->assignedWarehouse);
    // We set the time the courier is arriving at the order to the maximum of either the current time, or the time the picker or couriers are available (comission time for picker has already been accounted for before). We then add the distance to the warehouse
    newOrder->arrivalTime = std::max(currentTime, std::max(newOrder->assignedCourier->timeWhenAvailable, newOrder->assignedPicker->timeWhenAvailable)) + data->travelTime.get(newOrder->client->clientID, newOrder->assignedWarehouse->wareID);
    newOrder->assignedCourier->assignedToOrder = newOrder;
    if(newOrder->arrivalTime<timeNextCourierArrivesAtOrder){
        timeNextCourierArrivesAtOrder = newOrder->arrivalTime;
        nextOrderBeingServed = newOrder;
    }

    if (latestArrivalTime < newOrder->arrivalTime){
        latestArrivalTime = newOrder->arrivalTime;
    }

    saveRoute(std::max(currentTime, std::max(newOrder->assignedCourier->timeWhenAvailable, newOrder->assignedPicker->timeWhenAvailable)), newOrder->arrivalTime, newOrder->assignedCourier->assignedToWarehouse->lat, newOrder->assignedCourier->assignedToWarehouse->lon, newOrder->client->lat, newOrder->client->lon);

    // Remove order from vector of orders that have not been assigned to a courier yet (If applicable)   
    RemoveOrderFromVector(newOrder->assignedWarehouse->ordersNotAssignedToCourier, newOrder);
    // Remove courier from vector of couriers assigned to warehouse
    RemoveCourierFromVector(newOrder->assignedWarehouse->couriersAssigned, newOrder->assignedCourier);
    //newOrder->assignedCourier->assignedToWarehouse = nullptr;
    newOrder->assignedCourier->timeWhenAvailable = INT_MAX;
}


void Environment::chooseWarehouseForCourier(Courier* courier)
{
    // For now, we just assign the courier back to the warehouse he came from
    // draw service time needed to serve the client at the door
    courier->assignedToOrder->serviceTimeAtClient = timesToServe[courier->assignedToOrder->orderID];   
    // Compute the time the courier is available again, i.e., can leave the warehouse that we just assigned him to
    courier->timeWhenAvailable = nextOrderBeingServed->arrivalTime + courier->assignedToOrder->serviceTimeAtClient + data->travelTime.get(nextOrderBeingServed->client->clientID, courier->assignedToWarehouse->wareID);
    // Add the courier to the vector of assigned couriers at the respective warehouse
    courier->assignedToWarehouse->couriersAssigned.push_back(courier);
    // Increment the number of order that have been served
    nbOrdersServed ++;
    totalWaitingTime += courier->assignedToOrder->arrivalTime - courier->assignedToOrder->orderTime;
    if (highestWaitingTimeOfAnOrder < courier->assignedToOrder->arrivalTime - courier->assignedToOrder->orderTime)
    {
        highestWaitingTimeOfAnOrder = courier->assignedToOrder->arrivalTime - courier->assignedToOrder->orderTime;
    }
    if (latestArrivalTime < courier->timeWhenAvailable){
        latestArrivalTime = courier->timeWhenAvailable;
    }
    
    saveRoute(nextOrderBeingServed->arrivalTime, courier->timeWhenAvailable, nextOrderBeingServed->client->lat, nextOrderBeingServed->client->lon, courier->assignedToWarehouse->lat, courier->assignedToWarehouse->lon);
    
    // Remove the order from the order that have not been served
    RemoveOrderFromVector(ordersAssignedToCourierButNotServed, nextOrderBeingServed);
    // Update the order that will be served next
    updateOrderBeingServedNext();
    courier->assignedToOrder = nullptr;
}

void Environment::saveRoute(int startTime, int arrivalTime, double fromLat, double fromLon, double toLat, double toLon){
    Route* route = new Route;
    route->fromlLat = fromLat; route->fromlon = fromLon; route->tolLat = toLat; route->tolon = toLon;
    route->startTime = startTime;
    route->arrivalTime = arrivalTime;
    routes.push_back(route);
}

void Environment::writeRoutesAndOrdersToFile(std::string fileNameRoutes, std::string fileNameOrders){
	std::cout << "----- WRITING Routes IN : " << fileNameRoutes << " and Orders IN : " << fileNameOrders << std::endl;
	std::ofstream myfile(fileNameRoutes);
	if (myfile.is_open())
	{
		for (auto route : routes)
		{
            // Here we print the order of customers that we visit 
            myfile << route->startTime << " " << route->arrivalTime << " " << route->fromlLat << " " << route->fromlon << " " << route->tolLat << " " << route->tolon;
            myfile << std::endl;
		}
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileNameRoutes << std::endl;
    // Now the orders
    std::ofstream myfile2(fileNameOrders);
	if (myfile2.is_open())
	{
		for (auto order : orders)
		{
            if (order->accepted){
                if (order->arrivalTime == -1){
                    // Here we print the order of customers that we visit 
                    myfile2 << order->orderTime << " " << latestArrivalTime << " " << order->client->lat << " " << order->client->lon << " " << 1;
                    myfile2 << std::endl;
                }else{
                    myfile2 << order->orderTime << " " << order->arrivalTime << " " << order->client->lat << " " << order->client->lon << " " << 1;
                    myfile2 << std::endl;
                }
            }else{
                myfile2 << order->orderTime << " " << order->orderTime + 180<< " " << order->client->lat << " " << order->client->lon << " " << 0;
                myfile2 << std::endl;
            }
		}
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileNameOrders << std::endl;
}

void Environment::writeCostsToFile(std::vector<float> costs, float lambdaTemporal, float lambdaSpatial){
    std::string fileName = "data/trainingData/averageCosts" + std::to_string(lambdaTemporal) + std::to_string(lambdaSpatial) +".txt";
	std::cout << "----- WRITING COST VECTOR IN : " << fileName << std::endl;
	std::ofstream myfile(fileName);
	if (myfile.is_open())
	{
		for (auto cost : costs)
		{
            // Here we print the order of customers that we visit 
            myfile << cost;
            myfile << std::endl;
		}
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileName << std::endl;
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

void Environment::chooseWarehouseForOrder(Order* newOrder)
{
    // For now we just assign the order to the closest warehouse
    int indexClosestWarehouse;
    std::vector<int> distancesToWarehouses = data->travelTime.getRow(newOrder->client->clientID);
    indexClosestWarehouse = std::min_element(distancesToWarehouses.begin(), distancesToWarehouses.end())-distancesToWarehouses.begin();
    newOrder->assignedWarehouse = warehouses[indexClosestWarehouse];
    newOrder->accepted = true;
}

torch::Tensor Environment::getState(Order* order){
    // For now, the state is only the distances to the warehouses
    std::vector<int> distancesToWarehouses = data->travelTime.getRow(order->client->clientID);
    double maxElement = distancesToWarehouses[std::max_element(distancesToWarehouses.begin(), distancesToWarehouses.end())-distancesToWarehouses.begin()];
    
    std::vector<float> state;
    for (int i = 0; i < distancesToWarehouses.size(); i++) {
        state.push_back(distancesToWarehouses[i]);
    }
    
    std::vector<int> wareHouseLoad;
    for (Warehouse* w : warehouses){
        state.push_back(w->couriersAssigned.size());
        state.push_back(getFastestAvailablePicker(w)->timeWhenAvailable);
    }

    // vector to tensor
    auto options = torch::TensorOptions().dtype(at::kFloat);
    torch::Tensor inputs = torch::from_blob(state.data(), {1, data->nbWarehouses*3}, options).clone().to(torch::kFloat);
    return inputs;
}

int Environment::getObjValue(){
    int objectiveValue = 0;
    for (Order* order: orders){
        if (order->accepted){
            if (order->arrivalTime == -1){
                objectiveValue += penaltyForNotServing;
            }else{
                objectiveValue += (order->arrivalTime-order->orderTime); 
            }
        }else{
            objectiveValue += penaltyForNotServing;
        }
    } 
    return objectiveValue;
}


void Environment::warehouseForOrderREINFORCE(Order* newOrder, policyNetwork& n, bool train)
{
    torch::Tensor state = getState(newOrder);
    torch::Tensor prediction = n.forward(state);
    // Prediction tensor to vector
    std::vector<float> predVector(prediction.data_ptr<float>(), prediction.data_ptr<float>() + prediction.numel());
    int indexWarehouse;
    if (train){
        std::discrete_distribution<> discrete_dist(predVector.begin(), predVector.end());
        // Choose based on the distribution
        indexWarehouse = discrete_dist(data->rng);
    }else{
        indexWarehouse = std::max_element(predVector.begin(), predVector.end())-predVector.begin();
    }

    // If the index is nb.warehouses, we reject the order
    if (indexWarehouse >= data->nbWarehouses){
        newOrder->accepted = false;
    }else{
        newOrder->assignedWarehouse = warehouses[indexWarehouse];
        newOrder->accepted = true;
    }

    if (train){
        if (newOrder->orderID == 0){
            states = state;
            actions = torch::tensor({indexWarehouse});
        }else{
            states = torch::cat({states, state});
            actions = torch::cat({actions, torch::tensor({indexWarehouse})});
        }
    }

}

torch::Tensor Environment::getCostsVector(){
    std::vector<float> costsVec;
    int orderCounter = 0;
    for (Order* order: orders){
        orderCounter ++;
        if (order->accepted){
            if (order->arrivalTime == -1){
                costsVec.push_back(penaltyForNotServing);
            }else{
                costsVec.push_back((order->arrivalTime-order->orderTime));   
            }
        }else{
            costsVec.push_back(penaltyForNotServing);
        }
    }

    // vector to tensor
    auto options = torch::TensorOptions().dtype(at::kFloat);
    torch::Tensor costs = torch::from_blob(costsVec.data(), {1, orderCounter}, options).clone().to(torch::kFloat);
    return costs;
}

torch::Tensor Environment::getCostsVectorDiscounted(float lambdaTemporal, float lambdaSpatial){
    std::vector<float> costsVec;
    int orderCounter = 0;
    double costsForOrder;
    for (Order* order: orders){
        orderCounter ++;
        if (order->accepted){
            costsForOrder = order->arrivalTime-order->orderTime;  
            auto start_iter = std::next(orders.begin(), orderCounter);
            for (auto orderAfter = start_iter; orderAfter != orders.end(); ++orderAfter){
                //if ((*orderAfter)->assignedWarehouse == order->assignedWarehouse){
                    if ((*orderAfter)->arrivalTime != -1){
                        double dist = euclideanDistance((*orderAfter)->assignedWarehouse->lat, order->assignedWarehouse->lat,(*orderAfter)->assignedWarehouse->lon, order->assignedWarehouse->lon);
                        costsForOrder += ((*orderAfter)->arrivalTime-(*orderAfter)->orderTime)*pow(lambdaTemporal, (*orderAfter)->orderTime-order->orderTime) *pow(lambdaSpatial, dist);
                    }
                //}
            } 
        }else{
            costsForOrder = penaltyForNotServing;
        }
        //std::cout<<"Costs: "<<costsForOrder<<" "<<order->arrivalTime<<" "<<order->orderTime<<std::endl;
        costsVec.push_back(costsForOrder);
    }

    // vector to tensor
    auto options = torch::TensorOptions().dtype(at::kFloat);
    torch::Tensor costs = torch::from_blob(costsVec.data(), {1, orderCounter}, options).clone().to(torch::kFloat);
    return costs;
}

double Environment::euclideanDistance(double latFrom, double latTo, double lonFrom, double lonTo){
    double dx = latFrom - latTo;
    double dy = lonFrom - lonTo;
    return std::sqrt(dx * dx + dy * dy)*100;
}

void Environment::nearestWarehousePolicy(int timeLimit)
{
    std::cout<<"----- Simulation starts -----"<<std::endl;
    double running_costs = 0.0;
    double runningCounter = 0.0;
    for (int epoch = 1; epoch <= 1000; epoch++) {
        // Initialize data structures
        initialize(timeLimit);
        
        // Start with simulation
        int counter = 0;
        currentTime = 0;
        timeCustomerArrives = 0;
        timeNextCourierArrivesAtOrder = INT_MAX;
        while (currentTime < timeLimit || ordersAssignedToCourierButNotServed.size() > 0){
            // Keep track of current time
            if (counter == orderTimes.size()-1){
                currentTime = timeNextCourierArrivesAtOrder;
            }else{
                currentTime = std::min(timeCustomerArrives, timeNextCourierArrivesAtOrder);
            }

            if (timeCustomerArrives < timeNextCourierArrivesAtOrder && currentTime <= timeLimit && counter<orderTimes.size()-1){
                timeCustomerArrives += orderTimes[counter];
                counter += 1;
                // Draw new order and assign it to warehouse, picker and courier. MUST BE IN THAT ORDER!!!
                Order* newOrder = new Order;
                initOrder(timeCustomerArrives, newOrder);
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
            }else { // when a courier arrives at an order
                if (nextOrderBeingServed){
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
        }
        std::cout<<"----- Simulation finished -----"<<std::endl;
        //std::cout<<"----- Number of orders that arrived: " << orders.size() << " and served: " << nbOrdersServed << " Obj. value: " << getObjValue() << ". Mean wt: " << totalWaitingTime/nbOrdersServed <<" seconds. Highest wt: " << highestWaitingTimeOfAnOrder <<" seconds. -----" <<std::endl;
        //writeRoutesAndOrdersToFile("data/animationData/routes.txt", "data/animationData/orders.txt");
        running_costs += getObjValue();
        runningCounter += 1;
    }
    std::cout<< "Iterations: " << runningCounter <<" Average costs: " << running_costs / runningCounter <<std::endl;
}


void Environment::trainREINFORCE(int timeLimit, float lambdaTemporal, float lambdaSpatial)
{
    std::cout<<"----- Training REINFORCE starts with lambda temporal " << lambdaTemporal << " and lambda spatial " << lambdaSpatial << " -----"<<std::endl;
    // Create neural network where each output node is assigned to a warehouse and one extra node for the reject decision
    auto net = std::make_shared<policyNetwork>(data->nbWarehouses*3, data->nbWarehouses+1);
    torch::Tensor loss;
    // Create an instance of the custom loss function
    logLoss loss_fn;
    // Instantiate an Adam optimization algorithm to update our Net's parameters.
    torch::optim::Adam optimizer(net->parameters(), /*lr=*/0.00001);
    double running_costs = 0.0;
    double runningCounter = 0.0;
    std::vector< float> averageCostVector;
    for (int epoch = 1; epoch <= 50000; epoch++) {
        // Initialize data structures
        initialize(timeLimit);
        // Start with simulation
        int counter = 0;
        currentTime = 0;
        timeCustomerArrives = 0;
        timeNextCourierArrivesAtOrder = INT_MAX;
        while (currentTime < timeLimit || ordersAssignedToCourierButNotServed.size() > 0){
            // Keep track of current time
            if (counter == orderTimes.size()-1){
                currentTime = timeNextCourierArrivesAtOrder;
            }else{
                currentTime = std::min(timeCustomerArrives, timeNextCourierArrivesAtOrder);
            }
            if (timeCustomerArrives < timeNextCourierArrivesAtOrder && currentTime <= timeLimit && counter<orderTimes.size()-1){
                timeCustomerArrives += orderTimes[counter];
                counter += 1;
                // Draw new order and assign it to warehouse, picker and courier. MUST BE IN THAT ORDER!!!
                Order* newOrder = new Order;
                initOrder(timeCustomerArrives, newOrder);
                orders.push_back(newOrder);
                // We immediately assign the order to a warehouse and a picker
                warehouseForOrderREINFORCE(newOrder, *net, true);
                if (newOrder->accepted){
                    choosePickerForOrder(newOrder);
                    // If there are couriers assigned to the warehouse, we can assign a courier to the order
                    if (newOrder->assignedWarehouse->couriersAssigned.size()>0){
                        chooseCourierForOrder(newOrder);
                        ordersAssignedToCourierButNotServed.push_back(newOrder);
                    }else{ // else we add the order to list of orders that have not been assigned to a courier yet
                        newOrder->assignedWarehouse->ordersNotAssignedToCourier.push_back(newOrder);  
                    }
                }

            }else { // when a courier arrives at an order
                if (nextOrderBeingServed){
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
        }
        // Reset gradients of neural network.
        optimizer.zero_grad();
        torch::Tensor costs = getCostsVectorDiscounted(lambdaTemporal, lambdaSpatial);
        torch::Tensor pred = net->forward(states);
        auto rows = torch::arange(0, pred.size(0), torch::kLong);
        auto result = pred.index({rows, actions});
        loss = loss_fn.forward(result, costs);
        loss.backward();
        running_costs += getObjValue();//loss.item<double>();
        optimizer.step();       // Update the parameters based on the calculated gradients.
        runningCounter += 1;

        // 
        if (epoch % 100 == 0) {
            std::cout << "[Iteration: " << epoch << "] Average costs: " << running_costs / runningCounter << std::endl;
            averageCostVector.push_back(running_costs/runningCounter);
            running_costs = 0.0;
            runningCounter = 0.0;
        }
    
    }
    std::cout<<"----- REINFORCE training finished -----"<<std::endl;
    writeCostsToFile(averageCostVector, lambdaTemporal, lambdaSpatial);
    torch::save(net,"src/net_REINFORCE.pt");
    std::cout<<"----- Policy net saved in src/net_REINFORCE.pt -----"<<std::endl;
    
}


void Environment::testREINFORCE(int timeLimit)
{
    std::cout<<"----- Testing REINFORCE starts -----"<<std::endl;
    // Load neural network
    auto net = std::make_shared<policyNetwork>(data->nbWarehouses*3, data->nbWarehouses+1);
    torch::load(net, "src/net_REINFORCE.pt");
    net->eval();
    
    double running_costs = 0.0;
    double runningCounter = 0.0;
    for (int epoch = 1; epoch <= 1000; epoch++) {
        // Initialize data structures
        initialize(timeLimit);
        // Start with simulation
        currentTime = 0;
        timeCustomerArrives = 0;
        int counter = 0;
        timeNextCourierArrivesAtOrder = INT_MAX;
        while (currentTime < timeLimit || ordersAssignedToCourierButNotServed.size() > 0){
            // Keep track of current time
            if (counter == orderTimes.size()-1){
                currentTime = timeNextCourierArrivesAtOrder;
            }else{
                currentTime = std::min(timeCustomerArrives, timeNextCourierArrivesAtOrder);
            }

            if (timeCustomerArrives < timeNextCourierArrivesAtOrder && currentTime <= timeLimit && counter<orderTimes.size()-1){
                timeCustomerArrives += orderTimes[counter];
                counter += 1;
                // Draw new order and assign it to warehouse, picker and courier. MUST BE IN THAT ORDER!!!
                Order* newOrder = new Order;
                initOrder(timeCustomerArrives, newOrder);
                orders.push_back(newOrder);
                // We immediately assign the order to a warehouse and a picker
                warehouseForOrderREINFORCE(newOrder, *net, false);
                if (newOrder->accepted){
                    choosePickerForOrder(newOrder);
                    // If there are couriers assigned to the warehouse, we can assign a courier to the order
                    if (newOrder->assignedWarehouse->couriersAssigned.size()>0){
                        chooseCourierForOrder(newOrder);
                        ordersAssignedToCourierButNotServed.push_back(newOrder);
                    }else{ // else we add the order to list of orders that have not been assigned to a courier yet
                        newOrder->assignedWarehouse->ordersNotAssignedToCourier.push_back(newOrder);  
                    }
                }

            }else { // when a courier arrives at an order
                if (nextOrderBeingServed){
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
        }
        //std::cout<<"----- Iteration: " << epoch << " Number of orders that arrived: " << orders.size() << " and served: " << nbOrdersServed << " Obj. value: " << getObjValue() << ". Mean wt: " << totalWaitingTime/nbOrdersServed <<" seconds. Highest wt: " << highestWaitingTimeOfAnOrder <<" seconds. -----" <<std::endl;
        //writeRoutesAndOrdersToFile("data/animationData/routes_REINFORCE.txt", "data/animationData/orders_REINFORCE.txt");
        running_costs += getObjValue();
        runningCounter += 1;
    }
    std::cout<< "Iterations: " << runningCounter << " Average costs: " << running_costs / runningCounter <<std::endl;
    
}

void Environment::simulate(std::string policy, int timeLimit, float lambdaTemporal, float lambdaSpatial)
{
    if (policy == "nearestWarehouse"){
        nearestWarehousePolicy(timeLimit);
    }else if (policy == "trainREINFORCE"){
        trainREINFORCE(timeLimit, lambdaTemporal, lambdaSpatial);
    }else if (policy == "testREINFORCE"){
        testREINFORCE(timeLimit);
    }else{
        std::cerr<<"Method: " << policy << " not found."<<std::endl;
    }

}




