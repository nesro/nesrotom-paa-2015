#!/usr/bin/env bash
set -xv
make
for i in 1 2 3; do
  cat ./tests/uf/uf20-01.cnf | ./main -c $i -r 1 1>res$i.txt
  ./gnuplot.sh res$i "soubor: uf20-01.cnf | cenova funkce: cena$i"
done
set +xv

