import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.colors import ListedColormap

# cleanup stuff for log to tsv
# wfc_solve[ ][0-9|.|e|-]+ms\n

n_nodes = 'NNode'
n_domain = 'NDomain'
n_req = 'NReq'

figsize=(1.5*6,1.5*4)

def AD_plot():
    title = 'Adjacencency Size vs Validity Check Time(ms) with 20 Requirements'
    filename = 'turnin_images/thesis_may2023/performance/validity_timing_AD.tsv'
    xlabel = 'Domain Size'
    ylabel = '(ms)'

    df = pd.read_csv(filename, sep='\t')

    print(df.keys())

    plots = {'100 Neighbors' : (df[n_nodes] == 100, 'r'),
             '200 Neighbors' : (df[n_nodes] == 200, 'g'),
             '300 Neighbors' : (df[n_nodes] == 300, 'b')}
    
    df_arr = [(k, df[s[0]], s[1]) for k,s in plots.items()]

    ax = None
    for a,data,c in df_arr:
        ax = data.plot(ax=ax, x=n_domain,y="Time(ms)", kind="scatter", c=c, title=title, ylabel=ylabel, xlabel=xlabel, label=a, figsize=figsize)

    plt.show()


def DR_plot():
    title = 'Requirements Size vs Validity Check Time(ms) with 500 Neighbors'
    # filename = 'turnin_images/thesis_may2023/performance/validity_timing_DR_1.tsv'
    filename = 'validity_timing_DR_approx.tsv'
    xlabel = 'Requirements Size'
    ylabel = '(ms)'

    df = pd.read_csv(filename, sep='\t')

    print(df.keys())

    plots = {'1 Domain' : (df[n_domain] == 1, 'r'),
             '100 Domain' : (df[n_domain] == 101, 'g'),
             '200 Domain' : (df[n_domain] == 201, 'b')}
    
    df_arr = [(k, df[s[0]], s[1]) for k,s in plots.items()]

    ax = None
    for a,data,c in df_arr:
        ax = data.plot(ax=ax, x=n_req,y="Time(ms)", kind="scatter", c=c, title=title, ylabel=ylabel, xlabel=xlabel, label=a, figsize=figsize)

    plt.show()

def GWFC_plot():
    title = 'GWFC Single Iteration Timings during Application use'
    filename = 'turnin_images/thesis_may2023/performance/wfc_solve_timings.tsv'

    df = pd.read_csv(filename, sep='\t')

    domain_vals = [1, 2, 3, 4, 5]

    print(df.keys())
    print(df.head())

    fig = plt.figure(figsize=figsize)
    ax = fig.add_subplot(1, 1, (1, 1))#, projection='3d')

    X, Y, Z = (df[n_nodes], df[n_domain], df['Time(ms)'])

    # for v in domain_vals:
    #     data = df[df[n_domain] == v]
    #     ax.scatter(data[n_nodes], data[n_domain], data['Time(ms)'], cmap='viridis', c=data[n_domain])
    colors = ListedColormap(['royalblue', 'blue', 'purple', 'hotpink', 'orangered', 'red'])
    labels = ['Domain Size 0', 'Domain Size 1', 'Domain Size 2', 'Domain Size 3', 'Domain Size 4', 'Domain Size 5']
    sc=ax.scatter(X, Z, cmap='viridis', c=Z)#, cmap=colors, c=Y)

    # ax.scatter(xs=X, ys=0, zs=Z)#, cmap=colors, c=Y)
    # ax.set_proj_type('persp', focal_length=0.3)

    ax.set_xlabel('Neighborhood Size')
    # ax.set_ylabel('Domain Size')
    ax.set_ylabel('Time(ms)')

    # ax.view_init(elev=3., azim=126, roll=0)
    # ax.dist = 8    # define perspective (default=10)

    # plt.legend()
    print(sc.legend_elements())
    # ax.legend(bbox_to_anchor=(1, 0.8), ncols=2 , loc='upper right', handles=sc.legend_elements()[0], labels=labels)
    plt.title(title)
    plt.show()

def update_all_adjacency_plot():
    title = 'Sampled adjacency update timings during application use'
    filename = 'turnin_images/thesis_may2023/performance/update_all_adjacencies_timings.tsv'
    xlabel = 'Nodes in scene'
    ylabel = 'Time (ms)'

    df = pd.read_csv(filename, sep='\t')

    print(df.keys())

    ax = df.plot(x='N',y="Time(ms)", kind="scatter", title=title, ylabel=ylabel, xlabel=xlabel, figsize=figsize)

    plt.show()

# AD_plot()
# DR_plot()
GWFC_plot()
# update_all_adjacency_plot()