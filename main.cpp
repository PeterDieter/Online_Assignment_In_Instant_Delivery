#include <time.h>
#include <iostream>
#include <typeinfo>


#include "Params.h"
#include "Environment.h"

int main()
{
  // Reading the data file and initializing some data structures
  std::cout << "----- READING DATA SET -----" << std::endl;
  Params params;
  std::cout << "----- Instance with " << params.nbClients << " Clients, " << params.nbWarehouses << " Warehouses, and " << params.nbQuadrants << " Quadrants -----" << std::endl;

  // The optimization code should come here


  // Creating the Environment
  Environment environment(&params);
  environment.simulate(18200);


  // return 0 upon succesful completion
  return 0;
}