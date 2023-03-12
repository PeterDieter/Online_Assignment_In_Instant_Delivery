from shapely.ops import unary_union
from shapely.geometry import shape, mapping, Point
from shapely.geometry.polygon import Polygon
import geopandas as gpd
import json 
import pandas as pd
import plotly.graph_objects as go
from area import area


polygons = []
for i in range(10):
    with open('data/isochroneWarehouse_900s_' + str(i) + '.json', 'r') as json_data:
        d = json.load(json_data)
        polygons.append(shape(d['geometry']))

boundary15 = Polygon(gpd.GeoSeries(unary_union(polygons))[0])
boundaryGeoJson15 = gpd.GeoSeries([boundary15]).__geo_interface__

polygons = []
for i in range(10):
    with open('data/isochroneWarehouse_600s_' + str(i) + '.json', 'r') as json_data:
        d = json.load(json_data)
        polygons.append(shape(d['geometry']))

boundary10 = Polygon(gpd.GeoSeries(unary_union(polygons))[0])
boundaryGeoJson10 = gpd.GeoSeries([boundary10]).__geo_interface__

print(area(boundaryGeoJson10['features'][0]['geometry']))

with open('data/getirStores.json') as fp:
    getirStores = json.load(fp)

Stopls = pd.read_csv("data/stopData_Chicago.csv", header=None, names=['Latitude', 'Longitude', 'PackageVolume', 'Date']) 

df = pd.DataFrame(Stopls, columns =['Latitude', 'Longitude', 'PackageVolume', 'Date'])

df['inRange'] = 0
for index, row in df.iterrows():
    if (boundary10.contains(Point(row['Longitude'], row['Latitude']))):
        df.loc[index,'inRange'] = 1
df = df.drop(df[df.inRange < 1].index)
df = df[['Latitude', 'Longitude']]
df.to_csv('data/stopData10Minutes.csv', header=True)

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

fig.update_layout(
        mapbox={
            "layers": [
                {
                    "source": boundaryGeoJson10,
                    "below": "traces",
                    "type": "line",
                    "color": "black",
                    "line": {"width": 3.5},
                },
                {
                    "source": boundaryGeoJson15,
                    "below": "traces",
                    "type": "line",
                    "color": "blue",
                    "line": {"width": 3.5},
                }

            ],
        },
        margin={"l": 0, "r": 0, "t": 0, "b": 0},
    )

fig.update_layout(mapbox_style="stamen-terrain", mapbox_center_lon=180)
fig.update_layout(margin={"r":0,"t":0,"l":0,"b":0})
fig.show()

