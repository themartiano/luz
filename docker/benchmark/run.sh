#!/bin/bash

cd /Luz

repeat=10
total=0
for ((n = 0; n < $repeat; n++))
do
	renderDuration=`./Luz --benchmark --seed 42 | bc -l`
	total=`echo $total + $renderDuration | bc -l`
done

echo `echo $total / $repeat | bc -l`
