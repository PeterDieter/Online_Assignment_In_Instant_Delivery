#include <algorithm>
#include <set>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>

#include "Data.h"
#include "Matrix.h"
#include "xorshift128.h"



Data::Data(std::string instanceName)
{
	rng = XorShift128(42);
	nbClients = 0;
	nbQuadrants = 0;
	nbWarehouses = 0;
	nbCouriers = 0;
	nbPickers = 0;
	interArrivalTime = 15;
	meanCommissionTime = 120;
	meanServiceTimeAtClient = 60;
	paramClients = std::vector<Client>(40000); // 40000 is an upper limit, can be increase ofc
	paramWarehouses = std::vector<Warehouse>(30); // 30 is an upper limit, can be increased ofc
	std::string content, content2, content3;
	std::ifstream inputFile(instanceName);
	if (!inputFile) throw std::runtime_error("Could not find file instance");
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
			else if (content == "INTER_ARRIVAL_TIME")
				{
					inputFile >> content2 >> interArrivalTime;
				}
			else if (content == "MEAN_COMMISSION_TIME")
				{
					inputFile >> content2 >> meanCommissionTime;
				}
			else if (content == "MEAN_SERVICE_AT_CLIENT_TIME")
				{
					inputFile >> content2 >> meanServiceTimeAtClient;
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
						inputFile >> paramClients[i].clientID >> paramClients[i].lon >> paramClients[i].lat;
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


