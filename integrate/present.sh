#!/bin/bash

num=$1

if [ $# -lt 1 ]; then
    echo "Usage: ./present slaves"
fi

# Init master
echo "Initializing master..."
xterm -e "./test/integ master ${num}" &

for i in `seq 1 $num`; do
    echo "Creating slave $i..."
    sleep 3
    xterm -e "./test/integ slave -m '127.0.0.1' && read x" &
done
