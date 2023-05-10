import requests
import json


def downLoadIsochrones(warehouses):
    """Save isochrones to file with openrouteservice API
    Args:
        warehouses (dict) : Dict containing the warehouses
    Returns:
        None 
    """


    coord =[[val.get('longitude'), val.get('latitude')] for val in warehouses.values()]
    body = {"locations":coord[5:],"range":[600, 900]}

    headers = {
        'Accept': 'application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8',
        'Authorization': '5b3ce3597851110001cf62485cc6e778c4474849961f17cfd230ed0b',
        'Content-Type': 'application/json; charset=utf-8'
    }
    call = requests.post('https://api.openrouteservice.org/v2/isochrones/cycling-electric', json=body, headers=headers)

    print(call.status_code, call.reason)
    switch = True
    
    # The api only lets us compute 5 isochrones at a time
    counter = 5
    for i in range(len(call.json()['features'])):
        if switch:
            with open('dadta/isochroneWarehouse'+ '_600s_' + str(counter) + '.json', 'w') as f:
                json.dump(call.json()['features'][i], f)
        else:
            with open('datda/isochroneWarehouse'+ '_900s_' + str(counter) + '.json', 'w') as f:
                json.dump(call.json()['features'][i], f)
            counter += 1
        switch = not switch



if __name__ == "__main__":
    with open('data/getirStores.json') as fp:
        getirStores = json.load(fp)

    downLoadIsochrones(warehouses=getirStores)