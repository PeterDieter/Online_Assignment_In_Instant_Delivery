# Optimal Service Area Sizing for Ultra-Fast Delivery Services

Data is prepared in Python and an instance is then passed to C++. The raw and processed data is contained in folder [data](data). All code related to preprocessing data (including Isochrone API and DistanceMatrix API) and creating code to create instances is contained in [dataCode](dataCode).


We use make to create an executable. First run 

```
make clean
make all
```

you can then execute the code with:

```
./optimalAssignment instanceName
```

where instanceName gives the path to the .txt file containing the instance information. For example:

```
./optimalAssignment instances/instances900_8_3_30_120.txt
```
