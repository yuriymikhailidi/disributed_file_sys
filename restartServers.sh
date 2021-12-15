#!/bin/sh

#kill any previous process
killall dfs

#Start the 4 servers
./dfs DFS1 10001&
./dfs DFS2 10002&
./dfs DFS3 10003&
./dfs DFS4 10004&
