#!/usr/bin/env bash

out=$1
name=$2

gnuplot << __EOF__
set term pdf
set title "$name"
set xlabel "cena"
set ylabel "krok"
set output "$out.pdf"
plot "res.txt" title "" with linespoints
__EOF__
