import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from cycler import cycler

plt.rcParams['axes.spines.right'] = True 
plt.rcParams['axes.spines.bottom'] = True
plt.rcParams['axes.spines.left'] = True 
plt.rcParams['axes.axisbelow'] = True
plt.rcParams['axes.axisbelow'] = True


cores =     ["2","4","6", "", "2","4","6", "","2","4","6", "","", "2","4","6", "", "2","4","6", "", "2","4","6"]
dpdk =      [2.86, 3.72, 4.19, np.nan , 5.07,  6.59,7.38, np.nan, 9.37, 9.90, 9.98, np.nan , np.nan ,2.83,  3.34,4.14,np.nan , 4.97,  5.89,7.32,np.nan , 9.28,  10.01,10.01]

netmap =    [2.08, 2.38,  2.85, np.nan, 3.44, 3.897, 4.34, np.nan, 5.61, 8.012, 7.616, np.nan , np.nan ,0.62,  2.228,2.518,np.nan , 3.34,  3.797,4.165,np.nan , 5.56,  6.578,7.097]

socket =    [0.206,  0.2,  0.12, np.nan, 0.353, 0.23,  0.45, np.nan, 0.777, 0.97, 0.75, np.nan , np.nan, 0.154,  0.18,0.3,np.nan , 0.322, 0.24, 0.25,np.nan , 0.899,  0.748, 0.64]

dpdk_p = dpdk
netmap_p = netmap
socket_p = socket

numb = [64,64,64,0,128,128,128,0,256,256,256,0,0,64,64,64,0,128,128,128,0,256,256,256]

for i in range(0,len(cores)):
	if dpdk[i] != np.nan: 
		dpdk_p[i]=(dpdk[i]*1000)/((20+numb[i])*8) 
		netmap_p[i]=(netmap[i]*1000)/((20+numb[i])*8) 
		socket_p[i]=(socket[i]*1000)/((20+numb[i])*8) 
	else: 
		np.nan

ind = np.arange(len(cores))
width = 0.3
fig = plt.figure(figsize=(7,5.3))
ax = fig.add_subplot(111)
#ax.bar([p + width for p in ind], dpdk_p, width, color="#AFD2E9", edgecolor="black")
ax.bar([p + width for p in ind+0.3], dpdk_p, width, edgecolor="black",hatch="OO", lw=1., zorder = 0)

ax.bar([p + width for p in ind], netmap_p, width, edgecolor="black", lw=1., zorder = 0)

ax.bar([p + width for p in ind-0.3], socket_p, width, edgecolor="black",hatch="..", lw=1., zorder = 0)

ax.set_xticks(ind+2*(width/2))
xticks = ax.xaxis.get_major_ticks()

xticks[3].set_visible(False)
xticks[7].set_visible(False)
xticks[11].set_visible(False)
xticks[12].set_visible(False)
xticks[16].set_visible(False)
xticks[20].set_visible(False)
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

ax.text(1, -0.7, u'64',fontsize=10)
ax.text(4.8, -0.7, u'128',fontsize=10)
ax.text(8.8, -0.7, u'256',fontsize=10)
ax.text(14, -0.7, u'64',fontsize=10)
ax.text(17.8, -0.7, u'128',fontsize=10)
ax.text(21.8, -0.7, u'256',fontsize=10)
ax.text(9.45, -1.4, u'Number of cores',fontsize=10)
ax.text(4, -1, u'Uplink (UL)',fontsize=10)
ax.text(16.4, -1, u'Downlink (DL)',fontsize=10)
ax.set_ylim([0,7.4])

plt.legend(['DPDK', 'Netmap', 'Socket-mmap'], loc='upper right',frameon=False)


plt.tight_layout(pad=0.3, w_pad=0.5, h_pad=1)
plt.savefig("vxlan.png")
plt.savefig("vxlan.eps")
