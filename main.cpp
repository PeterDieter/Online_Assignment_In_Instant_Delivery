#include <time.h>
#include <iostream>

#include "Params.h"

int main()
{
    // Reading the data file and initializing some data structures
  std::cout << "----- READING DATA SET" << std::endl;
  Params data;
  std::cout << "----- Number of Clients: " << data.nbClients << std::endl;

  return 0;
}