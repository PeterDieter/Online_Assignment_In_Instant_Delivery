import pandas as pd
import plotly.graph_objects as go
import json



def plotData(clients, warehouses):
    df = pd.DataFrame(clients, columns =['Latitude', 'Longitude', 'PackageVolume', 'Date'])
    fig = go.Figure(go.Scattermapbox(lat=df.Latitude, lon=df.Longitude))

    fig.add_trace(go.Scattermapbox(
            lat=[val.get('latitude') for val in warehouses.values()],
            lon=[val.get('longitude') for val in warehouses.values()],
            mode='markers',
            marker=go.scattermapbox.Marker(
                size=16,
                color='rgb(0, 70, 0)',
                opacity=1
            ),
            hoverinfo='text'
        ))
    fig.update_layout(mapbox_style="stamen-terrain", mapbox_center_lon=-87.7493, mapbox_center_lat=41.958, mapbox_zoom=9)
    fig.update_layout(margin={"r":0,"t":0,"l":0,"b":0})
    fig.show()


if __name__ == "__main__":
    with open('data/getirStores.json') as fp:
        getirStores = json.load(fp)

    Stopls = pd.read_csv("data/stopData_Chicago.csv", header=None, names=['Latitude', 'Longitude', 'PackageVolume', 'Date']) 

    plotData(Stopls, getirStores)