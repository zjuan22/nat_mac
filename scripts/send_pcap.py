#!/usr/bin/env python

import random
import argparse
import socket
import threading
import socket
import time 
from scapy.all import sniff
from scapy.all import Ether, IP, IPv6, TCP, ICMP, GRE
from scapy.utils import rdpcap

#repo_dir='/home/messer/workarea/myrepo/PCAP/'
repo_dir='/home/lion/workspace/nat_mac/nat_pcaps/'

NUM_PACKETS = 1 
parser = argparse.ArgumentParser(description='run_test.py')

parser.add_argument('--dl',
		help='If send a NAT UL flow of pkts through veth1 to veth2',
		action="store_true",
                default=False)
parser.add_argument('--ul',
		help='If send a NAT DL flow of pkts through veth1 to veth2',
		action="store_true",
                default=False)
args = parser.parse_args()

class PacketQueue:
	def __init__(self):
		self.pkts = []
		self.lock = threading.Lock()
		self.ifaces = set()

	def add_iface(self, iface):
		self.ifaces.add(iface)

	def get(self):
		self.lock.acquire()
		if not self.pkts:
			self.lock.release()
			return None, None
		pkt = self.pkts.pop(0)
		self.lock.release()
		return pkt

	def add(self, iface, pkt):
		if iface not in self.ifaces:
			return
		self.lock.acquire()
		self.pkts.append( (iface, pkt) )
		self.lock.release()

queue = PacketQueue()

def pkt_handler(pkt, iface):
	if IPv6 in pkt:
		return
	queue.add(iface, pkt)

class SnifferThread(threading.Thread):
	def __init__(self, iface, handler = pkt_handler):
		threading.Thread.__init__(self)
		self.iface = iface
		self.handler = handler

	def run(self):
		sniff(
				iface = self.iface,
				prn = lambda x: self.handler(x, self.iface)
				)

class PacketDelay:
	def __init__(self, bsize, bdelay, imin, imax, num_pkts = 100):
		self.bsize = bsize
		self.bdelay = bdelay
		self.imin = imin
		self.imax = imax
		self.num_pkts = num_pkts
		self.current = 1

	def __iter__(self):
		return self

	def next(self):
		if self.num_pkts <= 0:
			raise StopIteration
		self.num_pkts -= 1
		if self.current == self.bsize:
			self.current = 1
			return random.randint(self.imin, self.imax)
		else:
			self.current += 1
			return self.bdelay

#pkt = Ether()/IP(dst='10.0.0.1', ttl=64)/TCP()


port_map = {
		1: "veth1",
		#1: "s1-eth1",
		2: "veth2",
		#2: "s1-eth2",
		}

#print "port_map[1]",port_map[1],"port_map[2]",port_map[2] 

iface_map = {}
for p, i in port_map.items():
	iface_map[i] = p

queue.add_iface("veth1")
queue.add_iface("veth2")

#queue.add_iface("s1-eth1")
#queue.add_iface("s1-eth2")

for p, iface in port_map.items():
	t = SnifferThread(iface)
	t.daemon = True
	t.start()

send_socket = socket.socket(socket.AF_PACKET, socket.SOCK_RAW,
		socket.htons(0x03))

if args.ul:
    pcaps=rdpcap(repo_dir+"nat_ul_100_entries/nfpa.trPR_tcp_100_random.128bytes.pcap")
    pcaps=rdpcap(repo_dir+"PCAP/nfpa.trPR_tcp_10000_random.128bytes.pcap")
    port2send = port_map[1]
elif args.dl:
    pcaps=rdpcap(repo_dir+"nat_dl_100_entries/nfpa.trPR_nat_dl_100_random.128bytes.pcap")
    port2send = port_map[1]

else:
    port2send = port_map[1]

send_socket.bind((port2send, 0))

print "number of packets"+ str(len(pcaps))
NtUM_PACKETS = len(pcaps)

delays = PacketDelay(10, 5, 25, 100, NUM_PACKETS)


ports = []
for d in pcaps:
	# sendp is too slow...
	# sendp(pkt, iface=port_map[3], verbose=0)
#    if args.random_dport:
#        pkt["TCP"].dport = random.randint(1025, 65535)
	#send_socket.send(str(pkt))
	send_socket.send(str(d))
	time.sleep(d / 1000.)
	#time.sleep(1)
print "Sending", NUM_PACKETS, "packets by ", port2send ,"..."
#time.sleep(2)
#iface, pkt = queue.get()
#while pkt:
#	ports.append(iface_map[iface])
#	iface, pkt = queue.get()
#'''
#'''
#print "DISTRIBUTION: "
#print "pkts sent from",port2send,"to", (int(port2send%2)+1)
#for p in port_map:
#	c = ports.count(p)
#	print "port {}: {:>3} [ {:>5}% ]".format(port_map[p], c, 100. * c / NUM_PACKETS)
