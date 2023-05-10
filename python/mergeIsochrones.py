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
    with open('data/warehouseIsochrones/isochroneWarehouse_900s_' + str(i) + '.json', 'r') as json_data:
        d = json.load(json_data)
        polygons.append(shape(d['geometry']))

boundary15 = Polygon(gpd.GeoSeries(unary_union(polygons))[0])
gpd.GeoSeries(unary_union(polygons)).to_file("data/Polygon_900s",driver='ESRI Shapefile')
boundaryGeoJson15 = gpd.GeoSeries([boundary15]).__geo_interface__

polygons = []
for i in range(10):
    with open('data/warehouseIsochrones/isochroneWarehouse_600s_' + str(i) + '.json', 'r') as json_data:
        d = json.load(json_data)
        polygons.append(shape(d['geometry']))

boundary10 = Polygon(gpd.GeoSeries(unary_union(polygons))[0])
boundaryGeoJson10 = gpd.GeoSeries([boundary10]).__geo_interface__

print("Area of the 10 minute polygon in sqm: ", round(area(boundaryGeoJson10['features'][0]['geometry'])))
print("Area of the 15 minute polygon in sqm: ", round(area(boundaryGeoJson15['features'][0]['geometry'])))



with open('data/getirStores.json') as fp:
    getirStores = json.load(fp)
Stopls = pd.read_csv("data/stopData_Chicago.csv", header=None, names=['Latitude', 'Longitude', 'PackageVolume', 'Date']) 

df = pd.DataFrame(Stopls, columns =['Latitude', 'Longitude', 'PackageVolume', 'Date'])

df['inRange15'], df['inRange10'] = 0, 0
for index, row in df.iterrows():
    if (boundary15.contains(Point(row['Longitude'], row['Latitude']))):
        df.loc[index,'inRange15'] = 1
        if (boundary10.contains(Point(row['Longitude'], row['Latitude']))):
            df.loc[index,'inRange10'] = 1

df = df.drop(df[df.inRange15 < 1].index)
df = df[['Latitude', 'Longitude']]
#df.to_csv('data/stopData15Minutes.csv', header=True)

fig = go.Figure(go.Scattermapbox(lat=df.Latitude, lon=df.Longitude))

fig.add_trace(go.Scattermapbox(
        lat=[val.get('latitude') for val in getirStores.values()],
        lon=[val.get('longitude') for val in getirStores.values()],
        mode='markers',
        marker=go.scattermapbox.Marker(
            size=20,
            color='black',
            opacity=1
        ),
        hoverinfo='text'
    ))

fig.update_layout(
        mapbox={
            "layers": [
                {
                    "source": boundaryGeoJson10,
                    "type": "line",
                    "color": "black",
                    "line": {"width": 3.5},
                },
                {
                    "source": boundaryGeoJson15,
                    "type": "line",
                    "color": "red",
                    "line": {"width": 3.5},
                }

            ],
        }
    )

fig.update_layout(mapbox_style="stamen-terrain", mapbox_center_lon=-87.7493, mapbox_center_lat=41.958, mapbox_zoom=9)
fig.update_layout(margin={"r":0,"t":0,"l":0,"b":0})
fig.show()

