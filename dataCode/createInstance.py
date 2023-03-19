import pandas as pd
import numpy as np
import json
from matplotlib import pyplot as plt 

def create_instance(fileName: str, limit: int=900, couriersPerWarehouse: int=5, pickersPerWarehouse: int=3, interArrivalTime: int=15, meanComissionTime: int=120, meanServiceTimeAtClient: int=60):
    df = pd.read_csv("data/allDurations15.csv", header=0) 
    df = df.to_numpy()
    clients = df[:,:3]
    matrix = df[:,3:].astype(int)

    with open('data/getirStores.json') as fp:
        getirStores = json.load(fp)
    warehouses = np.array([[val.get('longitude'),val.get('latitude')] for val in getirStores.values()])
    warehouses = np.c_[warehouses, np.ones(len(warehouses))*couriersPerWarehouse, np.ones(len(warehouses))*pickersPerWarehouse]

    # Remove clients where distance to warehouses is over limit
    minValues = matrix.min(axis=1)
    notInLimit = np.where(minValues > limit)[0]
    clients = np.delete(clients, notInLimit, axis=0)
    matrix = np.delete(matrix, notInLimit, axis=0)

    # For now we just assign each customer to the 0 quadrant
    clients = np.c_[clients, np.zeros(len(clients))]


    with open("instances/"+fileName+".txt", 'w') as f:
        f.write("\n".join([
            "{} : {}".format(k, v)
            for k, v in [
                ("NAME", fileName),
                ("NUMBER_CLIENTS", len(clients)),
                ("NUMBER_WAREHOUSES", len(warehouses)),
                ("NUMBER_QUADRANTS", 1),
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
            "{}\t{}\t{}\t{}".format(i, y, z, int(q))
            for i, (x, y, z, q) in enumerate(clients)
        ]))
        f.write("\n")

        f.write("EDGE_WEIGHT_SECTION\n")
        for row in matrix:
            f.write("\t".join(map(str, row)))
            f.write("\n")
        
        
        f.write("EOF\n")



if __name__ == "__main__":
    create_instance(fileName = "instance900_8_3_30_120_60", limit=900, couriersPerWarehouse=4, pickersPerWarehouse=3, interArrivalTime=60, meanComissionTime=120, meanServiceTimeAtClient=60)