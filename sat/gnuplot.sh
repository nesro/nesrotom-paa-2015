#!/usr/bin/env bash

file=$1
name=$2

gnuplot << __EOF__
set term pdf
unset key
set title "$name"
set xlabel "krok"
set ylabel "cena"
set output "$file.pdf"
plot "$file.txt" using 1:2 with dots, "$file.txt" using 1:3 with dots
__EOF__
