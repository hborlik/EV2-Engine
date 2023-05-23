import matplotlib.pyplot as plt
import pandas as pd


n_nodes = 'NNode'
n_domain = 'NDomain'
n_req = 'NReq'

def AD_plot():
    title = 'Adjacencency Size vs Validity Check Time(ms)'
    filename = 'validity_timing_AD.tsv'

    df = pd.read_csv(filename, sep='\t')

    print(df.keys())

    df1 = df[df[n_nodes] == 1]
    df2 = df[df[n_nodes] == 6]
    df3 = df[df[n_nodes] == 11]

    ax = df1.plot(x=n_domain,y="Time(ms)", kind="scatter", c='r', title=title, ylabel='(ms)', xlabel="Domain Size", label='1 Node')
    df2.plot(ax=ax, x=n_domain,y="Time(ms)", kind="scatter", c='g', xlabel="Domain Size", label='6 Nodes')
    df3.plot(ax=ax, x=n_domain,y="Time(ms)", kind="scatter", c='b', xlabel="Domain Size", label='11 Nodes')
    plt.show()

def DR_plot():
    title = 'Requirements Size vs Validity Check Time(ms) with 500 Neighbors'
    filename = 'validity_timing_DR_1.tsv'
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
        ax = data.plot(ax=ax, x=n_req,y="Time(ms)", kind="scatter", c=c, title=title, ylabel=ylabel, xlabel=xlabel, label=a)

    plt.show()

DR_plot()