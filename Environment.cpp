#include <algorithm>
#include <set>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>

#include "Params.h"
#include "Matrix.h"
#include "Environment.h"


Environment::Environment(Params* params)
{   
    // First we initialize the environment by assigning 
    InitializeEnvironment(params);

}



void Environment::InitializeEnvironment(Params* params)
{ 
    int courierCounter = 0;
    couriers = std::vector<Courier>(params->nbCouriers);
    orders = std::vector<Order>(0);
    warehouses = std::vector<Warehouse>(params->nbWarehouses);

    for (int wInd = 0; wInd < params->nbWarehouses; wInd++)
    {
        warehouses[wInd].wareInd = params->paramWarehouses[wInd].wareInd;
        warehouses[wInd].lat = params->paramWarehouses[wInd].lat;
        warehouses[wInd].lon = params->paramWarehouses[wInd].lon;
        warehouses[wInd].initialNbCouriers = params->paramWarehouses[wInd].initialNbCouriers;
        for (int cInd = 0; cInd < warehouses[wInd].initialNbCouriers; cInd++)
        {
            couriers[courierCounter].courierInd = courierCounter;
            couriers[courierCounter].available = true;
            couriers[courierCounter].currentLat = warehouses[wInd].lat;
            couriers[courierCounter].currentLon = warehouses[wInd].lon;
            couriers[courierCounter].assignedToWarehouse = &warehouses[wInd];
            warehouses[wInd].couriersAvailable.push_back(&couriers[courierCounter]);
            courierCounter ++;    
        }
        warehouses[wInd].workLoad = 0;
    }

}


