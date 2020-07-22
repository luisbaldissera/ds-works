#!/bin/bash
procs=(1 2 4 8 10)
dx=(0.0001 0.00001 0.000001)
tests=10

python3 <<EOF
import numpy as np
a = np.array([
$(for p in ${procs[@]}; do
    for d in ${dx[@]}; do
        echo "$p processes :: interval $d" 1>&2
        for i in `seq 1 $tests`; do
            td=`mpirun -np $p ./mpinteg $d -t | cut -f1 -d' '`
            echo -n '.' 1>&2
            echo "[$p,$d,$td],"
        done
        echo "" 1>&2
    done
done) ])
print("PROCESSES,DX,TIME")
for p in [1, 2, 4, 8, 10]:
    for d in [0.0001, 0.00001, 0.000001]:
        tmp = a[np.logical_and(a[:,0] == p, a[:,1] == d), 2]
        print("%s,%s,%s" % (p, d, tmp.mean()))
EOF
