import requests
import pandas as pd
import json
import time
import numpy as np


def getDistances(clients, warehouses, fileToSave):
    """Save distances to a file with openrouteservice API
    Args:
        clients (dataframe) : Pandas dataframe with all clients
        warehouses (dict) : Dict containing the warehouses
        fileToSave (string) : File where to save the data
    Returns:
        None 
    """
    
    coordWarehouses = [[val.get('longitude'),val.get('latitude')] for val in warehouses.values()],
    counter = 0
    allCoords = []
    chunk, allDurations = [], []
    # For each client
    for index, row in clients.iterrows():
        chunk.append([row['Longitude'], row['Latitude']])
        # OpenRoute only lets us compute 2500 distances per request, as we have 10 warehouses, we can fill our chunk with 250 clients
        if len(chunk) >= 250:
            coords = coordWarehouses[0]+chunk
            sources = list(range(10))
            destinations = list(range(10,len(coords)))
            body = {"locations":coords,"destinations":sources,"sources":destinations}

            headers = {
                'Accept': 'application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8',
                'Authorization': '5b3ce3597851110001cf62485cc6e778c4474849961f17cfd230ed0b',
                'Content-Type': 'application/json; charset=utf-8'
            }
            call = requests.post('https://api.openrouteservice.org/v2/matrix/cycling-electric', json=body, headers=headers)

            print(call.status_code, call.reason)
            allDurations += call.json()['durations']
            allCoords += coords[10:]
            dfToSave = pd.DataFrame(np.array(np.concatenate((np.array(allCoords), np.array(allDurations)),axis=1)))
            dfToSave.columns =["Longitude", "Latitude", "Store1", "Store2", "Store3", "Store4", "Store5", "Store6", "Store7", "Store8", "Store9", "Store10"]
            dfToSave.to_csv(fileToSave,index_label='Index_name')
            chunk = []
            counter += 1
            print(counter, counter*250)  
            # We put the code to sleep to not overload the API
            time.sleep(2)


if __name__ == "__main__":
    df = pd.read_csv("data/stopData15Minutes.csv", header=0) 
    with open('data/getirStores.json') as fp:
        getirStores = json.load(fp)

    getDistances(df, getirStores, "allDurations15.csv")
