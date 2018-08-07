#!/bin/bash
p4c-bm2-ss nat2.p4 -o nat2.json
sudo simple_switch -i 1@veth1 -i 2@veth3 --pcap --thrift-port 9090 --nanolog ipc:///tmp/bm-0-log.ipc --device-id 0 nat2.json --log-console
