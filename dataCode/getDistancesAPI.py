import requests
import pandas as pd
import json
import time
import numpy as np

df = pd.read_csv("data/stopData15Minutes.csv", header=0) 
with open('data/getirStores.json') as fp:
    getirStores = json.load(fp)


coordGetir = [[val.get('longitude'),val.get('latitude')] for val in getirStores.values()],
counter = 0
allCoords = []
chunk, allDurations = [], []
for index, row in df.iterrows():
    chunk.append([row['Longitude'], row['Latitude']])
    if len(chunk) >= 250:
        coords = coordGetir[0]+chunk
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
        dfToSave.to_csv("allDurations15.csv",index_label='Index_name')
        chunk = []
        counter += 1
        print(counter, counter*250)
        # if counter > 3:
        #     break    
        time.sleep(2)

