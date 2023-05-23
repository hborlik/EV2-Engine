import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

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
    filename = 'turnin_images/thesis_may2023/performance/validity_timing_DR_1.tsv'
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
    title = ''
    filename = 'turnin_images/thesis_may2023/performance/wfc_solve_timings.tsv'
    xlabel = 'Requirements Size'
    ylabel = '(ms)'

    df = pd.read_csv(filename, sep='\t')

    print(df.keys())
    print(df.head())

    ax = plt.figure(figsize=figsize).add_subplot(1, 1, (1, 1), projection='3d')
    ax.scatter(df[n_nodes], df[n_domain], df['Time(ms)'], cmap='viridis', c=df[n_domain])
    ax.set_xlabel('Neighborhood Size')
    ax.set_ylabel('Domain Size')
    ax.set_zlabel('Time(ms)')

    ax.view_init(elev=3., azim=126, roll=0)
    ax.dist = 6.5    # define perspective (default=10)
    plt.show()

# AD_plot()
# DR_plot()
GWFC_plot()