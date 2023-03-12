import pandas as pd
import plotly.graph_objects as go
import json

with open('data/getirStores.json') as fp:
    getirStores = json.load(fp)

Stopls = pd.read_csv("data/stopData_Chicago.csv", header=None, names=['Latitude', 'Longitude', 'PackageVolume', 'Date']) 

df = pd.DataFrame(Stopls, columns =['Latitude', 'Longitude', 'PackageVolume', 'Date'])
fig = go.Figure(go.Densitymapbox(lat=df.Latitude, lon=df.Longitude,
                                 radius=4))

fig.add_trace(go.Scattermapbox(
        lat=[val.get('latitude') for val in getirStores.values()],
        lon=[val.get('longitude') for val in getirStores.values()],
        mode='markers',
        marker=go.scattermapbox.Marker(
            size=16,
            color='rgb(0, 70, 0)',
            opacity=1
        ),
        hoverinfo='text'
    ))
fig.update_layout(mapbox_style="stamen-terrain", mapbox_center_lon=180)
fig.update_layout(margin={"r":0,"t":0,"l":0,"b":0})
fig.show()
