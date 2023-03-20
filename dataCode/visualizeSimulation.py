import osmnx as ox
import networkx as nx
import pandas as pd
import numpy as np
import geopandas as gpd
import matplotlib.pyplot as plt
import matplotlib
import matplotlib.patches as mpatches
from shapely.geometry import Point
import matplotlib.animation as ani
from IPython.display import HTML
import json
import math
import random

def createAnimation(graph, hubs, routes, orders, save: bool):
    """create an animation of a simulation as a gif file
    Args:
        graph (graph) : Graph of the road network
        hubs (dict): dict of warehouses
        routes (dataframe): pandas dataframe containing route start Time, arrival time and coordinates
        orders (dataframe): pandas dataframe containing the orders coordinates, order time and arrival time
        save (bool): Boolean indicating if gif should be saved or not (saving is time consuming)
    Returns:
        None 
    """
    
    
    warehouses, x, y = [], [], []
    for w in hubs:
        warehouses.append(ox.nearest_nodes(graph, hubs[w]["longitude"], hubs[w]["latitude"]))
    for index, w in enumerate(warehouses):
        x.append(graph.nodes[warehouses[index]]["x"])
        y.append(graph.nodes[warehouses[index]]["y"])
    data = pd.DataFrame(list(zip(x, y)), columns =['Lon', 'Lat'])

    warehousePoints = gpd.GeoDataFrame(data, geometry=gpd.points_from_xy(data.Lon, data.Lat))
    warehousePoints.crs = "EPSG:4326"

    # Determine the nearest nodes on the graph from our order/start and end nodes
    order_nodes = ox.nearest_nodes(graph, orders["Lon"], orders["Lat"])
    orig_nodes = ox.nearest_nodes(graph, routes['fLon'], routes['fLat'])
    dest_nodes = ox.nearest_nodes(graph, routes['tLon'], routes['tLat'])

    # Determine the routes. For the orders, the "route" is just always the same node
    routesList = []
    for orig_node, dest_node in zip(orig_nodes, dest_nodes):
        routesList.append(nx.shortest_path(graph, source=orig_node, target=dest_node, weight='length'))
    for node in order_nodes:
        routesList.append([node])
    
    # Create projected graph
    projected_graph = ox.project_graph(graph, to_crs="EPSG:3395")
    # Keep track of the position at each courier and order at the different time points
    final_route_coordinates = []
    counterRoutes = 0
    for index, route in enumerate(routesList):
        points = []
        counter = 0
        # Iterate until the end of the planning period, with steps to speed things up
        for t in range(0,routes["arrivalTime"].max(),18):
            counter += 1
            show, step = 0, 0
            if counterRoutes < len(routes):
                routeType = "courier"
                # If currentTime (t) is between start time and arrival time, show the courier on the map
                if routes.loc[index,"startTime"] <= t and routes.loc[index,"arrivalTime"] > t:
                    # Determine at which point (approximately) in the route the courier is at the currentTime t
                    ratio = (routes.loc[index,"arrivalTime"]-routes.loc[index,"startTime"])/len(route)
                    step = min(math.ceil((t-routes.loc[index,"startTime"])/ratio), len(route)-1)
                    show = 1
            else:
                routeType = "order"
                idx = index - len(routes)
                # If current time is between the time the order arrives and the order is served, show the order
                if orders.loc[idx,"orderTime"] < t and orders.loc[idx,"arrivalTime"] > t:
                    step = 0
                    show = 1
            
            x = projected_graph.nodes[route[step]]['x']
            y = projected_graph.nodes[route[step]]['y']
            if show:
                points.append([x, y, show, routeType])
            else:
                # Set x value to high value, so that the point wont be plotted on the grap
                points.append([1000, y, show, routeType])

        final_route_coordinates.append(points)
        counterRoutes += 1 
    
    # Create a plot 
    fig, ax = ox.plot_graph(projected_graph, node_size=0, edge_linewidth=0.5, show=False, close=False) # network
    fig.set_size_inches(4,5)
    ax.set_axis_off()
    fig.subplots_adjust(left=0, bottom=0, right=1, top=1)
    # Add warehouses to the plot
    warehousePoints.to_crs('EPSG:3395', inplace=True)
    warehousePoints.plot(ax=ax, color='red', label='warehouse', zorder = 3) # warehouses

    # Each list is a route. Length of this list = n_routes
    scatter_list = []
    # Determine data for the first scatter plot
    for j in range(len(routesList)):
        if (final_route_coordinates[j][0][3] == "courier"):
            scatter_list.append(ax.scatter(final_route_coordinates[j][0][0], # x coordiante of the first node of the j route
                                        final_route_coordinates[j][0][1], # y coordiante of the first node of the j route
                                        alpha=1,
                                        s = 16,
                                        label="courier",
                                        color="blue",
                                        zorder = 2))
        else:
            scatter_list.append(ax.scatter(final_route_coordinates[j][0][0], # x coordiante of the first node of the j route
                                final_route_coordinates[j][0][1], # y coordiante of the first node of the j route
                                alpha=1,
                                s = 16,
                                color="green",
                                label="order",
                                zorder = 1))

    # Plot the legend
    red_patch = plt.Line2D([], [], color="red", marker="o", linewidth=0, label ="Warehouse")
    blue_patch = plt.Line2D([], [], color="blue", marker="o", linewidth=0, label ="Courier")
    green_patch = plt.Line2D([], [], color="green", marker="o", linewidth=0, label ="Order")
    ax.legend(loc=3,handles=[red_patch, blue_patch, green_patch])
        
    def animate(i):
        """Animate scatter plot (courier movement and order pop ups)
        Args:
            i (int) : Iterable argument. 
        
        Returns:
            None 
        """
        # Iterate over all routes = number of routes and orders
        for j in range(len(routesList)):
            # Some routes are shorter than others
            # Therefore we need to use try except with continue construction
            try:
                if (final_route_coordinates[j][0][3] == "courier"):
                    # Try to plot a scatter plot
                    x_j = final_route_coordinates[j][i][0]
                    y_j = final_route_coordinates[j][i][1]
                    scatter_list[j].set_offsets(np.c_[x_j, y_j])
                    scatter_list[j].set_zorder(2)
                    scatter_list[j].set_color("blue")
                else:
                    # Try to plot a scatter plot
                    x_j = final_route_coordinates[j][i][0]
                    y_j = final_route_coordinates[j][i][1]
                    scatter_list[j].set_offsets(np.c_[x_j, y_j])
                    scatter_list[j].set_zorder(1)
                    scatter_list[j].set_color("green")

            except:
                # If i became > len(current_route) then continue to the next route
                continue
        return scatter_list

    # Finally we can create our animation
    animation = ani.FuncAnimation(fig, animate, frames=counter)
    if save:
        writergif = ani.PillowWriter(fps=10) 
        animation.save("animation.gif", writer=writergif, dpi=200, savefig_kwargs={"transparent": True, "facecolor": "none"})
    plt.show()



if __name__ == "__main__":
    with open('data/getirStores.json') as fp:
        getirStores = json.load(fp)

    ### These three lines code are meant to first download the graph and save it. As it is saved now, we can just load it
    # shapeFile = gpd.read_file("data/Polygon_900s").loc[0, 'geometry']
    # G = ox.graph_from_polygon(shapeFile, network_type='drive', simplify=False)
    # ox.save_graphml(G, 'data/chicago.graphml')
    graph = ox.load_graphml("data/animationData/chicago.graphml")


    routes = pd.read_csv('data/animationData/routes.txt', sep=" ", header=None)
    routes.columns = ["startTime", "arrivalTime", "fLat", "fLon", "tLat", "tLon"]

    orders = pd.read_csv('data/animationData/orders.txt', sep=" ", header=None)
    orders.columns = ["orderTime", "arrivalTime", "Lat", "Lon"]

    createAnimation(graph, getirStores, routes, orders, save = False)