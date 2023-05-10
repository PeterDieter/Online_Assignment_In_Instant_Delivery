# Spatio-temporal Credit Assignment in Reinforcement Learning for Multi-Depot Delivery Services

In the problem at hand, we operate multiple warehouse (depots) in a service region from which couriers start their trip to serve an order. When an order arrives, we now need to assign it to a warehouse and a courier. The goal is to minimize the waiting time of customers. We can also reject customers, which comes with certain costs. Furthermore, we can also rebalance couriers by re-assinging them to the warehouses. Therefore, we split up the problem into two subproblems: The problem of assigning orders to warehouses and couriers (**assignment problem**) and the problem of rebalancing the couriers (**rebalancing problem**).

Visualization of the problem (made in [visualizeSimulation.py](python/visualizeSimulation.py)):

<p align="center">
<img src="animation.gif" width="300" height="400" align="center">
</p>


## Data
For the warehouses, we use (the 10) Getir stores in Chicago. For the order data, we use publicly available anonymized customer data of Amazon customers in Chicago. We then draw random orders from these customers. Interarrival times, service times and comission times are all assumed to be exponentially distributed. An instance can be created in [createInstance.py](python/createInstance.py):

## Assignment problem

To solve the assignment problem, we now have implemented the REINFORCE algorithm. We aggregate the states by a set of features in use a deep neural network to approximate the policy function. In the REINFORCE method, a crucial part is the credit assignment problem, i.e., the problem to assign credit to a specific action. Naturally, we could just state that an action $a_i$ is the waiting time for customer $i$, i.e., $w_i$. Mathematically, we can express this as:

$$
\begin{align}
C(s_i, a_i) = w_i
\end{align}
$$

where $s_i$ is the state when the $i^{\text{\tiny th}}$ customer arrives. However, this neglects the fact, that an action taken in epoch $i$ has an effect on later actions and consequently, the associated costs. To tackle this issue, we add discounted costs of future customers to the current costs. Also, we only add costs of customers at the same warehouse, i.e, $D_i = D_j$. This cost assignment is possible in training phase, as we can look at the problem retrospectively. We therefore modify the initial cost function as follows: 

$$
\begin{align}
C(s_i, a_i) = w_i + \sum_{j>i \atop{D_i = D_j}} w_j \cdot \lambda_t^{t_j - t_i}
\end{align}
$$

where $\lambda_t$ is a parameter that determines how much future costs weight and $t_j$ ($t_j$) is the time customer $i$ ($j$) arrives in the system. Therefore, costs of customers further in the future weigh less than costs of customers in the close future. If $\lambda_t = 0$, the equation reduces to equation $(1)$. This approach resembles quite a bit to the TD($\lambda$) method. The following graph shows the convergence for different values of $\lambda_t$. Be ware that all results shown below are preliminary and at a very early stage. 

<p align="center">
<img src="convergenceTemporal.png" width="600" height="400" align="center"></p>

We can see that a value of $\lambda_t = 0.99$ leads to the best results, and saves around 9% of costs compared to $\lambda_t = 0$. Further, we can see that the higher $\lambda_t$, the slower the convergence.

What we now plan to do is to not only consider temporal vicinity, but also spatial vicinity, as future customers geographically close to customer $i$ should weigh more in the cost approximation of customer $i$, compared to customers further away. We implemented the following:

$$
\begin{align}
C(s_i, a_i) = w_i + \sum_{j>i} w_j \cdot \lambda_t^{t_j - t_i} \cdot \lambda_s^{d_{ij}}
\end{align}
$$

where $d_{ij}$ is the distance between warehouse of customer $i$ and $j$. Keeping $\lambda_t$ at  $0.99$, the following graph shows the convergence if we include a spatial lambda of $\lambda_s = 0.85$ as formulated in equation (3). We can see that compared to only temporal $\lambda_t$, including a spatial $\lambda_s$ leads to a slightly faster convergence and cost savings of around 10%. From other tests (not shown here), we also found that using spatial information for the credit assignment becomes increasingly important when interarrival time of orders is low, i.e., we need to make more deliberate deicisons to which warehouse to assign an order. 

<p align="center">
<img src="convergenceSpatioTemporal.png" width="600" height="400" align="center"></p>



## Rebalancing problem
To be done...

## C++ compiling 
Data is prepared in Python and an instance is then passed to C++. The raw and processed data is contained in folder [data](data). All code related to preprocessing data (including Isochrone API and DistanceMatrix API) and creating code to create instances is contained in [python](python).

For neural network stuff, we use Pytorch. So make sure that Pytorch is installed and check [this](https://github.com/pytorch/pytorch/issues/12449) out if cmake has trouble finding Pytorch. Furthermore, we use cmake to create an executable. Run 

```
cmake -DCMAKE_PREFIX_PATH=$PWD/../libtorch
make
```

## Running the program

You can then execute the code with:

```
./onlineAssignment instanceName methodName
```

where **instanceName** gives the path to the .txt file containing the instance information, **methodName** is a string that determines the method which will be applied/trained for the assignment problem. In case the REINFORCE method is trained, one additionally needs to specify  **lambdaTemporal** and **lambdaSpatial**, which are float parameters (see section above). For example:

```
./onlineAssignment instances/instance_900_8_3_30_120_60_train.txt trainREINFORCE 0.99 0.85
```

Currently, the following methods are available:
1. nearestWarehouse: In this policy, the nearest warehouse is selected for each order and each courier is also assigned back to his nearest warehouse. Each order is accepted.
2. trainREINFORCE: In this method, we train a neural network with the REINFORCE algorithm to assign orders to warehouses/ to reject orders. The neural network gets saved as "net_REINFORCE.pt".
3. testREINFORCE: We apply the policy net which was trained in the "trainREINFORCE" method.

