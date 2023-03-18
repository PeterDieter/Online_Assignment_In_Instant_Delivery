#include <time.h>
#include <iostream>
#include <typeinfo>


#include "Params.h"
#include "Environment.h"

int main(int argc, char * argv[])
{
  // Reading the data file and initializing some data structures
  std::cout << "----- READING DATA SET " << argv[1] << " -----" << std::endl;
  Params params(argv[1]);
  std::cout << "----- Instance with " << params.nbClients << " Clients, " << params.nbWarehouses << " Warehouses, and " << params.nbQuadrants << " Quadrants -----" << std::endl;

  // The optimization code should come here


  // Creating the Environment
  Environment environment(&params);
  environment.simulate(3600);


  // return 0 upon succesful completion
  return 0;
}