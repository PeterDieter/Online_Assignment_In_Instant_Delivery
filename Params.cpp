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
#include "xorshift128.h"



Params::Params(std::string instanceName)
{
	rng = XorShift128(42);
	nbClients = 0;
	nbQuadrants = 0;
	nbWarehouses = 0;
	nbCouriers = 0;
	nbPickers = 0;
	interArrivalTime = 15;
	meanCommissionTime = 80;
	paramClients = std::vector<Client>(40000);
	paramWarehouses = std::vector<Warehouse>(30);
	std::string content, content2, content3;
	std::ifstream inputFile(instanceName);
	if (inputFile.is_open())
	{
		for (inputFile >> content; content != "EOF"; inputFile >> content)
		{
			if (content == "NUMBER_CLIENTS")
				{
					inputFile >> content2 >> nbClients;
				}
			else if (content == "NUMBER_WAREHOUSES")
				{
					inputFile >> content2 >> nbWarehouses;
				}
			else if (content == "NUMBER_QUADRANTS")
				{
					inputFile >> content2 >> nbQuadrants;
				}
			else if (content == "INTER_ARRIVAL_TIME")
				{
					inputFile >> content2 >> interArrivalTime;
				}
			else if (content == "MEAN_COMMISSION_TIME")
				{
					inputFile >> content2 >> meanCommissionTime;
				}
			else if (content == "WAREHOUSE_SECTION")
				{
					// Reading warehouse data
					for (int i = 0; i < nbWarehouses; i++)
					{
						inputFile >> paramWarehouses[i].wareID >> paramWarehouses[i].lon >> paramWarehouses[i].lat >> paramWarehouses[i].initialNbCouriers >> paramWarehouses[i].initialNbPickers;
						nbCouriers += paramWarehouses[i].initialNbCouriers;
						nbPickers += paramWarehouses[i].initialNbPickers;
					}
					
					// Reduce the size of the vector of warehouses if possible
					paramWarehouses.resize(nbWarehouses);
				}
			else if (content == "CLIENT_SECTION")
				{
					// Reading client data
					for (int i = 0; i < nbClients; i++)
					{
						inputFile >> paramClients[i].clientID >> paramClients[i].lon >> paramClients[i].lat >> paramClients[i].inQuadrantID;
					}
								// Reduce the size of the vector of clients if possible
					paramClients.resize(nbClients);
				}
			else if (content == "EDGE_WEIGHT_SECTION")
				{
					travelTime = Matrix(nbClients, nbWarehouses);
					for (int i = 0; i < nbClients; i++)
					{
						for (int j = 0; j < nbWarehouses; j++)
						{	
							// Keep track of the largest distance between two clients (or the depot)
							int cost;
							inputFile >> cost;
							travelTime.set(i, j, cost);
						}
					}
				}
		}
	}

}


