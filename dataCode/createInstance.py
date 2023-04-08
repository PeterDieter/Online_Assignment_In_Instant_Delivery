import pandas as pd
import numpy as np
import json
import random

def create_instance(fileName: str, limit: int=900, couriersPerWarehouse: int=5, pickersPerWarehouse: int=3, interArrivalTime: int=15, meanComissionTime: int=120, meanServiceTimeAtClient: int=60):
    """Create a .txt file of a problem instance
    Args:
        fileName (str): File in which we save the instance parameters
        limit (int): Clients whose minimum distance (in seconds) to a warehouse is above this limit are excluded 
        couriersPerWarehouse (int): number of couriers that a warehouse has at the start
        pickersPerWarehouse (int): number of pickers that a warehouse has
        interArrivalTime (int): arrival rate (exponentially distributed) of orders (in seconds)
        meanComissionTime (int): Time a picker needs on average to comission an order (expoentially distributed) (in seconds)
        meanServiceTimeAtClient (int): Mean time a courier needs at the clients door to deliver the order (expoentially distributed) (in seconds)
    Returns:
        None 
    """
    
    df = pd.read_csv("data/allDurations15.csv", header=0) 
    df = df.to_numpy()
    random.seed(42)
    rndIdxs = random.sample(range(len(df)), round(len(df)*0.75))
    clients = df[rndIdxs,:3]
    matrix = df[rndIdxs,3:].astype(int)
    # clients = np.delete(clients, rndIdxs, axis=0)
    # matrix = np.delete(matrix, rndIdxs, axis=0)

    with open('data/getirStores.json') as fp:
        getirStores = json.load(fp)
    warehouses = np.array([[val.get('longitude'),val.get('latitude')] for val in getirStores.values()])
    warehouses = np.c_[warehouses, np.ones(len(warehouses))*couriersPerWarehouse, np.ones(len(warehouses))*pickersPerWarehouse]

    # Remove clients where distance to warehouses is over limit
    minValues = matrix.min(axis=1)
    notInLimit = np.where(minValues > limit)[0]
    clients = np.delete(clients, notInLimit, axis=0)
    matrix = np.delete(matrix, notInLimit, axis=0)

    with open("instances/"+fileName+".txt", 'w') as f:
        f.write("\n".join([
            "{} : {}".format(k, v)
            for k, v in [
                ("NAME", fileName),
                ("NUMBER_CLIENTS", len(clients)),
                ("NUMBER_WAREHOUSES", len(warehouses)),
                ("INTER_ARRIVAL_TIME", interArrivalTime),
                ("MEAN_COMMISSION_TIME", meanComissionTime),
                ("MEAN_SERVICE_AT_CLIENT_TIME", meanServiceTimeAtClient)]
        ]))
        f.write("\n")
        
        f.write("WAREHOUSE_SECTION\n")
        f.write("\n".join([
            "{}\t{}\t{}\t{}\t{}".format(i, y, z, int(q), int(p))
            for i, (y, z, q, p) in enumerate(warehouses)
        ]))
        f.write("\n")


        f.write("CLIENT_SECTION\n")
        f.write("\n".join([
            "{}\t{}\t{}".format(i, y, z)
            for i, (x, y, z) in enumerate(clients)
        ]))
        f.write("\n")

        f.write("EDGE_WEIGHT_SECTION\n")
        for row in matrix:
            f.write("\t".join(map(str, row)))
            f.write("\n")
        
        
        f.write("EOF\n")



if __name__ == "__main__":
    create_instance(fileName = "instance_900_8_3_30_120_60_train", limit=900, couriersPerWarehouse=5, pickersPerWarehouse=3, interArrivalTime=30, meanComissionTime=120, meanServiceTimeAtClient=60)