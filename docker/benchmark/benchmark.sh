#!/bin/bash
set -euo pipefail

cd /Luz

make Luz

repeat=10
total=0
for ((n = 0; n < $repeat; n++))
do
	renderDuration=$(./Luz --benchmark --seed 424242424 | bc -l)
	echo $renderDuration
	total=$(echo $total + $renderDuration | bc -l)
done

echo $(echo $total / $repeat | bc -l)
