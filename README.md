# Optimal Service Area Sizing for Ultra-Fast Delivery Services

Data is prepared in Python and an instance is then passed to C++. The raw and processed data is contained in folder [data](data). All code related to preprocessing data (including Isochrone API and DistanceMatrix API) and creating code to create instances is contained in [dataCode](dataCode).

For neural network stuff, we use Pytorch. So make sure that Pytorch is installed and check [this](https://github.com/pytorch/pytorch/issues/12449) out if cmake has trouble finding Pytorch. Furthermore, we use cmake to create an executable. Run 

```
cmake -DCMAKE_PREFIX_PATH=$PWD/../libtorch
make
```



You can then execute the code with:

```
./onlineAssignment instanceName policyName
```

where **instanceName** gives the path to the .txt file containing the instance information and **policyName** is a string that determines the policy which will be applied/trained. For example:

```
./onlineAssignment instances/instances_900_8_3_30_120_60.txt nearestWarehouse
```

Currently, the following policies are available:
1. nearestWarehouse: In this policy, the nearest warehouse is selected for each order and each courier is also assigned back to his nearest warehouse. Each order is accepted.
2. REINFORCE: In this policy, we train a neural network with the REINFORCE algorithm to assign orders to warehouses/ to reject orders.

Visualization of the problem (made in [visualizeSimulation.py](dataCode/visualizeSimulation.py)):

<p style="text-align:center;">
<img src="animation.gif" width="300" height="400" align="center"></p>