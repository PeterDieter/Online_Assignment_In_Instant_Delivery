#include <time.h>
#include <iostream>
#include <typeinfo>


#include "Data.h"
#include "Environment.h"

int main(int argc, char * argv[])
{
  // Reading the data file and initializing some data structures
  std::cout << "----- READING DATA SET " << argv[1] << " -----" << std::endl;
  Data data(argv[1]);
  std::cout << "----- Instance with " << data.nbClients << " Clients, " << data.nbWarehouses << " Warehouses -----"<< std::endl;

  // Creating the Environment
  Environment environment(&data);
  environment.simulate(argv[2], 7200);


  // return 0 upon succesful completion
  return 0;
}