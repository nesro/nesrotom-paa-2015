#!/usr/bin/env bash

file=$1
name=$2

gnuplot << __EOF__
set term pdf
set title "$name"
set xlabel "krok"
set ylabel "cena"
set output "$file.pdf"
plot "$file.txt" title "" with dots
__EOF__
