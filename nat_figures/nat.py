import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from cycler import cycler

plt.rcParams['axes.spines.right'] = True 
plt.rcParams['axes.spines.bottom'] = True
plt.rcParams['axes.spines.left'] = True 
plt.rcParams['axes.axisbelow'] = True
plt.rcParams['axes.axisbelow'] = True



cores =     ["64","","128", "", "256", "","512","", "1024", "","1280","","1518","","","64","","128","","256","","512","","1024","","1280","","1518"]
dpdk =      [3.72, np.nan,    6.59,   np.nan,  9.90,  np.nan , 9.90,  np.nan, 9.90,  np.nan,  9.90, np.nan, 9.90,    np.nan, np.nan,  3.34,    np.nan, 5.89,   np.nan,  10.01, np.nan, 10.01,  np.nan, 10.01,  np.nan,  10.01, np.nan,  10.01]

netmap =    [2.38,  np.nan,  3.897,    np.nan, 8.012,   np.nan, 9.905,  np.nan, 9.904,   np.nan, 9.904, np.nan, 9.904,     np.nan,  np.nan, 2.228,  np.nan,  3.797,  np.nan,   6.578, np.nan, 10.005, np.nan, 10.005,   np.nan, 10.005, np.nan,  10.005]

socket =    [0.20,  np.nan,  0.23,    np.nan, 0.97,  np.nan,  2.19,  np.nan, 4.20,  np.nan,   5.41, np.nan, 6.40,    np.nan, np.nan,  0.18,    np.nan, 0.24,  np.nan,    0.748,  np.nan, 2.24,  np.nan, 4.77,   np.nan,  5.60, np.nan,  6.42]

dpdk_p = dpdk
netmap_p = netmap
socket_p = socket

numb = [64,0,128,0,256,0,512,0,1024,0,1280,0,1518,0,0,114,0,128,0,256,0,512,0,1024,0,1280,0,1518]

for i in range(0,len(cores)):
	if dpdk[i] != np.nan: 
		dpdk_p[i]=(dpdk[i]*1000)/((20+numb[i])*8) 
		netmap_p[i]=(netmap[i]*1000)/((20+numb[i])*8) 
		socket_p[i]=(socket[i]*1000)/((20+numb[i])*8) 
	else: 
		np.nan

ind = np.arange(len(cores))
width = 0.5
fig = plt.figure(figsize=(7,3.3))
ax = fig.add_subplot(111)
#ax.bar([p + width for p in ind], dpdk_p, width, color="#AFD2E9", edgecolor="black")
ax.bar([p + width for p in ind+0.5], dpdk_p, width, edgecolor="black",hatch="OO", lw=1., zorder = 0)

ax.bar([p + width for p in ind], netmap_p, width, edgecolor="black", lw=1., zorder = 0)

ax.bar([p + width for p in ind-0.5], socket_p, width, edgecolor="black",hatch="..", lw=1., zorder = 0)

ax.set_xticks(ind+2*(width/2))
xticks = ax.xaxis.get_major_ticks()
xticks[1].set_visible(False)
xticks[3].set_visible(False)
xticks[5].set_visible(False)
xticks[7].set_visible(False)
xticks[9].set_visible(False)
xticks[11].set_visible(False)
xticks[13].set_visible(False)
xticks[14].set_visible(False)
xticks[16].set_visible(False)
xticks[18].set_visible(False)
xticks[20].set_visible(False)
xticks[22].set_visible(False)
xticks[24].set_visible(False)
ax.set_xticklabels(cores)

ax.set_ylabel('Throughput (Mpps)',fontsize=9)
#ax.yaxis.set_label_coords(-0.08,0.9)
ax.spines['right'].set_visible(False)
ax.spines['top'].set_visible(False)
ax.yaxis.set_ticks_position('left')
#ax.tick_params(axis='y',length=6)
ax.xaxis.set_ticks_position('bottom')
ax.set_xlabel("Packet size (Bytes)", fontsize=9)
ax.xaxis.set_label_coords(0.5,-0.2)
ax.yaxis.grid(linestyle='--')

ax.text(6.11, -1, u'UL',fontsize=10)
ax.text(21.49, -1, u'DL',fontsize=10,ha='center')


plt.legend(['DPDK', 'Netmap', 'Socket-mmap'], loc='upper right',frameon=False)
plt.tight_layout(pad=0.3, w_pad=0.5, h_pad=1)
plt.savefig("vxlan.png")
plt.savefig("vxlan.eps")
