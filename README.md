# Spatio-temporal Credit Assignment in Reinforcement Learning for Ultra-Fast Delivery Services

Data is prepared in Python and an instance is then passed to C++. The raw and processed data is contained in folder [data](data). All code related to preprocessing data (including Isochrone API and DistanceMatrix API) and creating code to create instances is contained in [dataCode](dataCode).

For neural network stuff, we use Pytorch. So make sure that Pytorch is installed and check [this](https://github.com/pytorch/pytorch/issues/12449) out if cmake has trouble finding Pytorch. Furthermore, we use cmake to create an executable. Run 

```
cmake -DCMAKE_PREFIX_PATH=$PWD/../libtorch
make
```



You can then execute the code with:

```
./onlineAssignment instanceName methodName
```

where **instanceName** gives the path to the .txt file containing the instance information and **methodName** is a string that determines the method which will be applied/trained. For example:

```
./onlineAssignment instances/instances_900_8_3_30_120_60_test.txt nearestWarehouse
```

Currently, the following methods are available:
1. nearestWarehouse: In this policy, the nearest warehouse is selected for each order and each courier is also assigned back to his nearest warehouse. Each order is accepted.
2. trainREINFORCE: In this method, we train a neural network with the REINFORCE algorithm to assign orders to warehouses/ to reject orders. The neural network gets saved as "net_REINFORCE.pt".
3. testREINFORCE: We apply the policy net which was trained in the "trainREINFORCE" method.

Visualization of the problem using the nearest Warehouse policy (made in [visualizeSimulation.py](dataCode/visualizeSimulation.py)):

<p style="text-align:center;">
<img src="animation.gif" width="300" height="400" align="center"></p>

## Spatio-temporal credit assignment
In the REINFORCE method, a crucial part is the credit assignment problem, i.e., the problem to assign credit to a specific action. Naturally, we could just state that an action $a_i$ is the waiting time for customer $i$, i.e., $w_i$. Mathematically, we can express this as:

$$
\begin{align}
C(s_i, a_i) = w_i
\end{align}
$$

where $s_i$ is the state when the $i^{\text{\tiny th}}$ customer arrives. However, this neglects the fact, that an action taken in epoch $i$ has an effect on later actions and consequently, the associated costs. To tackle this issue, we add discounted costs of future customers to the current costs. Also, we only add costs of customers at the same warehouse, i.e, $D_i = D_j$. This cost assignment is possible in training phase, as we can look at the problem retrospectively. We therefore modify the initial cost function as follows: 

$$
\begin{align}
C(s_i, a_i) = w_i + \sum_{j>i \atop{W_i = W_j}} w_j \cdot \lambda_t^{t_j - t_i}
\end{align}
$$

where $\lambda_t$ is a parameter that determines how much future costs weight and $t_j$ ($t_j$) is the time customer $i$ ($j$) arrives in the system. Therefore, costs of customers further in the future weigh less than costs of customers in the close future. If $\lambda_t = 0$, the equation reduces to equation $(1)$. This approach resembles quite a bit to the TD($\lambda_t$) method. The following graph shows the convergence for different values of $\lambda_t$.   

<p style="text-align:center;">
<img src="convergence.png" width="600" height="400" align="center"></p>

We can see that a value of $\lambda_t = 0.99$ leads to the best results, and saves around 8% of costs compared to $\lambda_t = 0$. Further, we can see that the higher $\lambda_t$, the slower the convergence.
What we now plan to do is to not only consider temporal vicinity, but also spatial vicinity, as future customers geographically close to customer $i$ should weigh more in the cost approximation of customer $i$, compared to customers further away. We plan to implement something like the following:

$$
\begin{align}
C(s_i, a_i) = w_i + \sum_{j>i} w_j \cdot \lambda_t^{t_j - t_i} \cdot \lambda_s^{d_{ij}}
\end{align}
$$

where $d_{ij}$ is the distance between warehouse of customer $i$ and $j$. 