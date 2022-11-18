#!/bin/bash

./masterWorker -q 7 -d pluto/pippo -t 2000 &
#valgrind  --leak-check=full --track-origins=yes -s ./server config.txt &
PID_SERVER=$!
sleep 1

kill -SIGINT $PID_SERVER
wait $PID_SERVER

