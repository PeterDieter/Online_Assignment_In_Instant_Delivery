import requests
import json

with open('data/getirStores.json') as fp:
    getirStores = json.load(fp)

coord =[[val.get('longitude'), val.get('latitude')] for val in getirStores.values()]
body = {"locations":coord[5:],"range":[600, 900]}

headers = {
    'Accept': 'application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8',
    'Authorization': '5b3ce3597851110001cf62485cc6e778c4474849961f17cfd230ed0b',
    'Content-Type': 'application/json; charset=utf-8'
}
call = requests.post('https://api.openrouteservice.org/v2/isochrones/cycling-electric', json=body, headers=headers)

print(call.status_code, call.reason)
switch = True
counter = 5
for i in range(len(call.json()['features'])):
    if switch:
        with open('data/isochroneWarehouse'+ '_600s_' + str(counter) + '.json', 'w') as f:
            json.dump(call.json()['features'][i], f)
    else:
        with open('data/isochroneWarehouse'+ '_900s_' + str(counter) + '.json', 'w') as f:
            json.dump(call.json()['features'][i], f)
        counter += 1
    switch = not switch