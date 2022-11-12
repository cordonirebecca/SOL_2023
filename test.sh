#!/bin/bash

./masterWorker -n 6 - q 8 -d pippo -t 1000 &
#valgrind  --leak-check=full --track-origins=yes -s ./server config.txt &
PID_SERVER=$!
sleep 1

kill -SIGINT $PID_SERVER
wait $PID_SERVER

