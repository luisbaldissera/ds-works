#!/bin/bash
launch() {
    N=$1
    D=$2
    mpirun -np $N ./mpinteg $D -t
}

average() {
    sum=0
    echo "Values: $@"
    echo "Total: $#"
    for val in $@; do
        sum=`awk 'BEGIN{ printf "%g", '$sum'+'$val' }'`
    done
    echo `awk 'BEGIN{ printf "%g", '$sum'/'$#' }'`
}

procs=(1 2 4 8 10)
dx=(0.0001 0.00001 0.000001)
tests=30

python3 > .table.txt <<EOF
import numpy as np
a = np.array([
$(for p in ${procs[@]}; do
    for d in ${dx[@]}; do
        echo "$p processors, DX = $d" 1>&2
        for i in `seq 1 $tests`; do
            td=`launch $p $d | cut -f1 -d' '`
            echo -n '.' 1>&2
            echo "[$p, $d, $td],"
        done
        echo "" 1>&2
    done
done) ])
for p in [1, 2, 4, 8, 10]:
    for d in [0.0001, 0.00001, 0.000001]:
        tmp = a[np.logical_and( a[:,0] == p, a[:,1] == d), 2]
        print("%s %s %s %s" % (p, d, tmp.mean(), tmp.std()))
EOF
