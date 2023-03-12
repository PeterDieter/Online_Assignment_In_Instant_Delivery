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



Params::Params()
{
	nbClients = 0;
	nbQuadrants = 0;
	nbWarehouses = 0;
	int client;
	clients = std::vector<Client>(40000);
	
	std::ifstream inputFile("data/allDurations15.txt");
	if (inputFile.is_open())
	{
		std::string line;
		while ( getline (inputFile,line) )
		{
			inputFile >> clients[nbClients].clientInd >> clients[nbClients].coordLat >> clients[nbClients].coordLon;
			nbClients++;
		}

		nbClients--;
		// Reduce the size of the vector of clients if possible
		clients.resize(nbClients);
	}

}




