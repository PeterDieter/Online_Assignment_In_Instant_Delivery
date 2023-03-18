import osmnx as ox
import networkx as nx
import pandas as pd
import numpy as np
import geopandas as gpd
import matplotlib.pyplot as plt
import matplotlib
from shapely.geometry import Point
import matplotlib.animation as ani
from IPython.display import HTML
import json
import math
import random

with open('data/getirStores.json') as fp:
    getirStores = json.load(fp)

# shapeFile = gpd.read_file("data/Polygon_900s").loc[0, 'geometry']
# G = ox.graph_from_polygon("shapeFile", network_type='drive')
# ox.save_graphml(G, 'data/chicago.graphml')
graph = ox.load_graphml("data/chicago.graphml")


df = pd.read_csv('routes.txt', sep=" ", header=None)
df.columns = ["startTime", "arrivalTime", "fLat", "fLon", "tLat", "tLon"]

warehouses = []
for w in getirStores:
    warehouses.append(ox.nearest_nodes(graph, getirStores[w]["longitude"], getirStores[w]["latitude"]))

x, y = [], []
for index, w in enumerate(warehouses):
    x.append(graph.nodes[warehouses[index]]["x"])
    y.append(graph.nodes[warehouses[index]]["y"])
data = pd.DataFrame(list(zip(x, y)), columns =['Lon', 'Lat'])

warehousePoints = gpd.GeoDataFrame(
    data, geometry=gpd.points_from_xy(data.Lon, data.Lat))
warehousePoints.crs = "EPSG:4326"

orig_nodes = ox.nearest_nodes(graph, df['fLon'], df['fLat'])
dest_nodes = ox.nearest_nodes(graph, df['tLon'], df['tLat'])

routes, lengths = [], []
for orig_node, dest_node in zip(orig_nodes, dest_nodes):
    routes.append(nx.shortest_path(graph, source=orig_node, target=dest_node, weight='length'))
    # lengths.append(nx.shortest_path_length(graph, source=orig_node, target=dest_node, weight='length'))

projected_graph = ox.project_graph(graph, to_crs="EPSG:3395")

route_coorindates = []
for route in routes:
    points = []
    for node_id in route:
        x = projected_graph.nodes[node_id]['x']
        y = projected_graph.nodes[node_id]['y']
        points.append([x, y])
    route_coorindates.append(points)
    
n_routes = len(route_coorindates)
max_route_len = max([len(x) for x in route_coorindates])

final_route_coordinates = []
for index, route in enumerate(routes):
    points = []
    counter = 0
    for t in range(0,df["arrivalTime"].max(),20):
        counter += 1
        show = 0
        if df.loc[index,"startTime"] < t and df.loc[index,"arrivalTime"] > t:
            step = min(math.ceil((t-df.loc[index,"startTime"])/20), len(route)-1)
            show = 1
        elif df.loc[index,"startTime"] > t:
            step = 0
        else:
            step = len(route)-1
        x = projected_graph.nodes[route[step]]['x']
        y = projected_graph.nodes[route[step]]['y']
        if show:
            points.append([x, y, show])
        else:
            points.append([2912, y, show])

    final_route_coordinates.append(points)
   
route_coorindates = final_route_coordinates
fig, ax = ox.plot_graph(projected_graph, node_size=0, edge_linewidth=0.5, show=False, close=False) # network
warehousePoints.to_crs('EPSG:3395', inplace=True)
warehousePoints.plot(ax=ax, color='red', label='warehouse', zorder = 2) # warehouses
warehousePoints.crs = "EPSG:4326"

# Each list is a route. Length of this list = n_routes
scatter_list = []
# Plot the first scatter plot (starting nodes = initial car locations = hospital locations)
for j in range(n_routes):
    scatter_list.append(ax.scatter(route_coorindates[j][0][0], # x coordiante of the first node of the j route
                                route_coorindates[j][0][1], # y coordiante of the first node of the j route
                                alpha=1,
                                color="green",
                                zorder = 1))

    
#plt.legend(frameon=False)

def animate(i):
    """Animate scatter plot (car movement)
    
    Args:
        i (int) : Iterable argument. 
    
    Returns:
        None
        
    """
    # Iterate over all routes = number of ambulance cars riding
    for j in range(n_routes):
        # Some routes are shorter than others
        # Therefore we need to use try except with continue construction
        try:
            # Try to plot a scatter plot
            x_j = route_coorindates[j][i][0]
            y_j = route_coorindates[j][i][1]
            scatter_list[j].set_offsets(np.c_[x_j, y_j])
            scatter_list[j].set_alpha(1)

        except:
            # If i became > len(current_route) then continue to the next route
            continue
    return scatter_list

# Make the animation
animation = ani.FuncAnimation(fig, animate, frames=counter)
writergif = ani.PillowWriter(fps=1) 
animation.save("animation.gif", writer=writergif, dpi=600)
plt.show()